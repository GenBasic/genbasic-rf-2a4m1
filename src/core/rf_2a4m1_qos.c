// SPDX-License-Identifier: GPL-2.0
//
// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
// 802.11e QoS/WMM (EDCA, UP->AC, TSPEC admission control)
//
// Copyright (c) GenBasic.
// Licensed under the GNU General Public License, version 2.
//
// This file is machine-generated. Do not hand-edit.


#include "rf_2a4m1_core.h"

/* file-scope constants + helper types */
#define RF_2A4M1_AC_REC_LEN 4
struct rf_2a4m1_ac_default { uint8_t aifsn, ecwmin, ecwmax; uint16_t txop; };

/* file-local forward declarations */
static bool rf_2a4m1_wmm_hdr_ok(const uint8_t *body, uint8_t dlen, uint8_t subtype, uint8_t min_len);
static struct rf_2a4m1_ts_admission *rf_2a4m1_adm_find(struct rf_2a4m1_qos_admission *a, uint8_t tsid);
static void rf_2a4m1_ac_rec_get(const uint8_t *b, struct rf_2a4m1_edca_ac_rec *r);
static void rf_2a4m1_ac_rec_put(uint8_t *b, const struct rf_2a4m1_edca_ac_rec *r);
static void rf_2a4m1_tspec_body_get(const uint8_t *b, struct rf_2a4m1_tspec *t);
static void rf_2a4m1_tspec_body_put(uint8_t *b, const struct rf_2a4m1_tspec *t);

uint32_t rf_2a4m1_ts_info_pack(const struct rf_2a4m1_ts_info *i)
{
	uint32_t v = i->traffic_type ? 1u : 0u;
	v |= (uint32_t)(i->tsid & 0x0f) << 1;
	v |= (uint32_t)(i->direction & 0x03) << 5;
	v |= (uint32_t)(i->access_policy & 0x03) << 7;
	if (i->aggregation) v |= 1u << 9;
	if (i->apsd)        v |= 1u << 10;
	v |= (uint32_t)(i->user_priority & 0x07) << 11;
	v |= (uint32_t)(i->ack_policy & 0x03) << 14;
	if (i->schedule)    v |= 1u << 16;
	return v & 0x00ffffffu;
}

void rf_2a4m1_ts_info_unpack(uint32_t v, struct rf_2a4m1_ts_info *out)
{
	out->traffic_type  = (v & 0x1u) != 0;
	out->tsid          = (uint8_t)((v >> 1) & 0x0f);
	out->direction     = (uint8_t)((v >> 5) & 0x03);
	out->access_policy = (uint8_t)((v >> 7) & 0x03);
	out->aggregation   = ((v >> 9) & 0x1u) != 0;
	out->apsd          = ((v >> 10) & 0x1u) != 0;
	out->user_priority = (uint8_t)((v >> 11) & 0x07);
	out->ack_policy    = (uint8_t)((v >> 14) & 0x03);
	out->schedule      = ((v >> 16) & 0x1u) != 0;
}

static void rf_2a4m1_tspec_body_put(uint8_t *b, const struct rf_2a4m1_tspec *t)
{
	uint32_t ti = rf_2a4m1_ts_info_pack(&t->ts);
	b[0] = (uint8_t)ti; b[1] = (uint8_t)(ti >> 8); b[2] = (uint8_t)(ti >> 16);
	b += 3;
	rf_2a4m1_qos_put_le16(b, t->nominal_msdu);  b += 2;
	rf_2a4m1_qos_put_le16(b, t->max_msdu);      b += 2;
	rf_2a4m1_qos_put_le32(b, t->min_si);        b += 4;
	rf_2a4m1_qos_put_le32(b, t->max_si);        b += 4;
	rf_2a4m1_qos_put_le32(b, t->inactivity);    b += 4;
	rf_2a4m1_qos_put_le32(b, t->suspension);    b += 4;
	rf_2a4m1_qos_put_le32(b, t->service_start); b += 4;
	rf_2a4m1_qos_put_le32(b, t->min_rate);      b += 4;
	rf_2a4m1_qos_put_le32(b, t->mean_rate);     b += 4;
	rf_2a4m1_qos_put_le32(b, t->peak_rate);     b += 4;
	rf_2a4m1_qos_put_le32(b, t->burst_size);    b += 4;
	rf_2a4m1_qos_put_le32(b, t->delay_bound);   b += 4;
	rf_2a4m1_qos_put_le32(b, t->min_phy_rate);  b += 4;
	rf_2a4m1_qos_put_le16(b, t->surplus_bw);    b += 2;
	rf_2a4m1_qos_put_le16(b, t->medium_time);
}

