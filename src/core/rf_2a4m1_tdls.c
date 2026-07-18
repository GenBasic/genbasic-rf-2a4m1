// SPDX-License-Identifier: GPL-2.0
//
// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
// 802.11z TDLS direct-link setup (peer-link SM + TPK)
//
// Copyright (c) GenBasic.
// Licensed under the GNU General Public License, version 2.
//
// This file is machine-generated. Do not hand-edit.


#include "rf_2a4m1_core.h"

/* file-scope constants + helper types */
#define RF_2A4M1_TDLS_MIC_ELEMS_MAX (2 + RF_2A4M1_TDLS_LINK_ID_BODY + 2 + RF_2A4M1_TDLS_TIE_BODY + 2 + RF_2A4M1_TDLS_FTIE_FIXED)

/* file-local forward declarations */
static int rf_2a4m1_tdls_bcmp(const uint8_t *a, const uint8_t *b, size_t n);
static size_t rf_2a4m1_tdls_put_enc_hdr(uint8_t *out, size_t cap, uint8_t action);
static void rf_2a4m1_tdls_frame_mic(const struct rf_2a4m1_tdls_peer *p, const struct rf_2a4m1_tdls_ftie *f,
                           const struct rf_2a4m1_tdls_link_id *lid, uint32_t key_lifetime,
                           uint8_t trans_seq, uint8_t mic[RF_2A4M1_TDLS_MIC_LEN]);

static int rf_2a4m1_tdls_bcmp(const uint8_t *a, const uint8_t *b, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (a[i] != b[i])
			return a[i] < b[i] ? -1 : 1;
	}
	return 0;
}

size_t rf_2a4m1_tdls_link_id_build(uint8_t *out, size_t cap, const struct rf_2a4m1_tdls_link_id *l)
{
	if (!out || !l || cap < 2 + RF_2A4M1_TDLS_LINK_ID_BODY)
		return 0;
	out[0] = RF_2A4M1_TDLS_EID_LINK_ID;
	out[1] = RF_2A4M1_TDLS_LINK_ID_BODY;
	memcpy(&out[2], l->bssid.a, RF_2A4M1_ETH_ALEN);
	memcpy(&out[8], l->init_addr.a, RF_2A4M1_ETH_ALEN);
	memcpy(&out[14], l->resp_addr.a, RF_2A4M1_ETH_ALEN);
	return 2 + RF_2A4M1_TDLS_LINK_ID_BODY;
}

bool rf_2a4m1_tdls_link_id_parse(const uint8_t *ie, size_t len, struct rf_2a4m1_tdls_link_id *l)
{
	if (!ie || !l)
		return false;
	/* EID(1) Len(1) BSSID(6) init-STA(6) resp-STA(6) — the fixed 20-octet element off the cursor. */
	struct rf_2a4m1_rd r;
	rf_2a4m1_rd_init(&r, ie, len);
	uint8_t eid  = rf_2a4m1_rd_u8(&r);
	uint8_t elen = rf_2a4m1_rd_u8(&r);
	const uint8_t *bssid = rf_2a4m1_rd_bytes(&r, RF_2A4M1_ETH_ALEN);
	const uint8_t *ia    = rf_2a4m1_rd_bytes(&r, RF_2A4M1_ETH_ALEN);
	const uint8_t *ra    = rf_2a4m1_rd_bytes(&r, RF_2A4M1_ETH_ALEN);
	if (!rf_2a4m1_rd_ok(&r))                                    /* was: len < 2 + TDLS_LINK_ID_BODY */
		return false;
	if (eid != RF_2A4M1_TDLS_EID_LINK_ID || elen != RF_2A4M1_TDLS_LINK_ID_BODY)
		return false;
	memcpy(l->bssid.a, bssid, RF_2A4M1_ETH_ALEN);
	memcpy(l->init_addr.a, ia, RF_2A4M1_ETH_ALEN);
	memcpy(l->resp_addr.a, ra, RF_2A4M1_ETH_ALEN);
	return true;
}

size_t rf_2a4m1_tdls_tie_build(uint8_t *out, size_t cap, uint32_t key_lifetime)
{
	if (!out || cap < 2 + RF_2A4M1_TDLS_TIE_BODY)
		return 0;
	out[0] = RF_2A4M1_TDLS_EID_TIMEOUT_INTERVAL;
	out[1] = RF_2A4M1_TDLS_TIE_BODY;
	out[2] = RF_2A4M1_TDLS_TIE_TYPE_KEY_LIFETIME;
	rf_2a4m1_tdls_put_le32(&out[3], key_lifetime);
	return 2 + RF_2A4M1_TDLS_TIE_BODY;
}

