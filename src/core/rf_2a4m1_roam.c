// SPDX-License-Identifier: GPL-2.0
//
// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
// roaming MLME (802.11r FT key hierarchy + 802.11k RRM)
//
// Copyright (c) GenBasic.
// Licensed under the GNU General Public License, version 2.
//
// This file is machine-generated. Do not hand-edit.


#include "rf_2a4m1_core.h"

/* file-scope constants + helper types */
#define RF_2A4M1_FTE_FIXED_LEN 82
#define RF_2A4M1_FTE_SUBELEM_R1KH 1
#define RF_2A4M1_FTE_SUBELEM_R0KH 3
#define RF_2A4M1_FT_FTE_MIC_EXTRA_MAX 160
#define RF_2A4M1_NR_SUBELEM_CANDIDATE_PREF 3
#define RF_2A4M1_RRM_BEACON_REQ_BODY 16
#define RF_2A4M1_RRM_BEACON_REP_BODY 29

/* file-local forward declarations */
static void rf_2a4m1_ft_name_sha256(const uint8_t *msg, size_t len, uint8_t out[RF_2A4M1_FT_KEY_NAME_LEN]);

static void rf_2a4m1_ft_name_sha256(const uint8_t *msg, size_t len, uint8_t out[RF_2A4M1_FT_KEY_NAME_LEN])
{
	uint8_t d[RF_2A4M1_SHA256_DIGEST_LEN];
	rf_2a4m1_sha256(msg, len, d);
	memcpy(out, d, RF_2A4M1_FT_KEY_NAME_LEN);
	rf_2a4m1_crypto_wipe(d, sizeof d);
}

bool rf_2a4m1_ft_derive_pmk_r0(const uint8_t *xxkey, size_t xxkey_len,
                      const uint8_t *ssid, size_t ssid_len,
                      const uint8_t mdid[RF_2A4M1_FT_MDID_LEN],
                      const uint8_t *r0kh_id, size_t r0kh_len,
                      const rf_2a4m1_mac_addr *s0kh_id,
                      uint8_t pmk_r0[RF_2A4M1_FT_PMK_R0_LEN], uint8_t pmk_r0_name[RF_2A4M1_FT_KEY_NAME_LEN])
{
	if (!xxkey || !ssid || !mdid || !r0kh_id || !s0kh_id || !pmk_r0 || !pmk_r0_name)
		return false;
	if (ssid_len > 32 || r0kh_len > 48)
		return false;
	/* context = SSIDlength(1) || SSID || MDID(2) || R0KHlength(1) || R0KH-ID || S0KH-ID(6) */
	uint8_t ctx[1 + 32 + 2 + 1 + 48 + 6];
	size_t p = 0;
	ctx[p++] = (uint8_t)ssid_len;
	memcpy(&ctx[p], ssid, ssid_len);            p += ssid_len;
	memcpy(&ctx[p], mdid, RF_2A4M1_FT_MDID_LEN);         p += RF_2A4M1_FT_MDID_LEN;
	ctx[p++] = (uint8_t)r0kh_len;
	memcpy(&ctx[p], r0kh_id, r0kh_len);         p += r0kh_len;
	memcpy(&ctx[p], s0kh_id->a, RF_2A4M1_ETH_ALEN);      p += RF_2A4M1_ETH_ALEN;

	uint8_t r0_key_data[RF_2A4M1_FT_PMK_R0_LEN + RF_2A4M1_FT_NAME_SALT_LEN];   /* 48: PMK-R0(32) || salt(16) */
	rf_2a4m1_ieee80211_kdf_length(xxkey, xxkey_len, "FT-R0", 5, ctx, p,
	                     r0_key_data, (RF_2A4M1_FT_PMK_R0_LEN + RF_2A4M1_FT_NAME_SALT_LEN) * 8);
	memcpy(pmk_r0, r0_key_data, RF_2A4M1_FT_PMK_R0_LEN);

	/* PMKR0Name = Truncate-128(SHA-256("FT-R0N" || PMK-R0-Name-Salt)) */
	uint8_t nbuf[6 + RF_2A4M1_FT_NAME_SALT_LEN];
	memcpy(nbuf, "FT-R0N", 6);
	memcpy(&nbuf[6], &r0_key_data[RF_2A4M1_FT_PMK_R0_LEN], RF_2A4M1_FT_NAME_SALT_LEN);
	rf_2a4m1_ft_name_sha256(nbuf, sizeof nbuf, pmk_r0_name);

	rf_2a4m1_crypto_wipe(r0_key_data, sizeof r0_key_data);
	rf_2a4m1_crypto_wipe(ctx, sizeof ctx);
	return true;
}