static void rf_2a4m1_tspec_body_get(const uint8_t *b, struct rf_2a4m1_tspec *t)
{
	uint32_t ti = (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16);
	rf_2a4m1_ts_info_unpack(ti, &t->ts);
	b += 3;
	t->nominal_msdu = rf_2a4m1_qos_get_le16(b); b += 2;
	t->max_msdu     = rf_2a4m1_qos_get_le16(b); b += 2;
	t->min_si       = rf_2a4m1_qos_get_le32(b); b += 4;
	t->max_si       = rf_2a4m1_qos_get_le32(b); b += 4;
	t->inactivity   = rf_2a4m1_qos_get_le32(b); b += 4;
	t->suspension   = rf_2a4m1_qos_get_le32(b); b += 4;
	t->service_start= rf_2a4m1_qos_get_le32(b); b += 4;
	t->min_rate     = rf_2a4m1_qos_get_le32(b); b += 4;
	t->mean_rate    = rf_2a4m1_qos_get_le32(b); b += 4;
	t->peak_rate    = rf_2a4m1_qos_get_le32(b); b += 4;
	t->burst_size   = rf_2a4m1_qos_get_le32(b); b += 4;
	t->delay_bound  = rf_2a4m1_qos_get_le32(b); b += 4;
	t->min_phy_rate = rf_2a4m1_qos_get_le32(b); b += 4;
	t->surplus_bw   = rf_2a4m1_qos_get_le16(b); b += 2;
	t->medium_time  = rf_2a4m1_qos_get_le16(b);
}

size_t rf_2a4m1_tspec_build(uint8_t *out, size_t cap, const struct rf_2a4m1_tspec *t)
{
	if (!out || !t || cap < 2u + RF_2A4M1_TSPEC_BODY_LEN)
		return 0;
	out[0] = RF_2A4M1_WLAN_EID_TSPEC;
	out[1] = RF_2A4M1_TSPEC_BODY_LEN;
	rf_2a4m1_tspec_body_put(out + 2, t);
	return 2u + RF_2A4M1_TSPEC_BODY_LEN;
}

bool rf_2a4m1_tspec_parse(const uint8_t *body, uint8_t dlen, struct rf_2a4m1_tspec *out)
{
	if (!body || !out || dlen != RF_2A4M1_TSPEC_BODY_LEN)
		return false;
	rf_2a4m1_tspec_body_get(body, out);
	return true;
}

size_t rf_2a4m1_addts_req_build(uint8_t *out, size_t cap, uint8_t dialog, const struct rf_2a4m1_tspec *t)
{
	if (!out || !t || cap < 3u + 2u + RF_2A4M1_TSPEC_BODY_LEN)
		return 0;
	out[0] = RF_2A4M1_WLAN_ACTION_CAT_QOS;
	out[1] = RF_2A4M1_QOS_ACTION_ADDTS_REQ;
	out[2] = dialog;
	size_t n = rf_2a4m1_tspec_build(out + 3, cap - 3, t);
	if (n == 0)
		return 0;
	return 3 + n;
}

bool rf_2a4m1_addts_req_parse(const uint8_t *in, size_t len, uint8_t *dialog, struct rf_2a4m1_tspec *out)
{
	if (!in || !out || len < 3u + 2u + RF_2A4M1_TSPEC_BODY_LEN)
		return false;
	if (in[0] != RF_2A4M1_WLAN_ACTION_CAT_QOS || in[1] != RF_2A4M1_QOS_ACTION_ADDTS_REQ)
		return false;
	if (in[3] != RF_2A4M1_WLAN_EID_TSPEC || in[4] != RF_2A4M1_TSPEC_BODY_LEN)
		return false;
	if (dialog)
		*dialog = in[2];
	return rf_2a4m1_tspec_parse(in + 5, in[4], out);
}

