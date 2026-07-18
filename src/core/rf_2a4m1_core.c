// SPDX-License-Identifier: GPL-2.0
//
// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
// rf-2a4m1 core
//
// Copyright (c) GenBasic.
// Licensed under the GNU General Public License, version 2.
//
// This file is machine-generated. Do not hand-edit.


#include "rf_2a4m1_core.h"

/* file-scope constants + helper types */
#define RF_2A4M1_MH_FC     0
#define RF_2A4M1_MH_DUR    2
#define RF_2A4M1_MH_ADDR1  4
#define RF_2A4M1_MH_ADDR2  10
#define RF_2A4M1_MH_ADDR3  16
#define RF_2A4M1_MH_SEQ    22

/* file-local forward declarations */
static size_t rf_2a4m1_put_bss_body(uint8_t *b, size_t cap, uint64_t timestamp, uint16_t beacon_int,
                           uint16_t cap_info, const uint8_t *ssid, uint8_t ssid_len,
                           uint8_t channel, const uint8_t *rates, uint8_t n_rates,
                           const uint8_t *rsn, uint8_t rsn_len);
static void rf_2a4m1_put_hdr(uint8_t *f, uint8_t stype, const rf_2a4m1_mac_addr *da,
                    const rf_2a4m1_mac_addr *sa, const rf_2a4m1_mac_addr *bssid, uint16_t seq_ctrl);

bool rf_2a4m1_ie_read_next(struct rf_2a4m1_ie_reader *r, uint8_t *id, const uint8_t **data, uint8_t *dlen)
{
	if (r->pos + 2 > r->len)               /* need at least Element ID + Length */
		return false;
	uint8_t eid  = r->buf[r->pos];
	uint8_t elen = r->buf[r->pos + 1];
	if (r->pos + 2 + (size_t)elen > r->len)
		return false;                  /* declared length overruns -> stop, no over-read */
	*id   = eid;
	*dlen = elen;
	*data = &r->buf[r->pos + 2];
	r->pos += 2 + (size_t)elen;
	return true;
}

bool rf_2a4m1_ie_write(struct rf_2a4m1_ie_writer *w, uint8_t id, const uint8_t *data, uint8_t dlen)
{
	if (w->pos + 2 + (size_t)dlen > w->cap)
		return false;                  /* would overflow -> write nothing */
	w->buf[w->pos++] = id;
	w->buf[w->pos++] = dlen;
	for (uint8_t i = 0; i < dlen; i++)
		w->buf[w->pos++] = data[i];
	return true;
}

bool rf_2a4m1_mgmt_parse_hdr(const uint8_t *f, uint16_t len, struct rf_2a4m1_mgmt_hdr *out)
{
	if (!f || !out || len < RF_2A4M1_MGMT_HDR_LEN)
		return false;
	uint16_t fc = rf_2a4m1_get_le16(f + RF_2A4M1_MH_FC);
	uint8_t  version = (uint8_t)(fc & 0x3);
	uint8_t  type    = (uint8_t)((fc >> 2) & 0x3);
	uint8_t  subtype = (uint8_t)((fc >> 4) & 0xf);
	if (version != 0)               /* §9.2.4.1.2: only version 0 is defined; 1-3 are reserved */
		return false;
	if (type != RF_2A4M1_WLAN_FC_TYPE_MGMT)
		return false;
	out->fc      = fc;
	out->type    = type;
	out->subtype = subtype;
	memcpy(out->addr1.a, f + RF_2A4M1_MH_ADDR1, RF_2A4M1_ETH_ALEN);
	memcpy(out->addr2.a, f + RF_2A4M1_MH_ADDR2, RF_2A4M1_ETH_ALEN);
	memcpy(out->addr3.a, f + RF_2A4M1_MH_ADDR3, RF_2A4M1_ETH_ALEN);
	out->seq_ctrl = rf_2a4m1_get_le16(f + RF_2A4M1_MH_SEQ);
	out->body     = f + RF_2A4M1_MGMT_HDR_LEN;
	out->body_len = (uint16_t)(len - RF_2A4M1_MGMT_HDR_LEN);
	return true;
}