bool rf_2a4m1_tdls_tie_parse(const uint8_t *ie, size_t len, uint32_t *key_lifetime)
{
	if (!ie)
		return false;
	/* EID(1) Len(1) Interval-Type(1) Value(4,LE) — the fixed 7-octet element off the cursor. */
	struct rf_2a4m1_rd r;
	rf_2a4m1_rd_init(&r, ie, len);
	uint8_t  eid   = rf_2a4m1_rd_u8(&r);
	uint8_t  elen  = rf_2a4m1_rd_u8(&r);
	uint8_t  type  = rf_2a4m1_rd_u8(&r);
	uint32_t value = rf_2a4m1_rd_le32(&r);
	if (!rf_2a4m1_rd_ok(&r))                                    /* was: len < 2 + TDLS_TIE_BODY */
		return false;
	if (eid != RF_2A4M1_TDLS_EID_TIMEOUT_INTERVAL || elen != RF_2A4M1_TDLS_TIE_BODY)
		return false;
	if (type != RF_2A4M1_TDLS_TIE_TYPE_KEY_LIFETIME)
		return false;
	if (key_lifetime)
		*key_lifetime = value;
	return true;
}

size_t rf_2a4m1_tdls_ftie_build(uint8_t *out, size_t cap, const struct rf_2a4m1_tdls_ftie *f)
{
	if (!out || !f || cap < 2 + RF_2A4M1_TDLS_FTIE_FIXED)
		return 0;
	out[0] = RF_2A4M1_TDLS_EID_FTIE;
	out[1] = RF_2A4M1_TDLS_FTIE_FIXED;
	out[2] = 0;                         /* MIC Control: Reserved */
	out[3] = 0;                         /* MIC Control: Element Count (unused by TDLS) */
	memcpy(&out[4], f->mic, RF_2A4M1_TDLS_MIC_LEN);
	memcpy(&out[20], f->anonce, RF_2A4M1_TDLS_NONCE_LEN);
	memcpy(&out[52], f->snonce, RF_2A4M1_TDLS_NONCE_LEN);
	return 2 + RF_2A4M1_TDLS_FTIE_FIXED;
}

bool rf_2a4m1_tdls_ftie_parse(const uint8_t *ie, size_t len, struct rf_2a4m1_tdls_ftie *f)
{
	if (!ie || !f)
		return false;
	/* EID(1) Len(1) then Len octets of element body, off the sticky-error cursor. Only the
	 * byte-level field extraction moves here — the FTIE MIC (AES-128-CMAC) is computed
	 * separately in tdls_ftie_mic() and is untouched. */
	struct rf_2a4m1_rd r;
	rf_2a4m1_rd_init(&r, ie, len);
	uint8_t eid  = rf_2a4m1_rd_u8(&r);
	uint8_t elen = rf_2a4m1_rd_u8(&r);
	if (!rf_2a4m1_rd_ok(&r))                                    /* was: len < 2 (subset of len < 2+FIXED) */
		return false;
	if (eid != RF_2A4M1_TDLS_EID_FTIE || elen < RF_2A4M1_TDLS_FTIE_FIXED)
		return false;
	const uint8_t *b = rf_2a4m1_rd_bytes(&r, elen);             /* was: (size_t)(2 + ie[1]) > len */
	if (!b)
		return false;
	/* MIC-Control(2) MIC(16) ANonce(32) SNonce(32) = 82 fixed octets; b points at the body. */
	memcpy(f->mic, &b[2], RF_2A4M1_TDLS_MIC_LEN);
	memcpy(f->anonce, &b[18], RF_2A4M1_TDLS_NONCE_LEN);
	memcpy(f->snonce, &b[50], RF_2A4M1_TDLS_NONCE_LEN);
	return true;
}