bool rf_2a4m1_ft_derive_pmk_r1(const uint8_t pmk_r0[RF_2A4M1_FT_PMK_R0_LEN],
                      const uint8_t pmk_r0_name[RF_2A4M1_FT_KEY_NAME_LEN],
                      const rf_2a4m1_mac_addr *r1kh_id, const rf_2a4m1_mac_addr *s1kh_id,
                      uint8_t pmk_r1[RF_2A4M1_FT_PMK_R1_LEN], uint8_t pmk_r1_name[RF_2A4M1_FT_KEY_NAME_LEN])
{
	if (!pmk_r0 || !pmk_r0_name || !r1kh_id || !s1kh_id || !pmk_r1 || !pmk_r1_name)
		return false;
	/* PMK-R1 = KDF-256(PMK-R0, "FT-R1", R1KH-ID || S1KH-ID) */
	uint8_t ctx[RF_2A4M1_ETH_ALEN + RF_2A4M1_ETH_ALEN];
	memcpy(&ctx[0], r1kh_id->a, RF_2A4M1_ETH_ALEN);
	memcpy(&ctx[RF_2A4M1_ETH_ALEN], s1kh_id->a, RF_2A4M1_ETH_ALEN);
	rf_2a4m1_ieee80211_kdf_length(pmk_r0, RF_2A4M1_FT_PMK_R0_LEN, "FT-R1", 5, ctx, sizeof ctx,
	                     pmk_r1, RF_2A4M1_FT_PMK_R1_LEN * 8);

	/* PMKR1Name = Truncate-128(SHA-256("FT-R1N" || PMKR0Name || R1KH-ID || S1KH-ID)) */
	uint8_t nbuf[6 + RF_2A4M1_FT_KEY_NAME_LEN + RF_2A4M1_ETH_ALEN + RF_2A4M1_ETH_ALEN];
	size_t p = 0;
	memcpy(&nbuf[p], "FT-R1N", 6);              p += 6;
	memcpy(&nbuf[p], pmk_r0_name, RF_2A4M1_FT_KEY_NAME_LEN); p += RF_2A4M1_FT_KEY_NAME_LEN;
	memcpy(&nbuf[p], r1kh_id->a, RF_2A4M1_ETH_ALEN);     p += RF_2A4M1_ETH_ALEN;
	memcpy(&nbuf[p], s1kh_id->a, RF_2A4M1_ETH_ALEN);     p += RF_2A4M1_ETH_ALEN;
	rf_2a4m1_ft_name_sha256(nbuf, p, pmk_r1_name);
	return true;
}