static void rf_2a4m1_put_hdr(uint8_t *f, uint8_t stype, const rf_2a4m1_mac_addr *da,
                    const rf_2a4m1_mac_addr *sa, const rf_2a4m1_mac_addr *bssid, uint16_t seq_ctrl)
{
	uint16_t fc = (uint16_t)((RF_2A4M1_WLAN_FC_TYPE_MGMT << 2) | ((uint16_t)stype << 4));
	rf_2a4m1_put_le16(f + RF_2A4M1_MH_FC, fc);
	rf_2a4m1_put_le16(f + RF_2A4M1_MH_DUR, 0);
	memcpy(f + RF_2A4M1_MH_ADDR1, da->a, RF_2A4M1_ETH_ALEN);
	memcpy(f + RF_2A4M1_MH_ADDR2, sa->a, RF_2A4M1_ETH_ALEN);
	memcpy(f + RF_2A4M1_MH_ADDR3, bssid->a, RF_2A4M1_ETH_ALEN);
	rf_2a4m1_put_le16(f + RF_2A4M1_MH_SEQ, seq_ctrl);
}

size_t rf_2a4m1_mgmt_build_probe_req(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *da,
                            const rf_2a4m1_mac_addr *sa, const rf_2a4m1_mac_addr *bssid, uint16_t seq_ctrl,
                            const uint8_t *ssid, uint8_t ssid_len,
                            const uint8_t *rates, uint8_t n_rates)
{
	if (!out || !da || !sa || !bssid || ssid_len > RF_2A4M1_MGMT_SSID_MAX)
		return 0;
	if (cap < RF_2A4M1_MGMT_HDR_LEN)
		return 0;
	rf_2a4m1_put_hdr(out, RF_2A4M1_WLAN_FC_STYPE_PROBE_REQ, da, sa, bssid, seq_ctrl);

	/* A Probe Request body is pure elements — no fixed fields. The SSID element is mandatory
	 * (a 0-length SSID is the wildcard that every AP answers); Supported Rates follows. */
	struct rf_2a4m1_ie_writer w;
	rf_2a4m1_ie_writer_init(&w, out + RF_2A4M1_MGMT_HDR_LEN, cap - RF_2A4M1_MGMT_HDR_LEN);
	if (!rf_2a4m1_ie_write(&w, RF_2A4M1_WLAN_EID_SSID, ssid, ssid_len))
		return 0;
	if (n_rates && !rf_2a4m1_ie_write(&w, RF_2A4M1_WLAN_EID_SUPP_RATES, rates, n_rates))
		return 0;
	return RF_2A4M1_MGMT_HDR_LEN + rf_2a4m1_ie_writer_len(&w);
}

size_t rf_2a4m1_mgmt_build_auth_open(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *bssid,
                            const rf_2a4m1_mac_addr *sa, uint16_t seq_ctrl)
{
	if (!out || !bssid || !sa || cap < RF_2A4M1_MGMT_HDR_LEN + 6)
		return 0;
	rf_2a4m1_put_hdr(out, RF_2A4M1_WLAN_FC_STYPE_AUTH, bssid, sa, bssid, seq_ctrl);
	uint8_t *b = out + RF_2A4M1_MGMT_HDR_LEN;
	rf_2a4m1_put_le16(b + 0, RF_2A4M1_WLAN_AUTH_OPEN);   /* Authentication Algorithm Number = 0 (Open System) */
	rf_2a4m1_put_le16(b + 2, 1);                /* Authentication Transaction Sequence Number = 1     */
	rf_2a4m1_put_le16(b + 4, 0);                /* Status Code = 0 (reserved in a request)            */
	return RF_2A4M1_MGMT_HDR_LEN + 6;
}

