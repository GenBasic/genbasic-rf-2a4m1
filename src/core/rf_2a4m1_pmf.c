// SPDX-License-Identifier: GPL-2.0
//
// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
// 802.11w Protected Management Frames (MFP, SA-Query, BIP/OCV/beacon-protection)
//
// Copyright (c) GenBasic.
// Licensed under the GNU General Public License, version 2.
//
// This file is machine-generated. Do not hand-edit.


#include "rf_2a4m1_core.h"

/* file-scope constants + helper types */
#define RF_2A4M1_RSN_CIPHER_CCMP128   4
#define RF_2A4M1_RSN_CIPHER_BIP_CMAC  6
#define RF_2A4M1_FC_TYPE_MGMT 0

/* file-local forward declarations */
static inline uint8_t rf_2a4m1_fc_subtype(uint16_t fc);
static inline uint8_t rf_2a4m1_fc_type(uint16_t fc);
static size_t rf_2a4m1_kde_find(const uint8_t *kd, size_t len, const uint8_t oui[3], uint8_t dt,
                       size_t min_body, size_t *body_len_out);
static void rf_2a4m1_ipn_be(uint8_t out[RF_2A4M1_BIP_IPN_LEN], uint64_t v);

static const uint8_t RF_2A4M1_RSN_OUI[3] = { 0x00, 0x0f, 0xac };

uint16_t rf_2a4m1_pmf_mode_caps(enum rf_2a4m1_pmf_mode mode)
{
	switch (mode) {
	case RF_2A4M1_PMF_MODE_REQUIRED: return (uint16_t)(RF_2A4M1_PMF_RSN_CAP_MFPC | RF_2A4M1_PMF_RSN_CAP_MFPR);
	case RF_2A4M1_PMF_MODE_OPTIONAL: return RF_2A4M1_PMF_RSN_CAP_MFPC;
	case RF_2A4M1_PMF_MODE_DISABLED:
	default:                return 0;
	}
}

uint16_t rf_2a4m1_pmf_negotiate(enum rf_2a4m1_pmf_mode local, uint16_t peer_rsn_caps, bool *mfp_active)
{
	uint16_t local_caps = rf_2a4m1_pmf_mode_caps(local);
	bool local_c = rf_2a4m1_pmf_caps_mfpc(local_caps), local_r = rf_2a4m1_pmf_caps_mfpr(local_caps);
	bool peer_c  = rf_2a4m1_pmf_caps_mfpc(peer_rsn_caps), peer_r = rf_2a4m1_pmf_caps_mfpr(peer_rsn_caps);

	/* A REQUIRE on either side against a not-CAPABLE peer is a policy violation: the
	 * association must be refused (§12.6.4 — the MFPC/MFPR combining rules). */
	if ((local_r && !peer_c) || (peer_r && !local_c)) {
		if (mfp_active)
			*mfp_active = false;
		return RF_2A4M1_PMF_STATUS_MFP_VIOLATION;
	}
	/* MFP is active iff BOTH ends are capable. */
	if (mfp_active)
		*mfp_active = (local_c && peer_c);
	return RF_2A4M1_PMF_STATUS_SUCCESS;
}