void rf_2a4m1_tdls_derive_tpk(const uint8_t snonce[RF_2A4M1_TDLS_NONCE_LEN], const uint8_t anonce[RF_2A4M1_TDLS_NONCE_LEN],
                     const rf_2a4m1_mac_addr *mac_i, const rf_2a4m1_mac_addr *mac_r, const rf_2a4m1_mac_addr *bssid,
                     uint8_t tpk[RF_2A4M1_TDLS_TPK_LEN])
{
	if (!snonce || !anonce || !mac_i || !mac_r || !bssid || !tpk)
		return;

	/* TPK-Key-Input = SHA-256(min(SNonce,ANonce) || max(SNonce,ANonce)). */
	uint8_t nbuf[2 * RF_2A4M1_TDLS_NONCE_LEN];
	if (rf_2a4m1_tdls_bcmp(snonce, anonce, RF_2A4M1_TDLS_NONCE_LEN) < 0) {
		memcpy(&nbuf[0], snonce, RF_2A4M1_TDLS_NONCE_LEN);
		memcpy(&nbuf[RF_2A4M1_TDLS_NONCE_LEN], anonce, RF_2A4M1_TDLS_NONCE_LEN);
	} else {
		memcpy(&nbuf[0], anonce, RF_2A4M1_TDLS_NONCE_LEN);
		memcpy(&nbuf[RF_2A4M1_TDLS_NONCE_LEN], snonce, RF_2A4M1_TDLS_NONCE_LEN);
	}
	uint8_t key_input[RF_2A4M1_SHA256_DIGEST_LEN];
	rf_2a4m1_sha256(nbuf, sizeof nbuf, key_input);

	/* ctx = min(MAC_I,MAC_R) || max(MAC_I,MAC_R) || BSSID. */
	uint8_t ctx[RF_2A4M1_ETH_ALEN * 3];
	if (rf_2a4m1_tdls_bcmp(mac_i->a, mac_r->a, RF_2A4M1_ETH_ALEN) < 0) {
		memcpy(&ctx[0], mac_i->a, RF_2A4M1_ETH_ALEN);
		memcpy(&ctx[RF_2A4M1_ETH_ALEN], mac_r->a, RF_2A4M1_ETH_ALEN);
	} else {
		memcpy(&ctx[0], mac_r->a, RF_2A4M1_ETH_ALEN);
		memcpy(&ctx[RF_2A4M1_ETH_ALEN], mac_i->a, RF_2A4M1_ETH_ALEN);
	}
	memcpy(&ctx[2 * RF_2A4M1_ETH_ALEN], bssid->a, RF_2A4M1_ETH_ALEN);

	/* TPK = KDF-256(TPK-Key-Input, "TDLS PMK", ctx). */
	rf_2a4m1_ieee80211_kdf_length(key_input, sizeof key_input, "TDLS PMK", 8, ctx, sizeof ctx,
	                     tpk, RF_2A4M1_TDLS_TPK_LEN * 8);

	rf_2a4m1_crypto_wipe(key_input, sizeof key_input);
	rf_2a4m1_crypto_wipe(nbuf, sizeof nbuf);
}

void rf_2a4m1_tdls_ftie_mic(const uint8_t kck[RF_2A4M1_TDLS_TPK_KCK_LEN],
                   const rf_2a4m1_mac_addr *init, const rf_2a4m1_mac_addr *resp, uint8_t trans_seq,
                   const uint8_t *lid, size_t lid_len,
                   const uint8_t *tie, size_t tie_len,
                   const uint8_t *ftie, size_t ftie_len,
                   uint8_t mic[RF_2A4M1_TDLS_MIC_LEN])
{
	if (!kck || !init || !resp || !ftie || !mic)
		return;
	if (lid_len > 2 + RF_2A4M1_TDLS_LINK_ID_BODY || tie_len > 2 + RF_2A4M1_TDLS_TIE_BODY ||
	    ftie_len > 2 + 255 || (lid_len + tie_len + ftie_len) > RF_2A4M1_TDLS_MIC_ELEMS_MAX)
		return;                                  /* out of scope for our frames — refuse */

	uint8_t buf[RF_2A4M1_ETH_ALEN + RF_2A4M1_ETH_ALEN + 1 + RF_2A4M1_TDLS_MIC_ELEMS_MAX];
	size_t p = 0;
	memcpy(&buf[p], init->a, RF_2A4M1_ETH_ALEN);          p += RF_2A4M1_ETH_ALEN;
	memcpy(&buf[p], resp->a, RF_2A4M1_ETH_ALEN);          p += RF_2A4M1_ETH_ALEN;
	buf[p++] = trans_seq;
	if (lid && lid_len) { memcpy(&buf[p], lid, lid_len); p += lid_len; }
	if (tie && tie_len) { memcpy(&buf[p], tie, tie_len); p += tie_len; }
	size_t ftie_at = p;
	memcpy(&buf[p], ftie, ftie_len);             p += ftie_len;
	/* zero the FTIE MIC field (element bytes [4..19]) regardless of its current contents. */
	if (ftie_len >= 2 + 2 + RF_2A4M1_TDLS_MIC_LEN)
		memset(&buf[ftie_at + 4], 0, RF_2A4M1_TDLS_MIC_LEN);
	rf_2a4m1_aes128_cmac(kck, buf, p, mic);
}