size_t rf_2a4m1_mgmt_build_auth_sae_commit(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *bssid,
                                  const rf_2a4m1_mac_addr *sa, uint16_t seq_ctrl, uint16_t status,
                                  uint16_t group,
                                  const uint8_t *scalar, uint8_t scalar_len,
                                  const uint8_t *element, uint8_t element_len)
{
	size_t need = (size_t)RF_2A4M1_MGMT_HDR_LEN + 6 + 2 + scalar_len + element_len;
	if (!out || !bssid || !sa || !scalar || !element || cap < need)
		return 0;
	rf_2a4m1_put_hdr(out, RF_2A4M1_WLAN_FC_STYPE_AUTH, bssid, sa, bssid, seq_ctrl);
	uint8_t *b = out + RF_2A4M1_MGMT_HDR_LEN;
	rf_2a4m1_put_le16(b + 0, RF_2A4M1_WLAN_AUTH_SAE);   /* Authentication Algorithm Number = 3 (SAE) */
	rf_2a4m1_put_le16(b + 2, 1);               /* Authentication Transaction Sequence Number = 1 */
	rf_2a4m1_put_le16(b + 4, status);          /* 0 (H&P) or 126 (H2E) */
	rf_2a4m1_put_le16(b + 6, group);           /* Finite Cyclic Group */
	memcpy(b + 8, scalar, scalar_len);
	memcpy(b + 8 + scalar_len, element, element_len);
	return need;
}

size_t rf_2a4m1_mgmt_build_auth_sae_confirm(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *bssid,
                                   const rf_2a4m1_mac_addr *sa, uint16_t seq_ctrl,
                                   uint16_t send_confirm,
                                   const uint8_t *confirm, uint8_t confirm_len)
{
	size_t need = (size_t)RF_2A4M1_MGMT_HDR_LEN + 6 + 2 + confirm_len;
	if (!out || !bssid || !sa || !confirm || cap < need)
		return 0;
	rf_2a4m1_put_hdr(out, RF_2A4M1_WLAN_FC_STYPE_AUTH, bssid, sa, bssid, seq_ctrl);
	uint8_t *b = out + RF_2A4M1_MGMT_HDR_LEN;
	rf_2a4m1_put_le16(b + 0, RF_2A4M1_WLAN_AUTH_SAE);   /* algo = 3 (SAE) */
	rf_2a4m1_put_le16(b + 2, 2);               /* txn seq = 2 (Confirm) */
	rf_2a4m1_put_le16(b + 4, 0);               /* Status Code = 0 */
	rf_2a4m1_put_le16(b + 6, send_confirm);    /* Send-Confirm counter (LE) */
	memcpy(b + 8, confirm, confirm_len);
	return need;
}

size_t rf_2a4m1_mgmt_build_assoc_req(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *bssid,
                            const rf_2a4m1_mac_addr *sa, uint16_t seq_ctrl,
                            uint16_t cap_info, uint16_t listen_interval,
                            const uint8_t *ssid, uint8_t ssid_len,
                            const uint8_t *rates, uint8_t n_rates,
                            const uint8_t *rsn, uint8_t rsn_len,
                            const uint8_t *extra_ies, uint8_t extra_ies_len)
{
	if (!out || !bssid || !sa || ssid_len > RF_2A4M1_MGMT_SSID_MAX)
		return 0;
	if (cap < RF_2A4M1_MGMT_HDR_LEN + 4)
		return 0;
	rf_2a4m1_put_hdr(out, RF_2A4M1_WLAN_FC_STYPE_ASSOC_REQ, bssid, sa, bssid, seq_ctrl);
	uint8_t *b = out + RF_2A4M1_MGMT_HDR_LEN;
	rf_2a4m1_put_le16(b + 0, cap_info);
	rf_2a4m1_put_le16(b + 2, listen_interval);

	struct rf_2a4m1_ie_writer w;
	rf_2a4m1_ie_writer_init(&w, b + 4, cap - (RF_2A4M1_MGMT_HDR_LEN + 4));
	if (!rf_2a4m1_ie_write(&w, RF_2A4M1_WLAN_EID_SSID, ssid, ssid_len))
		return 0;
	if (n_rates && !rf_2a4m1_ie_write(&w, RF_2A4M1_WLAN_EID_SUPP_RATES, rates, n_rates))
		return 0;
	if (rsn && rsn_len && !rf_2a4m1_ie_write(&w, RF_2A4M1_WLAN_EID_RSN, rsn, rsn_len))
		return 0;
	size_t total = RF_2A4M1_MGMT_HDR_LEN + 4 + rf_2a4m1_ie_writer_len(&w);
	if (extra_ies && extra_ies_len) {   /* verbatim FULL elements (e.g. the RSNXE) */
		if (total + extra_ies_len > cap)
			return 0;
		memcpy(out + total, extra_ies, extra_ies_len);
		total += extra_ies_len;
	}
	return total;
}