size_t rf_2a4m1_addts_rsp_build(uint8_t *out, size_t cap, uint8_t dialog, uint16_t status,
                       const struct rf_2a4m1_tspec *t)
{
	if (!out || !t || cap < 5u + 2u + RF_2A4M1_TSPEC_BODY_LEN)
		return 0;
	out[0] = RF_2A4M1_WLAN_ACTION_CAT_QOS;
	out[1] = RF_2A4M1_QOS_ACTION_ADDTS_RSP;
	out[2] = dialog;
	rf_2a4m1_qos_put_le16(out + 3, status);
	size_t n = rf_2a4m1_tspec_build(out + 5, cap - 5, t);
	if (n == 0)
		return 0;
	return 5 + n;
}

bool rf_2a4m1_addts_rsp_parse(const uint8_t *in, size_t len, uint8_t *dialog, uint16_t *status,
                     struct rf_2a4m1_tspec *out)
{
	if (!in || !out || len < 5u + 2u + RF_2A4M1_TSPEC_BODY_LEN)
		return false;
	if (in[0] != RF_2A4M1_WLAN_ACTION_CAT_QOS || in[1] != RF_2A4M1_QOS_ACTION_ADDTS_RSP)
		return false;
	if (in[5] != RF_2A4M1_WLAN_EID_TSPEC || in[6] != RF_2A4M1_TSPEC_BODY_LEN)
		return false;
	if (dialog)
		*dialog = in[2];
	if (status)
		*status = rf_2a4m1_qos_get_le16(in + 3);
	return rf_2a4m1_tspec_parse(in + 7, in[6], out);
}

size_t rf_2a4m1_delts_build(uint8_t *out, size_t cap, const struct rf_2a4m1_ts_info *ts, uint16_t reason)
{
	if (!out || !ts || cap < 7)
		return 0;
	out[0] = RF_2A4M1_WLAN_ACTION_CAT_QOS;
	out[1] = RF_2A4M1_QOS_ACTION_DELTS;
	uint32_t ti = rf_2a4m1_ts_info_pack(ts);
	out[2] = (uint8_t)ti; out[3] = (uint8_t)(ti >> 8); out[4] = (uint8_t)(ti >> 16);
	rf_2a4m1_qos_put_le16(out + 5, reason);
	return 7;
}

bool rf_2a4m1_delts_parse(const uint8_t *in, size_t len, struct rf_2a4m1_ts_info *ts, uint16_t *reason)
{
	if (!in || !ts || len < 7)
		return false;
	if (in[0] != RF_2A4M1_WLAN_ACTION_CAT_QOS || in[1] != RF_2A4M1_QOS_ACTION_DELTS)
		return false;
	uint32_t ti = (uint32_t)in[2] | ((uint32_t)in[3] << 8) | ((uint32_t)in[4] << 16);
	rf_2a4m1_ts_info_unpack(ti, ts);
	if (reason)
		*reason = rf_2a4m1_qos_get_le16(in + 5);
	return true;
}

void rf_2a4m1_qos_admission_init(struct rf_2a4m1_qos_admission *a)
{
	if (!a)
		return;
	for (int i = 0; i < RF_2A4M1_QOS_ADM_MAX_TS; i++) {
		a->ts[i].in_use      = false;
		a->ts[i].tsid        = 0;
		a->ts[i].ac          = 0;
		a->ts[i].direction   = 0;
		a->ts[i].medium_time = 0;
	}
	for (int ac = 0; ac < RF_2A4M1_QOS_AC_COUNT; ac++) {
		a->ac_total[ac] = 0;
		a->ac_limit[ac] = RF_2A4M1_QOS_ADM_DEFAULT_AC_LIMIT;
	}
}

