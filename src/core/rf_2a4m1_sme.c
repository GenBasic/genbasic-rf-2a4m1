// SPDX-License-Identifier: GPL-2.0
//
// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
// SME / MLME station+AP connect state machine (scan/auth/assoc/4-way, SAE/OWE, FT/TDLS transport)
//
// Copyright (c) GenBasic.
// Licensed under the GNU General Public License, version 2.
//
// This file is machine-generated. Do not hand-edit.


#include "rf_2a4m1_core.h"

/* file-scope constants + helper types */
#define RF_2A4M1_SME_INT_TAG      0x03u
#define RF_2A4M1_SME_INT_ENC(t)   ((uint8_t)(((t) << 2) | RF_2A4M1_SME_INT_TAG))
#define RF_2A4M1_SME_INT_IS(b)    (((b) & 0x03u) == RF_2A4M1_SME_INT_TAG)
#define RF_2A4M1_SME_INT_TYPE(b)  ((uint8_t)((b) >> 2))
#define RF_2A4M1_SME_PFX_TYPE 0
#define RF_2A4M1_SME_PFX_ADDR 1
#define RF_2A4M1_SME_PFX_LEN  7
#define RF_2A4M1_SME_RX_VIEW_MAX (RF_2A4M1_SME_PFX_LEN + 300)
#define RF_2A4M1_SME_MGMT_MAX (RF_2A4M1_MGMT_HDR_LEN + 6 + 2 + RF_2A4M1_MGMT_SSID_MAX + 2 + (int)sizeof rf_2a4m1_sme_supp_rates + 240)
#define RF_2A4M1_SME_LISTEN_INTERVAL 10
#define RF_2A4M1_SME_AP_AID 1
#define RF_2A4M1_EO_VERSION     0
#define RF_2A4M1_EO_PKTTYPE     1
#define RF_2A4M1_EO_BODYLEN     2
#define RF_2A4M1_EO_DESCTYPE    4
#define RF_2A4M1_EO_KEYINFO     5
#define RF_2A4M1_EO_KEYLEN      7
#define RF_2A4M1_EO_REPLAY      9
#define RF_2A4M1_EO_NONCE      17
#define RF_2A4M1_EO_IV         49
#define RF_2A4M1_EO_RSC        65
#define RF_2A4M1_EO_RESERVED   73
#define RF_2A4M1_EO_MIC        81
#define RF_2A4M1_EO_KEYDATALEN 97
#define RF_2A4M1_EO_KEYDATA    99
#define RF_2A4M1_EAPOL_BODY_FIXED (RF_2A4M1_EAPOL_FIXED_LEN - EAPOL_HDR_LEN)
#define RF_2A4M1_SME_KEY_LEN_CCMP 16
#define RF_2A4M1_SME_DATA_PN_LEN 6
#define RF_2A4M1_GTK_KDE_LEN 24
#define RF_2A4M1_IGTK_KDE_LEN 30
#define RF_2A4M1_EAPOL_SNAP_LEN 8
#define RF_2A4M1_DOT11_DATA_HDR_LEN 24
#define RF_2A4M1_DOT11_FC0_DATA     0x08
#define RF_2A4M1_DOT11_FC1_TODS     0x01
#define RF_2A4M1_DOT11_FC1_FROMDS   0x02
#define RF_2A4M1_TDLS_SNAP_LEN  8
#define RF_2A4M1_TDLS_BODY_MAX  160
#define RF_2A4M1_FT_MDE_LEN 5
#define RF_2A4M1_FILS_SESSION_EL_LEN     11
#define RF_2A4M1_FILS_NONCE_EL_LEN       19
#define RF_2A4M1_FILS_KEY_CONFIRM_EL_LEN 35
#define RF_2A4M1_FILS_PUBKEY_EL_LEN (4 + FILS_PK_PUBKEY_LEN)
#define RF_2A4M1_FILS_ASSOC_AAD_N 3
#define RF_2A4M1_SME_ADDTS_STATUS_ADMITTED 0
#define RF_2A4M1_SME_ADDTS_STATUS_REFUSED  37
enum { RF_2A4M1_F_PROBE_REQ = 1, RF_2A4M1_F_PROBE_RESP, RF_2A4M1_F_AUTH_REQ, RF_2A4M1_F_AUTH_RESP, RF_2A4M1_F_ASSOC_REQ, RF_2A4M1_F_ASSOC_RESP,
       RF_2A4M1_F_M1, RF_2A4M1_F_M2, RF_2A4M1_F_M3, RF_2A4M1_F_M4, RF_2A4M1_F_DATA, RF_2A4M1_F_QOS_DATA,
        
       RF_2A4M1_F_FT_AUTH1, RF_2A4M1_F_FT_AUTH2, RF_2A4M1_F_FT_REASSOC_REQ, RF_2A4M1_F_FT_REASSOC_RESP,
        
       RF_2A4M1_F_FT_DS_REQ, RF_2A4M1_F_FT_DS_RESP,
        
       RF_2A4M1_F_FILS_AUTH1, RF_2A4M1_F_FILS_AUTH2, RF_2A4M1_F_FILS_ASSOC_REQ, RF_2A4M1_F_FILS_ASSOC_RESP,
        
       RF_2A4M1_F_ADDBA_REQ, RF_2A4M1_F_ADDBA_RSP, RF_2A4M1_F_TWT_SETUP, RF_2A4M1_F_TWT_RESP,
        
       RF_2A4M1_F_ADDTS_REQ, RF_2A4M1_F_ADDTS_RSP, RF_2A4M1_F_DELTS,
        
       RF_2A4M1_F_REASSOC_REQ, RF_2A4M1_F_REASSOC_RESP, RF_2A4M1_F_SA_QUERY_REQ, RF_2A4M1_F_SA_QUERY_RESP,
        
       RF_2A4M1_F_BEACON,
       RF_2A4M1_F__MAX };

/* file-local forward declarations */
static bool rf_2a4m1_bytes_eq(const uint8_t *a, const uint8_t *b, size_t n);
static bool rf_2a4m1_bytes_lt(const uint8_t *a, const uint8_t *b, size_t n);
static bool rf_2a4m1_eapol_mic_ok(struct rf_2a4m1_sme *s, const uint8_t *pdu, size_t pdu_len);
static bool rf_2a4m1_ft_locate(const uint8_t *body, size_t len, uint16_t *mdid,
                      const uint8_t **mde, size_t *mde_len,
                      const uint8_t **fte, size_t *fte_len,
                      const uint8_t **trailing, size_t *trailing_len);
static bool rf_2a4m1_gtk_kde_find(const uint8_t *kd, size_t len, uint8_t gtk[RF_2A4M1_SME_GTK_LEN]);
static bool rf_2a4m1_gtk_kde_parse(const uint8_t *in, size_t len, uint8_t gtk[RF_2A4M1_SME_GTK_LEN]);
static bool rf_2a4m1_igtk_kde_find(const uint8_t *kd, size_t len, uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                          uint16_t *key_id, uint64_t *ipn);
static bool rf_2a4m1_mac_is_zero(const rf_2a4m1_mac_addr *m);
static bool rf_2a4m1_qos_scan_ies(const uint8_t *ies, size_t len,
                         bool *saw_wmm_info, struct rf_2a4m1_edca_param *edca_out, bool *saw_edca);
static bool rf_2a4m1_rx_dot11_data(struct rf_2a4m1_sme *s, const uint8_t *in, size_t len);
static bool rf_2a4m1_sme_akm_wpa3(const struct rf_2a4m1_sme *s);
static bool rf_2a4m1_sme_owe_dh_find(const uint8_t *ies, size_t len, uint16_t *group,
                            const uint8_t **pub_x, uint8_t *pub_x_len);
static bool rf_2a4m1_sme_owe_on_assoc_resp(struct rf_2a4m1_sme *s, const uint8_t *ies, size_t ies_len);
static bool rf_2a4m1_sme_sae_tx_commit(struct rf_2a4m1_sme *s);
static bool rf_2a4m1_sme_sae_tx_confirm(struct rf_2a4m1_sme *s);
static bool rf_2a4m1_tdls_is_encap(const uint8_t *payload, size_t plen);
static rf_2a4m1_s8021x_status rf_2a4m1_tx_protected_group_mgmt(struct rf_2a4m1_sme *s, uint8_t subtype, uint16_t reason);
static size_t rf_2a4m1_ft_build_mde_fte(struct rf_2a4m1_sme *s, uint8_t *buf, size_t cap, bool set_a, bool set_s,
                               const rf_2a4m1_mac_addr *r1kh, size_t *fte_off, size_t *mde_len);
static size_t rf_2a4m1_ft_build_reassoc_req_extra(struct rf_2a4m1_sme *s, uint8_t *out, size_t cap);
static size_t rf_2a4m1_ft_ds_target_process(struct rf_2a4m1_sme *t, const rf_2a4m1_mac_addr *sta, const rf_2a4m1_mac_addr *target,
                                   const uint8_t snonce[RF_2A4M1_FT_NONCE_LEN],
                                   const uint8_t pmk_r1[RF_2A4M1_FT_PMK_R1_LEN],
                                   const uint8_t pmk_r1_name[RF_2A4M1_FT_KEY_NAME_LEN],
                                   uint8_t *resp_out, size_t resp_cap);
static size_t rf_2a4m1_gencap_build_caps(struct rf_2a4m1_sme *s, uint8_t *ie, size_t cap, size_t n);
static size_t rf_2a4m1_gencap_build_opers(struct rf_2a4m1_sme *s, uint8_t *ie, size_t cap, size_t n);
static size_t rf_2a4m1_gtk_kde_build(uint8_t out[RF_2A4M1_GTK_KDE_LEN], const uint8_t *gtk, uint8_t keyid);
static size_t rf_2a4m1_igtk_kde_build(uint8_t out[RF_2A4M1_IGTK_KDE_LEN], const uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                             uint16_t key_id, uint64_t ipn);
static size_t rf_2a4m1_put_data_hdr(struct rf_2a4m1_sme *s, uint8_t *f, uint16_t seq_ctrl);
static size_t rf_2a4m1_sme_owe_dh_ie_build(uint8_t *out, size_t cap, const uint8_t pub_xy[RF_2A4M1_P256_POINT_LEN]);
static uint16_t rf_2a4m1_pmf_negotiate_ies(struct rf_2a4m1_sme *s, const uint8_t *ies, size_t len);
static uint16_t rf_2a4m1_sme_cap_info(void);
static uint16_t rf_2a4m1_sme_ki_m2(const struct rf_2a4m1_sme *s);
static uint16_t rf_2a4m1_sme_ki_m4(const struct rf_2a4m1_sme *s);
static uint16_t rf_2a4m1_sme_next_seq(struct rf_2a4m1_sme *s);
static uint64_t rf_2a4m1_get_pn48(const uint8_t *p);
static uint8_t rf_2a4m1_addts_ac_up(uint8_t ac);
static uint8_t rf_2a4m1_sme_rsn_akm(const struct rf_2a4m1_sme *s);
static void rf_2a4m1_addts_ap_on_req(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len);
static void rf_2a4m1_addts_on_delts(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len);
static void rf_2a4m1_addts_sta_on_resp(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len);
static void rf_2a4m1_ba_ap_on_addba_req(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len);
static void rf_2a4m1_eapol_compute_mic(struct rf_2a4m1_sme *s, uint8_t mic[RF_2A4M1_SME_MIC_LEN],
                              const uint8_t *pdu, size_t pdu_len);
static void rf_2a4m1_eapol_set_mic(struct rf_2a4m1_sme *s, uint8_t *pdu, size_t pdu_len);
static void rf_2a4m1_ft_ap_on_auth1(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *src, const uint8_t *body, size_t len);
static void rf_2a4m1_ft_ap_on_ds_request(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len);
static void rf_2a4m1_ft_ap_on_reassoc(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len);
static void rf_2a4m1_ft_fill_fte(struct rf_2a4m1_sme *s, struct rf_2a4m1_ft_fte *f, bool set_anonce, bool set_snonce,
                        const rf_2a4m1_mac_addr *r1kh);
static void rf_2a4m1_ft_gen_nonce32(struct rf_2a4m1_sme *s, uint8_t out[RF_2A4M1_FT_NONCE_LEN], uint64_t salt);
static void rf_2a4m1_ft_sta_on_auth2(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len);
static void rf_2a4m1_ft_sta_on_ds_response(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len);
static void rf_2a4m1_ft_sta_on_reassoc(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len);
static void rf_2a4m1_ft_tx(struct rf_2a4m1_sme *s, uint8_t type, const uint8_t *payload, size_t plen);
static void rf_2a4m1_ft_tx_auth1(struct rf_2a4m1_sme *s);
static void rf_2a4m1_ft_tx_auth2(struct rf_2a4m1_sme *s);
static void rf_2a4m1_ft_tx_ds_request(struct rf_2a4m1_sme *s);
static void rf_2a4m1_ft_tx_reassoc(struct rf_2a4m1_sme *s, uint8_t type, uint8_t seq, const rf_2a4m1_mac_addr *r1kh,
                          const uint8_t *extra, size_t extra_len);
static void rf_2a4m1_gencap_default_ht_cap(struct ht_cap *c);
static void rf_2a4m1_gencap_default_ht_oper(struct ht_oper *o);
static void rf_2a4m1_gencap_scan_ies(struct rf_2a4m1_sme *s, const uint8_t *ies, size_t len, bool sta);
static void rf_2a4m1_pmf_ap_on_spurious_reassoc(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *src);
static void rf_2a4m1_put_pn48(uint8_t *o, uint64_t pn)          /* big-endian, PN[0] = MSB */;
static void rf_2a4m1_qos_apply_lmac(struct rf_2a4m1_sme *s);
static void rf_2a4m1_rx_bss_announce(struct rf_2a4m1_sme *s, const struct rf_2a4m1_mgmt_hdr *h);
static void rf_2a4m1_rx_data(struct rf_2a4m1_sme *s, const uint8_t *in, size_t len);
static void rf_2a4m1_rx_dot11_mgmt(struct rf_2a4m1_sme *s, const uint8_t *in, size_t len, const struct rf_2a4m1_mgmt_hdr *h);
static void rf_2a4m1_rx_qos_data(struct rf_2a4m1_sme *s, const uint8_t *in, size_t len);
static void rf_2a4m1_rx_robust_mgmt(struct rf_2a4m1_sme *s, const uint8_t *frame, size_t len);
static void rf_2a4m1_sa_query_sta_respond(struct rf_2a4m1_sme *s, const uint8_t *body, size_t body_len);
static void rf_2a4m1_sa_query_tx(struct rf_2a4m1_sme *s, uint8_t type, const uint8_t *body, size_t body_len);
static void rf_2a4m1_sme_demux(struct rf_2a4m1_sme *s, uint8_t type, const uint8_t *in, size_t len);
static void rf_2a4m1_sme_derive_ptk(struct rf_2a4m1_sme *s);
static void rf_2a4m1_sme_fill_tx_params(struct rf_2a4m1_tx_params *tp, uint8_t ac, uint8_t tid);
static void rf_2a4m1_sme_gen_material(struct rf_2a4m1_sme *s, uint8_t *dst, size_t n, uint64_t salt);
static void rf_2a4m1_sme_hal_rx(struct rf_2a4m1_hal *h, const struct rf_2a4m1_rxinfo *rx, void *ctx);
static void rf_2a4m1_sme_install_data_key(struct rf_2a4m1_sme *s);
static void rf_2a4m1_sme_sae_enter_authed(struct rf_2a4m1_sme *s);
static void rf_2a4m1_sme_sae_rx_auth(struct rf_2a4m1_sme *s, const struct rf_2a4m1_mgmt_hdr *h);
static void rf_2a4m1_sta_on_m1(struct rf_2a4m1_sme *s, const uint8_t *pdu, size_t pdu_len);
static void rf_2a4m1_tdls_encap_tx(struct rf_2a4m1_sme *s, const uint8_t *body, size_t body_len);
static void rf_2a4m1_tdls_gen_nonce32(struct rf_2a4m1_sme *s, uint8_t out[RF_2A4M1_TDLS_NONCE_LEN], uint64_t salt);
static void rf_2a4m1_tdls_reset(struct rf_2a4m1_sme *s);
static void rf_2a4m1_tdls_sta_deliver(struct rf_2a4m1_sme *s, const uint8_t *body, size_t body_len);
static void rf_2a4m1_tx_action_dot11(struct rf_2a4m1_sme *s, const uint8_t *body, size_t body_len);
static void rf_2a4m1_tx_assoc_req(struct rf_2a4m1_sme *s);
static void rf_2a4m1_tx_assoc_resp(struct rf_2a4m1_sme *s);
static void rf_2a4m1_tx_eapol(struct rf_2a4m1_sme *s, uint16_t key_info, uint16_t key_len, uint64_t replay,
                     const uint8_t *nonce, const uint8_t *key_data,
                     uint16_t key_data_len);
static void rf_2a4m1_tx_mgmt(struct rf_2a4m1_sme *s, uint8_t type);
static void rf_2a4m1_tx_mgmt_ie(struct rf_2a4m1_sme *s, uint8_t type, const uint8_t *ie, size_t ie_len);
static void rf_2a4m1_tx_mgmt_to(struct rf_2a4m1_sme *s, uint8_t type, const rf_2a4m1_mac_addr *da,
                       const uint8_t *ie, size_t ie_len);
static void rf_2a4m1_tx_raw(struct rf_2a4m1_sme *s, uint8_t *f, size_t len);
static void rf_2a4m1_tx_raw_ac(struct rf_2a4m1_sme *s, uint8_t *f, size_t len, uint8_t ac, uint8_t tid);

static const uint8_t rf_2a4m1_sme_supp_rates[8] = { 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24 };

static uint16_t rf_2a4m1_sme_cap_info(void)
{
	return (uint16_t)(RF_2A4M1_WLAN_CAPABILITY_ESS | RF_2A4M1_WLAN_CAPABILITY_SHORT_PREAMBLE |
	                  RF_2A4M1_WLAN_CAPABILITY_PRIVACY);
}

static uint16_t rf_2a4m1_sme_next_seq(struct rf_2a4m1_sme *s)
{
	return (uint16_t)((s->seq++ & 0x0fff) << 4);
}

static bool rf_2a4m1_bytes_eq(const uint8_t *a, const uint8_t *b, size_t n)
{
	uint8_t diff = 0;
	for (size_t i = 0; i < n; i++)
		diff |= (uint8_t)(a[i] ^ b[i]);
	return diff == 0;
}

static bool rf_2a4m1_mac_is_zero(const rf_2a4m1_mac_addr *m)
{
	uint8_t acc = 0;
	for (int i = 0; i < RF_2A4M1_ETH_ALEN; i++)
		acc |= m->a[i];
	return acc == 0;
}

static void rf_2a4m1_put_pn48(uint8_t *o, uint64_t pn)          /* big-endian, PN[0] = MSB */
{
	for (int i = 0; i < RF_2A4M1_SME_DATA_PN_LEN; i++)
		o[i] = (uint8_t)(pn >> (8 * (RF_2A4M1_SME_DATA_PN_LEN - 1 - i)));
}

static uint64_t rf_2a4m1_get_pn48(const uint8_t *p)
{
	uint64_t v = 0;
	for (int i = 0; i < RF_2A4M1_SME_DATA_PN_LEN; i++)
		v = (v << 8) | p[i];
	return v;
}

static void rf_2a4m1_sme_install_data_key(struct rf_2a4m1_sme *s)
{
	const uint8_t *tk = &s->ptk[RF_2A4M1_SME_TK_OFF];
	if (s->data_key_valid && rf_2a4m1_bytes_eq(s->installed_tk, tk, RF_2A4M1_SME_TK_LEN))
		return;                          /* same TK re-installed → keep the RX replay state */
	rf_2a4m1_pn_replay_reset(&s->data_rx_replay);
	s->data_tx_pn = 0;
	memcpy(s->installed_tk, tk, RF_2A4M1_SME_TK_LEN);
	s->data_key_valid = true;
}

size_t rf_2a4m1_sme_eapol_key_encode(uint8_t *out, size_t cap, uint16_t key_info,
                            uint16_t key_len, uint64_t replay,
                            const uint8_t nonce[RF_2A4M1_SME_NONCE_LEN],
                            const uint8_t *key_data, uint16_t key_data_len)
{
	size_t total = (size_t)RF_2A4M1_EAPOL_FIXED_LEN + key_data_len;
	if (cap < total)
		return 0;
	memset(out, 0, total);
	out[RF_2A4M1_EO_VERSION] = RF_2A4M1_EAPOL_VERSION;
	out[RF_2A4M1_EO_PKTTYPE] = RF_2A4M1_EAPOL_TYPE_KEY;
	rf_2a4m1_put_be16(&out[RF_2A4M1_EO_BODYLEN], (uint16_t)(RF_2A4M1_EAPOL_BODY_FIXED + key_data_len));
	out[RF_2A4M1_EO_DESCTYPE] = RF_2A4M1_EAPOL_DESC_RSN;
	rf_2a4m1_put_be16(&out[RF_2A4M1_EO_KEYINFO], key_info);
	rf_2a4m1_put_be16(&out[RF_2A4M1_EO_KEYLEN], key_len);
	rf_2a4m1_put_be64(&out[RF_2A4M1_EO_REPLAY], replay);
	if (nonce)
		memcpy(&out[RF_2A4M1_EO_NONCE], nonce, RF_2A4M1_SME_NONCE_LEN);  /* full 32-octet Key Nonce */
	/* Key IV / RSC / Reserved / MIC stay zero (MIC filled later via sme_crypto) */
	rf_2a4m1_put_be16(&out[RF_2A4M1_EO_KEYDATALEN], key_data_len);
	if (key_data_len && key_data)
		memcpy(&out[RF_2A4M1_EO_KEYDATA], key_data, key_data_len);
	return total;
}