bool rf_2a4m1_ft_derive_ptk(const uint8_t pmk_r1[RF_2A4M1_FT_PMK_R1_LEN],
                   const uint8_t pmk_r1_name[RF_2A4M1_FT_KEY_NAME_LEN],
                   const uint8_t snonce[RF_2A4M1_FT_NONCE_LEN], const uint8_t anonce[RF_2A4M1_FT_NONCE_LEN],
                   const rf_2a4m1_mac_addr *bssid, const rf_2a4m1_mac_addr *sta_addr,
                   uint8_t ptk[RF_2A4M1_FT_PTK_LEN], uint8_t ptk_name[RF_2A4M1_FT_KEY_NAME_LEN])
{
	if (!pmk_r1 || !pmk_r1_name || !snonce || !anonce || !bssid || !sta_addr ||
	    !ptk || !ptk_name)
		return false;
	/* PTK = KDF-384(PMK-R1, "FT-PTK", SNonce || ANonce || BSSID || STA-ADDR) */
	uint8_t ctx[RF_2A4M1_FT_NONCE_LEN + RF_2A4M1_FT_NONCE_LEN + RF_2A4M1_ETH_ALEN + RF_2A4M1_ETH_ALEN];
	size_t p = 0;
	memcpy(&ctx[p], snonce, RF_2A4M1_FT_NONCE_LEN);   p += RF_2A4M1_FT_NONCE_LEN;
	memcpy(&ctx[p], anonce, RF_2A4M1_FT_NONCE_LEN);   p += RF_2A4M1_FT_NONCE_LEN;
	memcpy(&ctx[p], bssid->a, RF_2A4M1_ETH_ALEN);     p += RF_2A4M1_ETH_ALEN;
	memcpy(&ctx[p], sta_addr->a, RF_2A4M1_ETH_ALEN);  p += RF_2A4M1_ETH_ALEN;
	rf_2a4m1_ieee80211_kdf_length(pmk_r1, RF_2A4M1_FT_PMK_R1_LEN, "FT-PTK", 6, ctx, p, ptk, RF_2A4M1_FT_PTK_LEN * 8);

	/* PTKName = Truncate-128(SHA-256(PMKR1Name || "FT-PTKN" || SNonce || ANonce || BSSID || STA)) */
	uint8_t nbuf[RF_2A4M1_FT_KEY_NAME_LEN + 7 + RF_2A4M1_FT_NONCE_LEN + RF_2A4M1_FT_NONCE_LEN + RF_2A4M1_ETH_ALEN + RF_2A4M1_ETH_ALEN];
	size_t q = 0;
	memcpy(&nbuf[q], pmk_r1_name, RF_2A4M1_FT_KEY_NAME_LEN); q += RF_2A4M1_FT_KEY_NAME_LEN;
	memcpy(&nbuf[q], "FT-PTKN", 7);                 q += 7;
	memcpy(&nbuf[q], snonce, RF_2A4M1_FT_NONCE_LEN);         q += RF_2A4M1_FT_NONCE_LEN;
	memcpy(&nbuf[q], anonce, RF_2A4M1_FT_NONCE_LEN);         q += RF_2A4M1_FT_NONCE_LEN;
	memcpy(&nbuf[q], bssid->a, RF_2A4M1_ETH_ALEN);           q += RF_2A4M1_ETH_ALEN;
	memcpy(&nbuf[q], sta_addr->a, RF_2A4M1_ETH_ALEN);        q += RF_2A4M1_ETH_ALEN;
	rf_2a4m1_ft_name_sha256(nbuf, q, ptk_name);

	rf_2a4m1_crypto_wipe(ctx, sizeof ctx);
	return true;
}

size_t rf_2a4m1_ft_mde_build(uint8_t *out, size_t cap, uint16_t mdid, uint8_t ft_cap_policy)
{
	if (cap < 2 + RF_2A4M1_FT_MDE_BODY_LEN)
		return 0;
	out[0] = RF_2A4M1_ROAM_EID_MOBILITY_DOMAIN;
	out[1] = RF_2A4M1_FT_MDE_BODY_LEN;
	rf_2a4m1_roam_put_le16(&out[2], mdid);
	out[4] = ft_cap_policy;
	return 2 + RF_2A4M1_FT_MDE_BODY_LEN;
}

bool rf_2a4m1_ft_mde_parse(const uint8_t *ie, size_t len, uint16_t *mdid, uint8_t *ft_cap_policy)
{
	if (!ie)
		return false;
	/* EID(1) Len(1) MDID(2,LE) FT-Cap-and-Policy(1) — the fixed 5-octet element off the cursor. */
	struct rf_2a4m1_rd r;
	rf_2a4m1_rd_init(&r, ie, len);
	uint8_t  eid  = rf_2a4m1_rd_u8(&r);
	uint8_t  elen = rf_2a4m1_rd_u8(&r);
	uint16_t md   = rf_2a4m1_rd_le16(&r);
	uint8_t  cap  = rf_2a4m1_rd_u8(&r);
	if (!rf_2a4m1_rd_ok(&r))                                    /* was: len < 2 + FT_MDE_BODY_LEN */
		return false;
	if (eid != RF_2A4M1_ROAM_EID_MOBILITY_DOMAIN || elen != RF_2A4M1_FT_MDE_BODY_LEN)
		return false;
	if (mdid)
		*mdid = md;
	if (ft_cap_policy)
		*ft_cap_policy = cap;
	return true;
}