size_t rf_2a4m1_pmf_rsne_build(uint8_t *out, size_t cap, uint16_t rsn_caps, uint8_t akm)
{
	bool mfp = rf_2a4m1_pmf_caps_mfpc(rsn_caps);
	/* body = version(2) + group(4) + pairwise-cnt(2) + pairwise(4) + akm-cnt(2) + akm(4) +
	 *        caps(2) [ + pmkid-cnt(2) + group-mgmt(4) when MFPC ] */
	size_t body = 2 + 4 + 2 + 4 + 2 + 4 + 2 + (mfp ? (size_t)(2 + 4) : 0);
	size_t total = 2 + body;
	if (!out || cap < total)
		return 0;

	uint8_t *p = out;
	*p++ = RF_2A4M1_PMF_EID_RSN;
	*p++ = (uint8_t)body;
	rf_2a4m1_pmf_put_le16(p, 1); p += 2;                         /* Version = 1 */
	memcpy(p, RF_2A4M1_RSN_OUI, 3); p[3] = RF_2A4M1_RSN_CIPHER_CCMP128; p += 4;   /* Group Data Cipher = CCMP */
	rf_2a4m1_pmf_put_le16(p, 1); p += 2;                         /* Pairwise Cipher Suite Count = 1 */
	memcpy(p, RF_2A4M1_RSN_OUI, 3); p[3] = RF_2A4M1_RSN_CIPHER_CCMP128; p += 4;   /* Pairwise = CCMP */
	rf_2a4m1_pmf_put_le16(p, 1); p += 2;                         /* AKM Suite Count = 1 */
	memcpy(p, RF_2A4M1_RSN_OUI, 3); p[3] = akm; p += 4;          /* AKM (PSK/SAE/OWE) */
	rf_2a4m1_pmf_put_le16(p, rsn_caps); p += 2;                  /* RSN Capabilities (MFPC/MFPR) */
	if (mfp) {
		rf_2a4m1_pmf_put_le16(p, 0); p += 2;                 /* PMKID Count = 0 */
		memcpy(p, RF_2A4M1_RSN_OUI, 3); p[3] = RF_2A4M1_RSN_CIPHER_BIP_CMAC; p += 4; /* Group Mgmt = BIP */
	}
	return total;
}

bool rf_2a4m1_pmf_rsne_parse(const uint8_t *ie, size_t len, uint16_t *rsn_caps, bool *has_group_mgmt)
{
	if (rsn_caps)       *rsn_caps = 0;
	if (has_group_mgmt) *has_group_mgmt = false;
	if (!ie || len < 2 || ie[0] != RF_2A4M1_PMF_EID_RSN)
		return false;
	size_t dlen = ie[1];
	if ((size_t)2 + dlen > len)
		return false;

	const uint8_t *b = ie + 2;
	size_t rem = dlen;
	/* Version (2) — the only mandatory field. RSN Capabilities and everything after are
	 * OPTIONAL, so a shorter RSNE is still well-formed (MFP simply not advertised). */
	if (rem < 2)
		return false;
	rem -= 2; b += 2;                                   /* skip Version */

	/* Group Data Cipher Suite (4). */
	if (rem < 4) return true;
	rem -= 4; b += 4;
	/* Pairwise Cipher Suite Count (2) + list. */
	if (rem < 2) return true;
	uint16_t pw = rf_2a4m1_pmf_get_le16(b); rem -= 2; b += 2;
	if ((size_t)pw * 4 > rem) return false;            /* declared list overruns */
	rem -= (size_t)pw * 4; b += (size_t)pw * 4;
	/* AKM Suite Count (2) + list. */
	if (rem < 2) return true;
	uint16_t akm = rf_2a4m1_pmf_get_le16(b); rem -= 2; b += 2;
	if ((size_t)akm * 4 > rem) return false;
	rem -= (size_t)akm * 4; b += (size_t)akm * 4;
	/* RSN Capabilities (2). */
	if (rem < 2) return true;
	if (rsn_caps) *rsn_caps = rf_2a4m1_pmf_get_le16(b);
	rem -= 2; b += 2;
	/* PMKID Count (2) + list, then the Group Management Cipher Suite (4) — the PMF marker. */
	if (rem < 2) return true;
	uint16_t np = rf_2a4m1_pmf_get_le16(b); rem -= 2; b += 2;
	if ((size_t)np * 16 > rem) return false;
	rem -= (size_t)np * 16; b += (size_t)np * 16;
	if (rem >= 4) {
		if (has_group_mgmt)
			*has_group_mgmt = true;
	}
	return true;
}