bool rf_2a4m1_mgmt_parse_auth(const uint8_t *body, uint16_t blen,
                     uint16_t *algo, uint16_t *txn, uint16_t *status)
{
	if (!body || blen < 6)
		return false;
	if (algo)   *algo   = rf_2a4m1_get_le16(body + 0);
	if (txn)    *txn    = rf_2a4m1_get_le16(body + 2);
	if (status) *status = rf_2a4m1_get_le16(body + 4);
	return true;
}

bool rf_2a4m1_mgmt_parse_auth_sae_commit(const uint8_t *body, uint16_t blen, uint16_t *group,
                                const uint8_t **scalar, const uint8_t **element)
{
	/* algo(2) txn(2) status(2) group(2) scalar(32) element(64) — group 19 sizes. */
	if (!body || blen < 6 + 2 + 32 + 64)
		return false;
	if (group)   *group   = rf_2a4m1_get_le16(body + 6);
	if (scalar)  *scalar  = body + 8;
	if (element) *element = body + 8 + 32;
	return true;
}

bool rf_2a4m1_mgmt_parse_auth_sae_confirm(const uint8_t *body, uint16_t blen, uint16_t *send_confirm,
                                 const uint8_t **confirm)
{
	/* algo(2) txn(2) status(2) send-confirm(2) confirm(32). */
	if (!body || blen < 6 + 2 + 32)
		return false;
	if (send_confirm) *send_confirm = rf_2a4m1_get_le16(body + 6);
	if (confirm)      *confirm      = body + 8;
	return true;
}

bool rf_2a4m1_mgmt_parse_assoc_resp(const uint8_t *body, uint16_t blen,
                           uint16_t *cap, uint16_t *status, uint16_t *aid)
{
	if (!body || blen < 6)
		return false;
	if (cap)    *cap    = rf_2a4m1_get_le16(body + 0);
	if (status) *status = rf_2a4m1_get_le16(body + 2);
	if (aid)    *aid    = (uint16_t)(rf_2a4m1_get_le16(body + 4) & 0x3fff);  /* mask the 2 reserved MSBs */
	return true;
}

