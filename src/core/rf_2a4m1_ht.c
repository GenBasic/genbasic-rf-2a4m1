// SPDX-License-Identifier: GPL-2.0
//
// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
// Wi-Fi 4 / 802.11n HT (capability IEs + Block-Ack)
//
// Copyright (c) GenBasic.
// Licensed under the GNU General Public License, version 2.
//
// This file is machine-generated. Do not hand-edit.


#include "rf_2a4m1_core.h"

/* file-scope constants + helper types */
#define RF_2A4M1_HTC_CAP_INFO   0
#define RF_2A4M1_HTC_AMPDU      2
#define RF_2A4M1_HTC_MCS_SET    3
#define RF_2A4M1_HTC_EXT_CAP    19
#define RF_2A4M1_HTC_TXBF       21
#define RF_2A4M1_HTC_ASEL       25
#define RF_2A4M1_HTO_PRIMARY    0
#define RF_2A4M1_HTO_INFO       1
#define RF_2A4M1_HTO_BASIC_MCS  6

size_t rf_2a4m1_ba_build_addba_req(uint8_t *out, size_t cap, uint8_t dialog_token, uint8_t tid,
                          bool amsdu, bool immediate, uint16_t buf_size,
                          uint16_t timeout, uint16_t ssn)
{
	if (!out || cap < 9)
		return 0;
	uint16_t param = 0;
	if (amsdu)     param |= BA_PARAM_AMSDU;
	if (immediate) param |= BA_PARAM_POLICY_IMMEDIATE;
	param |= (uint16_t)((tid << BA_PARAM_TID_SHIFT) & BA_PARAM_TID_MASK);
	param |= (uint16_t)((buf_size << BA_PARAM_BUFSIZE_SHIFT) & BA_PARAM_BUFSIZE_MASK);
	out[0] = WLAN_ACTION_BLOCK_ACK;
	out[1] = WLAN_BA_ACTION_ADDBA_REQ;
	out[2] = dialog_token;
	rf_2a4m1_put_le16(out + 3, param);
	rf_2a4m1_put_le16(out + 5, timeout);
	/* Block Ack Starting Sequence Control: Fragment(4b)=0 || SSN(12b) << 4. */
	rf_2a4m1_put_le16(out + 7, (uint16_t)((ssn & 0x0fff) << 4));
	return 9;
}

bool rf_2a4m1_ba_parse_addba_req(const uint8_t *body, size_t len, uint8_t *dialog_token,
                        uint8_t *tid, bool *amsdu, bool *immediate, uint16_t *buf_size,
                        uint16_t *timeout, uint16_t *ssn)
{
	if (!body || len < 9 ||
	    body[0] != WLAN_ACTION_BLOCK_ACK || body[1] != WLAN_BA_ACTION_ADDBA_REQ)
		return false;
	uint16_t param = rf_2a4m1_get_le16(body + 3);
	if (dialog_token) *dialog_token = body[2];
	if (amsdu)        *amsdu     = (param & BA_PARAM_AMSDU) != 0;
	if (immediate)    *immediate = (param & BA_PARAM_POLICY_IMMEDIATE) != 0;
	if (tid)          *tid       = (uint8_t)((param & BA_PARAM_TID_MASK) >> BA_PARAM_TID_SHIFT);
	if (buf_size)     *buf_size  = (uint16_t)((param & BA_PARAM_BUFSIZE_MASK) >> BA_PARAM_BUFSIZE_SHIFT);
	if (timeout)      *timeout   = rf_2a4m1_get_le16(body + 5);
	if (ssn)          *ssn       = (uint16_t)((rf_2a4m1_get_le16(body + 7) >> 4) & 0x0fff);
	return true;
}

size_t rf_2a4m1_ba_build_addba_rsp(uint8_t *out, size_t cap, uint8_t dialog_token, uint16_t status,
                          uint8_t tid, bool amsdu, bool immediate, uint16_t buf_size,
                          uint16_t timeout)
{
	if (!out || cap < 9)
		return 0;
	uint16_t param = 0;
	if (amsdu)     param |= BA_PARAM_AMSDU;
	if (immediate) param |= BA_PARAM_POLICY_IMMEDIATE;
	param |= (uint16_t)((tid << BA_PARAM_TID_SHIFT) & BA_PARAM_TID_MASK);
	param |= (uint16_t)((buf_size << BA_PARAM_BUFSIZE_SHIFT) & BA_PARAM_BUFSIZE_MASK);
	out[0] = WLAN_ACTION_BLOCK_ACK;
	out[1] = WLAN_BA_ACTION_ADDBA_RSP;
	out[2] = dialog_token;
	rf_2a4m1_put_le16(out + 3, status);
	rf_2a4m1_put_le16(out + 5, param);
	rf_2a4m1_put_le16(out + 7, timeout);
	return 9;
}

bool rf_2a4m1_ba_parse_addba_rsp(const uint8_t *body, size_t len, uint8_t *dialog_token,
                        uint16_t *status, uint8_t *tid, bool *amsdu, bool *immediate,
                        uint16_t *buf_size, uint16_t *timeout)
{
	if (!body || len < 9 ||
	    body[0] != WLAN_ACTION_BLOCK_ACK || body[1] != WLAN_BA_ACTION_ADDBA_RSP)
		return false;
	uint16_t param = rf_2a4m1_get_le16(body + 5);
	if (dialog_token) *dialog_token = body[2];
	if (status)       *status    = rf_2a4m1_get_le16(body + 3);
	if (amsdu)        *amsdu     = (param & BA_PARAM_AMSDU) != 0;
	if (immediate)    *immediate = (param & BA_PARAM_POLICY_IMMEDIATE) != 0;
	if (tid)          *tid       = (uint8_t)((param & BA_PARAM_TID_MASK) >> BA_PARAM_TID_SHIFT);
	if (buf_size)     *buf_size  = (uint16_t)((param & BA_PARAM_BUFSIZE_MASK) >> BA_PARAM_BUFSIZE_SHIFT);
	if (timeout)      *timeout   = rf_2a4m1_get_le16(body + 7);
	return true;
}