size_t rf_2a4m1_pmf_comeback_ie_build(uint8_t *out, size_t cap, uint32_t comeback_tu)
{
	/* Timeout Interval element: EID 56, Len 5 = Type(1) + Value(4, LE). */
	if (!out || cap < 7)
		return 0;
	out[0] = RF_2A4M1_PMF_EID_TIMEOUT_INTERVAL;
	out[1] = 5;
	out[2] = RF_2A4M1_PMF_TIMEOUT_TYPE_ASSOC_COMEBACK;
	out[3] = (uint8_t)comeback_tu;
	out[4] = (uint8_t)(comeback_tu >> 8);
	out[5] = (uint8_t)(comeback_tu >> 16);
	out[6] = (uint8_t)(comeback_tu >> 24);
	return 7;
}

static inline uint8_t rf_2a4m1_fc_type(uint16_t fc)    { return (uint8_t)((fc >> 2) & 0x3); }

static inline uint8_t rf_2a4m1_fc_subtype(uint16_t fc) { return (uint8_t)((fc >> 4) & 0xf); }

bool rf_2a4m1_pmf_action_is_robust(uint8_t category)
{
	/* Robust = every category EXCEPT the fixed non-robust set (hostapd parity). */
	switch (category) {
	case RF_2A4M1_PMF_ACTION_PUBLIC:           /* 4  */
	case RF_2A4M1_PMF_ACTION_HT:               /* 7  */
	case RF_2A4M1_PMF_ACTION_UNPROTECTED_WNM:  /* 11 */
	case RF_2A4M1_PMF_ACTION_SELF_PROTECTED:   /* 15 */
	case RF_2A4M1_PMF_ACTION_UNPROTECTED_DMG:  /* 20 */
	case RF_2A4M1_PMF_ACTION_VHT:              /* 21 */
	case RF_2A4M1_PMF_ACTION_UNPROTECTED_S1G:  /* 23 */
	case RF_2A4M1_PMF_ACTION_VENDOR_SPECIFIC:  /* 127 */
		return false;
	default:
		return true;
	}
}

bool rf_2a4m1_pmf_is_robust_mgmt(uint16_t fc, const uint8_t *body, size_t body_len)
{
	if (rf_2a4m1_fc_type(fc) != RF_2A4M1_FC_TYPE_MGMT)
		return false;
	switch (rf_2a4m1_fc_subtype(fc)) {
	case RF_2A4M1_PMF_STYPE_DEAUTH:
	case RF_2A4M1_PMF_STYPE_DISASSOC:
		return true;
	case RF_2A4M1_PMF_STYPE_ACTION:
	case RF_2A4M1_PMF_STYPE_ACTION_NOACK:
		return body && body_len >= 1 && rf_2a4m1_pmf_action_is_robust(body[0]);
	default:
		return false;
	}
}

bool rf_2a4m1_pmf_rx_unprotected_ok(uint16_t fc, const uint8_t *body, size_t body_len, bool mfp_active)
{
	/* A robust management frame received UNPROTECTED while MFP is active must be dropped. */
	if (mfp_active && rf_2a4m1_pmf_is_robust_mgmt(fc, body, body_len))
		return false;
	return true;
}

void rf_2a4m1_pmf_igtk_install(struct rf_2a4m1_pmf_igtk *k, const uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                      uint16_t key_id, uint64_t ipn_start)
{
	if (!k)
		return;
	memcpy(k->rf_2a4m1_key, igtk, RF_2A4M1_BIP_IGTK_LEN);
	k->key_id = key_id;
	k->ipn_tx = ipn_start;
	k->ipn_rx = ipn_start;
	k->installed = true;
}

static void rf_2a4m1_ipn_be(uint8_t out[RF_2A4M1_BIP_IPN_LEN], uint64_t v)
{
	for (int i = 0; i < RF_2A4M1_BIP_IPN_LEN; i++)
		out[i] = (uint8_t)(v >> (8 * (RF_2A4M1_BIP_IPN_LEN - 1 - i)));
}