static size_t rf_2a4m1_tdls_put_enc_hdr(uint8_t *out, size_t cap, uint8_t action)
{
	if (cap < 3)
		return 0;
	out[0] = RF_2A4M1_TDLS_PAYLOAD_TYPE;
	out[1] = RF_2A4M1_TDLS_CATEGORY;
	out[2] = action;
	return 3;
}

bool rf_2a4m1_tdls_setup_req_parse(const uint8_t *in, size_t len, struct rf_2a4m1_tdls_setup_req *r)
{
	if (!in || !r)
		return false;
	/* PayloadType(0x02) Category(12) Action Dialog-Token(1) Capability(2,LE), then the FTIE /
	 * TIE / LinkID elements — each decoded by its own bounds-checked element parser, with the
	 * cursor advanced past the element it consumed. */
	struct rf_2a4m1_rd rr;
	rf_2a4m1_rd_init(&rr, in, len);
	uint8_t  pt   = rf_2a4m1_rd_u8(&rr);
	uint8_t  cat  = rf_2a4m1_rd_u8(&rr);
	uint8_t  act  = rf_2a4m1_rd_u8(&rr);
	uint8_t  tok  = rf_2a4m1_rd_u8(&rr);
	uint16_t cap  = rf_2a4m1_rd_le16(&rr);
	if (!rf_2a4m1_rd_ok(&rr))                                   /* was: enc-hdr len<3 + len < p+3 */
		return false;
	if (pt != RF_2A4M1_TDLS_PAYLOAD_TYPE || cat != RF_2A4M1_TDLS_CATEGORY || act != RF_2A4M1_TDLS_ACT_SETUP_REQ)
		return false;
	r->dialog_token = tok;
	r->cap_info = cap;
	const uint8_t *ftie = &rr.p[rr.pos];
	if (!rf_2a4m1_tdls_ftie_parse(ftie, rf_2a4m1_rd_remaining(&rr), &r->ftie))
		return false;
	rf_2a4m1_rd_skip(&rr, 2 + ftie[1]);   /* advance by the actual FTIE length (may carry subelements) */
	if (!rf_2a4m1_tdls_tie_parse(&rr.p[rr.pos], rf_2a4m1_rd_remaining(&rr), &r->key_lifetime))
		return false;
	rf_2a4m1_rd_skip(&rr, 2 + RF_2A4M1_TDLS_TIE_BODY);
	if (!rf_2a4m1_tdls_link_id_parse(&rr.p[rr.pos], rf_2a4m1_rd_remaining(&rr), &r->lid))
		return false;
	return true;
}

size_t rf_2a4m1_tdls_setup_resp_build(uint8_t *out, size_t cap, const struct rf_2a4m1_tdls_setup_resp *r)
{
	if (!out || !r)
		return 0;
	size_t p = rf_2a4m1_tdls_put_enc_hdr(out, cap, RF_2A4M1_TDLS_ACT_SETUP_RESP);
	if (!p)
		return 0;
	/* Status(2) Dialog Token(1) Capability(2) FTIE TIE LinkID */
	if (cap < p + 5)
		return 0;
	rf_2a4m1_tdls_put_le16(&out[p], r->status);             p += 2;
	out[p++] = r->dialog_token;
	rf_2a4m1_tdls_put_le16(&out[p], r->cap_info);           p += 2;
	size_t n;
	n = rf_2a4m1_tdls_ftie_build(&out[p], cap - p, &r->ftie);       if (!n) return 0; p += n;
	n = rf_2a4m1_tdls_tie_build(&out[p], cap - p, r->key_lifetime); if (!n) return 0; p += n;
	n = rf_2a4m1_tdls_link_id_build(&out[p], cap - p, &r->lid);     if (!n) return 0; p += n;
	return p;
}