void rf_2a4m1_qos_admission_set_limit(struct rf_2a4m1_qos_admission *a, uint8_t ac, uint32_t limit)
{
	if (!a || ac >= RF_2A4M1_QOS_AC_COUNT)
		return;
	a->ac_limit[ac] = limit;
}

static struct rf_2a4m1_ts_admission *rf_2a4m1_adm_find(struct rf_2a4m1_qos_admission *a, uint8_t tsid)
{
	for (int i = 0; i < RF_2A4M1_QOS_ADM_MAX_TS; i++)
		if (a->ts[i].in_use && a->ts[i].tsid == tsid)
			return &a->ts[i];
	return 0;
}

enum rf_2a4m1_qos_admit_result rf_2a4m1_qos_admission_admit(struct rf_2a4m1_qos_admission *a, uint8_t tsid, uint8_t ac,
                                          uint8_t direction, uint16_t medium_time)
{
	if (!a || tsid > 0x0fu || ac >= RF_2A4M1_QOS_AC_COUNT)
		return RF_2A4M1_QOS_ADMIT_REFUSED_ARG;
	if (rf_2a4m1_adm_find(a, tsid))
		return RF_2A4M1_QOS_ADMIT_REFUSED_DUP;          /* double-add: reject, never double-count */

	/* Overflow guard, subtraction form: room = limit - total, compared against `room` (never
	 * total + medium_time) so it cannot wrap upward. The subtraction itself must be guarded:
	 * qos_admission_set_limit() can lower an AC ceiling BELOW the already-admitted total (it
	 * does not evict streams), so total <= limit is NOT an invariant here — clamp to 0 (refuse)
	 * rather than let limit - total underflow to a huge room and wrongly admit an over-budget
	 * stream. Mirrors qos_admission_ac_remaining()'s guard. */
	uint32_t room = (a->ac_limit[ac] >= a->ac_total[ac])
	                    ? a->ac_limit[ac] - a->ac_total[ac]
	                    : 0u;
	if ((uint32_t)medium_time > room)
		return RF_2A4M1_QOS_ADMIT_REFUSED_LIMIT;

	struct rf_2a4m1_ts_admission *slot = 0;
	for (int i = 0; i < RF_2A4M1_QOS_ADM_MAX_TS; i++) {
		if (!a->ts[i].in_use) { slot = &a->ts[i]; break; }
	}
	if (!slot)
		return RF_2A4M1_QOS_ADMIT_REFUSED_FULL;

	slot->in_use      = true;
	slot->tsid        = tsid;
	slot->ac          = ac;
	slot->direction   = direction;
	slot->medium_time = medium_time;
	a->ac_total[ac] += medium_time;
	return RF_2A4M1_QOS_ADMIT_OK;
}

enum rf_2a4m1_qos_admit_result rf_2a4m1_qos_admission_admit_tspec(struct rf_2a4m1_qos_admission *a, const struct rf_2a4m1_tspec *granted)
{
	if (!a || !granted)
		return RF_2A4M1_QOS_ADMIT_REFUSED_ARG;
	uint8_t ac = rf_2a4m1_qos_up_to_ac(granted->ts.user_priority);
	return rf_2a4m1_qos_admission_admit(a, granted->ts.tsid, ac, granted->ts.direction,
	                           granted->medium_time);
}

bool rf_2a4m1_qos_admission_release(struct rf_2a4m1_qos_admission *a, uint8_t tsid)
{
	if (!a)
		return false;
	struct rf_2a4m1_ts_admission *e = rf_2a4m1_adm_find(a, tsid);
	if (!e)
		return false;                          /* delete-without-add: no-op, no underflow */
	uint8_t ac = e->ac & 0x03u;
	/* The admit-time invariant guarantees total >= this stream's medium_time; clamp anyway
	 * so a corrupted record can never underflow the AC total. */
	if (a->ac_total[ac] >= e->medium_time)
		a->ac_total[ac] -= e->medium_time;
	else
		a->ac_total[ac] = 0;
	e->in_use      = false;
	e->medium_time = 0;
	return true;
}