static size_t rf_2a4m1_put_bss_body(uint8_t *b, size_t cap, uint64_t timestamp, uint16_t beacon_int,
                           uint16_t cap_info, const uint8_t *ssid, uint8_t ssid_len,
                           uint8_t channel, const uint8_t *rates, uint8_t n_rates,
                           const uint8_t *rsn, uint8_t rsn_len)
{
	if (ssid_len > RF_2A4M1_MGMT_SSID_MAX || cap < 12)
		return 0;
	rf_2a4m1_put_le64(b + 0, timestamp);
	rf_2a4m1_put_le16(b + 8, beacon_int);
	rf_2a4m1_put_le16(b + 10, cap_info);

	struct rf_2a4m1_ie_writer w;
	rf_2a4m1_ie_writer_init(&w, b + 12, cap - 12);
	if (!rf_2a4m1_ie_write(&w, RF_2A4M1_WLAN_EID_SSID, ssid, ssid_len))
		return 0;
	if (n_rates && !rf_2a4m1_ie_write(&w, RF_2A4M1_WLAN_EID_SUPP_RATES, rates, n_rates))
		return 0;
	uint8_t ds = channel;
	if (!rf_2a4m1_ie_write(&w, RF_2A4M1_WLAN_EID_DS_PARAMS, &ds, 1))
		return 0;
	/* TIM: DTIM count 0, DTIM period 1, bitmap-control 0, 1-octet partial virtual bitmap 0. */
	static const uint8_t rf_2a4m1_tim[4] = { 0x00, 0x01, 0x00, 0x00 };
	if (!rf_2a4m1_ie_write(&w, 5 /* WLAN_EID_TIM */, rf_2a4m1_tim, sizeof rf_2a4m1_tim))
		return 0;
	if (rsn && rsn_len && !rf_2a4m1_ie_write(&w, RF_2A4M1_WLAN_EID_RSN, rsn, rsn_len))
		return 0;
	return 12 + rf_2a4m1_ie_writer_len(&w);
}

size_t rf_2a4m1_mgmt_build_beacon(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *bssid,
                         uint16_t seq_ctrl, uint64_t timestamp, uint16_t beacon_int,
                         uint16_t cap_info, const uint8_t *ssid, uint8_t ssid_len,
                         uint8_t channel, const uint8_t *rates, uint8_t n_rates,
                         const uint8_t *rsn, uint8_t rsn_len)
{
	static const rf_2a4m1_mac_addr rf_2a4m1_bcast = { { 0xff,0xff,0xff,0xff,0xff,0xff } };
	if (!out || !bssid || cap < RF_2A4M1_MGMT_HDR_LEN + 12)
		return 0;
	rf_2a4m1_put_hdr(out, RF_2A4M1_WLAN_FC_STYPE_BEACON, &rf_2a4m1_bcast, bssid /*SA*/, bssid /*BSSID*/, seq_ctrl);
	size_t bl = rf_2a4m1_put_bss_body(out + RF_2A4M1_MGMT_HDR_LEN, cap - RF_2A4M1_MGMT_HDR_LEN, timestamp, beacon_int,
	                         cap_info, ssid, ssid_len, channel, rates, n_rates, rsn, rsn_len);
	if (!bl)
		return 0;
	return RF_2A4M1_MGMT_HDR_LEN + bl;
}

size_t rf_2a4m1_mgmt_build_probe_resp(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *da,
                             const rf_2a4m1_mac_addr *bssid, uint16_t seq_ctrl, uint64_t timestamp,
                             uint16_t beacon_int, uint16_t cap_info,
                             const uint8_t *ssid, uint8_t ssid_len, uint8_t channel,
                             const uint8_t *rates, uint8_t n_rates,
                             const uint8_t *rsn, uint8_t rsn_len)
{
	if (!out || !da || !bssid || cap < RF_2A4M1_MGMT_HDR_LEN + 12)
		return 0;
	rf_2a4m1_put_hdr(out, RF_2A4M1_WLAN_FC_STYPE_PROBE_RESP, da, bssid /*SA*/, bssid /*BSSID*/, seq_ctrl);
	size_t bl = rf_2a4m1_put_bss_body(out + RF_2A4M1_MGMT_HDR_LEN, cap - RF_2A4M1_MGMT_HDR_LEN, timestamp, beacon_int,
	                         cap_info, ssid, ssid_len, channel, rates, n_rates, rsn, rsn_len);
	if (!bl)
		return 0;
	return RF_2A4M1_MGMT_HDR_LEN + bl;
}