bool rf_2a4m1_tdls_setup_resp_parse(const uint8_t *in, size_t len, struct rf_2a4m1_tdls_setup_resp *r)
{
	if (!in || !r)
		return false;
	/* PayloadType(0x02) Category(12) Action Status(2,LE) Dialog-Token(1) Capability(2,LE), then
	 * the FTIE / TIE / LinkID elements — each decoded by its own bounds-checked parser. */
	struct rf_2a4m1_rd rr;
	rf_2a4m1_rd_init(&rr, in, len);
	uint8_t  pt   = rf_2a4m1_rd_u8(&rr);
	uint8_t  cat  = rf_2a4m1_rd_u8(&rr);
	uint8_t  act  = rf_2a4m1_rd_u8(&rr);
	uint16_t st   = rf_2a4m1_rd_le16(&rr);
	uint8_t  tok  = rf_2a4m1_rd_u8(&rr);
	uint16_t cap  = rf_2a4m1_rd_le16(&rr);
	if (!rf_2a4m1_rd_ok(&rr))                                   /* was: enc-hdr len<3 + len < p+5 */
		return false;
	if (pt != RF_2A4M1_TDLS_PAYLOAD_TYPE || cat != RF_2A4M1_TDLS_CATEGORY || act != RF_2A4M1_TDLS_ACT_SETUP_RESP)
		return false;
	r->status = st;
	r->dialog_token = tok;
	r->cap_info = cap;
	const uint8_t *ftie = &rr.p[rr.pos];
	if (!rf_2a4m1_tdls_ftie_parse(ftie, rf_2a4m1_rd_remaining(&rr), &r->ftie))
		return false;
	rf_2a4m1_rd_skip(&rr, 2 + ftie[1]);   /* advance by the actual FTIE length (may carry subelements) */
	if (!rf_2a4m1_tdls_tie_parse(&rr.p[rr.pos], rf_2a4m1_rd_remaining(&rr), &r->key_lifetime))
		return false;
	rf_2a4m1_rd_skip(&rr, 2 + RF_2A4M1_TDLS_TIE_BODY);
	if (!rf_2a4m1_tdls_link_id_parse(&rr.p[rr.pos], rf_2a4m1_rd_remaining(&rr), &r->lid))
		return false;
	return true;
}

size_t rf_2a4m1_tdls_setup_confirm_build(uint8_t *out, size_t cap, const struct rf_2a4m1_tdls_setup_confirm *r)
{
	if (!out || !r)
		return 0;
	size_t p = rf_2a4m1_tdls_put_enc_hdr(out, cap, RF_2A4M1_TDLS_ACT_SETUP_CONFIRM);
	if (!p)
		return 0;
	/* Status(2) Dialog Token(1) FTIE TIE LinkID */
	if (cap < p + 3)
		return 0;
	rf_2a4m1_tdls_put_le16(&out[p], r->status);             p += 2;
	out[p++] = r->dialog_token;
	size_t n;
	n = rf_2a4m1_tdls_ftie_build(&out[p], cap - p, &r->ftie);       if (!n) return 0; p += n;
	n = rf_2a4m1_tdls_tie_build(&out[p], cap - p, r->key_lifetime); if (!n) return 0; p += n;
	n = rf_2a4m1_tdls_link_id_build(&out[p], cap - p, &r->lid);     if (!n) return 0; p += n;
	return p;
}

bool rf_2a4m1_tdls_setup_confirm_parse(const uint8_t *in, size_t len, struct rf_2a4m1_tdls_setup_confirm *r)
{
	if (!in || !r)
		return false;
	/* PayloadType(0x02) Category(12) Action Status(2,LE) Dialog-Token(1), then the FTIE / TIE /
	 * LinkID elements — each decoded by its own bounds-checked parser. */
	struct rf_2a4m1_rd rr;
	rf_2a4m1_rd_init(&rr, in, len);
	uint8_t  pt   = rf_2a4m1_rd_u8(&rr);
	uint8_t  cat  = rf_2a4m1_rd_u8(&rr);
	uint8_t  act  = rf_2a4m1_rd_u8(&rr);
	uint16_t st   = rf_2a4m1_rd_le16(&rr);
	uint8_t  tok  = rf_2a4m1_rd_u8(&rr);
	if (!rf_2a4m1_rd_ok(&rr))                                   /* was: enc-hdr len<3 + len < p+3 */
		return false;
	if (pt != RF_2A4M1_TDLS_PAYLOAD_TYPE || cat != RF_2A4M1_TDLS_CATEGORY || act != RF_2A4M1_TDLS_ACT_SETUP_CONFIRM)
		return false;
	r->status = st;
	r->dialog_token = tok;
	const uint8_t *ftie = &rr.p[rr.pos];
	if (!rf_2a4m1_tdls_ftie_parse(ftie, rf_2a4m1_rd_remaining(&rr), &r->ftie))
		return false;
	rf_2a4m1_rd_skip(&rr, 2 + ftie[1]);   /* advance by the actual FTIE length (may carry subelements) */
	if (!rf_2a4m1_tdls_tie_parse(&rr.p[rr.pos], rf_2a4m1_rd_remaining(&rr), &r->key_lifetime))
		return false;
	rf_2a4m1_rd_skip(&rr, 2 + RF_2A4M1_TDLS_TIE_BODY);
	if (!rf_2a4m1_tdls_link_id_parse(&rr.p[rr.pos], rf_2a4m1_rd_remaining(&rr), &r->lid))
		return false;
	return true;
}