bool rf_2a4m1_qos_admission_release_ts(struct rf_2a4m1_qos_admission *a, const struct rf_2a4m1_ts_info *ts)
{
	if (!a || !ts)
		return false;
	return rf_2a4m1_qos_admission_release(a, ts->tsid);
}

uint16_t rf_2a4m1_qos_ctrl_pack(const struct rf_2a4m1_qos_ctrl *q)
{
	uint16_t v = (uint16_t)(q->tid & 0x0f);
	if (q->eosp)
		v |= 0x0010;
	v |= (uint16_t)((q->ack_policy & 0x03) << 5);
	if (q->amsdu)
		v |= 0x0080;
	v |= (uint16_t)((uint16_t)q->hi << 8);
	return v;
}

void rf_2a4m1_qos_ctrl_unpack(uint16_t v, struct rf_2a4m1_qos_ctrl *out)
{
	out->tid        = (uint8_t)(v & 0x0f);
	out->eosp       = (v & 0x0010) != 0;
	out->ack_policy = (uint8_t)((v >> 5) & 0x03);
	out->amsdu      = (v & 0x0080) != 0;
	out->hi         = (uint8_t)(v >> 8);
}

size_t rf_2a4m1_qos_ctrl_build(uint8_t *out, size_t cap, const struct rf_2a4m1_qos_ctrl *q)
{
	if (!out || !q || cap < 2)
		return 0;
	rf_2a4m1_qos_put_le16(out, rf_2a4m1_qos_ctrl_pack(q));
	return 2;
}

bool rf_2a4m1_qos_ctrl_parse(const uint8_t *in, size_t len, struct rf_2a4m1_qos_ctrl *out)
{
	if (!in || !out || len < 2)
		return false;
	rf_2a4m1_qos_ctrl_unpack(rf_2a4m1_qos_get_le16(in), out);
	return true;
}

static void rf_2a4m1_ac_rec_put(uint8_t *b, const struct rf_2a4m1_edca_ac_rec *r)
{
	b[0] = r->aci_aifsn;
	b[1] = r->ecw;
	rf_2a4m1_qos_put_le16(b + 2, r->txop_limit);
}

static void rf_2a4m1_ac_rec_get(const uint8_t *b, struct rf_2a4m1_edca_ac_rec *r)
{
	r->aci_aifsn  = b[0];
	r->ecw        = b[1];
	r->txop_limit = rf_2a4m1_qos_get_le16(b + 2);
}

size_t rf_2a4m1_wmm_param_build(uint8_t *out, size_t cap, const struct rf_2a4m1_edca_param *p)
{
	if (!out || !p || cap < 2u + RF_2A4M1_WMM_PARAM_BODY_LEN)
		return 0;
	uint8_t *b = out;
	*b++ = RF_2A4M1_WLAN_EID_VENDOR_SPECIFIC;
	*b++ = RF_2A4M1_WMM_PARAM_BODY_LEN;
	*b++ = RF_2A4M1_WMM_OUI_0; *b++ = RF_2A4M1_WMM_OUI_1; *b++ = RF_2A4M1_WMM_OUI_2;
	*b++ = RF_2A4M1_WMM_OUI_TYPE;
	*b++ = RF_2A4M1_WMM_OUI_SUBTYPE_PARAM;
	*b++ = RF_2A4M1_WMM_VERSION;
	*b++ = p->qos_info;
	*b++ = 0;                         /* Reserved */
	for (int i = 0; i < RF_2A4M1_QOS_AC_COUNT; i++) {
		rf_2a4m1_ac_rec_put(b, &p->ac[i]);
		b += RF_2A4M1_AC_REC_LEN;
	}
	return 2u + RF_2A4M1_WMM_PARAM_BODY_LEN;
}

static bool rf_2a4m1_wmm_hdr_ok(const uint8_t *body, uint8_t dlen, uint8_t subtype, uint8_t min_len)
{
	if (!body || dlen < min_len)
		return false;
	return body[0] == RF_2A4M1_WMM_OUI_0 && body[1] == RF_2A4M1_WMM_OUI_1 && body[2] == RF_2A4M1_WMM_OUI_2 &&
	       body[3] == RF_2A4M1_WMM_OUI_TYPE && body[4] == subtype && body[5] == RF_2A4M1_WMM_VERSION;
}