size_t rf_2a4m1_ft_fte_build(uint8_t *out, size_t cap, const struct rf_2a4m1_ft_fte *f)
{
	if (!f)
		return 0;
	size_t body = RF_2A4M1_FTE_FIXED_LEN;
	if (f->has_r1kh)
		body += 2 + RF_2A4M1_ETH_ALEN;
	if (f->has_r0kh) {
		if (f->r0kh_len == 0 || f->r0kh_len > 48)
			return 0;
		body += 2 + f->r0kh_len;
	}
	if (body > 255 || cap < 2 + body)
		return 0;
	out[0] = RF_2A4M1_ROAM_EID_FAST_BSS_TRANS;
	out[1] = (uint8_t)body;
	out[2] = 0;                    /* MIC Control: Reserved */
	out[3] = f->element_count;     /* MIC Control: Element Count */
	memcpy(&out[4], f->mic, RF_2A4M1_FT_FTE_MIC_LEN);
	memcpy(&out[20], f->anonce, RF_2A4M1_FT_NONCE_LEN);
	memcpy(&out[52], f->snonce, RF_2A4M1_FT_NONCE_LEN);
	size_t pos = 84;               /* 2 (EID/Len) + 82 fixed */
	if (f->has_r1kh) {
		out[pos++] = RF_2A4M1_FTE_SUBELEM_R1KH;
		out[pos++] = RF_2A4M1_ETH_ALEN;
		memcpy(&out[pos], f->r1kh_id.a, RF_2A4M1_ETH_ALEN);
		pos += RF_2A4M1_ETH_ALEN;
	}
	if (f->has_r0kh) {
		out[pos++] = RF_2A4M1_FTE_SUBELEM_R0KH;
		out[pos++] = f->r0kh_len;
		memcpy(&out[pos], f->r0kh_id, f->r0kh_len);
		pos += f->r0kh_len;
	}
	return pos;
}

void rf_2a4m1_ft_fte_mic(const uint8_t kck[16], const rf_2a4m1_mac_addr *sta, const rf_2a4m1_mac_addr *ap, uint8_t seq,
                const uint8_t *mde, size_t mde_len,
                const uint8_t *fte, size_t fte_len,
                const uint8_t key_name[RF_2A4M1_FT_KEY_NAME_LEN],
                const uint8_t *extra, size_t extra_len, uint8_t mic[RF_2A4M1_FT_FTE_MIC_LEN])
{
	uint8_t buf[6 + 6 + 1 + 16 + 2 + 255 + RF_2A4M1_FT_KEY_NAME_LEN + RF_2A4M1_FT_FTE_MIC_EXTRA_MAX];
	size_t p = 0;
	if (mde_len > 16 || fte_len > 255 || extra_len > RF_2A4M1_FT_FTE_MIC_EXTRA_MAX)
		return;                              /* out of scope for our frames — refuse */
	memcpy(&buf[p], sta->a, RF_2A4M1_ETH_ALEN);       p += RF_2A4M1_ETH_ALEN;
	memcpy(&buf[p], ap->a,  RF_2A4M1_ETH_ALEN);       p += RF_2A4M1_ETH_ALEN;
	buf[p++] = seq;
	memcpy(&buf[p], mde, mde_len);           p += mde_len;
	size_t fte_at = p;
	memcpy(&buf[p], fte, fte_len);           p += fte_len;
	if (fte_len >= 2 + 2 + RF_2A4M1_FT_FTE_MIC_LEN)   /* zero the MIC field (element bytes [4..19]) */
		memset(&buf[fte_at + 4], 0, RF_2A4M1_FT_FTE_MIC_LEN);
	memcpy(&buf[p], key_name, RF_2A4M1_FT_KEY_NAME_LEN); p += RF_2A4M1_FT_KEY_NAME_LEN;
	/* the remaining protected reassociation elements trailing the PMKR1Name (OCI on the Req, the
	 * KEK-wrapped GTK on the Resp) — authenticated so an on-path attacker cannot rewrite them. */
	if (extra && extra_len) { memcpy(&buf[p], extra, extra_len); p += extra_len; }
	rf_2a4m1_aes128_cmac(kck, buf, p, mic);
}