bool rf_2a4m1_tdls_teardown_parse(const uint8_t *in, size_t len, struct rf_2a4m1_tdls_teardown *t)
{
	if (!in || !t)
		return false;
	/* PayloadType(0x02) Category(12) Action Reason(2,LE), then an OPTIONAL FTIE preceding the
	 * mandatory Link Identifier. */
	struct rf_2a4m1_rd rr;
	rf_2a4m1_rd_init(&rr, in, len);
	uint8_t  pt   = rf_2a4m1_rd_u8(&rr);
	uint8_t  cat  = rf_2a4m1_rd_u8(&rr);
	uint8_t  act  = rf_2a4m1_rd_u8(&rr);
	uint16_t rsn  = rf_2a4m1_rd_le16(&rr);
	if (!rf_2a4m1_rd_ok(&rr))                                   /* was: enc-hdr len<3 + len < p+2 */
		return false;
	if (pt != RF_2A4M1_TDLS_PAYLOAD_TYPE || cat != RF_2A4M1_TDLS_CATEGORY || act != RF_2A4M1_TDLS_ACT_TEARDOWN)
		return false;
	t->reason = rsn;
	/* An optional FTIE precedes the Link Identifier: EID 55 => a secure teardown. */
	t->has_ftie = false;
	if (rf_2a4m1_rd_remaining(&rr) >= 1 && rr.p[rr.pos] == RF_2A4M1_TDLS_EID_FTIE) {   /* was: len - p >= 1 && in[p] */
		const uint8_t *ftie = &rr.p[rr.pos];
		if (!rf_2a4m1_tdls_ftie_parse(ftie, rf_2a4m1_rd_remaining(&rr), &t->ftie))
			return false;
		t->has_ftie = true;
		rf_2a4m1_rd_skip(&rr, 2 + ftie[1]);   /* advance by the actual FTIE length (may carry subelements) */
	}
	if (!rf_2a4m1_tdls_link_id_parse(&rr.p[rr.pos], rf_2a4m1_rd_remaining(&rr), &t->lid))
		return false;
	return true;
}

static void rf_2a4m1_tdls_frame_mic(const struct rf_2a4m1_tdls_peer *p, const struct rf_2a4m1_tdls_ftie *f,
                           const struct rf_2a4m1_tdls_link_id *lid, uint32_t key_lifetime,
                           uint8_t trans_seq, uint8_t mic[RF_2A4M1_TDLS_MIC_LEN])
{
	uint8_t lidb[2 + RF_2A4M1_TDLS_LINK_ID_BODY];
	uint8_t tieb[2 + RF_2A4M1_TDLS_TIE_BODY];
	uint8_t ftieb[2 + RF_2A4M1_TDLS_FTIE_FIXED];
	size_t ll = rf_2a4m1_tdls_link_id_build(lidb, sizeof lidb, lid);
	size_t tl = rf_2a4m1_tdls_tie_build(tieb, sizeof tieb, key_lifetime);
	size_t fl = rf_2a4m1_tdls_ftie_build(ftieb, sizeof ftieb, f);
	/* TPK-KCK is the first 16 octets of the TPK. */
	rf_2a4m1_tdls_ftie_mic(p->tpk, &p->lid.init_addr, &p->lid.resp_addr, trans_seq,
	              lidb, ll, tieb, tl, ftieb, fl, mic);
}

void rf_2a4m1_tdls_peer_init(struct rf_2a4m1_tdls_peer *p, const rf_2a4m1_mac_addr *self, const rf_2a4m1_mac_addr *peer,
                    const rf_2a4m1_mac_addr *bssid, bool is_initiator,
                    uint8_t dialog_token, uint32_t key_lifetime)
{
	if (!p || !self || !peer || !bssid)
		return;
	memset(p, 0, sizeof *p);
	p->state = RF_2A4M1_TDLS_IDLE;
	p->is_initiator = is_initiator;
	p->self = *self;
	p->peer = *peer;
	p->bssid = *bssid;
	p->dialog_token = dialog_token;
	p->key_lifetime = key_lifetime;
	/* The Link Identifier binds the initiator/responder roles; both ends agree on it. */
	p->lid.bssid = *bssid;
	p->lid.init_addr = is_initiator ? *self : *peer;
	p->lid.resp_addr = is_initiator ? *peer : *self;
}