bool rf_2a4m1_wmm_param_parse(const uint8_t *body, uint8_t dlen, struct rf_2a4m1_edca_param *out)
{
	if (!out || dlen != RF_2A4M1_WMM_PARAM_BODY_LEN ||
	    !rf_2a4m1_wmm_hdr_ok(body, dlen, RF_2A4M1_WMM_OUI_SUBTYPE_PARAM, RF_2A4M1_WMM_PARAM_BODY_LEN))
		return false;
	out->qos_info = body[6];
	/* body[7] = Reserved */
	const uint8_t *r = body + 8;
	for (int i = 0; i < RF_2A4M1_QOS_AC_COUNT; i++) {
		rf_2a4m1_ac_rec_get(r, &out->ac[i]);
		r += RF_2A4M1_AC_REC_LEN;
	}
	return true;
}

size_t rf_2a4m1_wmm_info_build(uint8_t *out, size_t cap, uint8_t qos_info)
{
	if (!out || cap < 2u + RF_2A4M1_WMM_INFO_BODY_LEN)
		return 0;
	uint8_t *b = out;
	*b++ = RF_2A4M1_WLAN_EID_VENDOR_SPECIFIC;
	*b++ = RF_2A4M1_WMM_INFO_BODY_LEN;
	*b++ = RF_2A4M1_WMM_OUI_0; *b++ = RF_2A4M1_WMM_OUI_1; *b++ = RF_2A4M1_WMM_OUI_2;
	*b++ = RF_2A4M1_WMM_OUI_TYPE;
	*b++ = RF_2A4M1_WMM_OUI_SUBTYPE_INFO;
	*b++ = RF_2A4M1_WMM_VERSION;
	*b++ = qos_info;
	return 2u + RF_2A4M1_WMM_INFO_BODY_LEN;
}

bool rf_2a4m1_wmm_info_parse(const uint8_t *body, uint8_t dlen, uint8_t *qos_info_out)
{
	if (!qos_info_out || dlen != RF_2A4M1_WMM_INFO_BODY_LEN ||
	    !rf_2a4m1_wmm_hdr_ok(body, dlen, RF_2A4M1_WMM_OUI_SUBTYPE_INFO, RF_2A4M1_WMM_INFO_BODY_LEN))
		return false;
	*qos_info_out = body[6];
	return true;
}

bool rf_2a4m1_edca_param_parse(const uint8_t *body, uint8_t dlen, struct rf_2a4m1_edca_param *out)
{
	if (!body || !out || dlen != RF_2A4M1_EDCA_PARAM_BODY_LEN)
		return false;
	out->qos_info = body[0];
	/* body[1] = Reserved */
	const uint8_t *r = body + 2;
	for (int i = 0; i < RF_2A4M1_QOS_AC_COUNT; i++) {
		rf_2a4m1_ac_rec_get(r, &out->ac[i]);
		r += RF_2A4M1_AC_REC_LEN;
	}
	return true;
}

static const uint8_t rf_2a4m1_k_up_to_ac[8] = {
	RF_2A4M1_QOS_AC_BE, RF_2A4M1_QOS_AC_BK, RF_2A4M1_QOS_AC_BK, RF_2A4M1_QOS_AC_BE,
	RF_2A4M1_QOS_AC_VI, RF_2A4M1_QOS_AC_VI, RF_2A4M1_QOS_AC_VO, RF_2A4M1_QOS_AC_VO,
};

static const uint8_t rf_2a4m1_k_ac_to_aci[RF_2A4M1_QOS_AC_COUNT] = {
	[RF_2A4M1_QOS_AC_BK] = RF_2A4M1_QOS_ACI_BK, [RF_2A4M1_QOS_AC_BE] = RF_2A4M1_QOS_ACI_BE,
	[RF_2A4M1_QOS_AC_VI] = RF_2A4M1_QOS_ACI_VI, [RF_2A4M1_QOS_AC_VO] = RF_2A4M1_QOS_ACI_VO,
};