size_t rf_2a4m1_pmf_group_protect(struct rf_2a4m1_pmf_igtk *k, uint8_t *out, size_t cap,
                         const uint8_t *hdr, const uint8_t *body, size_t body_len)
{
	if (!k || !k->installed)
		return 0;
	uint8_t ipn[RF_2A4M1_BIP_IPN_LEN];
	uint64_t next = k->ipn_tx + 1;      /* IPN is strictly increasing per §12.5.4 */
	rf_2a4m1_ipn_be(ipn, next);
	size_t n = rf_2a4m1_bip_mmie_protect(out, cap, hdr, body, body_len, k->rf_2a4m1_key, ipn, k->key_id);
	if (n == 0)
		return 0;
	k->ipn_tx = next;                   /* commit only once the frame was built */
	return n;
}

int rf_2a4m1_pmf_group_verify(struct rf_2a4m1_pmf_igtk *k, const uint8_t *frame, size_t len)
{
	if (!k || !k->installed)
		return RF_2A4M1_BIP_ERR_MALFORMED;
	return rf_2a4m1_bip_mmie_verify(frame, len, k->rf_2a4m1_key, &k->ipn_rx, NULL);
}

size_t rf_2a4m1_pmf_build_protected_mgmt(struct rf_2a4m1_pmf_igtk *k, uint8_t *out, size_t cap, uint8_t subtype,
                                const rf_2a4m1_mac_addr *da, const rf_2a4m1_mac_addr *sa, const rf_2a4m1_mac_addr *bssid,
                                const uint8_t *body, size_t body_len, uint16_t seq_ctrl)
{
	if (!k || !k->installed || !da || !sa || !bssid)
		return 0;
	uint8_t hdr[RF_2A4M1_BIP_MGMT_HDR_LEN];
	uint16_t fc = (uint16_t)((RF_2A4M1_FC_TYPE_MGMT << 2) | ((subtype & 0xf) << 4));  /* mgmt, subtype */
	rf_2a4m1_pmf_put_le16(&hdr[0], fc);
	hdr[2] = 0; hdr[3] = 0;                          /* Duration */
	memcpy(&hdr[4],  da->a,    RF_2A4M1_ETH_ALEN);            /* A1 = DA (group for BIP) */
	memcpy(&hdr[10], sa->a,    RF_2A4M1_ETH_ALEN);            /* A2 = SA */
	memcpy(&hdr[16], bssid->a, RF_2A4M1_ETH_ALEN);            /* A3 = BSSID */
	rf_2a4m1_pmf_put_le16(&hdr[22], seq_ctrl);                /* SeqCtl */
	return rf_2a4m1_pmf_group_protect(k, out, cap, hdr, body, body_len);
}

size_t rf_2a4m1_pmf_sa_query_build(uint8_t *out, size_t cap, uint8_t action, uint16_t trans_id)
{
	if (!out || cap < RF_2A4M1_PMF_SA_QUERY_LEN)
		return 0;
	out[0] = RF_2A4M1_PMF_ACTION_SA_QUERY;          /* Category = 8 */
	out[1] = action;                       /* Action = 0 (Request) / 1 (Response) */
	rf_2a4m1_pmf_put_le16(&out[2], trans_id);       /* Transaction Identifier (echoed verbatim) */
	return RF_2A4M1_PMF_SA_QUERY_LEN;
}

bool rf_2a4m1_pmf_sa_query_parse(const uint8_t *in, size_t len, uint8_t *action, uint16_t *trans_id)
{
	if (!in || len < RF_2A4M1_PMF_SA_QUERY_LEN || in[0] != RF_2A4M1_PMF_ACTION_SA_QUERY)
		return false;
	if (in[1] != RF_2A4M1_PMF_SA_QUERY_REQUEST && in[1] != RF_2A4M1_PMF_SA_QUERY_RESPONSE)
		return false;
	if (action)   *action = in[1];
	if (trans_id) *trans_id = rf_2a4m1_pmf_get_le16(&in[2]);
	return true;
}