bool rf_2a4m1_ft_fte_parse(const uint8_t *ie, size_t len, struct rf_2a4m1_ft_fte *f)
{
	if (!ie || !f)
		return false;
	/* EID(1) Len(1) then Len octets of element body, off the sticky-error cursor. Only the
	 * byte-level field extraction moves here — the FTE MIC (AES-128-CMAC) is computed separately
	 * in ft_fte_mic() and is untouched. */
	struct rf_2a4m1_rd r;
	rf_2a4m1_rd_init(&r, ie, len);
	uint8_t eid  = rf_2a4m1_rd_u8(&r);
	uint8_t elen = rf_2a4m1_rd_u8(&r);
	if (!rf_2a4m1_rd_ok(&r))                                    /* was: len < 2 (subset of len < 2+FIXED) */
		return false;
	if (eid != RF_2A4M1_ROAM_EID_FAST_BSS_TRANS)
		return false;
	if (elen < RF_2A4M1_FTE_FIXED_LEN)
		return false;
	const uint8_t *b = rf_2a4m1_rd_bytes(&r, elen);             /* was: (size_t)(2 + elen) > len */
	if (!b)
		return false;
	/* MIC-Control(2: Reserved, Element-Count) MIC(16) ANonce(32) SNonce(32) = 82 fixed octets. */
	f->element_count = b[1];       /* MIC Control octet 1 */
	memcpy(f->mic, &b[2], RF_2A4M1_FT_FTE_MIC_LEN);
	memcpy(f->anonce, &b[18], RF_2A4M1_FT_NONCE_LEN);
	memcpy(f->snonce, &b[50], RF_2A4M1_FT_NONCE_LEN);
	f->has_r1kh = false;
	f->has_r0kh = false;
	f->r0kh_len = 0;
	/* subelements (id(1) len(1) data[len]) walked over a nested cursor bounded by the element
	 * body (elen), NOT the outer buffer, starting after the 82-octet fixed part. */
	struct rf_2a4m1_rd e;
	rf_2a4m1_rd_init(&e, b, elen);
	rf_2a4m1_rd_skip(&e, RF_2A4M1_FTE_FIXED_LEN);                        /* consume the fixed part (elen >= 82) */
	while (rf_2a4m1_rd_remaining(&e) >= 2) {                    /* was: pos + 2 <= elen */
		uint8_t sid  = rf_2a4m1_rd_u8(&e);
		uint8_t slen = rf_2a4m1_rd_u8(&e);
		const uint8_t *sdata = rf_2a4m1_rd_bytes(&e, slen);
		if (!sdata)                                /* was: pos + 2 + slen > elen — reject */
			return false;
		if (sid == RF_2A4M1_FTE_SUBELEM_R1KH && slen == RF_2A4M1_ETH_ALEN) {
			memcpy(f->r1kh_id.a, sdata, RF_2A4M1_ETH_ALEN);
			f->has_r1kh = true;
		} else if (sid == RF_2A4M1_FTE_SUBELEM_R0KH && slen >= 1 && slen <= 48) {
			memcpy(f->r0kh_id, sdata, slen);
			f->r0kh_len = slen;
			f->has_r0kh = true;
		}
	}
	return true;
}

size_t rf_2a4m1_ft_action_req_build(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *sta, const rf_2a4m1_mac_addr *target,
                           const uint8_t *elems, size_t elems_len)
{
	if (!out || !sta || !target)
		return 0;
	if (elems_len > 255 || cap < (size_t)RF_2A4M1_FT_ACTION_REQ_HDR + elems_len)
		return 0;
	out[0] = RF_2A4M1_ROAM_CAT_FT;
	out[1] = RF_2A4M1_FT_ACTION_REQUEST;
	memcpy(&out[2], sta->a, RF_2A4M1_ETH_ALEN);
	memcpy(&out[8], target->a, RF_2A4M1_ETH_ALEN);
	if (elems && elems_len)
		memcpy(&out[RF_2A4M1_FT_ACTION_REQ_HDR], elems, elems_len);
	return (size_t)RF_2A4M1_FT_ACTION_REQ_HDR + elems_len;
}