size_t rf_2a4m1_tdls_on_setup_req(struct rf_2a4m1_tdls_peer *p, const uint8_t *in, size_t len,
                         const uint8_t anonce[RF_2A4M1_TDLS_NONCE_LEN], uint8_t *out, size_t cap)
{
	if (!p || !in || !anonce || !out)
		return 0;
	if (p->state != RF_2A4M1_TDLS_IDLE || p->is_initiator)   /* duplicate / out-of-order / wrong role */
		return 0;

	struct rf_2a4m1_tdls_setup_req req;
	if (!rf_2a4m1_tdls_setup_req_parse(in, len, &req))
		return 0;
	if (!rf_2a4m1_tdls_link_id_eq(&req.lid, &p->lid))         /* a frame for a different direct link */
		return 0;

	memcpy(p->snonce, req.ftie.snonce, RF_2A4M1_TDLS_NONCE_LEN);
	memcpy(p->anonce, anonce, RF_2A4M1_TDLS_NONCE_LEN);
	rf_2a4m1_tdls_derive_tpk(p->snonce, p->anonce, &p->lid.init_addr, &p->lid.resp_addr, &p->bssid, p->tpk);
	p->have_tpk = true;
	p->dialog_token = req.dialog_token;

	struct rf_2a4m1_tdls_setup_resp resp;
	memset(&resp, 0, sizeof resp);
	resp.status = 0;
	resp.dialog_token = p->dialog_token;
	resp.cap_info = 0;
	memcpy(resp.ftie.anonce, p->anonce, RF_2A4M1_TDLS_NONCE_LEN);
	memcpy(resp.ftie.snonce, p->snonce, RF_2A4M1_TDLS_NONCE_LEN);
	resp.key_lifetime = p->key_lifetime;
	resp.lid = p->lid;
	rf_2a4m1_tdls_frame_mic(p, &resp.ftie, &resp.lid, resp.key_lifetime, RF_2A4M1_TDLS_MIC_SEQ_RESP, resp.ftie.mic);

	size_t n = rf_2a4m1_tdls_setup_resp_build(out, cap, &resp);
	if (!n)
		return 0;
	p->state = RF_2A4M1_TDLS_SETUP;
	return n;
}

size_t rf_2a4m1_tdls_on_setup_resp(struct rf_2a4m1_tdls_peer *p, const uint8_t *in, size_t len,
                          uint8_t *out, size_t cap)
{
	if (!p || !in || !out)
		return 0;
	if (p->state != RF_2A4M1_TDLS_REQ_SENT)
		return 0;

	struct rf_2a4m1_tdls_setup_resp resp;
	if (!rf_2a4m1_tdls_setup_resp_parse(in, len, &resp))
		return 0;
	if (resp.status != 0 || resp.dialog_token != p->dialog_token)
		return 0;
	if (!rf_2a4m1_tdls_link_id_eq(&resp.lid, &p->lid))
		return 0;
	if (!rf_2a4m1_crypto_ct_eq(resp.ftie.snonce, p->snonce, RF_2A4M1_TDLS_NONCE_LEN))   /* our SNonce echoed back */
		return 0;

	/* Derive the TPK into a LOCAL buffer + verify the Response MIC BEFORE committing state:
	 * a MIC failure must not advance the FSM. */
	uint8_t tpk[RF_2A4M1_TDLS_TPK_LEN];
	rf_2a4m1_tdls_derive_tpk(p->snonce, resp.ftie.anonce, &p->lid.init_addr, &p->lid.resp_addr,
	                &p->bssid, tpk);
	struct rf_2a4m1_tdls_peer probe = *p;
	memcpy(probe.tpk, tpk, RF_2A4M1_TDLS_TPK_LEN);
	uint8_t expect[RF_2A4M1_TDLS_MIC_LEN];
	rf_2a4m1_tdls_frame_mic(&probe, &resp.ftie, &resp.lid, resp.key_lifetime, RF_2A4M1_TDLS_MIC_SEQ_RESP, expect);
	if (!rf_2a4m1_crypto_ct_eq(expect, resp.ftie.mic, RF_2A4M1_TDLS_MIC_LEN)) {
		rf_2a4m1_crypto_wipe(tpk, sizeof tpk);
		return 0;                                    /* bad MIC — reject, state UNCHANGED */
	}

	memcpy(p->anonce, resp.ftie.anonce, RF_2A4M1_TDLS_NONCE_LEN);
	memcpy(p->tpk, tpk, RF_2A4M1_TDLS_TPK_LEN);
	p->have_tpk = true;
	rf_2a4m1_crypto_wipe(tpk, sizeof tpk);

	struct rf_2a4m1_tdls_setup_confirm conf;
	memset(&conf, 0, sizeof conf);
	conf.status = 0;
	conf.dialog_token = p->dialog_token;
	memcpy(conf.ftie.anonce, p->anonce, RF_2A4M1_TDLS_NONCE_LEN);
	memcpy(conf.ftie.snonce, p->snonce, RF_2A4M1_TDLS_NONCE_LEN);
	conf.key_lifetime = p->key_lifetime;
	conf.lid = p->lid;
	rf_2a4m1_tdls_frame_mic(p, &conf.ftie, &conf.lid, conf.key_lifetime, RF_2A4M1_TDLS_MIC_SEQ_CONFIRM, conf.ftie.mic);

	size_t n = rf_2a4m1_tdls_setup_confirm_build(out, cap, &conf);
	if (!n)
		return 0;
	p->state = RF_2A4M1_TDLS_ESTABLISHED;
	return n;
}