static const uint8_t rf_2a4m1_k_aci_to_ac[RF_2A4M1_QOS_AC_COUNT] = {
	[RF_2A4M1_QOS_ACI_BE] = RF_2A4M1_QOS_AC_BE, [RF_2A4M1_QOS_ACI_BK] = RF_2A4M1_QOS_AC_BK,
	[RF_2A4M1_QOS_ACI_VI] = RF_2A4M1_QOS_AC_VI, [RF_2A4M1_QOS_ACI_VO] = RF_2A4M1_QOS_AC_VO,
};

uint8_t rf_2a4m1_qos_up_to_ac(uint8_t up)  { return rf_2a4m1_k_up_to_ac[up & 0x07]; }

static const struct rf_2a4m1_ac_default rf_2a4m1_k_defaults[RF_2A4M1_QOS_AC_COUNT] = {
	[RF_2A4M1_QOS_AC_BK] = { 7, 4, 10, 0 },
	[RF_2A4M1_QOS_AC_BE] = { 3, 4, 10, 0 },
	[RF_2A4M1_QOS_AC_VI] = { 2, 3, 4, 94 },
	[RF_2A4M1_QOS_AC_VO] = { 2, 2, 3, 47 },
};

void rf_2a4m1_edca_defaults(struct rf_2a4m1_edca_param *p)
{
	if (!p)
		return;
	p->qos_info = 0;
	for (int aci = 0; aci < RF_2A4M1_QOS_AC_COUNT; aci++) {
		uint8_t ac = rf_2a4m1_k_aci_to_ac[aci];
		const struct rf_2a4m1_ac_default *d = &rf_2a4m1_k_defaults[ac];
		p->ac[aci].aci_aifsn  = (uint8_t)((d->aifsn & RF_2A4M1_EDCA_AIFSN_MASK) |
		                                  ((aci & 0x03) << RF_2A4M1_EDCA_ACI_SHIFT));
		p->ac[aci].ecw        = rf_2a4m1_edca_ecw(d->ecwmin, d->ecwmax);
		p->ac[aci].txop_limit = d->txop;
	}
}

void rf_2a4m1_edca_fill_lmac_defaults(struct rf_2a4m1_edca_ac *out)
{
	if (!out)
		return;
	for (int ac = 0; ac < RF_2A4M1_QOS_AC_COUNT; ac++) {
		const struct rf_2a4m1_ac_default *d = &rf_2a4m1_k_defaults[ac];
		out[ac].cw_min = rf_2a4m1_edca_cw_from_ecw(d->ecwmin);
		out[ac].cw_max = rf_2a4m1_edca_cw_from_ecw(d->ecwmax);
		out[ac].aifsn  = d->aifsn;
		out[ac].txop   = d->txop;
	}
}

void rf_2a4m1_edca_apply_to_lmac(const struct rf_2a4m1_edca_param *p, struct rf_2a4m1_edca_ac *out)
{
	if (!p || !out)
		return;
	for (int i = 0; i < RF_2A4M1_QOS_AC_COUNT; i++) {
		const struct rf_2a4m1_edca_ac_rec *r = &p->ac[i];
		/* Trust the record's own ACI subfield for placement; fall back to array order. */
		uint8_t aci = (uint8_t)((r->aci_aifsn & RF_2A4M1_EDCA_ACI_MASK) >> RF_2A4M1_EDCA_ACI_SHIFT);
		uint8_t ac  = rf_2a4m1_k_aci_to_ac[aci & 0x03];
		out[ac].cw_min = rf_2a4m1_edca_cw_from_ecw(rf_2a4m1_edca_ecwmin(r->ecw));
		out[ac].cw_max = rf_2a4m1_edca_cw_from_ecw(rf_2a4m1_edca_ecwmax(r->ecw));
		out[ac].aifsn  = (uint8_t)(r->aci_aifsn & RF_2A4M1_EDCA_AIFSN_MASK);
		out[ac].txop   = r->txop_limit;
	}
}