bool rf_2a4m1_sme_eapol_key_decode(const uint8_t *in, size_t len, uint16_t *key_info,
                          uint64_t *replay, uint8_t nonce[RF_2A4M1_SME_NONCE_LEN],
                          uint8_t *key_data, uint16_t key_data_cap,
                          uint16_t *key_data_len)
{
	if (len < RF_2A4M1_EAPOL_FIXED_LEN)
		return false;
	/* Accept EAPOL v1 (legacy) and v2 (802.1X-2004) — SoftMAC does not filter
	 * version and hostapd has shipped both; only the type/descriptor are load-bearing. */
	if ((in[RF_2A4M1_EO_VERSION] != 1 && in[RF_2A4M1_EO_VERSION] != RF_2A4M1_EAPOL_VERSION) ||
	    in[RF_2A4M1_EO_PKTTYPE] != RF_2A4M1_EAPOL_TYPE_KEY)
		return false;
	if (in[RF_2A4M1_EO_DESCTYPE] != RF_2A4M1_EAPOL_DESC_RSN)
		return false;
	uint16_t kdl = rf_2a4m1_get_be16(&in[RF_2A4M1_EO_KEYDATALEN]);
	if ((size_t)RF_2A4M1_EAPOL_FIXED_LEN + kdl > len)   /* declared Key Data must fit */
		return false;
	if (key_info)
		*key_info = rf_2a4m1_get_be16(&in[RF_2A4M1_EO_KEYINFO]);
	if (replay)
		*replay = rf_2a4m1_get_be64(&in[RF_2A4M1_EO_REPLAY]);
	if (nonce)
		memcpy(nonce, &in[RF_2A4M1_EO_NONCE], RF_2A4M1_SME_NONCE_LEN);
	if (key_data) {
		if (kdl > key_data_cap)   /* KDE larger than the caller's buffer — reject, don't truncate (F1) */
			return false;
		if (kdl)
			memcpy(key_data, &in[RF_2A4M1_EO_KEYDATA], kdl);
	}
	if (key_data_len)             /* only the fits-in-buffer length is ever reported */
		*key_data_len = kdl;
	return true;
}

static void rf_2a4m1_eapol_compute_mic(struct rf_2a4m1_sme *s, uint8_t mic[RF_2A4M1_SME_MIC_LEN],
                              const uint8_t *pdu, size_t pdu_len)
{
	if (s->akm == RF_2A4M1_SME_AKM_SAE) {
		rf_2a4m1_aes128_cmac(&s->ptk[RF_2A4M1_SME_KCK_OFF], pdu, pdu_len, mic);
		return;
	}
	if (s->akm == RF_2A4M1_SME_AKM_OWE) {
		uint8_t full[RF_2A4M1_SHA256_DIGEST_LEN];
		rf_2a4m1_hmac_sha256(&s->ptk[RF_2A4M1_SME_KCK_OFF], RF_2A4M1_SME_KCK_LEN, pdu, pdu_len, full);
		memcpy(mic, full, RF_2A4M1_SME_MIC_LEN);
		return;
	}
	s->crypto->mic(mic, &s->ptk[RF_2A4M1_SME_KCK_OFF], pdu, pdu_len);
}

static void rf_2a4m1_eapol_set_mic(struct rf_2a4m1_sme *s, uint8_t *pdu, size_t pdu_len)
{
	uint8_t mic[RF_2A4M1_SME_MIC_LEN];
	rf_2a4m1_eapol_compute_mic(s, mic, pdu, pdu_len);
	memcpy(&pdu[RF_2A4M1_EO_MIC], mic, RF_2A4M1_SME_MIC_LEN);
}

static bool rf_2a4m1_eapol_mic_ok(struct rf_2a4m1_sme *s, const uint8_t *pdu, size_t pdu_len)
{
	uint8_t tmp[RF_2A4M1_SME_FRAME_MAX];
	uint8_t mic[RF_2A4M1_SME_MIC_LEN];
	if (pdu_len > sizeof(tmp))
		return false;
	memcpy(tmp, pdu, pdu_len);
	memset(&tmp[RF_2A4M1_EO_MIC], 0, RF_2A4M1_EAPOL_MIC_LEN);
	rf_2a4m1_eapol_compute_mic(s, mic, tmp, pdu_len);
	return rf_2a4m1_bytes_eq(mic, &pdu[RF_2A4M1_EO_MIC], RF_2A4M1_SME_MIC_LEN);
}

static bool rf_2a4m1_bytes_lt(const uint8_t *a, const uint8_t *b, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++) {
		if (a[i] < b[i]) return true;
		if (a[i] > b[i]) return false;
	}
	return false;
}

static void rf_2a4m1_sme_derive_ptk(struct rf_2a4m1_sme *s)
{
	if (s->akm == RF_2A4M1_SME_AKM_SAE || s->akm == RF_2A4M1_SME_AKM_OWE) {
		uint8_t ctx[2 * RF_2A4M1_ETH_ALEN + 2 * RF_2A4M1_SME_NONCE_LEN];
		const uint8_t *lo = s->self.a, *hi = s->peer.a;
		const uint8_t *nlo = s->anonce, *nhi = s->snonce;
		if (rf_2a4m1_bytes_lt(s->peer.a, s->self.a, RF_2A4M1_ETH_ALEN)) {
			lo = s->peer.a; hi = s->self.a;
		}
		if (rf_2a4m1_bytes_lt(s->snonce, s->anonce, RF_2A4M1_SME_NONCE_LEN)) {
			nlo = s->snonce; nhi = s->anonce;
		}
		memcpy(ctx, lo, RF_2A4M1_ETH_ALEN);
		memcpy(ctx + RF_2A4M1_ETH_ALEN, hi, RF_2A4M1_ETH_ALEN);
		memcpy(ctx + 2 * RF_2A4M1_ETH_ALEN, nlo, RF_2A4M1_SME_NONCE_LEN);
		memcpy(ctx + 2 * RF_2A4M1_ETH_ALEN + RF_2A4M1_SME_NONCE_LEN, nhi, RF_2A4M1_SME_NONCE_LEN);
		rf_2a4m1_ieee80211_kdf_length(s->pmk, RF_2A4M1_SME_PMK_LEN, "Pairwise key expansion", 22,
		                     ctx, sizeof ctx, s->ptk, 384);
		return;
	}
	s->crypto->kdf(s->ptk, s->pmk, s->anonce, s->snonce, &s->self, &s->peer);
}

static bool rf_2a4m1_sme_akm_wpa3(const struct rf_2a4m1_sme *s)
{
	return s->akm == RF_2A4M1_SME_AKM_SAE || s->akm == RF_2A4M1_SME_AKM_OWE;
}

static uint16_t rf_2a4m1_sme_ki_m2(const struct rf_2a4m1_sme *s)
{
	return rf_2a4m1_sme_akm_wpa3(s) ? RF_2A4M1_SME_KI_M2_WPA3 : RF_2A4M1_SME_KI_M2;
}

static uint16_t rf_2a4m1_sme_ki_m4(const struct rf_2a4m1_sme *s)
{
	return rf_2a4m1_sme_akm_wpa3(s) ? RF_2A4M1_SME_KI_M4_WPA3 : RF_2A4M1_SME_KI_M4;
}

static uint8_t rf_2a4m1_sme_rsn_akm(const struct rf_2a4m1_sme *s)
{
	if (s->akm == RF_2A4M1_SME_AKM_SAE) return RF_2A4M1_PMF_AKM_SAE;
	if (s->akm == RF_2A4M1_SME_AKM_OWE) return RF_2A4M1_PMF_AKM_OWE;
	(void)s; return RF_2A4M1_PMF_AKM_PSK;
}

static const uint8_t rf_2a4m1_rsnxe_h2e[] = { 0xf4, 0x01, 0x20 };

static size_t rf_2a4m1_sme_owe_dh_ie_build(uint8_t *out, size_t cap, const uint8_t pub_xy[RF_2A4M1_P256_POINT_LEN])
{
	if (cap < 5 + RF_2A4M1_P256_COORD_LEN)
		return 0;
	out[0] = RF_2A4M1_WLAN_EID_EXTENSION;
	out[1] = (uint8_t)(1 + 2 + RF_2A4M1_P256_COORD_LEN);
	out[2] = RF_2A4M1_WLAN_EID_EXT_OWE_DH_PARAM;
	out[3] = (uint8_t)(RF_2A4M1_SAE_GROUP_19 & 0xff);
	out[4] = (uint8_t)(RF_2A4M1_SAE_GROUP_19 >> 8);
	memcpy(out + 5, pub_xy, RF_2A4M1_P256_COORD_LEN); /* X only on the wire */
	return 5 + RF_2A4M1_P256_COORD_LEN;
}

static bool rf_2a4m1_sme_owe_dh_find(const uint8_t *ies, size_t len, uint16_t *group,
                            const uint8_t **pub_x, uint8_t *pub_x_len)
{
	size_t o = 0;
	while (o + 2 <= len) {
		uint8_t id = ies[o], ln = ies[o + 1];
		if ((size_t)o + 2 + ln > len)
			break;
		if (id == RF_2A4M1_WLAN_EID_EXTENSION && ln >= 1 + 2 + RF_2A4M1_P256_COORD_LEN &&
		    ies[o + 2] == RF_2A4M1_WLAN_EID_EXT_OWE_DH_PARAM) {
			*group = (uint16_t)(ies[o + 3] | ((uint16_t)ies[o + 4] << 8));
			*pub_x = &ies[o + 5];
			*pub_x_len = (uint8_t)(ln - 3);
			return true;
		}
		o += (size_t)2 + ln;
	}
	return false;
}

static bool rf_2a4m1_sme_owe_on_assoc_resp(struct rf_2a4m1_sme *s, const uint8_t *ies, size_t ies_len)
{
	uint16_t grp = 0;
	const uint8_t *ap_x = NULL;
	uint8_t ap_x_len = 0, ap_pub[RF_2A4M1_P256_POINT_LEN], pmkid[16];

	if (!s->owe_kp_ok)
		return false;
	if (!rf_2a4m1_sme_owe_dh_find(ies, ies_len, &grp, &ap_x, &ap_x_len))
		return false;
	s->owe_dh_rx++;
	if (grp != RF_2A4M1_SAE_GROUP_19 || ap_x_len != RF_2A4M1_P256_COORD_LEN)
		return false;
	if (!rf_2a4m1_p256_point_from_x(ap_x, ap_pub))
		return false;
	if (!rf_2a4m1_owe_derive(s->owe_priv, s->owe_pub, ap_pub, true, s->pmk, pmkid))
		return false;
	rf_2a4m1_crypto_wipe(s->owe_priv, sizeof s->owe_priv);
	s->owe_kp_ok = false;
	s->owe_ok = true;
	return true;
}

static void rf_2a4m1_sme_gen_material(struct rf_2a4m1_sme *s, uint8_t *dst, size_t n, uint64_t salt)
{
	uint8_t nb[RF_2A4M1_SME_NONCE_LEN];
	s->crypto->gen_nonce(nb, s->pmk, &s->self, salt);
	memcpy(dst, nb, n);
	rf_2a4m1_crypto_wipe(nb, sizeof nb);
}

static size_t rf_2a4m1_gtk_kde_build(uint8_t out[RF_2A4M1_GTK_KDE_LEN], const uint8_t *gtk, uint8_t keyid)
{
	out[0] = 0xdd;
	out[1] = 22;                 /* length after this field: OUI(3)+dt(1)+keyinfo(2)+GTK(16) */
	out[2] = 0x00; out[3] = 0x0f; out[4] = 0xac;
	out[5] = 0x01;               /* KDE data type = GTK */
	out[6] = (uint8_t)(keyid & 0x03);   /* KeyID (bits 0-1); Tx (bit 2) = 0 */
	out[7] = 0x00;               /* reserved */
	memcpy(&out[8], gtk, RF_2A4M1_SME_GTK_LEN);
	return RF_2A4M1_GTK_KDE_LEN;
}

static bool rf_2a4m1_gtk_kde_parse(const uint8_t *in, size_t len, uint8_t gtk[RF_2A4M1_SME_GTK_LEN])
{
	/* OUI(3)+dt(1)+keyinfo(2)+GTK(16) => in[1] == 22. Reject rather than truncate a KDE whose
	 * key exceeds CCMP-128's 16-byte GTK (e.g. a 32-byte GCMP/CCMP-256 GTK) — only 128-bit is
	 * negotiated today, so a longer key means a wrong install, not a silent 16-byte copy. */
	if (len < RF_2A4M1_GTK_KDE_LEN || in[0] != 0xdd || in[1] != 22)
		return false;
	if (in[2] != 0x00 || in[3] != 0x0f || in[4] != 0xac || in[5] != 0x01)
		return false;
	memcpy(gtk, &in[8], RF_2A4M1_SME_GTK_LEN);
	return true;
}

static bool rf_2a4m1_gtk_kde_find(const uint8_t *kd, size_t len, uint8_t gtk[RF_2A4M1_SME_GTK_LEN])
{
	size_t off = 0;
	while (off + 2 <= len) {
		uint8_t t = kd[off], l = kd[off + 1];
		size_t klen = (size_t)2 + l;
		if (t == 0x00)
			break;                          /* pad / end */
		if (off + klen > len)
			break;
		if (t == 0xdd && rf_2a4m1_gtk_kde_parse(kd + off, klen, gtk))
			return true;
		off += klen;
	}
	return false;
}

static size_t rf_2a4m1_igtk_kde_build(uint8_t out[RF_2A4M1_IGTK_KDE_LEN], const uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                             uint16_t key_id, uint64_t ipn)
{
	out[0] = 0xdd;
	out[1] = 28;                 /* OUI(3)+dt(1)+KeyID(2)+IPN(6)+IGTK(16) */
	out[2] = 0x00; out[3] = 0x0f; out[4] = 0xac;
	out[5] = 0x09;               /* KDE data type = IGTK */
	out[6] = (uint8_t)key_id; out[7] = (uint8_t)(key_id >> 8);   /* KeyID, little-endian */
	for (int i = 0; i < 6; i++)
		out[8 + i] = (uint8_t)(ipn >> (8 * i));                 /* IPN (6 octets, LE) */
	memcpy(&out[14], igtk, RF_2A4M1_BIP_IGTK_LEN);
	return RF_2A4M1_IGTK_KDE_LEN;
}

static bool rf_2a4m1_igtk_kde_find(const uint8_t *kd, size_t len, uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                          uint16_t *key_id, uint64_t *ipn)
{
	size_t off = 0;
	while (off + 2 <= len) {
		uint8_t t = kd[off], l = kd[off + 1];
		size_t klen = (size_t)2 + l;
		if (t == 0x00)
			break;                          /* pad / end */
		if (off + klen > len)
			break;
		/* IGTK KDE: dd len 00-0f-ac-09 KeyID(2)|IPN(6)|IGTK(16) → l == 28. */
		if (t == 0xdd && l == 28 &&
		    kd[off + 2] == 0x00 && kd[off + 3] == 0x0f &&
		    kd[off + 4] == 0xac && kd[off + 5] == 0x09) {
			if (key_id)
				*key_id = (uint16_t)(kd[off + 6] | ((uint16_t)kd[off + 7] << 8));
			uint64_t p = 0;
			for (int i = 0; i < 6; i++)
				p |= (uint64_t)kd[off + 8 + i] << (8 * i);
			if (ipn)
				*ipn = p;
			memcpy(igtk, &kd[off + 14], RF_2A4M1_BIP_IGTK_LEN);
			return true;
		}
		off += klen;
	}
	return false;
}

static void rf_2a4m1_sme_fill_tx_params(struct rf_2a4m1_tx_params *tp, uint8_t ac, uint8_t tid)
{
	memset(tp, 0, sizeof *tp);
	tp->wcid = 0xff;
	tp->ac = ac;
	tp->tid = tid;
	tp->gen = RF_2A4M1_HAL_GEN_LEGACY;
	tp->phy_mode = RF_2A4M1_HAL_PHY_OFDM;
	tp->mcs = 0;               /* DESC_RATE 6M via OFDM mcs 0 ladder */
	tp->bw_mhz = 20;
	tp->n_ss = 1;
	tp->key_slot = 0xff;
}

static void rf_2a4m1_tx_raw(struct rf_2a4m1_sme *s, uint8_t *f, size_t len)
{
	struct rf_2a4m1_mpdu m = { f, (uint16_t)len, (uint16_t)len };
	struct rf_2a4m1_tx_params tp;
	rf_2a4m1_sme_fill_tx_params(&tp, /*ac=*/1, /*tid=*/0xff);
	rf_2a4m1_hal_tx(s->rf_2a4m1_hal, &m, &tp);
}

static void rf_2a4m1_tx_raw_ac(struct rf_2a4m1_sme *s, uint8_t *f, size_t len, uint8_t ac, uint8_t tid)
{
	struct rf_2a4m1_mpdu m = { f, (uint16_t)len, (uint16_t)len };
	struct rf_2a4m1_tx_params tp;
	rf_2a4m1_sme_fill_tx_params(&tp, ac, tid);
	rf_2a4m1_hal_tx(s->rf_2a4m1_hal, &m, &tp);
}

static void rf_2a4m1_tx_mgmt_to(struct rf_2a4m1_sme *s, uint8_t type, const rf_2a4m1_mac_addr *da,
                       const uint8_t *ie, size_t ie_len)
{
	uint8_t f[RF_2A4M1_SME_MGMT_MAX];
	uint16_t sc = rf_2a4m1_sme_next_seq(s);
	size_t n = 0;

	if (ie_len > 240)
		return;
	switch (type) {
	case RF_2A4M1_F_PROBE_REQ:
		/* A scan: DA and BSSID are both the target (a directed probe) or both broadcast (the
		 * wildcard probe every AP answers) when no BSSID was requested. */
		n = rf_2a4m1_mgmt_build_probe_req(f, sizeof f, da, &s->self, da, sc,
		                         s->ssid, s->ssid_len, rf_2a4m1_sme_supp_rates, sizeof rf_2a4m1_sme_supp_rates);
		break;
	case RF_2A4M1_F_PROBE_RESP:
		n = rf_2a4m1_mgmt_build_probe_resp(f, sizeof f, da, &s->self, sc, /*timestamp=*/0,
		                          /*beacon_int=*/100, rf_2a4m1_sme_cap_info(), s->ssid, s->ssid_len,
		                          s->channel, rf_2a4m1_sme_supp_rates, sizeof rf_2a4m1_sme_supp_rates, NULL, 0);
		break;
	case RF_2A4M1_F_BEACON:
		n = rf_2a4m1_mgmt_build_beacon(f, sizeof f, &s->self, sc, /*timestamp=*/0, /*beacon_int=*/100,
		                      rf_2a4m1_sme_cap_info(), s->ssid, s->ssid_len, s->channel,
		                      rf_2a4m1_sme_supp_rates, sizeof rf_2a4m1_sme_supp_rates, NULL, 0);
		break;
	case RF_2A4M1_F_AUTH_REQ:
		n = rf_2a4m1_mgmt_build_auth_open(f, sizeof f, da, &s->self, sc);
		break;
	case RF_2A4M1_F_AUTH_RESP:
		n = rf_2a4m1_mgmt_build_auth_resp(f, sizeof f, da, &s->self, sc, RF_2A4M1_WLAN_AUTH_OPEN,
		                         RF_2A4M1_WLAN_STATUS_SUCCESS);
		break;
	case RF_2A4M1_F_ASSOC_REQ:
		n = rf_2a4m1_mgmt_build_assoc_req(f, sizeof f, da, &s->self, sc, rf_2a4m1_sme_cap_info(),
		                         RF_2A4M1_SME_LISTEN_INTERVAL, s->ssid, s->ssid_len,
		                         rf_2a4m1_sme_supp_rates, sizeof rf_2a4m1_sme_supp_rates,
		                         /*rsn=*/NULL, 0, ie, (uint8_t)ie_len);
		break;
	case RF_2A4M1_F_ASSOC_RESP:
		n = rf_2a4m1_mgmt_build_assoc_resp(f, sizeof f, da, &s->self, sc, rf_2a4m1_sme_cap_info(),
		                          RF_2A4M1_WLAN_STATUS_SUCCESS, RF_2A4M1_SME_AP_AID,
		                          rf_2a4m1_sme_supp_rates, sizeof rf_2a4m1_sme_supp_rates, ie, (uint8_t)ie_len);
		break;
	default:
		return;
	}
	if (n)
		rf_2a4m1_tx_raw(s, f, n);
}

static void rf_2a4m1_tx_mgmt_ie(struct rf_2a4m1_sme *s, uint8_t type, const uint8_t *ie, size_t ie_len)
{
	rf_2a4m1_tx_mgmt_to(s, type, &s->peer, ie, ie_len);
}

static void rf_2a4m1_tx_mgmt(struct rf_2a4m1_sme *s, uint8_t type) { rf_2a4m1_tx_mgmt_ie(s, type, NULL, 0); }