bool rf_2a4m1_tdls_on_setup_confirm(struct rf_2a4m1_tdls_peer *p, const uint8_t *in, size_t len)
{
	if (!p || !in)
		return false;
	if (p->state != RF_2A4M1_TDLS_SETUP || !p->have_tpk)
		return false;

	struct rf_2a4m1_tdls_setup_confirm conf;
	if (!rf_2a4m1_tdls_setup_confirm_parse(in, len, &conf))
		return false;
	if (conf.status != 0 || conf.dialog_token != p->dialog_token)
		return false;
	if (!rf_2a4m1_tdls_link_id_eq(&conf.lid, &p->lid))
		return false;

	uint8_t expect[RF_2A4M1_TDLS_MIC_LEN];
	rf_2a4m1_tdls_frame_mic(p, &conf.ftie, &conf.lid, conf.key_lifetime, RF_2A4M1_TDLS_MIC_SEQ_CONFIRM, expect);
	if (!rf_2a4m1_crypto_ct_eq(expect, conf.ftie.mic, RF_2A4M1_TDLS_MIC_LEN))
		return false;                                /* bad MIC — reject, state UNCHANGED */

	p->state = RF_2A4M1_TDLS_ESTABLISHED;
	return true;
}

bool rf_2a4m1_tdls_on_teardown(struct rf_2a4m1_tdls_peer *p, const uint8_t *in, size_t len)
{
	if (!p || !in)
		return false;
	struct rf_2a4m1_tdls_teardown td;
	if (!rf_2a4m1_tdls_teardown_parse(in, len, &td))
		return false;
	if (!rf_2a4m1_tdls_link_id_eq(&td.lid, &p->lid))          /* a teardown for a different link */
		return false;
	if (p->have_tpk) {
		/* A live TPKSA link's teardown is MIC-bound (the sender builds it so in
		 * tdls_teardown). The receiver must symmetrically REQUIRE the MIC-bearing
		 * FTIE: accepting a teardown that merely OMITS the FTIE would let an
		 * off-path attacker forge an unauthenticated teardown of a secure direct
		 * link (correct Link Identifier, no key) and tear it down — a DoS
		 * (IEEE 802.11 §11.20.4: a secure-link teardown carries the FTIE MIC and an
		 * invalid/absent one is discarded). Only a still-keyless link (have_tpk
		 * false, e.g. torn down while REQ-SENT) accepts an unprotected teardown. */
		if (!td.has_ftie)
			return false;                        /* secure link, no MIC — reject */
		uint8_t expect[RF_2A4M1_TDLS_MIC_LEN];
		rf_2a4m1_tdls_frame_mic(p, &td.ftie, &td.lid, p->key_lifetime, RF_2A4M1_TDLS_MIC_SEQ_TEARDOWN, expect);
		if (!rf_2a4m1_crypto_ct_eq(expect, td.ftie.mic, RF_2A4M1_TDLS_MIC_LEN))
			return false;                        /* bad MIC — reject, state UNCHANGED */
	}
	p->state = RF_2A4M1_TDLS_TEARDOWN;
	return true;
}