bool rf_2a4m1_ft_action_req_parse(const uint8_t *in, size_t len, struct rf_2a4m1_ft_action_req *r)
{
	if (!in || !r)
		return false;
	/* Category(1) Action(1) STA-Address(6) Target-AP-Address(6) then the FT elements span. */
	struct rf_2a4m1_rd rr;
	rf_2a4m1_rd_init(&rr, in, len);
	uint8_t cat = rf_2a4m1_rd_u8(&rr);
	uint8_t act = rf_2a4m1_rd_u8(&rr);
	const uint8_t *sta = rf_2a4m1_rd_bytes(&rr, RF_2A4M1_ETH_ALEN);
	const uint8_t *tgt = rf_2a4m1_rd_bytes(&rr, RF_2A4M1_ETH_ALEN);
	if (!rf_2a4m1_rd_ok(&rr))                                   /* was: len < FT_ACTION_REQ_HDR */
		return false;
	if (cat != RF_2A4M1_ROAM_CAT_FT || act != RF_2A4M1_FT_ACTION_REQUEST)
		return false;
	memcpy(r->sta_addr.a, sta, RF_2A4M1_ETH_ALEN);
	memcpy(r->target_addr.a, tgt, RF_2A4M1_ETH_ALEN);
	r->elems = &rr.p[rr.pos];                          /* &in[FT_ACTION_REQ_HDR] */
	r->elems_len = rf_2a4m1_rd_remaining(&rr);                  /* len - FT_ACTION_REQ_HDR */
	return true;
}

size_t rf_2a4m1_ft_action_resp_build(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *sta, const rf_2a4m1_mac_addr *target,
                            uint16_t status, const uint8_t *elems, size_t elems_len)
{
	if (!out || !sta || !target)
		return 0;
	if (elems_len > 255 || cap < (size_t)RF_2A4M1_FT_ACTION_RESP_HDR + elems_len)
		return 0;
	out[0] = RF_2A4M1_ROAM_CAT_FT;
	out[1] = RF_2A4M1_FT_ACTION_RESPONSE;
	memcpy(&out[2], sta->a, RF_2A4M1_ETH_ALEN);
	memcpy(&out[8], target->a, RF_2A4M1_ETH_ALEN);
	rf_2a4m1_roam_put_le16(&out[14], status);
	if (elems && elems_len)
		memcpy(&out[RF_2A4M1_FT_ACTION_RESP_HDR], elems, elems_len);
	return (size_t)RF_2A4M1_FT_ACTION_RESP_HDR + elems_len;
}

bool rf_2a4m1_ft_action_resp_parse(const uint8_t *in, size_t len, struct rf_2a4m1_ft_action_resp *r)
{
	if (!in || !r)
		return false;
	/* Category(1) Action(1) STA(6) Target(6) Status-Code(2,LE) then the FT elements span. */
	struct rf_2a4m1_rd rr;
	rf_2a4m1_rd_init(&rr, in, len);
	uint8_t  cat    = rf_2a4m1_rd_u8(&rr);
	uint8_t  act    = rf_2a4m1_rd_u8(&rr);
	const uint8_t *sta = rf_2a4m1_rd_bytes(&rr, RF_2A4M1_ETH_ALEN);
	const uint8_t *tgt = rf_2a4m1_rd_bytes(&rr, RF_2A4M1_ETH_ALEN);
	uint16_t status = rf_2a4m1_rd_le16(&rr);
	if (!rf_2a4m1_rd_ok(&rr))                                   /* was: len < FT_ACTION_RESP_HDR */
		return false;
	if (cat != RF_2A4M1_ROAM_CAT_FT || act != RF_2A4M1_FT_ACTION_RESPONSE)
		return false;
	memcpy(r->sta_addr.a, sta, RF_2A4M1_ETH_ALEN);
	memcpy(r->target_addr.a, tgt, RF_2A4M1_ETH_ALEN);
	r->status = status;
	r->elems = &rr.p[rr.pos];                          /* &in[FT_ACTION_RESP_HDR] */
	r->elems_len = rf_2a4m1_rd_remaining(&rr);                  /* len - FT_ACTION_RESP_HDR */
	return true;
}