size_t rf_2a4m1_ba_originator_addba(struct ba_agreement *a, uint8_t *out, size_t cap,
                           uint8_t dialog_token, uint8_t tid, bool amsdu, bool immediate,
                           uint16_t buf_size, uint16_t timeout, uint16_t ssn)
{
	if (!a)
		return 0;
	size_t n = rf_2a4m1_ba_build_addba_req(out, cap, dialog_token, tid, amsdu, immediate,
	                              buf_size, timeout, ssn);
	if (!n)
		return 0;
	a->state        = BA_PENDING;
	a->tid          = tid;
	a->dialog_token = dialog_token;
	a->amsdu        = amsdu;
	a->immediate    = immediate;
	a->buf_size     = buf_size;
	a->timeout      = timeout;
	a->ssn          = (uint16_t)(ssn & 0x0fff);
	return n;
}

rf_2a4m1_s8021x_status rf_2a4m1_ba_originator_on_addba_rsp(struct ba_agreement *a, const uint8_t *body,
                                         size_t len)
{
	if (!a)
		return RF_2A4M1_S8021X_ERR_INVAL;
	uint8_t  dtok, tid;
	uint16_t status, buf_size, timeout;
	bool     amsdu, immediate;
	if (!rf_2a4m1_ba_parse_addba_rsp(body, len, &dtok, &status, &tid, &amsdu, &immediate,
	                        &buf_size, &timeout))
		return RF_2A4M1_S8021X_ERR_INVAL;
	if (a->state != BA_PENDING || dtok != a->dialog_token || tid != a->tid)
		return RF_2A4M1_S8021X_ERR_STATE;
	if (status != 0) {                      /* peer declined the agreement */
		a->state = BA_IDLE;
		return RF_2A4M1_S8021X_ERR_REJECTED;
	}
	/* Adopt the responder's negotiated-down parameters. */
	a->amsdu     = amsdu;
	a->immediate = immediate;
	if (buf_size)                           /* responder may shrink the window */
		a->buf_size = buf_size;
	a->timeout   = timeout;
	a->state     = BA_ESTABLISHED;
	return RF_2A4M1_S8021X_OK;
}

size_t rf_2a4m1_ht_cap_build(uint8_t *out, size_t cap, const struct ht_cap *c)
{
	if (!out || !c || cap < 2 + HT_CAP_BODY_LEN)
		return 0;
	out[0] = WLAN_EID_HT_CAP;
	out[1] = HT_CAP_BODY_LEN;
	uint8_t *b = out + 2;
	rf_2a4m1_put_le16(b + RF_2A4M1_HTC_CAP_INFO, c->cap_info);
	b[RF_2A4M1_HTC_AMPDU] = c->ampdu_params;
	memcpy(b + RF_2A4M1_HTC_MCS_SET, c->mcs_set, HT_MCS_SET_LEN);
	rf_2a4m1_put_le16(b + RF_2A4M1_HTC_EXT_CAP, c->ext_cap);
	rf_2a4m1_put_le32(b + RF_2A4M1_HTC_TXBF, c->txbf_cap);
	b[RF_2A4M1_HTC_ASEL] = c->asel_cap;
	return 2 + HT_CAP_BODY_LEN;
}

bool rf_2a4m1_ht_cap_parse(const uint8_t *body, uint8_t dlen, struct ht_cap *out)
{
	if (!body || !out || dlen != HT_CAP_BODY_LEN)
		return false;                        /* wrong length -> reject, no over-read */
	out->cap_info     = rf_2a4m1_get_le16(body + RF_2A4M1_HTC_CAP_INFO);
	out->ampdu_params = body[RF_2A4M1_HTC_AMPDU];
	memcpy(out->mcs_set, body + RF_2A4M1_HTC_MCS_SET, HT_MCS_SET_LEN);
	out->ext_cap      = rf_2a4m1_get_le16(body + RF_2A4M1_HTC_EXT_CAP);
	out->txbf_cap     = rf_2a4m1_get_le32(body + RF_2A4M1_HTC_TXBF);
	out->asel_cap     = body[RF_2A4M1_HTC_ASEL];
	return true;
}

size_t rf_2a4m1_ht_oper_build(uint8_t *out, size_t cap, const struct ht_oper *o)
{
	if (!out || !o || cap < 2 + HT_OPER_BODY_LEN)
		return 0;
	out[0] = WLAN_EID_HT_OPERATION;
	out[1] = HT_OPER_BODY_LEN;
	uint8_t *b = out + 2;
	b[RF_2A4M1_HTO_PRIMARY] = o->primary_chan;
	memcpy(b + RF_2A4M1_HTO_INFO, o->rf_2a4m1_info, 5);
	memcpy(b + RF_2A4M1_HTO_BASIC_MCS, o->basic_mcs_set, HT_MCS_SET_LEN);
	return 2 + HT_OPER_BODY_LEN;
}

bool rf_2a4m1_ht_oper_parse(const uint8_t *body, uint8_t dlen, struct ht_oper *out)
{
	if (!body || !out || dlen != HT_OPER_BODY_LEN)
		return false;
	out->primary_chan = body[RF_2A4M1_HTO_PRIMARY];
	memcpy(out->rf_2a4m1_info, body + RF_2A4M1_HTO_INFO, 5);
	memcpy(out->basic_mcs_set, body + RF_2A4M1_HTO_BASIC_MCS, HT_MCS_SET_LEN);
	return true;
}