size_t rf_2a4m1_mgmt_build_auth_resp(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *da,
                            const rf_2a4m1_mac_addr *bssid, uint16_t seq_ctrl,
                            uint16_t algo, uint16_t status)
{
	if (!out || !da || !bssid || cap < RF_2A4M1_MGMT_HDR_LEN + 6)
		return 0;
	rf_2a4m1_put_hdr(out, RF_2A4M1_WLAN_FC_STYPE_AUTH, da, bssid /*SA*/, bssid /*BSSID*/, seq_ctrl);
	uint8_t *b = out + RF_2A4M1_MGMT_HDR_LEN;
	rf_2a4m1_put_le16(b + 0, algo);    /* Authentication Algorithm Number (echo; 0 = Open System) */
	rf_2a4m1_put_le16(b + 2, 2);       /* Authentication Transaction Sequence Number = 2 (response) */
	rf_2a4m1_put_le16(b + 4, status);  /* Status Code (0 = success) */
	return RF_2A4M1_MGMT_HDR_LEN + 6;
}

size_t rf_2a4m1_mgmt_build_assoc_resp(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *da,
                             const rf_2a4m1_mac_addr *bssid, uint16_t seq_ctrl, uint16_t cap_info,
                             uint16_t status, uint16_t aid,
                             const uint8_t *rates, uint8_t n_rates,
                             const uint8_t *extra_ies, uint8_t extra_ies_len)
{
	if (!out || !da || !bssid || cap < RF_2A4M1_MGMT_HDR_LEN + 6)
		return 0;
	rf_2a4m1_put_hdr(out, RF_2A4M1_WLAN_FC_STYPE_ASSOC_RESP, da, bssid /*SA*/, bssid /*BSSID*/, seq_ctrl);
	uint8_t *b = out + RF_2A4M1_MGMT_HDR_LEN;
	rf_2a4m1_put_le16(b + 0, cap_info);
	rf_2a4m1_put_le16(b + 2, status);
	rf_2a4m1_put_le16(b + 4, (uint16_t)(0xc000 | (aid & 0x3fff)));  /* two reserved MSBs set on the wire */

	struct rf_2a4m1_ie_writer w;
	rf_2a4m1_ie_writer_init(&w, b + 6, cap - (RF_2A4M1_MGMT_HDR_LEN + 6));
	if (n_rates && !rf_2a4m1_ie_write(&w, RF_2A4M1_WLAN_EID_SUPP_RATES, rates, n_rates))
		return 0;
	size_t total = RF_2A4M1_MGMT_HDR_LEN + 6 + rf_2a4m1_ie_writer_len(&w);
	if (extra_ies && extra_ies_len) {   /* verbatim FULL elements (WMM Param / RSNE / gen Oper) */
		if (total + extra_ies_len > cap)
			return 0;
		memcpy(out + total, extra_ies, extra_ies_len);
		total += extra_ies_len;
	}
	return total;
}

bool rf_2a4m1_mgmt_parse_beacon(const uint8_t *body, uint16_t blen, struct rf_2a4m1_beacon_info *bi)
{
	if (!body || !bi || blen < 12)     /* Timestamp(8) + Beacon Interval(2) + Capability(2) */
		return false;
	memset(bi, 0, sizeof *bi);
	bi->cap = rf_2a4m1_get_le16(body + 10);

	struct rf_2a4m1_ie_reader r;
	rf_2a4m1_ie_reader_init(&r, body + 12, (size_t)blen - 12);
	uint8_t id, dlen;
	const uint8_t *data;
	while (rf_2a4m1_ie_read_next(&r, &id, &data, &dlen)) {
		switch (id) {
		case RF_2A4M1_WLAN_EID_SSID:
			if (dlen <= RF_2A4M1_MGMT_SSID_MAX) {
				bi->ssid_len = dlen;
				if (dlen)
					memcpy(bi->ssid, data, dlen);
			}
			break;
		case RF_2A4M1_WLAN_EID_DS_PARAMS:
			if (dlen >= 1)
				bi->channel = data[0];
			break;
		case RF_2A4M1_WLAN_EID_RSN:
			bi->has_rsn = true;
			break;
		default:
			break;
		}
	}
	return true;
}