void rf_2a4m1_pmf_sa_query_init(struct rf_2a4m1_pmf_sa_query *q, unsigned max_retries, unsigned timeout_ticks)
{
	if (!q)
		return;
	q->state = RF_2A4M1_PMF_SA_QUERY_IDLE;
	q->trans_id = 0;
	q->next_tid = 1;
	q->retries = 0;
	q->max_retries = max_retries ? max_retries : 1;
	q->timeout_ticks = timeout_ticks ? timeout_ticks : 1;
	q->elapsed = 0;
}

size_t rf_2a4m1_pmf_sa_query_start(struct rf_2a4m1_pmf_sa_query *q, uint8_t *out, size_t cap)
{
	if (!q)
		return 0;
	q->trans_id = q->next_tid++;
	if (q->next_tid == 0)                  /* keep the id non-zero (cosmetic) */
		q->next_tid = 1;
	q->state = RF_2A4M1_PMF_SA_QUERY_PENDING;
	q->retries = 1;
	q->elapsed = 0;
	return rf_2a4m1_pmf_sa_query_build(out, cap, RF_2A4M1_PMF_SA_QUERY_REQUEST, q->trans_id);
}

size_t rf_2a4m1_pmf_sa_query_tick(struct rf_2a4m1_pmf_sa_query *q, uint8_t *out, size_t cap)
{
	if (!q || q->state != RF_2A4M1_PMF_SA_QUERY_PENDING)
		return 0;
	if (++q->elapsed < q->timeout_ticks)   /* still inside the current attempt's window */
		return 0;
	q->elapsed = 0;
	if (q->retries >= q->max_retries) {    /* budget exhausted — the peer is gone */
		q->state = RF_2A4M1_PMF_SA_QUERY_TIMEOUT;
		return 0;
	}
	q->retries++;                          /* retransmit the SAME transaction id */
	return rf_2a4m1_pmf_sa_query_build(out, cap, RF_2A4M1_PMF_SA_QUERY_REQUEST, q->trans_id);
}

bool rf_2a4m1_pmf_sa_query_on_response(struct rf_2a4m1_pmf_sa_query *q, const uint8_t *in, size_t len)
{
	if (!q || q->state != RF_2A4M1_PMF_SA_QUERY_PENDING)
		return false;
	uint8_t act; uint16_t tid;
	if (!rf_2a4m1_pmf_sa_query_parse(in, len, &act, &tid))
		return false;
	if (act != RF_2A4M1_PMF_SA_QUERY_RESPONSE || tid != q->trans_id)
		return false;                      /* wrong action / stale transaction id */
	q->state = RF_2A4M1_PMF_SA_QUERY_ANSWERED;
	return true;
}

size_t rf_2a4m1_pmf_sa_query_respond(const uint8_t *in, size_t len, uint8_t *out, size_t cap)
{
	uint8_t act; uint16_t tid;
	if (!rf_2a4m1_pmf_sa_query_parse(in, len, &act, &tid) || act != RF_2A4M1_PMF_SA_QUERY_REQUEST)
		return 0;
	return rf_2a4m1_pmf_sa_query_build(out, cap, RF_2A4M1_PMF_SA_QUERY_RESPONSE, tid);
}

static size_t rf_2a4m1_kde_find(const uint8_t *kd, size_t len, const uint8_t oui[3], uint8_t dt,
                       size_t min_body, size_t *body_len_out)
{
	size_t off = 0;
	while (off + 2 <= len) {
		uint8_t t = kd[off], l = kd[off + 1];
		if (t == 0x00)                           /* pad / end of Key Data */
			break;
		if (t == 0xdd && l == 0)                 /* the 0xdd 0x00 pad marker */
			break;
		size_t klen = (size_t)2 + l;
		if (off + klen > len)
			break;
		/* min_body >= 4 for every caller, so off+2..off+5 are in bounds here. */
		if (t == 0xdd && l >= min_body && kd[off + 2] == oui[0] && kd[off + 3] == oui[1] &&
		    kd[off + 4] == oui[2] && kd[off + 5] == dt) {
			if (body_len_out)
				*body_len_out = (size_t)l - 4;   /* payload after OUI(3)+dt(1) */
			return off + 6;                          /* offset of the payload */
		}
		off += klen;                                     /* skip: a non-matching KDE, or an IE */
	}
	return (size_t)-1;
}