static bool rf_2a4m1_qos_scan_ies(const uint8_t *ies, size_t len,
                         bool *saw_wmm_info, struct rf_2a4m1_edca_param *edca_out, bool *saw_edca)
{
	*saw_wmm_info = false;
	*saw_edca = false;
	const uint8_t *p = ies;
	size_t rem = len;
	while (rem >= 2) {
		uint8_t eid = p[0], elen = p[1];
		if ((size_t)2 + elen > rem)
			break;                                  /* declared length overruns -> stop */
		const uint8_t *body = p + 2;
		if (eid == RF_2A4M1_WLAN_EID_VENDOR_SPECIFIC) {
			uint8_t qi;
			if (elen == RF_2A4M1_WMM_INFO_BODY_LEN && rf_2a4m1_wmm_info_parse(body, elen, &qi))
				*saw_wmm_info = true;
			else if (elen == RF_2A4M1_WMM_PARAM_BODY_LEN && rf_2a4m1_wmm_param_parse(body, elen, edca_out))
				*saw_edca = true;
		} else if (eid == RF_2A4M1_WLAN_EID_EDCA_PARAM_SET) {
			if (rf_2a4m1_edca_param_parse(body, elen, edca_out))
				*saw_edca = true;
		}
		p += (size_t)2 + elen;
		rem -= (size_t)2 + elen;
	}
	return *saw_wmm_info || *saw_edca;
}

static void rf_2a4m1_qos_apply_lmac(struct rf_2a4m1_sme *s)
{
	struct rf_2a4m1_lmac_cfg cfg;
	memset(&cfg, 0, sizeof cfg);
	rf_2a4m1_edca_fill_lmac_defaults(cfg.ac);          /* 802.11 default EDCA (BK,BE,VI,VO) */
	rf_2a4m1_edca_apply_to_lmac(&s->edca, cfg.ac);     /* overlay the AP's advertised parameters */
	cfg.sifs_us = 10;
	cfg.slot_us = 9;
	cfg.auto_resp = true;
	rf_2a4m1_hal_set_lower_mac(s->rf_2a4m1_hal, &cfg);
}

static void rf_2a4m1_gencap_default_ht_cap(struct ht_cap *c)
{
	memset(c, 0, sizeof *c);
	c->cap_info = HT_CAP_SUP_WIDTH_20_40 | HT_CAP_SGI_20 | HT_CAP_SGI_40;
	c->mcs_set[0] = 0xff;                 /* MCS 0-7 (one spatial stream) */
}

static void rf_2a4m1_gencap_default_ht_oper(struct ht_oper *o)
{
	memset(o, 0, sizeof *o);
	o->primary_chan = 1;
	o->basic_mcs_set[0] = 0xff;
}

static void rf_2a4m1_gencap_scan_ies(struct rf_2a4m1_sme *s, const uint8_t *ies, size_t len, bool sta)
{
	const uint8_t *p = ies;
	size_t rem = len;
	while (rem >= 2) {
		uint8_t eid = p[0], elen = p[1];
		if ((size_t)2 + elen > rem)
			break;                                  /* declared length overruns -> stop */
		const uint8_t *body = p + 2;
		if (eid == WLAN_EID_HT_CAP) {
			struct ht_cap c;
			if (rf_2a4m1_ht_cap_parse(body, elen, &c)) {
				s->ht_peer_cap = c; s->ht_peer_cap_set = true;
				if (!sta) s->ht_enabled = true;     /* AP: the STA is HT-capable */
			}
		} else if (eid == WLAN_EID_HT_OPERATION) {
			struct ht_oper o;
			if (rf_2a4m1_ht_oper_parse(body, elen, &o) && sta) { s->ht_oper = o; s->ht_enabled = true; }
		}
		p += (size_t)2 + elen;
		rem -= (size_t)2 + elen;
	}
}

static size_t rf_2a4m1_gencap_build_caps(struct rf_2a4m1_sme *s, uint8_t *ie, size_t cap, size_t n)
{
	if (s->ht_advertise) {
		struct ht_cap c; rf_2a4m1_gencap_default_ht_cap(&c);
		n += rf_2a4m1_ht_cap_build(&ie[n], cap - n, &c);
	}
	return n;
}

static size_t rf_2a4m1_gencap_build_opers(struct rf_2a4m1_sme *s, uint8_t *ie, size_t cap, size_t n)
{
	if (s->ht_enabled) {
		struct ht_oper o;
		if (s->ht_oper_advert_set) o = s->ht_oper_advert; else rf_2a4m1_gencap_default_ht_oper(&o);
		n += rf_2a4m1_ht_oper_build(&ie[n], cap - n, &o);
		if (s->ht_peer_cap_set)
			n += rf_2a4m1_ht_cap_build(&ie[n], cap - n, &s->ht_peer_cap);   /* echo the STA's HT Cap */
	}
	return n;
}

static void rf_2a4m1_tx_assoc_req(struct rf_2a4m1_sme *s)
{
	uint8_t ie[240];   
	size_t n = 0;
	/* Always emit RSNE first (SoftMAC / hostapd order: RSN before WMM).
	 * AKM = PSK or SAE; caps follow local pmf_mode (SAE forces REQUIRED). */
	n += rf_2a4m1_pmf_rsne_build(&ie[n], sizeof ie - n,
	                    rf_2a4m1_pmf_mode_caps((enum rf_2a4m1_pmf_mode)s->rf_2a4m1_pmf_mode),
	                    rf_2a4m1_sme_rsn_akm(s));
	/* SAE-H2E: hostapd sae_pwe=1 requires RSNXE bit 5 or it rejects the assoc. */
	if (s->akm == RF_2A4M1_SME_AKM_SAE && n + sizeof rf_2a4m1_rsnxe_h2e <= sizeof ie) {
		memcpy(&ie[n], rf_2a4m1_rsnxe_h2e, sizeof rf_2a4m1_rsnxe_h2e);
		n += sizeof rf_2a4m1_rsnxe_h2e;
	}
	/* OWE: RSN(OWE) + OWE DH Parameter (group 19, our public X). */
	if (s->akm == RF_2A4M1_SME_AKM_OWE && s->owe_kp_ok) {
		size_t dn = rf_2a4m1_sme_owe_dh_ie_build(&ie[n], sizeof ie - n, s->owe_pub);
		if (dn) {
			n += dn;
			s->owe_dh_tx++;
		}
	}
	if (s->qos_advertise)
		n += rf_2a4m1_wmm_info_build(&ie[n], sizeof ie - n, 0x00 /* STA QoS Info: U-APSD off */);
	if (n)
		rf_2a4m1_tx_mgmt_ie(s, RF_2A4M1_F_ASSOC_REQ, ie, n);
	else
		rf_2a4m1_tx_mgmt(s, RF_2A4M1_F_ASSOC_REQ);
}

static void rf_2a4m1_tx_assoc_resp(struct rf_2a4m1_sme *s)
{
	uint8_t ie[240];
	size_t n = 0;
	if (s->qos_enabled) {
		struct rf_2a4m1_edca_param adv;
		if (s->edca_advert_set)
			adv = s->edca_advert;
		else
			rf_2a4m1_edca_defaults(&adv);
		n += rf_2a4m1_wmm_param_build(&ie[n], sizeof ie - n, &adv);
	}
	if (s->rf_2a4m1_pmf_mode != RF_2A4M1_PMF_MODE_DISABLED)
		n += rf_2a4m1_pmf_rsne_build(&ie[n], sizeof ie - n,
		                    rf_2a4m1_pmf_mode_caps((enum rf_2a4m1_pmf_mode)s->rf_2a4m1_pmf_mode), RF_2A4M1_PMF_AKM_PSK);
	if (n)
		rf_2a4m1_tx_mgmt_ie(s, RF_2A4M1_F_ASSOC_RESP, ie, n);
	else
		rf_2a4m1_tx_mgmt(s, RF_2A4M1_F_ASSOC_RESP);
}

static void rf_2a4m1_tx_action_dot11(struct rf_2a4m1_sme *s, const uint8_t *body, size_t body_len)
{
	uint8_t hdr[24], out[256];
	uint16_t sc = rf_2a4m1_sme_next_seq(s);
	size_t n;

	if (!body || body_len == 0 || body_len > 200)
		return;
	memset(hdr, 0, sizeof hdr);
	hdr[0] = (uint8_t)((RF_2A4M1_WLAN_FC_TYPE_MGMT << 2) | (RF_2A4M1_WLAN_FC_STYPE_ACTION << 4));
	hdr[1] = 0x00;
	if (s->role == RF_2A4M1_SME_ROLE_AP) {
		memcpy(hdr + 4, s->peer.a, RF_2A4M1_ETH_ALEN);    /* DA = STA */
		memcpy(hdr + 10, s->self.a, RF_2A4M1_ETH_ALEN);   /* SA = AP */
		memcpy(hdr + 16, s->self.a, RF_2A4M1_ETH_ALEN);   /* BSSID = AP */
	} else {
		memcpy(hdr + 4, s->peer.a, RF_2A4M1_ETH_ALEN);    /* DA = BSSID */
		memcpy(hdr + 10, s->self.a, RF_2A4M1_ETH_ALEN);   /* SA = STA */
		memcpy(hdr + 16, s->peer.a, RF_2A4M1_ETH_ALEN);   /* BSSID */
	}
	hdr[22] = (uint8_t)(sc & 0xff);
	hdr[23] = (uint8_t)((sc >> 8) & 0xff);
	if (s->mfp_active && s->ptk_installed) {
		n = rf_2a4m1_ccmp_encrypt(out, sizeof out, hdr, 24, body, body_len,
		                 &s->ptk[RF_2A4M1_SME_TK_OFF], 0, ++s->mgmt_tx_pn);
		if (!n)
			return;
		rf_2a4m1_tx_raw(s, out, n);
		return;
	}
	if (24 + body_len > sizeof out)
		return;
	memcpy(out, hdr, 24);
	memcpy(out + 24, body, body_len);
	rf_2a4m1_tx_raw(s, out, 24 + body_len);
}

static uint16_t rf_2a4m1_pmf_negotiate_ies(struct rf_2a4m1_sme *s, const uint8_t *ies, size_t len)
{
	const uint8_t *p = ies;
	size_t rem = len;
	while (rem >= 2) {
		uint8_t eid = p[0], elen = p[1];
		if ((size_t)2 + elen > rem)
			break;
		if (eid == RF_2A4M1_PMF_EID_RSN) {
			uint16_t caps = 0; bool gm = false;
			if (rf_2a4m1_pmf_rsne_parse(p, (size_t)2 + elen, &caps, &gm)) {
				s->pmf_peer_caps = caps;
				bool active = false;
				uint16_t st = rf_2a4m1_pmf_negotiate((enum rf_2a4m1_pmf_mode)s->rf_2a4m1_pmf_mode, caps, &active);
				s->mfp_active = active;
				return st;
			}
		}
		p += (size_t)2 + elen;
		rem -= (size_t)2 + elen;
	}
	return RF_2A4M1_PMF_STATUS_SUCCESS;
}

static void rf_2a4m1_rx_robust_mgmt(struct rf_2a4m1_sme *s, const uint8_t *frame, size_t len)
{
	uint16_t fc = (uint16_t)(frame[0] | ((uint16_t)frame[1] << 8));
	const uint8_t *body = frame + RF_2A4M1_BIP_MGMT_HDR_LEN;
	size_t body_len = len - RF_2A4M1_BIP_MGMT_HDR_LEN;
	if (!rf_2a4m1_pmf_is_robust_mgmt(fc, body, body_len))
		return;                                     /* not a robust mgmt frame — policy N/A */
	/* Protected iff the frame ends in a well-formed MME (EID 76, Len 16) — the BIP group-mgmt MIC. */
	bool protectd = body_len >= RF_2A4M1_BIP_MMIE_LEN &&
	                frame[len - RF_2A4M1_BIP_MMIE_LEN]     == RF_2A4M1_BIP_MMIE_EID &&
	                frame[len - RF_2A4M1_BIP_MMIE_LEN + 1] == RF_2A4M1_BIP_MMIE_BODY;
	if (s->mfp_active && !protectd) {
		/* the tested drop policy: an unprotected robust mgmt frame under an active MFP SA is dropped. */
		if (!rf_2a4m1_pmf_rx_unprotected_ok(fc, body, body_len, s->mfp_active)) {
			s->mgmt_unprot_dropped++;
			return;                                 /* never acted on */
		}
	}
	if (protectd) {
		/* verify the MME MIC + IPN replay under the installed IGTK; a forgery / replay is dropped. */
		if (!s->igtk.installed || rf_2a4m1_pmf_group_verify(&s->igtk, frame, len) != RF_2A4M1_BIP_OK) {
			s->mgmt_prot_badmic++;
			return;
		}
	}
	/* honored: a Deauth / Disassoc that survived the policy tears the association down. */
	uint8_t subtype = (uint8_t)((fc >> 4) & 0xf);
	if (subtype == RF_2A4M1_PMF_STYPE_DEAUTH || subtype == RF_2A4M1_PMF_STYPE_DISASSOC) {
		/* A STA acts on a Deauth/Disassoc only once it actually HAS an
		 * association to tear down. While still discovering / authenticating
		 * (state < SME_ASSOCED) the frame is a stray -- another cell's
		 * broadcast Disassoc, or one aimed at a different STA on the same
		 * channel -- and honoring it resets the connecting STA all the way to
		 * SME_INIT, after which the SCANNING->authenticate step (gated on the
		 * SME state) ignores every subsequent probe-response and the
		 * association silently stalls. This is the intermittent connect
		 * failure: a single stray Disassoc landing in the ~10 s connect
		 * window aborts an otherwise-good association (auth is never sent).
		 * An associated STA (>= SME_ASSOCED) still tears down normally. */
		if (s->role == RF_2A4M1_SME_ROLE_STA && s->state < RF_2A4M1_SME_ASSOCED)
			return;
		s->mgmt_prot_honored++;
		s->connected = false;
		s->state = RF_2A4M1_SME_INIT;
	}
}

static void rf_2a4m1_sa_query_tx(struct rf_2a4m1_sme *s, uint8_t type, const uint8_t *body, size_t body_len)
{
	(void)type; /* type was the internal F_* tag; on-air carries category 8 in the body */
	if (body_len > RF_2A4M1_PMF_SA_QUERY_LEN)
		return;
	rf_2a4m1_tx_action_dot11(s, body, body_len);
}