size_t rf_2a4m1_ric_rde_build(uint8_t *out, size_t cap, const struct rf_2a4m1_ric_rde *r)
{
	if (!out || !r || cap < 2 + RF_2A4M1_RIC_RDE_BODY_LEN)
		return 0;
	out[0] = RF_2A4M1_RIC_EID_RDE;
	out[1] = RF_2A4M1_RIC_RDE_BODY_LEN;
	out[2] = r->rdie_id;
	out[3] = r->desc_count;
	rf_2a4m1_roam_put_le16(&out[4], r->status);
	return 2 + RF_2A4M1_RIC_RDE_BODY_LEN;
}

bool rf_2a4m1_ric_rde_parse(const uint8_t *ie, size_t len, struct rf_2a4m1_ric_rde *r)
{
	if (!ie || !r)
		return false;
	/* EID(1) Len(1) RDIE-Id(1) Desc-Count(1) Status(2,LE) — the fixed 6-octet element. */
	struct rf_2a4m1_rd rr;
	rf_2a4m1_rd_init(&rr, ie, len);
	uint8_t  eid    = rf_2a4m1_rd_u8(&rr);
	uint8_t  elen   = rf_2a4m1_rd_u8(&rr);
	uint8_t  rdie   = rf_2a4m1_rd_u8(&rr);
	uint8_t  count  = rf_2a4m1_rd_u8(&rr);
	uint16_t status = rf_2a4m1_rd_le16(&rr);
	if (!rf_2a4m1_rd_ok(&rr))                                   /* was: len < 2 + RIC_RDE_BODY_LEN */
		return false;
	if (eid != RF_2A4M1_RIC_EID_RDE || elen != RF_2A4M1_RIC_RDE_BODY_LEN)
		return false;
	r->rdie_id    = rdie;
	r->desc_count = count;
	r->status     = status;
	return true;
}

size_t rf_2a4m1_ric_request_build(uint8_t *out, size_t cap, uint8_t rdie_id,
                         const struct rf_2a4m1_tspec *list, size_t n)
{
	if (!out || (!list && n) || n > RF_2A4M1_RIC_MAX_RESOURCES)
		return 0;
	struct rf_2a4m1_ric_rde rde = { rdie_id, (uint8_t)n, RF_2A4M1_RIC_STATUS_SUCCESS };
	size_t p = rf_2a4m1_ric_rde_build(out, cap, &rde);
	if (p == 0)
		return 0;
	for (size_t i = 0; i < n; i++) {
		size_t t = rf_2a4m1_tspec_build(&out[p], cap - p, &list[i]);
		if (t == 0)
			return 0;
		p += t;
	}
	return p;
}

bool rf_2a4m1_ric_request_parse(const uint8_t *in, size_t len, struct rf_2a4m1_ric_request *out)
{
	if (!in || !out)
		return false;
	struct rf_2a4m1_ric_rde rde;
	if (!rf_2a4m1_ric_rde_parse(in, len, &rde) || rde.desc_count > RF_2A4M1_RIC_MAX_RESOURCES)
		return false;
	out->rdie_id = rde.rdie_id;
	out->count   = rde.desc_count;
	/* Walk the TSPEC elements trailing the RDE off the cursor; tspec_parse (the REUSED qos codec)
	 * decodes each body — only the surrounding framing moves to the cursor. */
	struct rf_2a4m1_rd r;
	rf_2a4m1_rd_init(&r, in, len);
	rf_2a4m1_rd_skip(&r, 2 + RF_2A4M1_RIC_RDE_BODY_LEN);                 /* past the RDE (present when rde_parse ok) */
	for (size_t i = 0; i < rde.desc_count; i++) {
		if (rf_2a4m1_rd_remaining(&r) < 2)                  /* was: p + 2 > len */
			return false;
		uint8_t eid  = rf_2a4m1_rd_u8(&r);
		uint8_t dlen = rf_2a4m1_rd_u8(&r);
		if (eid != RF_2A4M1_WLAN_EID_TSPEC)
			return false;
		const uint8_t *tsp = rf_2a4m1_rd_bytes(&r, dlen);   /* was: (size_t)(2 + dlen) > len - p */
		if (!tsp || !rf_2a4m1_tspec_parse(tsp, dlen, &out->rf_2a4m1_tspec[i]))
			return false;
	}
	return true;
}