size_t rf_2a4m1_pmf_bigtk_kde_build(uint8_t *out, size_t cap, const uint8_t bigtk[RF_2A4M1_BIP_IGTK_LEN],
                           uint16_t key_id, uint64_t bipn)
{
	/* dd len 00-0f-ac 0a KeyID(2,LE) BIPN(6,LE) BIGTK(16): body = OUI(3)+dt(1)+2+6+16 = 28. */
	if (!out || cap < 30)
		return 0;
	out[0] = 0xdd;
	out[1] = 28;
	memcpy(&out[2], RF_2A4M1_RSN_OUI, 3);
	out[5] = RF_2A4M1_PMF_BIGTK_KDE_DATATYPE;
	out[6] = (uint8_t)key_id; out[7] = (uint8_t)(key_id >> 8);
	for (int i = 0; i < 6; i++)
		out[8 + i] = (uint8_t)(bipn >> (8 * i));      /* BIPN (6 octets, LE) */
	memcpy(&out[14], bigtk, RF_2A4M1_BIP_IGTK_LEN);
	return 30;
}

bool rf_2a4m1_pmf_bigtk_kde_find(const uint8_t *kd, size_t len, uint8_t bigtk[RF_2A4M1_BIP_IGTK_LEN],
                        uint16_t *key_id, uint64_t *bipn)
{
	size_t blen = 0;
	size_t p = rf_2a4m1_kde_find(kd, len, RF_2A4M1_RSN_OUI, RF_2A4M1_PMF_BIGTK_KDE_DATATYPE, 24, &blen);
	/* KeyID(2)+BIPN(6)+BIGTK(16) = 24. Reject rather than truncate a KDE whose key length
	 * exceeds BIP-CMAC-128's 16-byte key (e.g. a 256-bit-BIP 32-byte BIGTK) — the only cipher
	 * negotiated today; a longer key means a wrong install, not a silent 16-byte copy. */
	if (p == (size_t)-1 || blen != 24)
		return false;
	if (key_id)
		*key_id = (uint16_t)(kd[p] | ((uint16_t)kd[p + 1] << 8));
	if (bipn) {
		uint64_t v = 0;
		for (int i = 0; i < 6; i++)
			v |= (uint64_t)kd[p + 2 + i] << (8 * i);
		*bipn = v;
	}
	memcpy(bigtk, &kd[p + 8], RF_2A4M1_BIP_IGTK_LEN);
	return true;
}

size_t rf_2a4m1_pmf_oci_element_build(uint8_t *out, size_t cap, const struct rf_2a4m1_pmf_oci *oci)
{
	/* EID 255, Len 4 (Ext-ID + 3 OCI octets), Ext-ID 54, op-class, primary-channel, seg1. */
	if (!out || !oci || cap < 6)
		return 0;
	out[0] = RF_2A4M1_PMF_EID_EXTENSION;
	out[1] = 1 + RF_2A4M1_PMF_OCI_LEN;
	out[2] = RF_2A4M1_PMF_EID_EXT_OCI;
	out[3] = oci->op_class;
	out[4] = oci->prim_chan;
	out[5] = oci->seg1_freq;
	return 6;
}

bool rf_2a4m1_pmf_oci_element_parse(const uint8_t *ie, size_t len, struct rf_2a4m1_pmf_oci *oci)
{
	if (!ie || len < 6 || ie[0] != RF_2A4M1_PMF_EID_EXTENSION || ie[1] != 1 + RF_2A4M1_PMF_OCI_LEN)
		return false;
	if (ie[2] != RF_2A4M1_PMF_EID_EXT_OCI)
		return false;
	if (oci) {
		oci->op_class  = ie[3];
		oci->prim_chan = ie[4];
		oci->seg1_freq = ie[5];
	}
	return true;
}