static void rf_2a4m1_pmf_ap_on_spurious_reassoc(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *src)
{
	if (s->role != RF_2A4M1_SME_ROLE_AP || !s->mfp_active || s->state != RF_2A4M1_SME_CONNECTED)
		return;                                     /* only a live MFP association is defended */
	if (!rf_2a4m1_bytes_eq(src->a, s->peer.a, RF_2A4M1_ETH_ALEN))
		return;                                     /* a (Re)Assoc-Req for a different STA — not our SA */
	/* 1. (Re)Assoc-Resp status 30 (comeback) + an Association Comeback Time (1000 TUs). Do NOT tear
	 *    the SA down (s->connected / s->state are left untouched). */
	uint8_t resp[2 + 8];
	rf_2a4m1_pmf_put_le16(&resp[0], RF_2A4M1_PMF_STATUS_COMEBACK);
	size_t cb = rf_2a4m1_pmf_comeback_ie_build(&resp[2], sizeof resp - 2, /*comeback_tu=*/1000);
	if (cb) {
		uint8_t f[RF_2A4M1_SME_PFX_LEN + sizeof resp];
		f[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(RF_2A4M1_F_REASSOC_RESP);
		memcpy(&f[RF_2A4M1_SME_PFX_ADDR], s->self.a, RF_2A4M1_ETH_ALEN);
		memcpy(&f[RF_2A4M1_SME_PFX_LEN], resp, 2 + cb);
		rf_2a4m1_tx_raw(s, f, RF_2A4M1_SME_PFX_LEN + 2 + cb);
		s->sa_query_comebacks++;
	}
	/* 2. start the SA Query to the genuine STA (the first Request; retransmits on sme_sa_query_tick). */
	uint8_t q[RF_2A4M1_PMF_SA_QUERY_LEN];
	size_t qn = rf_2a4m1_pmf_sa_query_start(&s->sa_query, q, sizeof q);
	if (qn)
		rf_2a4m1_sa_query_tx(s, RF_2A4M1_F_SA_QUERY_REQ, q, qn);
}

static void rf_2a4m1_sa_query_sta_respond(struct rf_2a4m1_sme *s, const uint8_t *body, size_t body_len)
{
	uint8_t r[RF_2A4M1_PMF_SA_QUERY_LEN];
	size_t rn = rf_2a4m1_pmf_sa_query_respond(body, body_len, r, sizeof r);
	if (rn)
		rf_2a4m1_sa_query_tx(s, RF_2A4M1_F_SA_QUERY_RESP, r, rn);
}

static void rf_2a4m1_rx_qos_data(struct rf_2a4m1_sme *s, const uint8_t *in, size_t len)
{
	if (s->state != RF_2A4M1_SME_CONNECTED || len < RF_2A4M1_SME_PFX_LEN + 2 + RF_2A4M1_SME_DATA_PN_LEN + RF_2A4M1_SME_MIC_LEN)
		return;
	size_t body = len - RF_2A4M1_SME_MIC_LEN;
	size_t plen = body - RF_2A4M1_SME_PFX_LEN - 2 - RF_2A4M1_SME_DATA_PN_LEN;
	if (plen > RF_2A4M1_SME_DATA_MAX)
		return;
	uint8_t mic[RF_2A4M1_SME_MIC_LEN];
	s->crypto->mic(mic, &s->ptk[RF_2A4M1_SME_TK_OFF], in, body);       /* MIC over [pfx][qos-ctrl][PN][payload] */
	if (!rf_2a4m1_bytes_eq(mic, &in[body], RF_2A4M1_SME_MIC_LEN)) { s->bad_mic++; return; }
	struct rf_2a4m1_qos_ctrl qc;
	if (!rf_2a4m1_qos_ctrl_parse(&in[RF_2A4M1_SME_PFX_LEN], 2, &qc))
		return;
	/* MIC-before-replay: only after the MIC verifies do we consult the per-TID PN counter (the
	 * QoS TID keys its own slot). A replayed / out-of-order-old PN is rejected, never delivered. */
	uint64_t pn = rf_2a4m1_get_pn48(&in[RF_2A4M1_SME_PFX_LEN + 2]);
	if (!rf_2a4m1_pn_replay_ok(&s->data_rx_replay, qc.tid, pn)) { s->data_replay++; return; }
	rf_2a4m1_pn_replay_commit(&s->data_rx_replay, qc.tid, pn);
	s->rx_tid = qc.tid;
	s->rx_ac  = rf_2a4m1_qos_up_to_ac(qc.tid);
	memcpy(s->rf_2a4m1_rx_data, &in[RF_2A4M1_SME_PFX_LEN + 2 + RF_2A4M1_SME_DATA_PN_LEN], plen);
	s->rx_data_len = (uint16_t)plen;
	s->data_rx++;
}

static const uint8_t rf_2a4m1_eapol_snap_hdr[RF_2A4M1_EAPOL_SNAP_LEN] = { 0xAA, 0xAA, 0x03, 0x00,
                                                        0x00, 0x00, 0x88, 0x8E };

static size_t rf_2a4m1_put_data_hdr(struct rf_2a4m1_sme *s, uint8_t *f, uint16_t seq_ctrl)
{
	bool ap = (s->role == RF_2A4M1_SME_ROLE_AP);
	f[0] = RF_2A4M1_DOT11_FC0_DATA;
	f[1] = ap ? RF_2A4M1_DOT11_FC1_FROMDS : RF_2A4M1_DOT11_FC1_TODS;
	f[2] = 0; f[3] = 0;                                     /* Duration — the HW fills it */
	memcpy(f + 4,  s->peer.a, RF_2A4M1_ETH_ALEN);                    /* addr1 = RA (BSSID / the STA) */
	memcpy(f + 10, s->self.a, RF_2A4M1_ETH_ALEN);                    /* addr2 = TA (us)              */
	memcpy(f + 16, ap ? s->self.a : s->peer.a, RF_2A4M1_ETH_ALEN);   /* addr3 = SA (AP) / DA (the AP) */
	rf_2a4m1_put_le16(f + 22, seq_ctrl);
	return RF_2A4M1_DOT11_DATA_HDR_LEN;
}

static void rf_2a4m1_tx_eapol(struct rf_2a4m1_sme *s, uint16_t key_info, uint16_t key_len, uint64_t replay,
                     const uint8_t *nonce, const uint8_t *key_data,
                     uint16_t key_data_len)
{
	uint8_t f[RF_2A4M1_SME_FRAME_MAX];
	size_t o = rf_2a4m1_put_data_hdr(s, f, rf_2a4m1_sme_next_seq(s));
	memcpy(f + o, rf_2a4m1_eapol_snap_hdr, RF_2A4M1_EAPOL_SNAP_LEN);
	o += RF_2A4M1_EAPOL_SNAP_LEN;
	uint8_t *pdu = &f[o];
	size_t pdu_len = rf_2a4m1_sme_eapol_key_encode(pdu, sizeof(f) - o, key_info,
	                                       key_len, replay, nonce, key_data, key_data_len);
	if (pdu_len == 0)
		return;
	if (key_info & RF_2A4M1_SME_KI_MIC)
		rf_2a4m1_eapol_set_mic(s, pdu, pdu_len);
	rf_2a4m1_tx_raw(s, f, o + pdu_len);
}

static const uint8_t rf_2a4m1_tdls_snap_hdr[RF_2A4M1_TDLS_SNAP_LEN] = { 0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x89, 0x0D };

static void rf_2a4m1_tdls_gen_nonce32(struct rf_2a4m1_sme *s, uint8_t out[RF_2A4M1_TDLS_NONCE_LEN], uint64_t salt)
{
	rf_2a4m1_sme_gen_material(s, out, RF_2A4M1_TDLS_NONCE_LEN, salt);
}

static void rf_2a4m1_tdls_encap_tx(struct rf_2a4m1_sme *s, const uint8_t *body, size_t body_len)
{
	if (s->state != RF_2A4M1_SME_CONNECTED || body_len == 0 || body_len > RF_2A4M1_TDLS_BODY_MAX)
		return;
	uint8_t f[RF_2A4M1_SME_PFX_LEN + RF_2A4M1_SME_DATA_PN_LEN + RF_2A4M1_TDLS_SNAP_LEN + RF_2A4M1_TDLS_BODY_MAX + RF_2A4M1_SME_MIC_LEN];
	f[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(RF_2A4M1_F_DATA);
	memcpy(&f[RF_2A4M1_SME_PFX_ADDR], s->self.a, RF_2A4M1_ETH_ALEN);
	rf_2a4m1_put_pn48(&f[RF_2A4M1_SME_PFX_LEN], ++s->data_tx_pn);                /* fresh per-frame 48-bit PN */
	memcpy(&f[RF_2A4M1_SME_PFX_LEN + RF_2A4M1_SME_DATA_PN_LEN], rf_2a4m1_tdls_snap_hdr, RF_2A4M1_TDLS_SNAP_LEN);
	memcpy(&f[RF_2A4M1_SME_PFX_LEN + RF_2A4M1_SME_DATA_PN_LEN + RF_2A4M1_TDLS_SNAP_LEN], body, body_len);
	size_t blen = RF_2A4M1_SME_PFX_LEN + RF_2A4M1_SME_DATA_PN_LEN + RF_2A4M1_TDLS_SNAP_LEN + body_len;
	s->crypto->mic(&f[blen], &s->ptk[RF_2A4M1_SME_TK_OFF], f, blen);    /* MIC over [pfx][PN][SNAP][body] */
	rf_2a4m1_tx_raw(s, f, blen + RF_2A4M1_SME_MIC_LEN);
}

static bool rf_2a4m1_tdls_is_encap(const uint8_t *payload, size_t plen)
{
	return plen >= RF_2A4M1_TDLS_SNAP_LEN + 1 &&
	       rf_2a4m1_bytes_eq(payload, rf_2a4m1_tdls_snap_hdr, RF_2A4M1_TDLS_SNAP_LEN) &&
	       payload[RF_2A4M1_TDLS_SNAP_LEN] == RF_2A4M1_TDLS_PAYLOAD_TYPE;
}

static void rf_2a4m1_tdls_reset(struct rf_2a4m1_sme *s)
{
	rf_2a4m1_crypto_wipe(&s->tdls, sizeof s->tdls);   /* wipes the TPK + returns state to TDLS_IDLE (0) */
	s->tdls_active = false;
}

static void rf_2a4m1_tdls_sta_deliver(struct rf_2a4m1_sme *s, const uint8_t *body, size_t body_len)
{
	if (body_len < 3)
		return;
	uint8_t action = body[2];
	uint8_t out[RF_2A4M1_TDLS_BODY_MAX];
	switch (action) {
	case RF_2A4M1_TDLS_ACT_SETUP_REQ: {
		/* Responder: learn the initiator from the Setup Request's Link Identifier, (re)init the
		 * peer-link as the responder ONLY when idle (a Setup-Req mid-session is a duplicate/out-of-
		 * order and is rejected by the FSM with no re-init), derive the TPK, and tunnel the Response. */
		struct rf_2a4m1_tdls_setup_req req;
		if (!rf_2a4m1_tdls_setup_req_parse(body, body_len, &req))
			return;
		if (!rf_2a4m1_mac_eq(&req.lid.resp_addr, &s->self) || !rf_2a4m1_mac_eq(&req.lid.bssid, &s->peer))
			return;                                   /* not addressed to us as responder on our BSS */
		if (!s->tdls_active || s->tdls.state == RF_2A4M1_TDLS_IDLE || s->tdls.state == RF_2A4M1_TDLS_TEARDOWN) {
			rf_2a4m1_tdls_peer_init(&s->tdls, &s->self, &req.lid.init_addr, &s->peer, /*is_initiator=*/false,
			               req.dialog_token, req.key_lifetime);
			s->tdls_active = true;
		}
		uint8_t anonce[RF_2A4M1_TDLS_NONCE_LEN];
		rf_2a4m1_tdls_gen_nonce32(s, anonce, s->nonce_salt ^ 0x7d15u);
		size_t n = rf_2a4m1_tdls_on_setup_req(&s->tdls, body, body_len, anonce, out, sizeof out);
		rf_2a4m1_crypto_wipe(anonce, sizeof anonce);
		if (n) { s->tdls_rx++; rf_2a4m1_tdls_encap_tx(s, out, n); }
		break;
	}
	case RF_2A4M1_TDLS_ACT_SETUP_RESP: {
		/* Initiator: process the Response, derive+verify the TPK, tunnel the Confirm (a MIC failure
		 * rejects with NO state advance, inside tdls_on_setup_resp). */
		size_t n = rf_2a4m1_tdls_on_setup_resp(&s->tdls, body, body_len, out, sizeof out);
		if (n) { s->tdls_rx++; rf_2a4m1_tdls_encap_tx(s, out, n); }
		break;
	}
	case RF_2A4M1_TDLS_ACT_SETUP_CONFIRM:
		/* Responder: verify the Confirm MIC (SETUP -> ESTABLISHED); a bad MIC does not advance. */
		if (rf_2a4m1_tdls_on_setup_confirm(&s->tdls, body, body_len))
			s->tdls_rx++;
		break;
	case RF_2A4M1_TDLS_ACT_TEARDOWN:
		/* Either end: a valid teardown tears the direct link down; reset the FSM to IDLE. */
		if (rf_2a4m1_tdls_on_teardown(&s->tdls, body, body_len)) { s->tdls_rx++; rf_2a4m1_tdls_reset(s); }
		break;
	default:
		break;
	}
	rf_2a4m1_crypto_wipe(out, sizeof out);
}

static void rf_2a4m1_rx_data(struct rf_2a4m1_sme *s, const uint8_t *in, size_t len)
{
	if (s->state != RF_2A4M1_SME_CONNECTED || len < RF_2A4M1_SME_PFX_LEN + RF_2A4M1_SME_DATA_PN_LEN + RF_2A4M1_SME_MIC_LEN)
		return;
	size_t body = len - RF_2A4M1_SME_MIC_LEN;             /* [pfx][PN][payload], trailing MIC excluded */
	size_t plen = body - RF_2A4M1_SME_PFX_LEN - RF_2A4M1_SME_DATA_PN_LEN;
	uint8_t mic[RF_2A4M1_SME_MIC_LEN];
	s->crypto->mic(mic, &s->ptk[RF_2A4M1_SME_TK_OFF], in, body);       /* MIC over [pfx][PN][payload] */
	if (!rf_2a4m1_bytes_eq(mic, &in[body], RF_2A4M1_SME_MIC_LEN)) { s->bad_mic++; return; }
	/* MIC-before-replay: a non-QoS data frame keys the shared non-QoS/mgmt replay slot. */
	uint64_t pn = rf_2a4m1_get_pn48(&in[RF_2A4M1_SME_PFX_LEN]);
	if (!rf_2a4m1_pn_replay_ok(&s->data_rx_replay, RF_2A4M1_PN_REPLAY_NONQOS, pn)) { s->data_replay++; return; }
	rf_2a4m1_pn_replay_commit(&s->data_rx_replay, RF_2A4M1_PN_REPLAY_NONQOS, pn);
	const uint8_t *payload = &in[RF_2A4M1_SME_PFX_LEN + RF_2A4M1_SME_DATA_PN_LEN];
	/* A TDLS AP-encapsulated Data frame: de-encapsulate + route. An AP RELAYS the inner TDLS action
	 * frame onto the sibling STA's link (re-encapsulated under that STA's TK); a STA feeds it to the
	 * peer-link FSM. A non-TDLS Data frame falls through to the ordinary app-data delivery below. */
	if (rf_2a4m1_tdls_is_encap(payload, plen)) {
		const uint8_t *tbody = payload + RF_2A4M1_TDLS_SNAP_LEN;
		size_t tlen = plen - RF_2A4M1_TDLS_SNAP_LEN;
		if (s->role == RF_2A4M1_SME_ROLE_AP) {
			if (s->tdls_relay) { rf_2a4m1_tdls_encap_tx(s->tdls_relay, tbody, tlen); s->tdls_relayed++; }
		} else {
			rf_2a4m1_tdls_sta_deliver(s, tbody, tlen);
		}
		return;
	}
	if (plen > RF_2A4M1_SME_DATA_MAX)
		return;
	memcpy(s->rf_2a4m1_rx_data, payload, plen);
	s->rx_data_len = (uint16_t)plen;
	s->data_rx++;
}

static void rf_2a4m1_ft_fill_fte(struct rf_2a4m1_sme *s, struct rf_2a4m1_ft_fte *f, bool set_anonce, bool set_snonce,
                        const rf_2a4m1_mac_addr *r1kh)
{
	memset(f, 0, sizeof *f);
	f->element_count = 3;                       /* MIC Control: # of protected IEs (RSNE+MDE+FTE) */
	if (set_anonce)
		memcpy(f->anonce, s->ft_anonce, RF_2A4M1_FT_NONCE_LEN);
	if (set_snonce)
		memcpy(f->snonce, s->ft_snonce, RF_2A4M1_FT_NONCE_LEN);
	if (r1kh) { f->r1kh_id = *r1kh; f->has_r1kh = true; }
	if (s->ft_r0kh_len) {
		memcpy(f->r0kh_id, s->ft_r0kh_id, s->ft_r0kh_len);
		f->r0kh_len = s->ft_r0kh_len;
		f->has_r0kh = true;
	}
}

static void rf_2a4m1_ft_gen_nonce32(struct rf_2a4m1_sme *s, uint8_t out[RF_2A4M1_FT_NONCE_LEN], uint64_t salt)
{
	rf_2a4m1_sme_gen_material(s, out, RF_2A4M1_FT_NONCE_LEN, salt);
}

static void rf_2a4m1_ft_tx(struct rf_2a4m1_sme *s, uint8_t type, const uint8_t *payload, size_t plen)
{
	uint8_t f[RF_2A4M1_SME_PFX_LEN + 300];   /* headroom for MDE+FTE+name + OCI + RIC + wrapped-GTK */
	if (plen > sizeof f - RF_2A4M1_SME_PFX_LEN)
		return;
	f[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(type);
	memcpy(&f[RF_2A4M1_SME_PFX_ADDR], s->self.a, RF_2A4M1_ETH_ALEN);
	memcpy(&f[RF_2A4M1_SME_PFX_LEN], payload, plen);
	rf_2a4m1_tx_raw(s, f, RF_2A4M1_SME_PFX_LEN + plen);
}

static bool rf_2a4m1_ft_locate(const uint8_t *body, size_t len, uint16_t *mdid,
                      const uint8_t **mde, size_t *mde_len,
                      const uint8_t **fte, size_t *fte_len,
                      const uint8_t **trailing, size_t *trailing_len)
{
	uint8_t pol;
	if (len < RF_2A4M1_FT_MDE_LEN || !rf_2a4m1_ft_mde_parse(body, len, mdid, &pol))
		return false;
	const uint8_t *fp = body + RF_2A4M1_FT_MDE_LEN;
	size_t rem = len - RF_2A4M1_FT_MDE_LEN;
	if (rem < 2)
		return false;
	size_t felen = (size_t)2 + fp[1];
	if (felen > rem)
		return false;
	*mde = body; *mde_len = RF_2A4M1_FT_MDE_LEN;
	*fte = fp;   *fte_len = felen;
	*trailing = fp + felen; *trailing_len = rem - felen;
	return true;
}

static size_t rf_2a4m1_ft_build_mde_fte(struct rf_2a4m1_sme *s, uint8_t *buf, size_t cap, bool set_a, bool set_s,
                               const rf_2a4m1_mac_addr *r1kh, size_t *fte_off, size_t *mde_len)
{
	size_t n = rf_2a4m1_ft_mde_build(buf, cap, s->ft_mdid, RF_2A4M1_FT_MDE_CAP_FT_OVER_DS);
	if (n == 0)
		return 0;
	*mde_len = n;
	*fte_off = n;
	struct rf_2a4m1_ft_fte f;
	rf_2a4m1_ft_fill_fte(s, &f, set_a, set_s, r1kh);
	size_t fn = rf_2a4m1_ft_fte_build(&buf[n], cap - n, &f);
	if (fn == 0)
		return 0;
	return n + fn;
}

static size_t rf_2a4m1_ft_build_reassoc_req_extra(struct rf_2a4m1_sme *s, uint8_t *out, size_t cap)
{
	size_t n = 0;
	if (s->ocv_enabled) {
		size_t o = rf_2a4m1_pmf_oci_element_build(&out[n], cap - n, &s->oci_local);
		if (o == 0)
			return n;
		n += o;
	}
	if (s->ric_request_on) {
		size_t r = rf_2a4m1_ric_request_build(&out[n], cap - n, /*rdie_id=*/1, &s->ric_req, 1);
		if (r == 0)
			return n;
		n += r;
	}
	return n;
}

static void rf_2a4m1_ft_tx_auth1(struct rf_2a4m1_sme *s)
{
	uint8_t buf[220];
	size_t fte_off = 0, mde_len = 0;
	size_t n = rf_2a4m1_ft_build_mde_fte(s, buf, sizeof buf, false, true, NULL, &fte_off, &mde_len);
	if (n == 0 || n + RF_2A4M1_FT_KEY_NAME_LEN > sizeof buf)
		return;
	memcpy(&buf[n], s->ft_pmk_r0_name, RF_2A4M1_FT_KEY_NAME_LEN);
	n += RF_2A4M1_FT_KEY_NAME_LEN;
	rf_2a4m1_ft_tx(s, RF_2A4M1_F_FT_AUTH1, buf, n);
}

static void rf_2a4m1_ft_tx_auth2(struct rf_2a4m1_sme *s)
{
	uint8_t buf[220];
	size_t fte_off = 0, mde_len = 0;
	size_t n = rf_2a4m1_ft_build_mde_fte(s, buf, sizeof buf, true, true, &s->self, &fte_off, &mde_len);
	if (n == 0)
		return;
	rf_2a4m1_ft_tx(s, RF_2A4M1_F_FT_AUTH2, buf, n);
}

static void rf_2a4m1_ft_tx_reassoc(struct rf_2a4m1_sme *s, uint8_t type, uint8_t seq, const rf_2a4m1_mac_addr *r1kh,
                          const uint8_t *extra, size_t extra_len)
{
	uint8_t buf[300];
	size_t fte_off = 0, mde_len = 0;
	size_t n = rf_2a4m1_ft_build_mde_fte(s, buf, sizeof buf, true, true, r1kh, &fte_off, &mde_len);
	if (n == 0)
		return;
	size_t fte_len = n - fte_off;
	if (n + RF_2A4M1_FT_KEY_NAME_LEN + extra_len > sizeof buf)
		return;
	/* the STA is the S1KH; the AP address is the R1KH (target on the STA, self on the AP). */
	const rf_2a4m1_mac_addr *sta = (s->role == RF_2A4M1_SME_ROLE_STA) ? &s->self : &s->peer;
	uint8_t mic[RF_2A4M1_FT_FTE_MIC_LEN];
	/* the FTE MIC MUST also cover the trailing protected elements (the OCI on the Req, the
	 * KEK-wrapped GTK on the Resp) so an on-path attacker cannot rewrite them (OCV integrity). */
	rf_2a4m1_ft_fte_mic(&s->ptk[RF_2A4M1_SME_KCK_OFF], sta, r1kh, seq, &buf[0], mde_len,
	           &buf[fte_off], fte_len, s->ft_pmk_r1_name, extra, extra_len, mic);
	memcpy(&buf[fte_off + 4], mic, RF_2A4M1_FT_FTE_MIC_LEN);   /* FTE MIC field: EID(0)Len(1)Ctl(2,3)MIC(4..) */
	memcpy(&buf[n], s->ft_pmk_r1_name, RF_2A4M1_FT_KEY_NAME_LEN);
	n += RF_2A4M1_FT_KEY_NAME_LEN;
	if (extra_len) {
		memcpy(&buf[n], extra, extra_len);
		n += extra_len;
	}
	rf_2a4m1_ft_tx(s, type, buf, n);
}

static void rf_2a4m1_ft_ap_on_auth1(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *src, const uint8_t *body, size_t len)
{
	uint16_t mdid; const uint8_t *mde, *fte, *tr; size_t mdel, ftel, trl;
	if (!rf_2a4m1_ft_locate(body, len, &mdid, &mde, &mdel, &fte, &ftel, &tr, &trl))
		return;
	if (mdid != s->ft_mdid || trl < RF_2A4M1_FT_KEY_NAME_LEN)
		return;
	struct rf_2a4m1_ft_fte f;
	if (!rf_2a4m1_ft_fte_parse(fte, ftel, &f))
		return;
	s->peer = *src;                                   /* S0KH/S1KH = the STA */
	memcpy(s->ft_snonce, f.snonce, RF_2A4M1_FT_NONCE_LEN);
	uint8_t mdid_le[2] = { (uint8_t)s->ft_mdid, (uint8_t)(s->ft_mdid >> 8) };
	if (!rf_2a4m1_ft_derive_pmk_r0(s->pmk, RF_2A4M1_SME_PMK_LEN, s->ft_ssid, s->ft_ssid_len, mdid_le,
	                      s->ft_r0kh_id, s->ft_r0kh_len, &s->peer,
	                      s->ft_pmk_r0, s->ft_pmk_r0_name))
		return;
	s->ft_r0_valid = true;
	if (!rf_2a4m1_bytes_eq(s->ft_pmk_r0_name, tr, RF_2A4M1_FT_KEY_NAME_LEN))
		return;                                       /* PMKR0Name mismatch (wrong MD/key) — reject */
	if (!rf_2a4m1_ft_derive_pmk_r1(s->ft_pmk_r0, s->ft_pmk_r0_name, &s->self, &s->peer,
	                      s->ft_pmk_r1, s->ft_pmk_r1_name))
		return;
	rf_2a4m1_ft_gen_nonce32(s, s->ft_anonce, s->nonce_salt + 10);
	if (!rf_2a4m1_ft_derive_ptk(s->ft_pmk_r1, s->ft_pmk_r1_name, s->ft_snonce, s->ft_anonce,
	                   &s->self, &s->peer, s->ptk, s->ft_ptk_name))
		return;
	rf_2a4m1_ft_tx_auth2(s);
}

static void rf_2a4m1_ft_ap_on_reassoc(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len)
{
	uint16_t mdid; const uint8_t *mde, *fte, *tr; size_t mdel, ftel, trl;
	if (!rf_2a4m1_ft_locate(body, len, &mdid, &mde, &mdel, &fte, &ftel, &tr, &trl))
		return;
	if (mdid != s->ft_mdid || trl < RF_2A4M1_FT_KEY_NAME_LEN)
		return;
	struct rf_2a4m1_ft_fte f;
	if (!rf_2a4m1_ft_fte_parse(fte, ftel, &f))
		return;
	if (!rf_2a4m1_bytes_eq(tr, s->ft_pmk_r1_name, RF_2A4M1_FT_KEY_NAME_LEN))
		return;                                       /* wrong PMKR1Name — reject */
	uint8_t mic[RF_2A4M1_FT_FTE_MIC_LEN];
	/* verify over MDE||FTE||PMKR1Name||(trailing OCI) — the trailing OCI is now MIC-covered, so a
	 * rewritten OCI (OCV bypass) fails the MIC here before OCV even runs. */
	rf_2a4m1_ft_fte_mic(&s->ptk[RF_2A4M1_SME_KCK_OFF], &s->peer, &s->self, 5, mde, mdel, fte, ftel,
	           s->ft_pmk_r1_name, tr + RF_2A4M1_FT_KEY_NAME_LEN, trl - RF_2A4M1_FT_KEY_NAME_LEN, mic);
	if (!rf_2a4m1_bytes_eq(mic, f.mic, RF_2A4M1_FT_FTE_MIC_LEN)) { s->bad_mic++; return; }   /* FTE MIC fail */
	/* OCV (N23): validate the STA's OCI element (trailing the PMKR1Name) against our channel. */
	if (s->ocv_enabled) {
		struct rf_2a4m1_pmf_oci sta_oci;
		if (!rf_2a4m1_pmf_oci_element_parse(tr + RF_2A4M1_FT_KEY_NAME_LEN, trl - RF_2A4M1_FT_KEY_NAME_LEN, &sta_oci) ||
		    !rf_2a4m1_pmf_ocv_verify(&sta_oci, &s->oci_local)) {
			s->ocv_fail++;
			return;                                   /* channel mismatch — reject the reassoc */
		}
	}
	/* RIC (resource reservation): if the STA's Reassoc-Req carried a RIC-Request (an RDE, EID 57 —
	 * possibly after the OCI element) in the protected trailing, grant/deny each resource and answer
	 * with a RIC-Response inside the Reassoc-Resp. Scan the trailing-after-name for the RDE. */
	uint8_t ric_resp[128]; size_t ric_resp_len = 0;
	{
		const uint8_t *region = tr + RF_2A4M1_FT_KEY_NAME_LEN;
		size_t region_len = trl - RF_2A4M1_FT_KEY_NAME_LEN;
		size_t pos = 0;
		while (pos + 2 <= region_len) {
			uint8_t eid = region[pos], elen = region[pos + 1];
			if ((size_t)(2 + elen) > region_len - pos)
				break;
			if (eid == RF_2A4M1_RIC_EID_RDE) {
				struct rf_2a4m1_ric_request rq;
				if (rf_2a4m1_ric_request_parse(&region[pos], region_len - pos, &rq) &&
				    rq.count >= 1 && rq.count <= RF_2A4M1_RIC_MAX_RESOURCES) {
					uint16_t st[RF_2A4M1_RIC_MAX_RESOURCES];
					for (size_t i = 0; i < rq.count && i < RF_2A4M1_RIC_MAX_RESOURCES; i++) {
						st[i] = s->ric_ap_deny ? RF_2A4M1_RIC_STATUS_DENIED : RF_2A4M1_RIC_STATUS_SUCCESS;
						if (s->ric_ap_deny) s->ric_ap_denials++; else s->ric_ap_grants++;
					}
					ric_resp_len = rf_2a4m1_ric_response_build(ric_resp, sizeof ric_resp, rq.rdie_id,
					                                  rq.rf_2a4m1_tspec, st, rq.count);
				}
				break;
			}
			pos += (size_t)2 + elen;
		}
	}

	/* install keys WITHOUT a 4-way: generate + wrap the GTK, send the Reassoc-Resp. */
	rf_2a4m1_sme_gen_material(s, s->gtk, RF_2A4M1_SME_GTK_LEN, s->nonce_salt + 21);
	uint8_t kde[RF_2A4M1_SME_KDE_MAX];
	size_t kl = rf_2a4m1_gtk_kde_build(kde, s->gtk, 1);
	uint8_t wrapped[RF_2A4M1_SME_KDE_MAX];
	size_t wl = s->crypto->key_wrap(wrapped, sizeof wrapped, &s->ptk[RF_2A4M1_SME_KEK_OFF], kde, kl);
	rf_2a4m1_crypto_wipe(kde, sizeof kde);
	if (wl == 0)
		return;
	/* the Reassoc-Resp trailing after the PMKR1Name = [RIC-Response (if any)][KEK-wrapped GTK] —
	 * both inside the FTE-MIC scope. The STA knows whether a RIC-Response leads (it sent the
	 * RIC-Request), so the wrapped-GTK "rest" is located unambiguously. */
	uint8_t respextra[RF_2A4M1_SME_KDE_MAX + 160];
	size_t re = 0;
	if (ric_resp_len) { memcpy(&respextra[re], ric_resp, ric_resp_len); re += ric_resp_len; }
	memcpy(&respextra[re], wrapped, wl); re += wl;
	rf_2a4m1_ft_tx_reassoc(s, RF_2A4M1_F_FT_REASSOC_RESP, 6, &s->self, respextra, re);
	rf_2a4m1_crypto_wipe(wrapped, sizeof wrapped);
	s->ptk_installed = s->gtk_installed = true;
	rf_2a4m1_sme_install_data_key(s);                          /* KRACK: keep RX PN state on a same-TK reinstall */
	s->connected = true;
	s->ft_completed = true;
	s->ft_reassocs++;
	s->state = RF_2A4M1_SME_CONNECTED;
}

static void rf_2a4m1_ft_sta_on_auth2(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len)
{
	uint16_t mdid; const uint8_t *mde, *fte, *tr; size_t mdel, ftel, trl;
	if (!rf_2a4m1_ft_locate(body, len, &mdid, &mde, &mdel, &fte, &ftel, &tr, &trl))
		return;
	if (mdid != s->ft_mdid)
		return;
	struct rf_2a4m1_ft_fte f;
	if (!rf_2a4m1_ft_fte_parse(fte, ftel, &f))
		return;
	if (!rf_2a4m1_bytes_eq(f.snonce, s->ft_snonce, RF_2A4M1_FT_NONCE_LEN))
		return;                                       /* the AP must echo our SNonce */
	memcpy(s->ft_anonce, f.anonce, RF_2A4M1_FT_NONCE_LEN);
	if (!rf_2a4m1_ft_derive_pmk_r1(s->ft_pmk_r0, s->ft_pmk_r0_name, &s->ft_target, &s->self,
	                      s->ft_pmk_r1, s->ft_pmk_r1_name))
		return;
	if (!rf_2a4m1_ft_derive_ptk(s->ft_pmk_r1, s->ft_pmk_r1_name, s->ft_snonce, s->ft_anonce,
	                   &s->ft_target, &s->self, s->ptk, s->ft_ptk_name))
		return;
	/* Carry the OCI element (OCV, N23) + a RIC-Request (resource reservation) in the FT Reassoc-Req
	 * — both inside the FTE-MIC scope. */
	uint8_t extra[220];
	size_t extra_len = rf_2a4m1_ft_build_reassoc_req_extra(s, extra, sizeof extra);
	rf_2a4m1_ft_tx_reassoc(s, RF_2A4M1_F_FT_REASSOC_REQ, 5, &s->ft_target, extra_len ? extra : NULL, extra_len);
}

static void rf_2a4m1_ft_sta_on_reassoc(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len)
{
	uint16_t mdid; const uint8_t *mde, *fte, *tr; size_t mdel, ftel, trl;
	if (!rf_2a4m1_ft_locate(body, len, &mdid, &mde, &mdel, &fte, &ftel, &tr, &trl))
		return;
	if (mdid != s->ft_mdid || trl < RF_2A4M1_FT_KEY_NAME_LEN)
		return;
	struct rf_2a4m1_ft_fte f;
	if (!rf_2a4m1_ft_fte_parse(fte, ftel, &f))
		return;
	uint8_t mic[RF_2A4M1_FT_FTE_MIC_LEN];
	/* verify over MDE||FTE||PMKR1Name||(trailing KEK-wrapped GTK) — the trailing bytes are now
	 * MIC-covered, so tampering the delivered GTK is caught here too. */
	rf_2a4m1_ft_fte_mic(&s->ptk[RF_2A4M1_SME_KCK_OFF], &s->self, &s->ft_target, 6, mde, mdel, fte, ftel,
	           s->ft_pmk_r1_name, tr + RF_2A4M1_FT_KEY_NAME_LEN, trl - RF_2A4M1_FT_KEY_NAME_LEN, mic);
	if (!rf_2a4m1_bytes_eq(mic, f.mic, RF_2A4M1_FT_FTE_MIC_LEN)) { s->bad_mic++; return; }
	/* the trailing after the PMKR1Name = [RIC-Response (iff we sent a RIC-Request)][KEK-wrapped GTK].
	 * Consume the RIC-Response first (it is self-delimiting), then the wrapped GTK is the rest. */
	const uint8_t *region = tr + RF_2A4M1_FT_KEY_NAME_LEN;
	size_t region_len = trl - RF_2A4M1_FT_KEY_NAME_LEN;
	size_t ric_consumed = 0;
	if (s->ric_request_on) {
		struct rf_2a4m1_ric_response rr;
		if (rf_2a4m1_ric_response_parse(region, region_len, &rr, &ric_consumed) && rr.count >= 1) {
			s->ric_resp_received = true;
			s->ric_status  = rr.status[0];
			s->ric_granted = rr.granted[0];
			if (rr.granted[0])
				s->ric_granted_tspec = rr.rf_2a4m1_tspec[0];
		} else {
			ric_consumed = 0;
		}
	}
	const uint8_t *wrapped = region + ric_consumed;
	size_t wlen = region_len - ric_consumed;
	uint8_t kde[RF_2A4M1_SME_KDE_MAX];
	size_t kl = s->crypto->key_unwrap(kde, sizeof kde, &s->ptk[RF_2A4M1_SME_KEK_OFF], wrapped, wlen);
	if (kl == 0) { s->bad_mic++; return; }
	uint8_t gtk[RF_2A4M1_SME_GTK_LEN];
	bool gtk_ok = rf_2a4m1_gtk_kde_parse(kde, kl, gtk);
	rf_2a4m1_crypto_wipe(kde, sizeof kde);
	if (!gtk_ok) { rf_2a4m1_crypto_wipe(gtk, sizeof gtk); return; }
	memcpy(s->gtk, gtk, RF_2A4M1_SME_GTK_LEN);
	rf_2a4m1_crypto_wipe(gtk, sizeof gtk);
	s->peer = s->ft_target;                           /* switch to the new AP; PTK already in s->ptk */
	s->ptk_installed = s->gtk_installed = true;
	rf_2a4m1_sme_install_data_key(s);                          /* KRACK: keep RX PN state on a same-TK reinstall */
	s->connected = true;
	s->ft_completed = true;
	s->ft_reassocs++;
	s->state = RF_2A4M1_SME_CONNECTED;
}

static size_t rf_2a4m1_ft_ds_target_process(struct rf_2a4m1_sme *t, const rf_2a4m1_mac_addr *sta, const rf_2a4m1_mac_addr *target,
                                   const uint8_t snonce[RF_2A4M1_FT_NONCE_LEN],
                                   const uint8_t pmk_r1[RF_2A4M1_FT_PMK_R1_LEN],
                                   const uint8_t pmk_r1_name[RF_2A4M1_FT_KEY_NAME_LEN],
                                   uint8_t *resp_out, size_t resp_cap)
{
	if (!t->ft_enabled || !rf_2a4m1_bytes_eq(target->a, t->self.a, RF_2A4M1_ETH_ALEN))
		return 0;                                     /* the request must target THIS AP */
	t->peer = *sta;                                       /* learn the STA (S1KH) */
	memcpy(t->ft_snonce, snonce, RF_2A4M1_FT_NONCE_LEN);
	memcpy(t->ft_pmk_r1, pmk_r1, RF_2A4M1_FT_PMK_R1_LEN);          /* PMK-R1 PUSHED over the DS — not rederived */
	memcpy(t->ft_pmk_r1_name, pmk_r1_name, RF_2A4M1_FT_KEY_NAME_LEN);
	rf_2a4m1_ft_gen_nonce32(t, t->ft_anonce, t->nonce_salt + 12);
	if (!rf_2a4m1_ft_derive_ptk(t->ft_pmk_r1, t->ft_pmk_r1_name, t->ft_snonce, t->ft_anonce,
	                   &t->self, &t->peer, t->ptk, t->ft_ptk_name))
		return 0;
	/* build the FT Response: MDE + FTE(ANonce, SNonce, R1KH=self, R0KH) + PMKR1Name. */
	uint8_t elems[220];
	size_t fte_off = 0, mde_len = 0;
	size_t n = rf_2a4m1_ft_build_mde_fte(t, elems, sizeof elems, true, true, &t->self, &fte_off, &mde_len);
	if (n == 0 || n + RF_2A4M1_FT_KEY_NAME_LEN > sizeof elems)
		return 0;
	memcpy(&elems[n], t->ft_pmk_r1_name, RF_2A4M1_FT_KEY_NAME_LEN);
	n += RF_2A4M1_FT_KEY_NAME_LEN;
	return rf_2a4m1_ft_action_resp_build(resp_out, resp_cap, sta, &t->self, /*status=*/0, elems, n);
}

static void rf_2a4m1_ft_ap_on_ds_request(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len)
{
	struct rf_2a4m1_ft_action_req req;
	if (!rf_2a4m1_ft_action_req_parse(body, len, &req))
		return;
	uint16_t mdid; const uint8_t *mde, *fte, *tr; size_t mdel, ftel, trl;
	if (!rf_2a4m1_ft_locate(req.elems, req.elems_len, &mdid, &mde, &mdel, &fte, &ftel, &tr, &trl))
		return;
	if (mdid != s->ft_mdid || trl < RF_2A4M1_FT_KEY_NAME_LEN || !s->ft_r0_valid || !s->ft_ds_target)
		return;
	struct rf_2a4m1_ft_fte f;
	if (!rf_2a4m1_ft_fte_parse(fte, ftel, &f))
		return;
	if (!rf_2a4m1_bytes_eq(tr, s->ft_pmk_r0_name, RF_2A4M1_FT_KEY_NAME_LEN))
		return;                                       /* wrong PMKR0Name (wrong MD/key) — do not relay */
	/* R0KH derives PMK-R1 for the target R1KH-ID (= req.target_addr) + S1KH (= the STA) — the DS push. */
	uint8_t pmk_r1[RF_2A4M1_FT_PMK_R1_LEN], pmk_r1_name[RF_2A4M1_FT_KEY_NAME_LEN];
	if (!rf_2a4m1_ft_derive_pmk_r1(s->ft_pmk_r0, s->ft_pmk_r0_name, &req.target_addr, &req.sta_addr,
	                      pmk_r1, pmk_r1_name)) {
		rf_2a4m1_crypto_wipe(pmk_r1, sizeof pmk_r1);
		return;
	}
	uint8_t resp[260];
	size_t rlen = rf_2a4m1_ft_ds_target_process(s->ft_ds_target, &req.sta_addr, &req.target_addr, f.snonce,
	                                   pmk_r1, pmk_r1_name, resp, sizeof resp);
	rf_2a4m1_crypto_wipe(pmk_r1, sizeof pmk_r1);
	if (rlen == 0)
		return;
	rf_2a4m1_ft_tx(s, RF_2A4M1_F_FT_DS_RESP, resp, rlen);               /* forward the FT Response to the STA over the air */
}

static void rf_2a4m1_ft_tx_ds_request(struct rf_2a4m1_sme *s)
{
	uint8_t elems[220];
	size_t fte_off = 0, mde_len = 0;
	size_t n = rf_2a4m1_ft_build_mde_fte(s, elems, sizeof elems, false, true, NULL, &fte_off, &mde_len);
	if (n == 0 || n + RF_2A4M1_FT_KEY_NAME_LEN > sizeof elems)
		return;
	memcpy(&elems[n], s->ft_pmk_r0_name, RF_2A4M1_FT_KEY_NAME_LEN);
	n += RF_2A4M1_FT_KEY_NAME_LEN;
	uint8_t f[RF_2A4M1_SME_PFX_LEN + 280];
	size_t body = rf_2a4m1_ft_action_req_build(&f[RF_2A4M1_SME_PFX_LEN], sizeof f - RF_2A4M1_SME_PFX_LEN,
	                                  &s->self, &s->ft_target, elems, n);
	if (body == 0)
		return;
	f[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(RF_2A4M1_F_FT_DS_REQ);
	memcpy(&f[RF_2A4M1_SME_PFX_ADDR], s->self.a, RF_2A4M1_ETH_ALEN);
	rf_2a4m1_tx_raw(s, f, RF_2A4M1_SME_PFX_LEN + body);                 /* to the current AP (peer) over the air */
}

static void rf_2a4m1_ft_sta_on_ds_response(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len)
{
	struct rf_2a4m1_ft_action_resp resp;
	if (!rf_2a4m1_ft_action_resp_parse(body, len, &resp) || resp.status != 0)
		return;
	uint16_t mdid; const uint8_t *mde, *fte, *tr; size_t mdel, ftel, trl;
	if (!rf_2a4m1_ft_locate(resp.elems, resp.elems_len, &mdid, &mde, &mdel, &fte, &ftel, &tr, &trl))
		return;
	if (mdid != s->ft_mdid)
		return;
	struct rf_2a4m1_ft_fte f;
	if (!rf_2a4m1_ft_fte_parse(fte, ftel, &f))
		return;
	if (!rf_2a4m1_bytes_eq(f.snonce, s->ft_snonce, RF_2A4M1_FT_NONCE_LEN))
		return;                                       /* the target must echo our SNonce */
	memcpy(s->ft_anonce, f.anonce, RF_2A4M1_FT_NONCE_LEN);
	if (!rf_2a4m1_ft_derive_pmk_r1(s->ft_pmk_r0, s->ft_pmk_r0_name, &s->ft_target, &s->self,
	                      s->ft_pmk_r1, s->ft_pmk_r1_name))
		return;
	if (!rf_2a4m1_ft_derive_ptk(s->ft_pmk_r1, s->ft_pmk_r1_name, s->ft_snonce, s->ft_anonce,
	                   &s->ft_target, &s->self, s->ptk, s->ft_ptk_name))
		return;
	/* reuse the over-air FT Reassociation at the target (OCI + RIC ride the same extra). */
	uint8_t extra[220];
	size_t extra_len = rf_2a4m1_ft_build_reassoc_req_extra(s, extra, sizeof extra);
	rf_2a4m1_ft_tx_reassoc(s, RF_2A4M1_F_FT_REASSOC_REQ, 5, &s->ft_target, extra_len ? extra : NULL, extra_len);
}

static void rf_2a4m1_ba_ap_on_addba_req(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len)
{
	uint8_t dt, tid; bool amsdu, immediate; uint16_t bufsz, timeout, ssn;
	uint8_t rsp[16];
	size_t n;
	if (!rf_2a4m1_ba_parse_addba_req(body, len, &dt, &tid, &amsdu, &immediate, &bufsz, &timeout, &ssn))
		return;
	s->ba_recipient     = true;
	s->ba_recip_tid     = tid;
	s->ba_recip_bufsize = bufsz;
	n = rf_2a4m1_ba_build_addba_rsp(rsp, sizeof rsp, dt, 0 /* success */,
	                       tid, amsdu, immediate, bufsz, timeout);
	if (n == 0)
		return;
	rf_2a4m1_tx_action_dot11(s, rsp, n);   /* real 802.11 Action ADDBA Response */
}

static void rf_2a4m1_addts_ap_on_req(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len)
{
	uint8_t dialog; struct rf_2a4m1_tspec req;
	if (!rf_2a4m1_addts_req_parse(body, len, &dialog, &req))
		return;
	enum rf_2a4m1_qos_admit_result r = rf_2a4m1_qos_admission_admit_tspec(&s->admission, &req);
	uint16_t status;
	if (r == RF_2A4M1_QOS_ADMIT_OK) {
		status = RF_2A4M1_SME_ADDTS_STATUS_ADMITTED;
		s->addts_grants++;
	} else {
		status = RF_2A4M1_SME_ADDTS_STATUS_REFUSED;
		req.medium_time = 0;              /* refused: the granted TSPEC carries zero medium time */
		s->addts_denials++;
	}
	uint8_t f[RF_2A4M1_SME_PFX_LEN + 5 + 2 + RF_2A4M1_TSPEC_BODY_LEN];
	f[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(RF_2A4M1_F_ADDTS_RSP);
	memcpy(&f[RF_2A4M1_SME_PFX_ADDR], s->self.a, RF_2A4M1_ETH_ALEN);
	size_t n = rf_2a4m1_addts_rsp_build(&f[RF_2A4M1_SME_PFX_LEN], sizeof f - RF_2A4M1_SME_PFX_LEN, dialog, status, &req);
	if (n == 0)
		return;
	rf_2a4m1_tx_raw(s, f, RF_2A4M1_SME_PFX_LEN + n);
}

static void rf_2a4m1_addts_sta_on_resp(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len)
{
	uint16_t status; struct rf_2a4m1_tspec granted;
	if (!rf_2a4m1_addts_rsp_parse(body, len, NULL, &status, &granted))
		return;
	s->addts_resp_received = true;
	s->addts_last_status = status;
	if (status == RF_2A4M1_SME_ADDTS_STATUS_ADMITTED)
		rf_2a4m1_qos_admission_admit_tspec(&s->admission, &granted);   /* record the admitted medium time */
}

static void rf_2a4m1_addts_on_delts(struct rf_2a4m1_sme *s, const uint8_t *body, size_t len)
{
	struct rf_2a4m1_ts_info ts;
	if (!rf_2a4m1_delts_parse(body, len, &ts, NULL))
		return;
	rf_2a4m1_qos_admission_release_ts(&s->admission, &ts);
}

void rf_2a4m1_sme_init(struct rf_2a4m1_sme *s, enum rf_2a4m1_sme_role role, const rf_2a4m1_mac_addr *self,
              const uint8_t pmk[RF_2A4M1_SME_PMK_LEN], const struct rf_2a4m1_sme_crypto *crypto,
              uint64_t nonce_salt)
{
	memset(s, 0, sizeof(*s));
	s->role = (uint8_t)role;
	s->state = RF_2A4M1_SME_INIT;
	s->self = *self;
	memcpy(s->pmk, pmk, RF_2A4M1_SME_PMK_LEN);
	s->crypto = crypto;
	s->nonce_salt = nonce_salt;
	s->qos_advertise = true;   /* announce WMM/QoS by default (real STAs/APs do) */
	rf_2a4m1_qos_admission_init(&s->admission);   /* zero the admitted-TS table + set per-AC budgets to default */
	/* PMF SA Query (N21): the AP's spurious-(Re)Assoc transaction context — up to 3 Requests spaced
	 * one caller tick apart before the transaction times out (the peer is declared gone). */
	rf_2a4m1_pmf_sa_query_init(&s->sa_query, /*max_retries=*/3, /*timeout_ticks=*/1);
	/* announce each compiled-in Wi-Fi generation's Capabilities by default (N13-N16), so a target
	 * with the gen enabled advertises it out of the box — mirroring s->qos_advertise. */
	s->ht_advertise = true;
}

void rf_2a4m1_sme_set_peer(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *peer) { s->peer = *peer; }

static void rf_2a4m1_sme_hal_rx(struct rf_2a4m1_hal *h, const struct rf_2a4m1_rxinfo *rx, void *ctx)
{
	(void)h;
	rf_2a4m1_sme_rx((struct rf_2a4m1_sme *)ctx, rx->data, rx->len);
}

void rf_2a4m1_sme_bind_hal(struct rf_2a4m1_sme *s, struct rf_2a4m1_hal *rf_2a4m1_hal)
{
	s->rf_2a4m1_hal = rf_2a4m1_hal;
	struct rf_2a4m1_hal_cfg cfg = { 0 };
	cfg.addr = s->self;
	cfg.opmode = (s->role == RF_2A4M1_SME_ROLE_AP) ? RF_2A4M1_HAL_OPMODE_AP : RF_2A4M1_HAL_OPMODE_STA;
	cfg.rx_cb = rf_2a4m1_sme_hal_rx;
	cfg.rx_ctx = s;
	rf_2a4m1_hal_start(rf_2a4m1_hal, &cfg);              /* registers sme_hal_rx as the HAL's RX callback */
}

rf_2a4m1_s8021x_status rf_2a4m1_sme_connect_start(struct rf_2a4m1_sme *s)
{
	static const rf_2a4m1_mac_addr rf_2a4m1_bcast = {{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }};
	if (s->role != RF_2A4M1_SME_ROLE_STA)
		return RF_2A4M1_S8021X_ERR_INVAL;
	s->state = RF_2A4M1_SME_SCANNING;
	/* A BSSID requested through sme_set_peer() gets a DIRECTED probe; with none requested we
	 * scan with the wildcard (broadcast) probe every AP answers. Either way a Beacon from the
	 * BSS is now an equally good way to leave SCANNING (rx_bss_announce). */
	rf_2a4m1_tx_mgmt_to(s, RF_2A4M1_F_PROBE_REQ, rf_2a4m1_mac_is_zero(&s->peer) ? &rf_2a4m1_bcast : &s->peer, NULL, 0);
	return RF_2A4M1_S8021X_OK;
}

static bool rf_2a4m1_sme_sae_tx_commit(struct rf_2a4m1_sme *s)
{
	uint8_t pt[RF_2A4M1_P256_POINT_LEN], rnd[RF_2A4M1_SAE_KEY_LEN], mask[RF_2A4M1_SAE_KEY_LEN];
	uint8_t f[256];
	size_t n;
	int attempt;

	if (!s->sae_pw_len || !s->ssid_len)
		return false;
	if (!rf_2a4m1_sae_derive_pt_h2e(pt, s->ssid, s->ssid_len, s->sae_pw, s->sae_pw_len, NULL, 0))
		return false;
	for (attempt = 0; attempt < 16; attempt++) {
		/* Two full nonces as rand/mask (freestanding CSPRNG via crypto plug). */
		s->crypto->gen_nonce(rnd, s->pmk, &s->self, s->nonce_salt + (uint64_t)attempt * 2);
		s->crypto->gen_nonce(mask, s->pmk, &s->self,
		                     s->nonce_salt + (uint64_t)attempt * 2 + 1);
		if (rf_2a4m1_sae_commit(&s->rf_2a4m1_sae, pt, &s->self, &s->peer, rnd, mask))
			break;
	}
	rf_2a4m1_crypto_wipe(pt, sizeof pt);
	rf_2a4m1_crypto_wipe(rnd, sizeof rnd);
	rf_2a4m1_crypto_wipe(mask, sizeof mask);
	if (attempt >= 16)
		return false;
	n = rf_2a4m1_mgmt_build_auth_sae_commit(f, sizeof f, &s->peer, &s->self, rf_2a4m1_sme_next_seq(s),
	                               RF_2A4M1_WLAN_STATUS_SAE_HASH_TO_ELEMENT, RF_2A4M1_SAE_GROUP_19,
	                               s->rf_2a4m1_sae.commit_scalar, RF_2A4M1_SAE_KEY_LEN,
	                               s->rf_2a4m1_sae.commit_element, RF_2A4M1_P256_POINT_LEN);
	if (!n)
		return false;
	rf_2a4m1_tx_raw(s, f, n);
	s->sae_phase = 1;
	s->sae_commit_tx++;
	return true;
}

static bool rf_2a4m1_sme_sae_tx_confirm(struct rf_2a4m1_sme *s)
{
	uint8_t confirm[RF_2A4M1_SAE_KEY_LEN], f[128];
	size_t n;

	rf_2a4m1_sae_confirm(&s->rf_2a4m1_sae, 1, confirm);
	n = rf_2a4m1_mgmt_build_auth_sae_confirm(f, sizeof f, &s->peer, &s->self, rf_2a4m1_sme_next_seq(s),
	                                1, confirm, RF_2A4M1_SAE_KEY_LEN);
	rf_2a4m1_crypto_wipe(confirm, sizeof confirm);
	if (!n)
		return false;
	rf_2a4m1_tx_raw(s, f, n);
	s->sae_phase = 2;
	s->sae_confirm_tx++;
	return true;
}

static void rf_2a4m1_sme_sae_enter_authed(struct rf_2a4m1_sme *s)
{
	memcpy(s->pmk, s->rf_2a4m1_sae.pmk, RF_2A4M1_SME_PMK_LEN);
	s->sae_ok = true;
	s->sae_phase = 3;
	s->sae_confirm_rx++;
	rf_2a4m1_tx_assoc_req(s);
	s->state = RF_2A4M1_SME_AUTHED;
}

static void rf_2a4m1_sme_sae_rx_auth(struct rf_2a4m1_sme *s, const struct rf_2a4m1_mgmt_hdr *h)
{
	uint16_t algo = 0, txn = 0, status = 0;

	if (s->role != RF_2A4M1_SME_ROLE_STA || s->state != RF_2A4M1_SME_SCANNING)
		return;
	if (!rf_2a4m1_mgmt_parse_auth(h->body, h->body_len, &algo, &txn, &status))
		return;
	if (algo != RF_2A4M1_WLAN_AUTH_SAE)
		return;
	if (txn == 1 && s->sae_phase == 1) {
		uint16_t group = 0;
		const uint8_t *pscalar = NULL, *pelement = NULL;
		if (status != RF_2A4M1_WLAN_STATUS_SUCCESS &&
		    status != RF_2A4M1_WLAN_STATUS_SAE_HASH_TO_ELEMENT)
			return;
		if (!rf_2a4m1_mgmt_parse_auth_sae_commit(h->body, h->body_len, &group,
		                                &pscalar, &pelement))
			return;
		s->sae_commit_rx++;
		if (group != RF_2A4M1_SAE_GROUP_19 ||
		    !rf_2a4m1_sae_process_commit(&s->rf_2a4m1_sae, pscalar, pelement)) {
			s->state = RF_2A4M1_SME_FAILED;
			return;
		}
		if (!rf_2a4m1_sme_sae_tx_confirm(s))
			s->state = RF_2A4M1_SME_FAILED;
		return;
	}
	if (txn == 2 && s->sae_phase == 2) {
		uint16_t peer_sc = 0;
		const uint8_t *pconfirm = NULL;
		if (status != RF_2A4M1_WLAN_STATUS_SUCCESS)
			return;
		if (!rf_2a4m1_mgmt_parse_auth_sae_confirm(h->body, h->body_len, &peer_sc,
		                                 &pconfirm) ||
		    !rf_2a4m1_sae_verify_confirm(&s->rf_2a4m1_sae, peer_sc, pconfirm)) {
			s->state = RF_2A4M1_SME_FAILED;
			return;
		}
		rf_2a4m1_sme_sae_enter_authed(s);
	}
}

static void rf_2a4m1_rx_bss_announce(struct rf_2a4m1_sme *s, const struct rf_2a4m1_mgmt_hdr *h)
{
	if (s->role != RF_2A4M1_SME_ROLE_STA || s->state != RF_2A4M1_SME_SCANNING)
		return;
	if (!rf_2a4m1_mac_is_zero(&s->peer) && !rf_2a4m1_bytes_eq(h->addr3.a, s->peer.a, RF_2A4M1_ETH_ALEN))
		return;                            /* not the BSS we were asked to join */
	struct rf_2a4m1_beacon_info bi;
	if (!rf_2a4m1_mgmt_parse_beacon(h->body, h->body_len, &bi))
		return;
	/* An SSID set through sme_set_ssid() is a filter; with none set, adopt the BSS's own SSID so
	 * the (Re)Assoc Request carries the SSID this AP actually advertises (an AP rejects an assoc
	 * whose SSID element does not match its own). */
	if (s->ssid_len) {
		if (bi.ssid_len != s->ssid_len || !rf_2a4m1_bytes_eq(bi.ssid, s->ssid, s->ssid_len))
			return;
	} else if (bi.ssid_len) {
		memcpy(s->ssid, bi.ssid, bi.ssid_len);
		s->ssid_len = bi.ssid_len;
	}
	if (bi.channel)
		s->channel = bi.channel;
	s->peer = h->addr3;                        /* the BSSID (== addr2 on a beacon/probe-resp) */
	/* Pre-negotiate MFP from the Beacon/Probe-Resp RSN (hostapd often only
	 * advertises MFPC there; Assoc-Resp may still carry the full RSNE but
	 * Beacon is the reliable source for optional-PMF APs). */
	if (s->rf_2a4m1_pmf_mode != RF_2A4M1_PMF_MODE_DISABLED && h->body_len > 12)
		(void)rf_2a4m1_pmf_negotiate_ies(s, h->body + 12, (size_t)h->body_len - 12);
	if (s->akm == RF_2A4M1_SME_AKM_SAE) {
		if (!rf_2a4m1_sme_sae_tx_commit(s))
			s->state = RF_2A4M1_SME_FAILED;
		return;
	}
	rf_2a4m1_tx_mgmt(s, RF_2A4M1_F_AUTH_REQ);                    /* BSS discovered: open-system authenticate */
}

static void rf_2a4m1_rx_dot11_mgmt(struct rf_2a4m1_sme *s, const uint8_t *in, size_t len, const struct rf_2a4m1_mgmt_hdr *h)
{
	uint8_t type;
	const uint8_t *ies;
	size_t ies_len;
	(void)in; (void)len;

	/* Deauth / Disassoc: BIP group-mgmt drop policy (not the connect ladder). */
	if (h->subtype == RF_2A4M1_PMF_STYPE_DEAUTH || h->subtype == RF_2A4M1_PMF_STYPE_DISASSOC) {
		if (len >= RF_2A4M1_BIP_MGMT_HDR_LEN)
			rf_2a4m1_rx_robust_mgmt(s, in, len);
		return;
	}
	/* Action: real 802.11 Action frames (BA / SA Query). Under MFP individual
	 * Action is SW-CCMP (Protected bit) — decrypt first, then demux by category.
	 * SoftMAC helpers no longer required for the --sme path. */
	if (h->subtype == RF_2A4M1_WLAN_FC_STYPE_ACTION || h->subtype == 14 /* Action No Ack */) {
		const uint8_t *abody = h->body;
		size_t ablen = h->body_len;
		uint8_t plain[128];
		uint8_t view[RF_2A4M1_SME_RX_VIEW_MAX];
		uint8_t cat, act;

		if (s->mfp_active && s->ptk_installed && (in[1] & 0x40) &&
		    len > 24 + 8 + 8) {
			/* Host-sim has no FCS; SoftMAC live RX often appends APP_FCS.
			 * Try full frame first, then len-4 (FCS strip) if MIC fails. */
			int bl = rf_2a4m1_ccmp_decrypt(in, len, 24, &s->ptk[RF_2A4M1_SME_TK_OFF],
			                      plain, sizeof plain, NULL);
			if (bl < 0 && len >= 28)
				bl = rf_2a4m1_ccmp_decrypt(in, len - 4, 24, &s->ptk[RF_2A4M1_SME_TK_OFF],
				                  plain, sizeof plain, NULL);
			if (bl < 0) {
				s->mgmt_prot_badmic++;
				return;
			}
			abody = plain;
			ablen = (size_t)bl;
		} else if (s->mfp_active && ablen >= 2 &&
		           rf_2a4m1_pmf_is_robust_mgmt(h->fc, abody, ablen) &&
		           !(in[1] & 0x40)) {
			/* Unprotected robust Action under MFP → drop. */
			if (!rf_2a4m1_pmf_rx_unprotected_ok(h->fc, abody, ablen, true)) {
				s->mgmt_unprot_dropped++;
				return;
			}
		}
		if (ablen < 2)
			return;
		cat = abody[0];
		act = abody[1];
		type = 0;
		if (cat == WLAN_ACTION_BLOCK_ACK) {
			if (act == WLAN_BA_ACTION_ADDBA_REQ)
				type = RF_2A4M1_F_ADDBA_REQ;
			else if (act == WLAN_BA_ACTION_ADDBA_RSP)
				type = RF_2A4M1_F_ADDBA_RSP;
		}
		if (cat == RF_2A4M1_PMF_ACTION_SA_QUERY) {
			if (act == RF_2A4M1_PMF_SA_QUERY_REQUEST)
				type = RF_2A4M1_F_SA_QUERY_REQ;
			else if (act == RF_2A4M1_PMF_SA_QUERY_RESPONSE)
				type = RF_2A4M1_F_SA_QUERY_RESP;
		}
		if (!type)
			return; /* unknown Action category — ignore */
		if (RF_2A4M1_SME_PFX_LEN + ablen > sizeof view)
			return;
		memset(view, 0, sizeof view);
		view[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(type);
		memcpy(&view[RF_2A4M1_SME_PFX_ADDR], h->addr2.a, RF_2A4M1_ETH_ALEN);
		memcpy(&view[RF_2A4M1_SME_PFX_LEN], abody, ablen);
		rf_2a4m1_sme_demux(s, type, view, RF_2A4M1_SME_PFX_LEN + ablen);
		return;
	}
	switch (h->subtype) {
	case RF_2A4M1_WLAN_FC_STYPE_BEACON:
	case RF_2A4M1_WLAN_FC_STYPE_PROBE_RESP:
		rf_2a4m1_rx_bss_announce(s, h);             /* the scan: BSSID gate + SSID learn + leave SCANNING */
		return;
	case RF_2A4M1_WLAN_FC_STYPE_PROBE_REQ:
		if (s->role == RF_2A4M1_SME_ROLE_AP)
			rf_2a4m1_tx_mgmt_to(s, RF_2A4M1_F_PROBE_RESP, &h->addr2, NULL, 0);   /* discoverable: answer it */
		return;
	case RF_2A4M1_WLAN_FC_STYPE_AUTH: {
		uint16_t algo = 0, txn = 0, status = 0;
		if (!rf_2a4m1_mgmt_parse_auth(h->body, h->body_len, &algo, &txn, &status))
			return;
		/* SAE Commit/Confirm ride a dedicated path (not the open-auth ladder). */
		if (algo == RF_2A4M1_WLAN_AUTH_SAE) {
			rf_2a4m1_sme_sae_rx_auth(s, h);
			return;
		}
		if (algo != RF_2A4M1_WLAN_AUTH_OPEN)
			return;
		if (txn == 1) {
			type = RF_2A4M1_F_AUTH_REQ;
		} else if (txn == 2) {
			if (status != RF_2A4M1_WLAN_STATUS_SUCCESS) {
				s->state = RF_2A4M1_SME_FAILED;     /* the AP refused to authenticate us */
				return;
			}
			type = RF_2A4M1_F_AUTH_RESP;
		} else {
			return;
		}
		ies = h->body + 6; ies_len = (size_t)h->body_len - 6;
		break;
	}
	case RF_2A4M1_WLAN_FC_STYPE_ASSOC_REQ:
	case RF_2A4M1_WLAN_FC_STYPE_REASSOC_REQ:
		if (h->body_len < 4)               /* Capability + Listen Interval */
			return;
		type = (h->subtype == RF_2A4M1_WLAN_FC_STYPE_ASSOC_REQ) ? RF_2A4M1_F_ASSOC_REQ : RF_2A4M1_F_REASSOC_REQ;
		ies = h->body + 4; ies_len = (size_t)h->body_len - 4;
		break;
	case RF_2A4M1_WLAN_FC_STYPE_ASSOC_RESP:
	case RF_2A4M1_WLAN_FC_STYPE_REASSOC_RESP: {
		uint16_t cap = 0, status = 0, aid = 0;
		if (!rf_2a4m1_mgmt_parse_assoc_resp(h->body, h->body_len, &cap, &status, &aid))
			return;
		if (status != RF_2A4M1_WLAN_STATUS_SUCCESS) {
			s->state = RF_2A4M1_SME_FAILED;     /* the AP refused the association */
			return;
		}
		s->aid = aid;
		type = (h->subtype == RF_2A4M1_WLAN_FC_STYPE_ASSOC_RESP) ? RF_2A4M1_F_ASSOC_RESP : RF_2A4M1_F_REASSOC_RESP;
		ies = h->body + 6; ies_len = (size_t)h->body_len - 6;
		break;
	}
	default:
		return;                            /* a subtype this state machine does not drive */
	}

	/* Zeroed, not merely bounds-checked: this buffer is handed to the element scanners, which
	 * walk attacker-controlled frames. The length arithmetic below already keeps them inside the
	 * copied span, but a scratch buffer that carries hostile bytes into a parse loop should never
	 * have an undefined tail to reason about at all. */
	uint8_t view[RF_2A4M1_SME_RX_VIEW_MAX] = { 0 };
	if (RF_2A4M1_SME_PFX_LEN + ies_len > sizeof view)
		return;
	view[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(type);
	memcpy(&view[RF_2A4M1_SME_PFX_ADDR], h->addr2.a, RF_2A4M1_ETH_ALEN);   /* src = the transmitter */
	if (ies_len)
		memcpy(&view[RF_2A4M1_SME_PFX_LEN], ies, ies_len);
	rf_2a4m1_sme_demux(s, type, view, RF_2A4M1_SME_PFX_LEN + ies_len);
}

static bool rf_2a4m1_rx_dot11_data(struct rf_2a4m1_sme *s, const uint8_t *in, size_t len)
{
	size_t hdr, body_off;
	const uint8_t *pdu;
	size_t pdu_len;
	uint16_t ki = 0;
	bool qos;

	if (len < RF_2A4M1_DOT11_DATA_HDR_LEN + RF_2A4M1_EAPOL_SNAP_LEN)
		return false;
	if ((in[0] & 0x03) != 0 || ((in[0] >> 2) & 0x03) != 2)   /* Protocol Version 0, Type = Data */
		return false;
	/* Protected bit: 4-way M1..M4 are cleartext EAPOL (M3 MIC is over the PDU, not CCMP). */
	if (in[1] & 0x40)
		return true;
	/* SoftMAC extract_eapol: RA (addr1) must be us (or group). BSSID (addr2 on
	 * FromDS) must match the peer once known. */
	if (!(in[4] & 0x01) && !rf_2a4m1_bytes_eq(in + 4, s->self.a, RF_2A4M1_ETH_ALEN))
		return true;
	if (!rf_2a4m1_mac_is_zero(&s->peer) && !rf_2a4m1_bytes_eq(in + 10, s->peer.a, RF_2A4M1_ETH_ALEN) &&
	    !rf_2a4m1_bytes_eq(in + 16, s->peer.a, RF_2A4M1_ETH_ALEN))
		return true;
	qos = ((in[0] >> 4) & 0x08) != 0;
	hdr = RF_2A4M1_DOT11_DATA_HDR_LEN + (qos ? 2u : 0u);
	if (in[1] & 0x80)
		hdr += 4;                              /* Order / HT Control */
	if (len < hdr + RF_2A4M1_EAPOL_SNAP_LEN)
		return true;
	body_off = hdr;
	if (!rf_2a4m1_bytes_eq(in + body_off, rf_2a4m1_eapol_snap_hdr, RF_2A4M1_EAPOL_SNAP_LEN))
		return true;                   /* an 802.11 Data frame, but not EAPOL */
	pdu = in + body_off + RF_2A4M1_EAPOL_SNAP_LEN;
	pdu_len = len - body_off - RF_2A4M1_EAPOL_SNAP_LEN;
	/* Prefer the on-wire EAPOL body-length when sane (excl. 4-octet 802.1X hdr).
	 * This also drops a trailing APP_FCS (4 B) on SoftMAC station RCR without
	 * corrupting host-sim frames that have no FCS trailer. */
	if (pdu_len >= 4) {
		uint16_t eapol_bl = (uint16_t)(((uint16_t)pdu[2] << 8) | pdu[3]);
		uint16_t eapol_total = (uint16_t)(4u + eapol_bl);
		if (eapol_total >= 95 && eapol_total <= pdu_len)
			pdu_len = eapol_total;
	}
	if (!rf_2a4m1_sme_eapol_key_decode(pdu, pdu_len, &ki, NULL, NULL, NULL, 0, NULL))
		return true;                   /* not a well-formed EAPOL-Key PDU */

	bool mic = (ki & RF_2A4M1_SME_KI_MIC) != 0, ack = (ki & RF_2A4M1_SME_KI_ACK) != 0,
	     sec = (ki & RF_2A4M1_SME_KI_SECURE) != 0;
	uint8_t type;
	if (!mic && ack)      type = RF_2A4M1_F_M1;
	else if (mic && !sec) type = RF_2A4M1_F_M2;
	else if (mic && ack)  type = RF_2A4M1_F_M3;
	else if (mic)         type = RF_2A4M1_F_M4;
	else                  return true;     /* no message of the 4-way looks like this */

	uint8_t view[RF_2A4M1_SME_RX_VIEW_MAX] = { 0 };   /* zeroed for the reason above */
	if (RF_2A4M1_SME_PFX_LEN + pdu_len > sizeof view)
		return true;
	view[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(type);
	memcpy(&view[RF_2A4M1_SME_PFX_ADDR], in + 10, RF_2A4M1_ETH_ALEN);      /* addr2 = the transmitter */
	memcpy(&view[RF_2A4M1_SME_PFX_LEN], pdu, pdu_len);
	rf_2a4m1_sme_demux(s, type, view, RF_2A4M1_SME_PFX_LEN + pdu_len);
	return true;
}

static void rf_2a4m1_sta_on_m1(struct rf_2a4m1_sme *s, const uint8_t *pdu, size_t pdu_len)
{
	uint16_t rx_ki = 0, rx_kdl = 0;
	uint64_t rx_replay = 0;
	uint8_t  rx_nonce[RF_2A4M1_SME_NONCE_LEN];
	uint8_t  rx_kd[RF_2A4M1_SME_KDE_MAX];
	uint8_t m2buf[96];
	uint16_t m2kl = 0;

	if (!rf_2a4m1_sme_eapol_key_decode(pdu, pdu_len, &rx_ki, &rx_replay, rx_nonce,
	                          rx_kd, sizeof rx_kd, &rx_kdl))
		return;
	s->eapol_m1_rx++;
	memcpy(s->anonce, rx_nonce, RF_2A4M1_SME_NONCE_LEN);
	s->replay = rx_replay;
	if (s->state == RF_2A4M1_SME_ASSOCED)
		s->crypto->gen_nonce(s->snonce, s->pmk, &s->self, s->nonce_salt);
	rf_2a4m1_sme_derive_ptk(s);
	m2kl = (uint16_t)rf_2a4m1_pmf_rsne_build(m2buf, sizeof m2buf,
	                                rf_2a4m1_pmf_mode_caps((enum rf_2a4m1_pmf_mode)s->rf_2a4m1_pmf_mode),
	                                rf_2a4m1_sme_rsn_akm(s));
	/* hostapd: RSNXE in M2 Key Data must match Assoc-Req (WPA3-SAE H2E). */
	if (s->akm == RF_2A4M1_SME_AKM_SAE &&
	    (size_t)m2kl + sizeof rf_2a4m1_rsnxe_h2e <= sizeof m2buf) {
		memcpy(m2buf + m2kl, rf_2a4m1_rsnxe_h2e, sizeof rf_2a4m1_rsnxe_h2e);
		m2kl = (uint16_t)(m2kl + sizeof rf_2a4m1_rsnxe_h2e);
	}
	if (s->ocv_enabled && s->mfp_active &&
	    (size_t)m2kl + 9u <= sizeof m2buf)
		m2kl = (uint16_t)(m2kl + rf_2a4m1_pmf_oci_kde_build(
			m2buf + m2kl, sizeof m2buf - m2kl, &s->oci_local));
	rf_2a4m1_tx_eapol(s, rf_2a4m1_sme_ki_m2(s), 0, s->replay, s->snonce,
	         m2kl ? m2buf : NULL, m2kl);
	s->eapol_m2_tx++;
	s->state = RF_2A4M1_SME_4WAY;
}

void rf_2a4m1_sme_rx(struct rf_2a4m1_sme *s, const uint8_t *in, size_t len)
{
	if (len == 0 || s->state == RF_2A4M1_SME_FAILED)
		return;
	struct rf_2a4m1_mgmt_hdr h;
	if (len <= 0xffff && rf_2a4m1_mgmt_parse_hdr(in, (uint16_t)len, &h)) {
		rf_2a4m1_rx_dot11_mgmt(s, in, len, &h);
		return;
	}
	if (rf_2a4m1_rx_dot11_data(s, in, len))
		return;
	if (len < RF_2A4M1_SME_PFX_LEN || !RF_2A4M1_SME_INT_IS(in[RF_2A4M1_SME_PFX_TYPE]))
		return;                            /* neither real 802.11 nor one of ours */
	rf_2a4m1_sme_demux(s, RF_2A4M1_SME_INT_TYPE(in[RF_2A4M1_SME_PFX_TYPE]), in, len);
}

static void rf_2a4m1_sme_demux(struct rf_2a4m1_sme *s, uint8_t type, const uint8_t *in, size_t len)
{
	if (len < RF_2A4M1_SME_PFX_LEN || s->state == RF_2A4M1_SME_FAILED)
		return;
	if (s->state == RF_2A4M1_SME_CONNECTED && type != RF_2A4M1_F_DATA && type != RF_2A4M1_F_QOS_DATA
	    && (type < RF_2A4M1_F_FT_AUTH1 || type > RF_2A4M1_F_FT_DS_RESP)
	    && type != RF_2A4M1_F_ADDBA_REQ && type != RF_2A4M1_F_ADDBA_RSP   /* Block-Ack (N13) */
	    && type != RF_2A4M1_F_ADDTS_REQ && type != RF_2A4M1_F_ADDTS_RSP   /* QoS admission (N17) */
	    && type != RF_2A4M1_F_DELTS
	    /* the spurious-(Re)Assoc SA Query defence (N21): a (Re)Assoc-Req + the SA Query action
	     * frames flow while CONNECTED (the AP defends without tearing the SA down). */
	    && type != RF_2A4M1_F_REASSOC_REQ && type != RF_2A4M1_F_SA_QUERY_REQ
	    && type != RF_2A4M1_F_SA_QUERY_RESP
	   )
		return;                                  
	rf_2a4m1_mac_addr src;
	memcpy(src.a, &in[RF_2A4M1_SME_PFX_ADDR], RF_2A4M1_ETH_ALEN);

	/* the EAPOL-Key PDU (if any) begins right after the 7-byte prefix. */
	const uint8_t *pdu = &in[RF_2A4M1_SME_PFX_LEN];
	size_t pdu_len = len - RF_2A4M1_SME_PFX_LEN;
	uint16_t rx_ki = 0, rx_kdl = 0;
	uint64_t rx_replay = 0;
	uint8_t  rx_nonce[RF_2A4M1_SME_NONCE_LEN];
	uint8_t  rx_kd[RF_2A4M1_SME_KDE_MAX];
	bool eapol_ok = false;
	if (type >= RF_2A4M1_F_M1 && type <= RF_2A4M1_F_M4)
		eapol_ok = rf_2a4m1_sme_eapol_key_decode(pdu, pdu_len, &rx_ki, &rx_replay, rx_nonce,
		                                rx_kd, sizeof(rx_kd), &rx_kdl);

	if (s->role == RF_2A4M1_SME_ROLE_AP) {
		switch (type) {
		case RF_2A4M1_F_PROBE_REQ:
			rf_2a4m1_tx_mgmt(s, RF_2A4M1_F_PROBE_RESP);        /* AP is discoverable: answer the scan */
			break;
		case RF_2A4M1_F_AUTH_REQ:
			s->peer = src;                       /* learn the STA */
			rf_2a4m1_tx_mgmt(s, RF_2A4M1_F_AUTH_RESP);
			s->state = RF_2A4M1_SME_AUTHED;
			break;
		case RF_2A4M1_F_ASSOC_REQ:
			/* QoS negotiation: a WMM Info Element in the assoc-req marks the STA QoS-capable;
			 * answer with our advertised WMM Parameter Set. */
			if (s->qos_advertise) {
				bool wmm = false, edca = false; struct rf_2a4m1_edca_param tmp;
				if (rf_2a4m1_qos_scan_ies(&in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN, &wmm, &tmp, &edca) && wmm)
					s->qos_enabled = true;
			}
			/* MFP negotiation (N21): parse the STA's RSNE Capabilities, decide the MFP state
			 * (mfp_active) — the AP then advertises its own RSNE in the (Re)Assoc-Resp.
			 * §12.6.4: a REQUIRED-vs-not-capable mismatch is a violation — REFUSE (do not tx the
			 * resp / start the 4-way) rather than associate a STA whose mandatory MFP we cannot
			 * honor. */
			if (rf_2a4m1_pmf_negotiate_ies(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN) != RF_2A4M1_PMF_STATUS_SUCCESS) {
				s->state = RF_2A4M1_SME_FAILED;
				break;
			}
			rf_2a4m1_tx_assoc_resp(s);
			/* start the 4-way: generate ANonce, send M1 (ACK, no MIC) */
			s->crypto->gen_nonce(s->anonce, s->pmk, &s->self, s->nonce_salt);
			s->replay++;
			rf_2a4m1_tx_eapol(s, RF_2A4M1_SME_KI_M1, RF_2A4M1_SME_KEY_LEN_CCMP, s->replay,
			         s->anonce, NULL, 0);
			s->state = RF_2A4M1_SME_4WAY;
			break;
		case RF_2A4M1_F_M2:
			if (s->state != RF_2A4M1_SME_4WAY || rx_replay != s->replay)  /* F6/F7: gate state + replay */
				break;
			if (!eapol_ok)
				break;
			memcpy(s->snonce, rx_nonce, RF_2A4M1_SME_NONCE_LEN);
			rf_2a4m1_sme_derive_ptk(s);
			if (!rf_2a4m1_eapol_mic_ok(s, pdu, pdu_len)) { s->bad_mic++; break; }  /* wrong PMK / tamper */
			/* OCV (N23): the STA's OCI (M2 Key Data) must match our operating channel, else
			 * reject the handshake (the multi-channel MITM defence) — do not send M3. */
			if (s->ocv_enabled && s->mfp_active) {
				struct rf_2a4m1_pmf_oci sta_oci;
				if (!rf_2a4m1_pmf_oci_kde_find(rx_kd, rx_kdl, &sta_oci) ||
				    !rf_2a4m1_pmf_ocv_verify(&sta_oci, &s->oci_local)) {
					s->ocv_fail++;
					break;
				}
			}
			/* M3: carry the GTK (+ the IGTK when MFP is active) as KEK-wrapped RSN KDEs + MIC;
			 * install nothing yet. */
			rf_2a4m1_sme_gen_material(s, s->gtk, RF_2A4M1_SME_GTK_LEN, s->nonce_salt + 1);
			uint8_t kde[RF_2A4M1_SME_KDE_MAX];
			size_t klen = rf_2a4m1_gtk_kde_build(kde, s->gtk, 1);   /* GTK KDE first (plaintext) */
			if (s->mfp_active) {
				/* generate + install the IGTK, append its KDE; then the WPA3-R3 KDEs (N23);
				 * finally pad the Key Data to an 8-octet multiple (AES Key Wrap = 64-bit blocks). */
				uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN];
				rf_2a4m1_sme_gen_material(s, igtk, RF_2A4M1_BIP_IGTK_LEN, s->nonce_salt + 3);
				rf_2a4m1_pmf_igtk_install(&s->igtk, igtk, 4, 0);
				klen += rf_2a4m1_igtk_kde_build(&kde[klen], igtk, 4, 0);
				rf_2a4m1_crypto_wipe(igtk, sizeof igtk);
				/* Beacon Protection (N23): the BIGTK — the beacon analog of the IGTK, KeyID 6. */
				if (s->beacon_prot_enabled) {
					uint8_t bigtk[RF_2A4M1_BIP_IGTK_LEN];
					rf_2a4m1_sme_gen_material(s, bigtk, RF_2A4M1_BIP_IGTK_LEN, s->nonce_salt + 5);
					rf_2a4m1_pmf_igtk_install(&s->bigtk, bigtk, 6, 0);
					klen += rf_2a4m1_pmf_bigtk_kde_build(&kde[klen], sizeof kde - klen, bigtk, 6, 0);
					rf_2a4m1_crypto_wipe(bigtk, sizeof bigtk);
				}
				/* OCV (N23): our operating channel, validated by the STA against its own. */
				if (s->ocv_enabled)
					klen += rf_2a4m1_pmf_oci_kde_build(&kde[klen], sizeof kde - klen, &s->oci_local);
				/* Transition Disable (N23): the disabled-modes bitmap the STA must honor. */
				if (s->td_advertise)
					klen += rf_2a4m1_pmf_td_kde_build(&kde[klen], sizeof kde - klen, s->td_advert_bitmap);
				if (klen % 8) {
					kde[klen++] = 0xdd;            /* pad: a 0xdd marker then zero fill */
					while (klen % 8)
						kde[klen++] = 0x00;
				}
			}
			uint8_t wrapped[RF_2A4M1_SME_KDE_MAX];
			size_t wl = s->crypto->key_wrap(wrapped, sizeof wrapped,
			                                &s->ptk[RF_2A4M1_SME_KEK_OFF], kde, klen);
			rf_2a4m1_crypto_wipe(kde, sizeof kde);           /* F12: plaintext KDEs consumed */
			if (wl == 0)
				break;
			s->replay++;
			rf_2a4m1_tx_eapol(s, RF_2A4M1_SME_KI_M3, RF_2A4M1_SME_KEY_LEN_CCMP, s->replay,
			         s->anonce, wrapped, (uint16_t)wl);
			rf_2a4m1_crypto_wipe(wrapped, sizeof wrapped);   /* F12: wrapped GTK consumed */
			break;
		case RF_2A4M1_F_M4:
			if (s->state != RF_2A4M1_SME_4WAY || rx_replay != s->replay)  /* F6/F7: gate state + replay */
				break;
			if (!eapol_ok || !rf_2a4m1_eapol_mic_ok(s, pdu, pdu_len)) { s->bad_mic++; break; }
			s->ptk_installed = s->gtk_installed = true;
			rf_2a4m1_sme_install_data_key(s);             /* KRACK: keep RX PN state on a same-TK reinstall */
			s->connected = true;
			s->state = RF_2A4M1_SME_CONNECTED;
			if (s->ft_enabled) rf_2a4m1_sme_ft_establish(s);   /* establish PMK-R0 for this MD (N22) */
			break;
		case RF_2A4M1_F_DATA:
			rf_2a4m1_rx_data(s, in, len);                 /* TK-protected data plane */
			break;
		case RF_2A4M1_F_QOS_DATA:
			rf_2a4m1_rx_qos_data(s, in, len);             /* QoS-Data plane (QoS Control + TID/AC) */
			break;
		case RF_2A4M1_F_FT_AUTH1:                         /* FT-over-air: STA→AP Authentication seq 1 */
			if (s->ft_enabled)
				rf_2a4m1_ft_ap_on_auth1(s, &src, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
			break;
		case RF_2A4M1_F_FT_REASSOC_REQ:                   /* FT Reassociation Req → install keys, no 4-way */
			if (s->ft_enabled)
				rf_2a4m1_ft_ap_on_reassoc(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
			break;
		case RF_2A4M1_F_FT_DS_REQ:                        /* FT-over-DS: STA→current-AP FT Request Action */
			if (s->ft_enabled)
				rf_2a4m1_ft_ap_on_ds_request(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
			break;
		case RF_2A4M1_F_ADDBA_REQ:                        /* Block-Ack (N13): STA→AP ADDBA Request */
			rf_2a4m1_ba_ap_on_addba_req(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
			break;
		case RF_2A4M1_F_ADDTS_REQ:                        /* QoS admission (N17): STA→AP ADDTS Request */
			rf_2a4m1_addts_ap_on_req(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
			break;
		case RF_2A4M1_F_DELTS:                            /* QoS admission (N17): DELTS teardown → free budget */
			rf_2a4m1_addts_on_delts(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
			break;
		case RF_2A4M1_F_REASSOC_REQ:                      /* PMF (N21): a spurious (Re)Assoc-Req → status-30 + SA Query */
			rf_2a4m1_pmf_ap_on_spurious_reassoc(s, &src);
			break;
		case RF_2A4M1_F_SA_QUERY_RESP:                    /* PMF (N21): the genuine STA answered → the SA is proven alive */
			if (s->mfp_active &&
			    rf_2a4m1_pmf_sa_query_on_response(&s->sa_query, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN))
				s->sa_query_resolved++;
			break;
		case RF_2A4M1_F_SA_QUERY_REQ:                     /* PMF (N21): an AP that also holds a live SA answers a Request */
			if (s->mfp_active)
				rf_2a4m1_sa_query_sta_respond(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
			break;
		default: break;
		}
		return;
	}

	/* STA */
	switch (type) {
	case RF_2A4M1_F_PROBE_RESP:
		if (s->state == RF_2A4M1_SME_SCANNING) {          /* AP discovered: learn it, then authenticate */
			s->peer = src;
			if (s->akm == RF_2A4M1_SME_AKM_SAE) {
				if (!rf_2a4m1_sme_sae_tx_commit(s))
					s->state = RF_2A4M1_SME_FAILED;
				break;
			}
			rf_2a4m1_tx_mgmt(s, RF_2A4M1_F_AUTH_REQ);
		}
		break;
	case RF_2A4M1_F_AUTH_RESP:
		/* Only the first AUTH_RESP advances the ladder. AP retransmits of
		 * auth-resp must NOT re-fire Assoc-Req (hostapd then logs multiple
		 * "associated" + AP-STA-POSSIBLE-PSK-MISMATCH as 4-way collides with
		 * re-assoc). SoftMAC sends one Assoc-Req per open-auth success. */
		if (s->state != RF_2A4M1_SME_SCANNING && s->state != RF_2A4M1_SME_INIT)
			break;
		rf_2a4m1_tx_assoc_req(s);                         /* advertise WMM/QoS + per-gen Capabilities (N13-N16) */
		s->state = RF_2A4M1_SME_AUTHED;
		break;
	case RF_2A4M1_F_ASSOC_RESP:
		/* Parse the AP's advertised WMM/EDCA Parameter Set, store it, and push the negotiated
		 * per-AC EDCA down to the radio's lower MAC. */
		{
			bool wmm = false, edca = false;
			if (s->qos_advertise &&
			    rf_2a4m1_qos_scan_ies(&in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN, &wmm, &s->edca, &edca) && edca) {
				s->qos_enabled = true;
				rf_2a4m1_qos_apply_lmac(s);
			}
		}
		/* MFP negotiation (N21): parse the AP's advertised RSNE Capabilities → mfp_active.
		 * §12.6.4: a REQUIRED-vs-not-capable policy mismatch is a violation — REFUSE the
		 * association (do NOT proceed to the 4-way). Without this, a MITM stripping the AP's
		 * advertised MFPC/MFPR downgrades a required-MFP STA to mfp_active=false, after which an
		 * unprotected Deauth can evict it — the exact attack MFP-required prevents. */
		if (rf_2a4m1_pmf_negotiate_ies(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN) != RF_2A4M1_PMF_STATUS_SUCCESS) {
			s->state = RF_2A4M1_SME_FAILED;
			break;
		}
		/* OWE: AP DH in Assoc-Resp → ECDH+HKDF PMK before the 4-way (must precede M1). */
		if (s->akm == RF_2A4M1_SME_AKM_OWE) {
			if (!rf_2a4m1_sme_owe_on_assoc_resp(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN)) {
				s->state = RF_2A4M1_SME_FAILED;
				break;
			}
		}
		s->state = RF_2A4M1_SME_ASSOCED;                  /* now wait for M1 */
		/* SoftMAC race: hostapd often TX'd M1 already (stashed while AUTHED). */
		if (s->early_m1_valid) {
			s->early_m1_valid = false;
			rf_2a4m1_sta_on_m1(s, s->early_m1, s->early_m1_len);
		}
		break;
	case RF_2A4M1_F_M1:
		/* SoftMAC accepts M1 while ASSOCED and re-answers M1 retransmits during
		 * the 4-way (hostapd retries M1 if M2 is lost / not ACKed). An M1 that
		 * arrives before ASSOCED is stashed (hostapd fires it with Assoc-Resp). */
		if (!eapol_ok)
			break;
		if (s->state != RF_2A4M1_SME_ASSOCED && s->state != RF_2A4M1_SME_4WAY) {
			if (s->role == RF_2A4M1_SME_ROLE_STA && pdu_len <= sizeof s->early_m1) {
				memcpy(s->early_m1, pdu, pdu_len);
				s->early_m1_len = (uint16_t)pdu_len;
				s->early_m1_valid = true;
				s->eapol_m1_early++;
			}
			break;
		}
		rf_2a4m1_sta_on_m1(s, pdu, pdu_len);
		break;
	case RF_2A4M1_F_M3:
		if (s->state != RF_2A4M1_SME_4WAY || !eapol_ok)   /* F7: M3 only during the 4-way */
			break;
		if (rx_replay <= s->replay) { break; }               /* replay / stale */
		if (!rf_2a4m1_eapol_mic_ok(s, pdu, pdu_len)) { s->bad_mic++; break; }
		s->eapol_m3_rx++;
		/* unwrap the KEK-encrypted Key Data, then parse the RSN GTK KDE */
		uint8_t kde[RF_2A4M1_SME_KDE_MAX];
		size_t kl = s->crypto->key_unwrap(kde, sizeof kde, &s->ptk[RF_2A4M1_SME_KEK_OFF],
		                                  rx_kd, rx_kdl);
		if (kl == 0) { s->bad_mic++; break; }                /* unwrap integrity fail */
		uint8_t gtk[RF_2A4M1_SME_GTK_LEN];
		bool gtk_ok = rf_2a4m1_gtk_kde_find(kde, kl, gtk);           /* walk KDEs (RSN may precede GTK) */
		bool ocv_reject = false;
		/* When MFP is active, install the IGTK + the WPA3-R3 keys/policy from the M3 Key Data. */
		if (gtk_ok && s->mfp_active) {
			uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN]; uint16_t kid = 4; uint64_t ipn = 0;
			if (rf_2a4m1_igtk_kde_find(kde, kl, igtk, &kid, &ipn))
				rf_2a4m1_pmf_igtk_install(&s->igtk, igtk, kid, ipn);
			rf_2a4m1_crypto_wipe(igtk, sizeof igtk);
			/* Beacon Protection (N23): install the BIGTK from its KDE (KeyID 6/7). */
			if (s->beacon_prot_enabled) {
				uint8_t bigtk[RF_2A4M1_BIP_IGTK_LEN]; uint16_t bkid = 6; uint64_t bipn = 0;
				if (rf_2a4m1_pmf_bigtk_kde_find(kde, kl, bigtk, &bkid, &bipn))
					rf_2a4m1_pmf_igtk_install(&s->bigtk, bigtk, bkid, bipn);
				rf_2a4m1_crypto_wipe(bigtk, sizeof bigtk);
			}
			/* OCV (N23): the AP's OCI must match our operating channel — else REJECT (MITM). */
			if (s->ocv_enabled) {
				struct rf_2a4m1_pmf_oci ap_oci;
				if (!rf_2a4m1_pmf_oci_kde_find(kde, kl, &ap_oci) ||
				    !rf_2a4m1_pmf_ocv_verify(&ap_oci, &s->oci_local)) {
					s->ocv_fail++;
					ocv_reject = true;
				}
			}
			/* Transition Disable (N23): record + honor the advertised bitmap. */
			uint8_t tdbm;
			if (rf_2a4m1_pmf_td_kde_find(kde, kl, &tdbm)) {
				s->td_bitmap = tdbm;
				s->td_received = true;
			}
		}
		rf_2a4m1_crypto_wipe(kde, sizeof kde);                        /* F12: plaintext KDE consumed */
		if (!gtk_ok) { rf_2a4m1_crypto_wipe(gtk, sizeof gtk); break; }
		if (ocv_reject) { rf_2a4m1_crypto_wipe(gtk, sizeof gtk); break; }   /* OCV mismatch → drop M3, no M4 */
		s->replay = rx_replay;
		memcpy(s->gtk, gtk, RF_2A4M1_SME_GTK_LEN);
		rf_2a4m1_crypto_wipe(gtk, sizeof gtk);                        /* F12: local GTK copy consumed */
		s->ptk_installed = s->gtk_installed = true;
		rf_2a4m1_sme_install_data_key(s);                             /* KRACK: keep RX PN state on a same-TK reinstall */
		/* M4: MIC + Secure, echo the replay counter, no key length/data */
		rf_2a4m1_tx_eapol(s, rf_2a4m1_sme_ki_m4(s), 0, s->replay, NULL, NULL, 0);
		s->eapol_m4_tx++;
		s->connected = true;
		s->state = RF_2A4M1_SME_CONNECTED;
		if (s->ft_enabled) rf_2a4m1_sme_ft_establish(s);   /* establish PMK-R0 for this MD (N22) */
		break;
	case RF_2A4M1_F_DATA:
		rf_2a4m1_rx_data(s, in, len);                     /* TK-protected data plane */
		break;
	case RF_2A4M1_F_QOS_DATA:
		rf_2a4m1_rx_qos_data(s, in, len);                 /* QoS-Data plane (QoS Control + TID/AC) */
		break;
	case RF_2A4M1_F_FT_AUTH2:                             /* FT-over-air: AP→STA Authentication seq 2 */
		if (s->ft_enabled && s->ft_r0_valid)
			rf_2a4m1_ft_sta_on_auth2(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
		break;
	case RF_2A4M1_F_FT_REASSOC_RESP:                      /* FT Reassociation Resp → install keys, no 4-way */
		if (s->ft_enabled && s->ft_r0_valid)
			rf_2a4m1_ft_sta_on_reassoc(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
		break;
	case RF_2A4M1_F_FT_DS_RESP:                          /* FT-over-DS: current-AP→STA FT Response Action */
		if (s->ft_enabled && s->ft_r0_valid)
			rf_2a4m1_ft_sta_on_ds_response(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
		break;
	case RF_2A4M1_F_ADDBA_RSP:                            /* Block-Ack (N13): AP→STA ADDBA Response */
		if (s->ba.state == BA_PENDING) {
			s->ba_rx_rsp++;
			if (rf_2a4m1_ba_originator_on_addba_rsp(&s->ba, &in[RF_2A4M1_SME_PFX_LEN],
			                               len - RF_2A4M1_SME_PFX_LEN) == RF_2A4M1_S8021X_OK ||
			    s->ba.state == BA_ESTABLISHED)
				s->ba_ok++;
		}
		break;
	case RF_2A4M1_F_ADDTS_RSP:                            /* QoS admission (N17): AP→STA ADDTS Response */
		rf_2a4m1_addts_sta_on_resp(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
		break;
	case RF_2A4M1_F_DELTS:                                /* QoS admission (N17): AP→STA DELTS teardown */
		rf_2a4m1_addts_on_delts(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
		break;
	case RF_2A4M1_F_SA_QUERY_REQ:                         /* PMF (N21): AP→STA SA Query Request → answer with a Response */
		if (s->mfp_active)
			rf_2a4m1_sa_query_sta_respond(s, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN);
		break;
	case RF_2A4M1_F_SA_QUERY_RESP:                        /* PMF: STA-initiated SA Query Response (or AP-side resolve) */
		if (s->mfp_active &&
		    rf_2a4m1_pmf_sa_query_on_response(&s->sa_query, &in[RF_2A4M1_SME_PFX_LEN], len - RF_2A4M1_SME_PFX_LEN)) {
			s->saq_rx_rsp++;
			s->saq_ok++;
		}
		break;
	default: break;
	}
}

rf_2a4m1_s8021x_status rf_2a4m1_sme_data_tx(struct rf_2a4m1_sme *s, const uint8_t *payload, uint16_t len)
{
	if (s->state != RF_2A4M1_SME_CONNECTED)
		return RF_2A4M1_S8021X_ERR_STATE;
	if (len > RF_2A4M1_SME_DATA_MAX)
		return RF_2A4M1_S8021X_ERR_INVAL;
	uint8_t f[RF_2A4M1_SME_PFX_LEN + RF_2A4M1_SME_DATA_PN_LEN + RF_2A4M1_SME_DATA_MAX + RF_2A4M1_SME_MIC_LEN];
	f[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(RF_2A4M1_F_DATA);
	memcpy(&f[RF_2A4M1_SME_PFX_ADDR], s->self.a, RF_2A4M1_ETH_ALEN);
	rf_2a4m1_put_pn48(&f[RF_2A4M1_SME_PFX_LEN], ++s->data_tx_pn);              /* fresh per-frame 48-bit PN */
	if (len)
		memcpy(&f[RF_2A4M1_SME_PFX_LEN + RF_2A4M1_SME_DATA_PN_LEN], payload, len);
	size_t body = RF_2A4M1_SME_PFX_LEN + RF_2A4M1_SME_DATA_PN_LEN + len;
	s->crypto->mic(&f[body], &s->ptk[RF_2A4M1_SME_TK_OFF], f, body);   /* MIC over [pfx][PN][payload] */
	rf_2a4m1_tx_raw(s, f, body + RF_2A4M1_SME_MIC_LEN);
	return RF_2A4M1_S8021X_OK;
}

void rf_2a4m1_sme_set_qos_advertise(struct rf_2a4m1_sme *s, bool on) { s->qos_advertise = on; }

void rf_2a4m1_sme_set_edca_advert(struct rf_2a4m1_sme *s, const struct rf_2a4m1_edca_param *p)
{
	if (!p)
		return;
	s->edca_advert = *p;
	s->edca_advert_set = true;
}

rf_2a4m1_s8021x_status rf_2a4m1_sme_data_tx_qos(struct rf_2a4m1_sme *s, const uint8_t *payload, uint16_t len, uint8_t up)
{
	if (s->state != RF_2A4M1_SME_CONNECTED)
		return RF_2A4M1_S8021X_ERR_STATE;
	if (len > RF_2A4M1_SME_DATA_MAX)
		return RF_2A4M1_S8021X_ERR_INVAL;
	if (!s->qos_enabled)
		return rf_2a4m1_sme_data_tx(s, payload, len);      /* peer is not QoS-capable */

	uint8_t f[RF_2A4M1_SME_PFX_LEN + 2 + RF_2A4M1_SME_DATA_PN_LEN + RF_2A4M1_SME_DATA_MAX + RF_2A4M1_SME_MIC_LEN];
	f[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(RF_2A4M1_F_QOS_DATA);
	memcpy(&f[RF_2A4M1_SME_PFX_ADDR], s->self.a, RF_2A4M1_ETH_ALEN);
	struct rf_2a4m1_qos_ctrl qc = { .tid = (uint8_t)(up & 0x07), .eosp = false,
	                       .ack_policy = RF_2A4M1_QOS_ACK_NORMAL, .amsdu = false, .hi = 0 };
	rf_2a4m1_qos_ctrl_build(&f[RF_2A4M1_SME_PFX_LEN], 2, &qc);
	rf_2a4m1_put_pn48(&f[RF_2A4M1_SME_PFX_LEN + 2], ++s->data_tx_pn);          /* fresh per-frame 48-bit PN */
	if (len)
		memcpy(&f[RF_2A4M1_SME_PFX_LEN + 2 + RF_2A4M1_SME_DATA_PN_LEN], payload, len);
	size_t body = RF_2A4M1_SME_PFX_LEN + 2 + RF_2A4M1_SME_DATA_PN_LEN + len;
	s->crypto->mic(&f[body], &s->ptk[RF_2A4M1_SME_TK_OFF], f, body);   /* MIC over [pfx][qos-ctrl][PN][payload] */
	rf_2a4m1_tx_raw_ac(s, f, body + RF_2A4M1_SME_MIC_LEN, rf_2a4m1_qos_up_to_ac(up), (uint8_t)(up & 0x07));
	return RF_2A4M1_S8021X_OK;
}

static uint8_t rf_2a4m1_addts_ac_up(uint8_t ac)
{
	return (ac == RF_2A4M1_QOS_AC_VO) ? (uint8_t)6 : (uint8_t)5;   /* VO or VI (the ACM-marked ACs) */
}

rf_2a4m1_s8021x_status rf_2a4m1_sme_addts_request(struct rf_2a4m1_sme *s, uint8_t tsid, uint8_t ac,
                                uint8_t direction, uint16_t medium_time)
{
	if (s->role != RF_2A4M1_SME_ROLE_STA || s->state != RF_2A4M1_SME_CONNECTED || !s->qos_enabled)
		return RF_2A4M1_S8021X_ERR_STATE;
	if (tsid > 0x0f || (ac != RF_2A4M1_QOS_AC_VI && ac != RF_2A4M1_QOS_AC_VO))
		return RF_2A4M1_S8021X_ERR_INVAL;   /* admission control applies only to the ACM-marked VI/VO ACs */
	struct rf_2a4m1_tspec t;
	memset(&t, 0, sizeof t);
	t.ts.traffic_type  = true;                       /* aperiodic — typical for a VI/VO stream */
	t.ts.tsid          = tsid;
	t.ts.direction     = (uint8_t)(direction & 0x03);
	t.ts.access_policy = 1;                          /* EDCA / contention-based admission */
	t.ts.user_priority = rf_2a4m1_addts_ac_up(ac);
	t.medium_time      = medium_time;                /* the airtime the accumulator accounts */
	s->addts_resp_received = false;
	s->addts_last_status   = 0;
	uint8_t f[RF_2A4M1_SME_PFX_LEN + 3 + 2 + RF_2A4M1_TSPEC_BODY_LEN];
	f[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(RF_2A4M1_F_ADDTS_REQ);
	memcpy(&f[RF_2A4M1_SME_PFX_ADDR], s->self.a, RF_2A4M1_ETH_ALEN);
	size_t n = rf_2a4m1_addts_req_build(&f[RF_2A4M1_SME_PFX_LEN], sizeof f - RF_2A4M1_SME_PFX_LEN, ++s->addts_dialog, &t);
	if (n == 0)
		return RF_2A4M1_S8021X_ERR_INVAL;
	rf_2a4m1_tx_raw(s, f, RF_2A4M1_SME_PFX_LEN + n);
	return RF_2A4M1_S8021X_OK;
}

rf_2a4m1_s8021x_status rf_2a4m1_sme_delts(struct rf_2a4m1_sme *s, uint8_t tsid)
{
	if (s->role != RF_2A4M1_SME_ROLE_STA || s->state != RF_2A4M1_SME_CONNECTED)
		return RF_2A4M1_S8021X_ERR_STATE;
	if (tsid > 0x0f)
		return RF_2A4M1_S8021X_ERR_INVAL;
	rf_2a4m1_qos_admission_release(&s->admission, tsid);      /* free our own accumulated medium time first */
	struct rf_2a4m1_ts_info ts;
	memset(&ts, 0, sizeof ts);
	ts.tsid = tsid;
	uint8_t f[RF_2A4M1_SME_PFX_LEN + 7];
	f[RF_2A4M1_SME_PFX_TYPE] = RF_2A4M1_SME_INT_ENC(RF_2A4M1_F_DELTS);
	memcpy(&f[RF_2A4M1_SME_PFX_ADDR], s->self.a, RF_2A4M1_ETH_ALEN);
	size_t n = rf_2a4m1_delts_build(&f[RF_2A4M1_SME_PFX_LEN], sizeof f - RF_2A4M1_SME_PFX_LEN, &ts, 0 /* reason */);
	if (n == 0)
		return RF_2A4M1_S8021X_ERR_INVAL;
	rf_2a4m1_tx_raw(s, f, RF_2A4M1_SME_PFX_LEN + n);
	return RF_2A4M1_S8021X_OK;
}

void rf_2a4m1_sme_set_admission_limit(struct rf_2a4m1_sme *s, uint8_t ac, uint32_t limit)
{
	rf_2a4m1_qos_admission_set_limit(&s->admission, ac, limit);
}

void rf_2a4m1_sme_set_pmf(struct rf_2a4m1_sme *s, enum rf_2a4m1_pmf_mode mode) { s->rf_2a4m1_pmf_mode = (uint8_t)mode; }

static rf_2a4m1_s8021x_status rf_2a4m1_tx_protected_group_mgmt(struct rf_2a4m1_sme *s, uint8_t subtype, uint16_t reason)
{
	if (s->state != RF_2A4M1_SME_CONNECTED || !s->mfp_active || !s->igtk.installed)
		return RF_2A4M1_S8021X_ERR_STATE;
	static const rf_2a4m1_mac_addr rf_2a4m1_bcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};
	uint8_t body[2] = { (uint8_t)reason, (uint8_t)(reason >> 8) };   /* reason code, LE */
	uint8_t f[RF_2A4M1_BIP_MGMT_HDR_LEN + 2 + RF_2A4M1_BIP_MMIE_LEN];
	size_t n = rf_2a4m1_pmf_build_protected_mgmt(&s->igtk, f, sizeof f, subtype,
	                                    &rf_2a4m1_bcast, &s->self, &s->peer, body, sizeof body, 0);
	if (n == 0)
		return RF_2A4M1_S8021X_ERR_INVAL;
	rf_2a4m1_tx_raw(s, f, n);
	return RF_2A4M1_S8021X_OK;
}

rf_2a4m1_s8021x_status rf_2a4m1_sme_tx_protected_deauth(struct rf_2a4m1_sme *s, uint16_t reason)
{ return rf_2a4m1_tx_protected_group_mgmt(s, RF_2A4M1_PMF_STYPE_DEAUTH, reason); }

rf_2a4m1_s8021x_status rf_2a4m1_sme_tx_protected_disassoc(struct rf_2a4m1_sme *s, uint16_t reason)
{ return rf_2a4m1_tx_protected_group_mgmt(s, RF_2A4M1_PMF_STYPE_DISASSOC, reason); }

void rf_2a4m1_sme_set_beacon_protection(struct rf_2a4m1_sme *s, bool on) { s->beacon_prot_enabled = on; }

void rf_2a4m1_sme_set_transition_disable(struct rf_2a4m1_sme *s, uint8_t bitmap)
{ s->td_advertise = true; s->td_advert_bitmap = bitmap; }

void rf_2a4m1_sme_set_ocv(struct rf_2a4m1_sme *s, bool on, uint8_t op_class, uint8_t prim_chan, uint8_t seg1_freq)
{
	s->ocv_enabled = on;
	s->oci_local.op_class  = op_class;
	s->oci_local.prim_chan = prim_chan;
	s->oci_local.seg1_freq = seg1_freq;
}

void rf_2a4m1_sme_rx_mgmt(struct rf_2a4m1_sme *s, const uint8_t *frame, size_t len)
{
	if (len < RF_2A4M1_BIP_MGMT_HDR_LEN)
		return;
	rf_2a4m1_rx_robust_mgmt(s, frame, len);
}

void rf_2a4m1_sme_rx_spurious_reassoc(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *src)
{
	if (src)
		rf_2a4m1_pmf_ap_on_spurious_reassoc(s, src);
}

void rf_2a4m1_sme_sa_query_tick(struct rf_2a4m1_sme *s)
{
	if (s->role != RF_2A4M1_SME_ROLE_AP || !s->mfp_active)
		return;
	uint8_t q[RF_2A4M1_PMF_SA_QUERY_LEN];
	size_t qn = rf_2a4m1_pmf_sa_query_tick(&s->sa_query, q, sizeof q);
	if (qn)
		rf_2a4m1_sa_query_tx(s, RF_2A4M1_F_SA_QUERY_REQ, q, qn);
}

void rf_2a4m1_sme_set_ft(struct rf_2a4m1_sme *s, uint16_t mdid, const uint8_t *ssid, size_t ssid_len,
                const uint8_t *r0kh_id, size_t r0kh_len)
{
	s->ft_enabled = true;
	s->ft_mdid = mdid;
	if (ssid_len > sizeof s->ft_ssid) ssid_len = sizeof s->ft_ssid;
	if (ssid && ssid_len) memcpy(s->ft_ssid, ssid, ssid_len);
	s->ft_ssid_len = (uint8_t)ssid_len;
	if (r0kh_len > sizeof s->ft_r0kh_id) r0kh_len = sizeof s->ft_r0kh_id;
	if (r0kh_id && r0kh_len) memcpy(s->ft_r0kh_id, r0kh_id, r0kh_len);
	s->ft_r0kh_len = (uint8_t)r0kh_len;
}

void rf_2a4m1_sme_ft_establish(struct rf_2a4m1_sme *s)
{
	if (!s->ft_enabled)
		return;
	/* S0KH-ID = the STA MAC: self on the STA, the associated peer on the AP. */
	const rf_2a4m1_mac_addr *s0kh = (s->role == RF_2A4M1_SME_ROLE_STA) ? &s->self : &s->peer;
	uint8_t mdid_le[2] = { (uint8_t)s->ft_mdid, (uint8_t)(s->ft_mdid >> 8) };
	if (rf_2a4m1_ft_derive_pmk_r0(s->pmk, RF_2A4M1_SME_PMK_LEN, s->ft_ssid, s->ft_ssid_len, mdid_le,
	                     s->ft_r0kh_id, s->ft_r0kh_len, s0kh,
	                     s->ft_pmk_r0, s->ft_pmk_r0_name))
		s->ft_r0_valid = true;
}

rf_2a4m1_s8021x_status rf_2a4m1_sme_ft_roam(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *target)
{
	if (s->role != RF_2A4M1_SME_ROLE_STA || !s->ft_enabled || !s->ft_r0_valid ||
	    s->state != RF_2A4M1_SME_CONNECTED || !target)
		return RF_2A4M1_S8021X_ERR_STATE;
	s->ft_target = *target;
	s->ft_completed = false;
	s->ric_resp_received = false;
	s->ric_granted = false;
	rf_2a4m1_ft_gen_nonce32(s, s->ft_snonce, s->nonce_salt + 30);
	rf_2a4m1_ft_tx_auth1(s);
	return RF_2A4M1_S8021X_OK;
}

rf_2a4m1_s8021x_status rf_2a4m1_sme_ft_roam_over_ds(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *target)
{
	if (s->role != RF_2A4M1_SME_ROLE_STA || !s->ft_enabled || !s->ft_r0_valid ||
	    s->state != RF_2A4M1_SME_CONNECTED || !target)
		return RF_2A4M1_S8021X_ERR_STATE;
	s->ft_target = *target;
	s->ft_completed = false;
	s->ric_resp_received = false;
	s->ric_granted = false;
	rf_2a4m1_ft_gen_nonce32(s, s->ft_snonce, s->nonce_salt + 40);   /* fresh SNonce (distinct from over-air) */
	rf_2a4m1_ft_tx_ds_request(s);                                   /* FT Request → the current AP over the air */
	return RF_2A4M1_S8021X_OK;
}

void rf_2a4m1_sme_set_ric_request(struct rf_2a4m1_sme *s, const struct rf_2a4m1_tspec *t)
{
	if (t) { s->ric_req = *t; s->ric_request_on = true; }
}

void rf_2a4m1_sme_set_ric_deny(struct rf_2a4m1_sme *s, bool deny) { s->ric_ap_deny = deny; }

void rf_2a4m1_sme_set_ht_advertise(struct rf_2a4m1_sme *s, bool on) { s->ht_advertise = on; }

void rf_2a4m1_sme_set_ht_oper(struct rf_2a4m1_sme *s, const struct ht_oper *o)
{ if (o) { s->ht_oper_advert = *o; s->ht_oper_advert_set = true; } }

rf_2a4m1_s8021x_status rf_2a4m1_sme_ba_establish(struct rf_2a4m1_sme *s, uint8_t tid)
{
	uint8_t body[16];
	size_t n;
	if (s->state != RF_2A4M1_SME_CONNECTED)
		return RF_2A4M1_S8021X_ERR_STATE;
	/* originator ADDBA through the REUSED ht/block_ack state machine (IDLE→PENDING). */
	n = rf_2a4m1_ba_originator_addba(&s->ba, body, sizeof body,
	                        /*dialog_token=*/1, tid, /*amsdu=*/false, /*immediate=*/true,
	                        /*buf_size=*/64, /*timeout=*/0, /*ssn=*/0);
	if (n == 0)
		return RF_2A4M1_S8021X_ERR_INVAL;
	rf_2a4m1_tx_action_dot11(s, body, n);   /* real 802.11 Action ADDBA Request */
	s->ba_tx_req++;
	return RF_2A4M1_S8021X_OK;
}