size_t rf_2a4m1_ric_response_build(uint8_t *out, size_t cap, uint8_t rdie_id, const struct rf_2a4m1_tspec *list,
                          const uint16_t *status, size_t n)
{
	if (!out || !status || (!list && n) || n > RF_2A4M1_RIC_MAX_RESOURCES)
		return 0;
	uint16_t overall = RF_2A4M1_RIC_STATUS_SUCCESS;
	for (size_t i = 0; i < n; i++)
		if (status[i] != RF_2A4M1_RIC_STATUS_SUCCESS) { overall = RF_2A4M1_RIC_STATUS_DENIED; break; }
	struct rf_2a4m1_ric_rde rde = { rdie_id, (uint8_t)n, overall };
	size_t p = rf_2a4m1_ric_rde_build(out, cap, &rde);
	if (p == 0)
		return 0;
	for (size_t i = 0; i < n; i++) {
		if (p + 2 > cap)
			return 0;
		rf_2a4m1_roam_put_le16(&out[p], status[i]);
		p += 2;
		if (status[i] == RF_2A4M1_RIC_STATUS_SUCCESS) {
			size_t t = rf_2a4m1_tspec_build(&out[p], cap - p, &list[i]);
			if (t == 0)
				return 0;
			p += t;
		}
	}
	return p;
}

bool rf_2a4m1_ric_response_parse(const uint8_t *in, size_t len, struct rf_2a4m1_ric_response *out, size_t *consumed)
{
	if (!in || !out)
		return false;
	struct rf_2a4m1_ric_rde rde;
	if (!rf_2a4m1_ric_rde_parse(in, len, &rde) || rde.desc_count > RF_2A4M1_RIC_MAX_RESOURCES)
		return false;
	out->rdie_id = rde.rdie_id;
	out->count   = rde.desc_count;
	/* Per resource: Status(2,LE) then, when granted, the granted TSPEC element (tspec_parse — the
	 * REUSED qos codec — decodes the body). Only the framing moves to the cursor. */
	struct rf_2a4m1_rd r;
	rf_2a4m1_rd_init(&r, in, len);
	rf_2a4m1_rd_skip(&r, 2 + RF_2A4M1_RIC_RDE_BODY_LEN);                 /* past the RDE (present when rde_parse ok) */
	for (size_t i = 0; i < rde.desc_count; i++) {
		if (rf_2a4m1_rd_remaining(&r) < 2)                  /* was: p + 2 > len */
			return false;
		out->status[i]  = rf_2a4m1_rd_le16(&r);
		out->granted[i] = (out->status[i] == RF_2A4M1_RIC_STATUS_SUCCESS);
		if (out->granted[i]) {
			if (rf_2a4m1_rd_remaining(&r) < 2)          /* was: p + 2 > len */
				return false;
			uint8_t eid  = rf_2a4m1_rd_u8(&r);
			uint8_t dlen = rf_2a4m1_rd_u8(&r);
			if (eid != RF_2A4M1_WLAN_EID_TSPEC)
				return false;
			const uint8_t *tsp = rf_2a4m1_rd_bytes(&r, dlen);   /* was: (size_t)(2 + dlen) > len - p */
			if (!tsp || !rf_2a4m1_tspec_parse(tsp, dlen, &out->rf_2a4m1_tspec[i]))
				return false;
		}
	}
	if (consumed)
		*consumed = r.pos;                         /* was: p — total RIC-Response byte length */
	return true;
}