size_t rf_2a4m1_pmf_oci_kde_build(uint8_t *out, size_t cap, const struct rf_2a4m1_pmf_oci *oci)
{
	/* dd 07 00-0f-ac 0d op-class prim-chan seg1: body = OUI(3)+dt(1)+3 = 7. Total 9. */
	if (!out || !oci || cap < 9)
		return 0;
	out[0] = 0xdd;
	out[1] = 7;
	memcpy(&out[2], RF_2A4M1_RSN_OUI, 3);
	out[5] = RF_2A4M1_PMF_OCI_KDE_DATATYPE;
	out[6] = oci->op_class;
	out[7] = oci->prim_chan;
	out[8] = oci->seg1_freq;
	return 9;
}

bool rf_2a4m1_pmf_oci_kde_find(const uint8_t *kd, size_t len, struct rf_2a4m1_pmf_oci *oci)
{
	size_t blen = 0;
	size_t p = rf_2a4m1_kde_find(kd, len, RF_2A4M1_RSN_OUI, RF_2A4M1_PMF_OCI_KDE_DATATYPE, 4 + RF_2A4M1_PMF_OCI_LEN, &blen);
	if (p == (size_t)-1 || blen < RF_2A4M1_PMF_OCI_LEN)
		return false;
	if (oci) {
		oci->op_class  = kd[p];
		oci->prim_chan = kd[p + 1];
		oci->seg1_freq = kd[p + 2];
	}
	return true;
}

bool rf_2a4m1_pmf_ocv_verify(const struct rf_2a4m1_pmf_oci *peer_oci, const struct rf_2a4m1_pmf_oci *local_chan)
{
	if (!peer_oci || !local_chan)
		return false;
	/* §12.2.9: the operating class + primary channel must match. The frequency-segment-1 channel
	 * must match when either side is nonzero (a wide 80+80/160 MHz operating channel). */
	if (peer_oci->op_class != local_chan->op_class)
		return false;
	if (peer_oci->prim_chan != local_chan->prim_chan)
		return false;
	if ((peer_oci->seg1_freq || local_chan->seg1_freq) &&
	    peer_oci->seg1_freq != local_chan->seg1_freq)
		return false;
	return true;
}

size_t rf_2a4m1_pmf_td_kde_build(uint8_t *out, size_t cap, uint8_t bitmap)
{
	/* dd 05 50-6f-9a 08 bitmap: body = OUI(3)+dt(1)+1 = 5. Total 7. */
	if (!out || cap < 7)
		return 0;
	out[0] = 0xdd;
	out[1] = 5;
	out[2] = RF_2A4M1_PMF_WFA_OUI_0; out[3] = RF_2A4M1_PMF_WFA_OUI_1; out[4] = RF_2A4M1_PMF_WFA_OUI_2;
	out[5] = RF_2A4M1_PMF_TD_KDE_DATATYPE;
	out[6] = bitmap;
	return 7;
}

bool rf_2a4m1_pmf_td_kde_find(const uint8_t *kd, size_t len, uint8_t *bitmap)
{
	static const uint8_t rf_2a4m1_wfa[3] = { RF_2A4M1_PMF_WFA_OUI_0, RF_2A4M1_PMF_WFA_OUI_1, RF_2A4M1_PMF_WFA_OUI_2 };
	size_t blen = 0;
	size_t p = rf_2a4m1_kde_find(kd, len, rf_2a4m1_wfa, RF_2A4M1_PMF_TD_KDE_DATATYPE, 5, &blen);
	if (p == (size_t)-1 || blen < 1)
		return false;
	if (bitmap)
		*bitmap = kd[p];
	return true;
}

