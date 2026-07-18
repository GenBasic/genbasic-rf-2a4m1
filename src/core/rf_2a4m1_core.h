// SPDX-License-Identifier: GPL-2.0
//
// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
// Shared core declarations (types, constants, prototypes).
//
// Copyright (c) GenBasic.
// Licensed under the GNU General Public License, version 2.
//
// This file is machine-generated. Do not hand-edit.

// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB dongles.
//
// Device support (USB VID:PID match table):
//   0e8d:7601   MediaTek MT7601U
//   148f:7601   Ralink/MediaTek MT7601U
//   148f:760b   Ralink/MediaTek MT7601U (OEM)
//
// This driver is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2, as
// published by the Free Software Foundation.  MODULE_LICENSE("GPL").

#ifndef RF_2A4M1_CORE_H
#define RF_2A4M1_CORE_H

#ifdef RF_2A4M1_FREESTANDING
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
/* Host/kernel/platform-supplied primitives. */
void *memcpy(void *, const void *, size_t);
void *memset(void *, int, size_t);
#else
#include <linux/types.h>	/* uint8_t..uint64_t, bool, size_t */
#include <linux/string.h>	/* memcpy / memset */
#endif
size_t plat_random(uint8_t *buf, size_t len);
uint64_t plat_time_ms(void);
void plat_crit_enter(void);
void plat_crit_exit(void);

/* -- constants (macros) -- */
#define RF_2A4M1_AES128_KEY_LEN 16
#define AES256_KEY_LEN 32
#define RF_2A4M1_AES_BLOCK_LEN  16
#define BA_MAX_WIN               64
#define BA_PARAM_AMSDU           0x0001
#define BA_PARAM_BUFSIZE_MASK    0xffc0
#define BA_PARAM_BUFSIZE_SHIFT   6
#define BA_PARAM_POLICY_IMMEDIATE 0x0002
#define BA_PARAM_TID_MASK        0x003c
#define BA_PARAM_TID_SHIFT       2
#define RF_2A4M1_BIP_IGTK_LEN     16
#define RF_2A4M1_BIP_IPN_LEN      6
#define RF_2A4M1_BIP_KEYID_LEN    2
#define RF_2A4M1_BIP_MGMT_HDR_LEN 24
#define RF_2A4M1_BIP_MIC_LEN      8
#define RF_2A4M1_BIP_MMIE_BODY    (RF_2A4M1_BIP_KEYID_LEN + RF_2A4M1_BIP_IPN_LEN + RF_2A4M1_BIP_MIC_LEN)
#define RF_2A4M1_BIP_MMIE_EID     76
#define RF_2A4M1_BIP_MMIE_LEN     (2 + RF_2A4M1_BIP_MMIE_BODY)
#define RF_2A4M1_CCMP_HDR_LEN 8
#define RF_2A4M1_CCMP_MIC_LEN 8
#define DELBA_PARAM_INITIATOR    0x0800
#define DELBA_PARAM_TID_MASK     0xf000
#define DELBA_PARAM_TID_SHIFT    12
#define RF_2A4M1_EAPOL_DESC_RSN  2
#define RF_2A4M1_EAPOL_FIXED_LEN 99
#define EAPOL_HDR_LEN   4
#define RF_2A4M1_EAPOL_MIC_LEN   16
#define RF_2A4M1_EAPOL_TYPE_KEY  3
#define RF_2A4M1_EAPOL_VERSION   2
#define RF_2A4M1_EDCA_ACI_MASK             0x60
#define RF_2A4M1_EDCA_ACI_SHIFT            5
#define RF_2A4M1_EDCA_AIFSN_MASK           0x0f
#define RF_2A4M1_EDCA_PARAM_BODY_LEN       18
#define RF_2A4M1_ETH_ALEN 6
#define RF_2A4M1_FT_ACTION_REQUEST   1
#define RF_2A4M1_FT_ACTION_REQ_HDR   14
#define RF_2A4M1_FT_ACTION_RESPONSE  2
#define RF_2A4M1_FT_ACTION_RESP_HDR  16
#define RF_2A4M1_FT_FTE_MIC_LEN 16
#define RF_2A4M1_FT_KEY_NAME_LEN     16
#define RF_2A4M1_FT_MDE_BODY_LEN 3
#define RF_2A4M1_FT_MDE_CAP_FT_OVER_DS  0x01
#define RF_2A4M1_FT_MDID_LEN          2
#define RF_2A4M1_FT_NAME_SALT_LEN    16
#define RF_2A4M1_FT_NONCE_LEN        32
#define RF_2A4M1_FT_PMK_R0_LEN       32
#define RF_2A4M1_FT_PMK_R1_LEN       32
#define RF_2A4M1_FT_PTK_LEN          48
#define RF_2A4M1_HAL_BAND_2G4  0x01
#define HAL_MAX_CHAINS 4
#define HAL_TX_MAX_STAGES 4
#define HT_CAP_BODY_LEN        26
#define HT_CAP_SGI_20            0x0020
#define HT_CAP_SGI_40            0x0040
#define HT_CAP_SUP_WIDTH_20_40   0x0002
#define HT_MCS_SET_LEN         16
#define HT_OPER_BODY_LEN       22
#define RF_2A4M1_MGMT_HDR_LEN 24
#define RF_2A4M1_MGMT_SSID_MAX 32
#define RF_2A4M1_MT7601U_RXWI_SIZE 28
#define RF_2A4M1_MT7601U_TXWI_SIZE 20
#define MT7601U_USB_MAX_FRAME  2048
#define RF_2A4M1_MT_DMA_FCE_LEN   4
#define RF_2A4M1_MT_DMA_HDRS      8
#define RF_2A4M1_MT_DMA_HDR_LEN   4
#define RF_2A4M1_MT_INFO_TYPE_PACKET 0
#define RF_2A4M1_MT_MSG_PORT_WLAN    0
#define RF_2A4M1_MT_PHY_TYPE_CCK   0
#define RF_2A4M1_MT_PHY_TYPE_HT    2
#define RF_2A4M1_MT_PHY_TYPE_HT_GF 3
#define RF_2A4M1_MT_PHY_TYPE_OFDM  1
#define RF_2A4M1_MT_QSEL_EDCA        2
#define RF_2A4M1_MT_RATE_BW_40        0x0080
#define RF_2A4M1_MT_RATE_MCS_MASK     0x007f
#define RF_2A4M1_MT_RATE_PHY_MASK     0xc000
#define RF_2A4M1_MT_RATE_PHY_SHIFT    14
#define RF_2A4M1_MT_RATE_SGI          0x0100
#define RF_2A4M1_MT_RATE_STBC_MASK    0x0600
#define RF_2A4M1_MT_RATE_STBC_SHIFT   9
#define RF_2A4M1_MT_RXD_INFO_LEN_MASK   0x00003fffu
#define RF_2A4M1_MT_RXD_PKT_INFO_80211  0x00080000u
#define RF_2A4M1_MT_RXINFO_AMPDU    0x00008000u
#define RF_2A4M1_MT_RXINFO_CRCERR   0x00000100u
#define RF_2A4M1_MT_RXINFO_DECRYPT  0x00010000u
#define RF_2A4M1_MT_RXINFO_ICVERR   0x00000200u
#define RF_2A4M1_MT_RXINFO_MICERR   0x00000400u
#define RF_2A4M1_MT_RXWI_CTL_BSS_IDX_MASK  0x00001c00u
#define RF_2A4M1_MT_RXWI_CTL_BSS_IDX_SHIFT 10
#define RF_2A4M1_MT_RXWI_CTL_KEY_IDX_MASK  0x00000300u
#define RF_2A4M1_MT_RXWI_CTL_KEY_IDX_SHIFT 8
#define RF_2A4M1_MT_RXWI_CTL_MPDU_LEN_MASK  0x0fff0000u
#define RF_2A4M1_MT_RXWI_CTL_MPDU_LEN_SHIFT 16
#define RF_2A4M1_MT_RXWI_CTL_TID_MASK      0xf0000000u
#define RF_2A4M1_MT_RXWI_CTL_TID_SHIFT     28
#define RF_2A4M1_MT_RXWI_CTL_WCID_MASK     0x000000ffu
#define RF_2A4M1_MT_RXWI_GAIN_LNA_ID_SHIFT  6
#define RF_2A4M1_MT_RXWI_GAIN_RSSI_VAL_MASK 0x3f
#define RF_2A4M1_MT_RXWI_OFF_CTL      4
#define RF_2A4M1_MT_RXWI_OFF_FREQ_OFF 19
#define RF_2A4M1_MT_RXWI_OFF_GAIN     18
#define RF_2A4M1_MT_RXWI_OFF_RATE     10
#define RF_2A4M1_MT_RXWI_OFF_RXINFO   0
#define RF_2A4M1_MT_RXWI_OFF_SNR      16
#define RF_2A4M1_MT_TXD_INFO_DPORT_SHIFT    27
#define RF_2A4M1_MT_TXD_INFO_LEN_MASK       0x0000ffffu
#define RF_2A4M1_MT_TXD_INFO_TYPE_SHIFT     30
#define RF_2A4M1_MT_TXD_PKT_INFO_80211      0x00080000u
#define RF_2A4M1_MT_TXD_PKT_INFO_QSEL_SHIFT 25
#define RF_2A4M1_MT_TXD_PKT_INFO_WIV        0x01000000u
#define RF_2A4M1_MT_TXS_FIFO_ACKREQ      0x00000080u
#define RF_2A4M1_MT_TXS_FIFO_AGGR        0x00000040u
#define RF_2A4M1_MT_TXS_FIFO_PID_MASK    0x0000001eu
#define RF_2A4M1_MT_TXS_FIFO_PID_SHIFT   1
#define RF_2A4M1_MT_TXS_FIFO_RATE_MASK   0xffff0000u
#define RF_2A4M1_MT_TXS_FIFO_RATE_SHIFT  16
#define RF_2A4M1_MT_TXS_FIFO_SUCCESS     0x00000020u
#define RF_2A4M1_MT_TXS_FIFO_VALID  0x00000001u
#define RF_2A4M1_MT_TXS_FIFO_WCID_MASK   0x0000ff00u
#define RF_2A4M1_MT_TXS_FIFO_WCID_SHIFT  8
#define RF_2A4M1_MT_TXWI_ACK_CTL_NSEQ  0x02
#define RF_2A4M1_MT_TXWI_ACK_CTL_REQ   0x01
#define RF_2A4M1_MT_TXWI_FLAGS_AMPDU   0x0010
#define RF_2A4M1_MT_TXWI_LEN_BYTE_MASK  0x0fff
#define RF_2A4M1_MT_TXWI_LEN_PKTID_MASK  0xf000
#define RF_2A4M1_MT_TXWI_LEN_PKTID_SHIFT 12
#define RF_2A4M1_MT_TXWI_OFF_ACK_CTL  4
#define RF_2A4M1_MT_TXWI_OFF_EIV      12
#define RF_2A4M1_MT_TXWI_OFF_FLAGS    0
#define RF_2A4M1_MT_TXWI_OFF_IV       8
#define RF_2A4M1_MT_TXWI_OFF_LEN      6
#define RF_2A4M1_MT_TXWI_OFF_RATE     2
#define RF_2A4M1_MT_TXWI_OFF_WCID     5
#define RF_2A4M1_P256_COORD_LEN  32
#define RF_2A4M1_P256_POINT_LEN  64
#define RF_2A4M1_P256_SCALAR_LEN 32
#define RF_2A4M1_PMF_ACTION_HT                 7
#define RF_2A4M1_PMF_ACTION_PUBLIC             4
#define RF_2A4M1_PMF_ACTION_SA_QUERY           8
#define RF_2A4M1_PMF_ACTION_SELF_PROTECTED   15
#define RF_2A4M1_PMF_ACTION_UNPROTECTED_DMG  20
#define RF_2A4M1_PMF_ACTION_UNPROTECTED_S1G  23
#define RF_2A4M1_PMF_ACTION_UNPROTECTED_WNM   11
#define RF_2A4M1_PMF_ACTION_VENDOR_SPECIFIC  127
#define RF_2A4M1_PMF_ACTION_VHT              21
#define RF_2A4M1_PMF_AKM_OWE           18
#define RF_2A4M1_PMF_AKM_PSK            2
#define RF_2A4M1_PMF_AKM_SAE            8
#define RF_2A4M1_PMF_BIGTK_KDE_DATATYPE 10
#define RF_2A4M1_PMF_EID_EXTENSION       255
#define RF_2A4M1_PMF_EID_EXT_OCI          54
#define RF_2A4M1_PMF_EID_RSN            48
#define RF_2A4M1_PMF_EID_TIMEOUT_INTERVAL       56
#define RF_2A4M1_PMF_OCI_KDE_DATATYPE     13
#define RF_2A4M1_PMF_OCI_LEN               3
#define RF_2A4M1_PMF_RSN_CAP_MFPC   0x0080
#define RF_2A4M1_PMF_RSN_CAP_MFPR   0x0040
#define RF_2A4M1_PMF_SA_QUERY_LEN       4
#define RF_2A4M1_PMF_SA_QUERY_REQUEST   0
#define RF_2A4M1_PMF_SA_QUERY_RESPONSE  1
#define RF_2A4M1_PMF_STATUS_COMEBACK             30
#define RF_2A4M1_PMF_STATUS_MFP_VIOLATION        31
#define RF_2A4M1_PMF_STATUS_SUCCESS              0
#define RF_2A4M1_PMF_STYPE_ACTION     13
#define RF_2A4M1_PMF_STYPE_ACTION_NOACK 14
#define PMF_STYPE_BEACON        8
#define RF_2A4M1_PMF_STYPE_DEAUTH     12
#define RF_2A4M1_PMF_STYPE_DISASSOC   10
#define RF_2A4M1_PMF_TD_KDE_DATATYPE 8
#define RF_2A4M1_PMF_TIMEOUT_TYPE_ASSOC_COMEBACK 3
#define RF_2A4M1_PMF_WFA_OUI_0 0x50
#define RF_2A4M1_PMF_WFA_OUI_1 0x6f
#define RF_2A4M1_PMF_WFA_OUI_2 0x9a
#define RF_2A4M1_PN_REPLAY_NONQOS 16
#define RF_2A4M1_PN_REPLAY_PN_MAX 0xffffffffffffULL
#define RF_2A4M1_PN_REPLAY_TIDS   17
#define RF_2A4M1_QOS_ACTION_ADDTS_REQ      0
#define RF_2A4M1_QOS_ACTION_ADDTS_RSP      1
#define RF_2A4M1_QOS_ACTION_DELTS          2
#define RF_2A4M1_QOS_AC_COUNT 4
#define RF_2A4M1_QOS_ADM_DEFAULT_AC_LIMIT  0x8000u
#define RF_2A4M1_QOS_ADM_MAX_TS 8
#define QOS_INFO_UAPSD_BE         0x08
#define QOS_INFO_UAPSD_BK         0x04
#define QOS_INFO_UAPSD_VI         0x02
#define QOS_INFO_UAPSD_VO         0x01
#define RIC_EID_DESCRIPTOR   75
#define RF_2A4M1_RIC_EID_RDE          57
#define RF_2A4M1_RIC_MAX_RESOURCES     4
#define RF_2A4M1_RIC_RDE_BODY_LEN      4
#define RF_2A4M1_RIC_STATUS_DENIED     1
#define RF_2A4M1_RIC_STATUS_SUCCESS    0
#define RF_2A4M1_ROAM_CAT_FT           6
#define ROAM_CAT_RADIO_MEAS   5
#define RF_2A4M1_ROAM_EID_FAST_BSS_TRANS  55
#define ROAM_EID_MEAS_REPORT    39
#define ROAM_EID_MEAS_REQUEST   38
#define RF_2A4M1_ROAM_EID_MOBILITY_DOMAIN 54
#define ROAM_EID_NEIGHBOR_REPORT 52
#define ROAM_EID_RM_ENABLED_CAPS 70
#define ROAM_EID_TPC_REPORT     35
#define RRM_ACT_LINK_MEAS_REP    3
#define RRM_ACT_LINK_MEAS_REQ    2
#define RRM_ACT_NEIGHBOR_REP     5
#define RRM_ACT_NEIGHBOR_REQ     4
#define RRM_CAP_LEN 5
#define RRM_MEAS_TYPE_BEACON     5
#define RRM_NEIGHBOR_MAX 8
#define RRM_NRI_REACHABLE       0x0003
#define RRM_NR_FIXED_LEN 13
#define SAE_CONFIRM_LEN        32
#define SAE_ELEMENT_LEN        64
#define RF_2A4M1_SAE_GROUP_19         19
#define RF_2A4M1_SAE_KEY_LEN 32
#define SAE_SCALAR_LEN         32
#define RF_2A4M1_SHA1_BLOCK_LEN  64
#define RF_2A4M1_SHA1_DIGEST_LEN 20
#define RF_2A4M1_SHA256_BLOCK_LEN  64
#define RF_2A4M1_SHA256_DIGEST_LEN 32
#define RF_2A4M1_SME_DATA_MAX  64
#define RF_2A4M1_SME_FRAME_MAX 320
#define RF_2A4M1_SME_GTK_LEN   16
#define RF_2A4M1_SME_KCK_LEN   16
#define RF_2A4M1_SME_KCK_OFF   0
#define RF_2A4M1_SME_KDE_MAX   128
#define RF_2A4M1_SME_KEK_OFF   16
#define RF_2A4M1_SME_KI_ACK       0x0080
#define SME_KI_ENCRYPTED 0x1000
#define SME_KI_INSTALL   0x0040
#define RF_2A4M1_SME_KI_M1  (SME_KI_VER_AES | SME_KI_PAIRWISE | RF_2A4M1_SME_KI_ACK)
#define RF_2A4M1_SME_KI_M2  (SME_KI_VER_AES | SME_KI_PAIRWISE | RF_2A4M1_SME_KI_MIC)
#define RF_2A4M1_SME_KI_M2_WPA3  (SME_KI_VER_AKM | SME_KI_PAIRWISE | RF_2A4M1_SME_KI_MIC)
#define RF_2A4M1_SME_KI_M3  (SME_KI_VER_AES | SME_KI_PAIRWISE | SME_KI_INSTALL | RF_2A4M1_SME_KI_ACK | RF_2A4M1_SME_KI_MIC | RF_2A4M1_SME_KI_SECURE | SME_KI_ENCRYPTED)
#define RF_2A4M1_SME_KI_M4  (SME_KI_VER_AES | SME_KI_PAIRWISE | RF_2A4M1_SME_KI_MIC | RF_2A4M1_SME_KI_SECURE)
#define RF_2A4M1_SME_KI_M4_WPA3  (SME_KI_VER_AKM | SME_KI_PAIRWISE | RF_2A4M1_SME_KI_MIC | RF_2A4M1_SME_KI_SECURE)
#define RF_2A4M1_SME_KI_MIC       0x0100
#define SME_KI_PAIRWISE  0x0008
#define RF_2A4M1_SME_KI_SECURE    0x0200
#define SME_KI_VER_AES   0x0002
#define SME_KI_VER_AKM   0x0000
#define RF_2A4M1_SME_MIC_LEN   16
#define RF_2A4M1_SME_NONCE_LEN 32
#define RF_2A4M1_SME_PMK_LEN   32
#define SME_PTK_LEN   48
#define RF_2A4M1_SME_TK_LEN    16
#define RF_2A4M1_SME_TK_OFF    32
#define TDLS_ACT_DISCOVERY_REQ   10
#define RF_2A4M1_TDLS_ACT_SETUP_CONFIRM   2
#define RF_2A4M1_TDLS_ACT_SETUP_REQ       0
#define RF_2A4M1_TDLS_ACT_SETUP_RESP      1
#define RF_2A4M1_TDLS_ACT_TEARDOWN        3
#define RF_2A4M1_TDLS_CATEGORY            12
#define RF_2A4M1_TDLS_EID_FTIE             55
#define RF_2A4M1_TDLS_EID_LINK_ID          101
#define RF_2A4M1_TDLS_EID_TIMEOUT_INTERVAL 56
#define RF_2A4M1_TDLS_FTIE_FIXED   82
#define RF_2A4M1_TDLS_LINK_ID_BODY 18
#define RF_2A4M1_TDLS_MIC_LEN      16
#define RF_2A4M1_TDLS_MIC_SEQ_CONFIRM  3
#define RF_2A4M1_TDLS_MIC_SEQ_RESP     2
#define RF_2A4M1_TDLS_MIC_SEQ_TEARDOWN 4
#define RF_2A4M1_TDLS_NONCE_LEN    32
#define RF_2A4M1_TDLS_PAYLOAD_TYPE        0x02
#define TDLS_PUBLIC_ACT_DISC_RESP 14
#define TDLS_PUBLIC_CATEGORY     4
#define RF_2A4M1_TDLS_TIE_BODY     5
#define RF_2A4M1_TDLS_TIE_TYPE_KEY_LIFETIME 2
#define RF_2A4M1_TDLS_TPK_KCK_LEN  16
#define RF_2A4M1_TDLS_TPK_LEN      (RF_2A4M1_TDLS_TPK_KCK_LEN + TDLS_TPK_TK_LEN)
#define TDLS_TPK_TK_LEN   16
#define RF_2A4M1_TSPEC_BODY_LEN            55
#define WLAN_ACTION_BLOCK_ACK    3
#define RF_2A4M1_WLAN_ACTION_CAT_QOS       1
#define RF_2A4M1_WLAN_AUTH_OPEN       0
#define RF_2A4M1_WLAN_AUTH_SAE        3
#define WLAN_BA_ACTION_ADDBA_REQ 0
#define WLAN_BA_ACTION_ADDBA_RSP 1
#define WLAN_BA_ACTION_DELBA     2
#define RF_2A4M1_WLAN_CAPABILITY_ESS      0x0001
#define RF_2A4M1_WLAN_CAPABILITY_PRIVACY  0x0010
#define RF_2A4M1_WLAN_CAPABILITY_SHORT_PREAMBLE 0x0020
#define RF_2A4M1_WLAN_EID_DS_PARAMS   3
#define RF_2A4M1_WLAN_EID_EDCA_PARAM_SET   12
#define RF_2A4M1_WLAN_EID_EXTENSION   255
#define RF_2A4M1_WLAN_EID_EXT_OWE_DH_PARAM 32
#define WLAN_EID_HT_CAP        45
#define WLAN_EID_HT_OPERATION  61
#define RF_2A4M1_WLAN_EID_RSN         48
#define RF_2A4M1_WLAN_EID_SSID        0
#define RF_2A4M1_WLAN_EID_SUPP_RATES  1
#define RF_2A4M1_WLAN_EID_TSPEC            13
#define RF_2A4M1_WLAN_EID_VENDOR_SPECIFIC  221
#define RF_2A4M1_WLAN_FC_STYPE_ACTION     13
#define RF_2A4M1_WLAN_FC_STYPE_ASSOC_REQ  0
#define RF_2A4M1_WLAN_FC_STYPE_ASSOC_RESP 1
#define RF_2A4M1_WLAN_FC_STYPE_AUTH       11
#define RF_2A4M1_WLAN_FC_STYPE_BEACON     8
#define WLAN_FC_STYPE_DEAUTH     12
#define RF_2A4M1_WLAN_FC_STYPE_PROBE_REQ  4
#define RF_2A4M1_WLAN_FC_STYPE_PROBE_RESP 5
#define RF_2A4M1_WLAN_FC_STYPE_REASSOC_REQ  2
#define RF_2A4M1_WLAN_FC_STYPE_REASSOC_RESP 3
#define RF_2A4M1_WLAN_FC_TYPE_MGMT        0
#define RF_2A4M1_WLAN_STATUS_SAE_HASH_TO_ELEMENT   126
#define RF_2A4M1_WLAN_STATUS_SUCCESS               0
#define RF_2A4M1_WMM_INFO_BODY_LEN         7
#define RF_2A4M1_WMM_OUI_0                 0x00
#define RF_2A4M1_WMM_OUI_1                 0x50
#define RF_2A4M1_WMM_OUI_2                 0xf2
#define RF_2A4M1_WMM_OUI_SUBTYPE_INFO      0
#define RF_2A4M1_WMM_OUI_SUBTYPE_PARAM     1
#define RF_2A4M1_WMM_OUI_TYPE              2
#define RF_2A4M1_WMM_PARAM_BODY_LEN        24
#define RF_2A4M1_WMM_VERSION               1
#define WNM_GTK_LEN   16
#define WNM_GTK_RSC_LEN 8
#define RF_2A4M1_WPA_NONCE_LEN 32
#define RF_2A4M1_WPA_PMK_LEN   32
#define RF_2A4M1_WPA_PTK_LEN   48

/* -- forward declarations -- */
struct aes256_ctx;
struct ba_agreement;
struct coex_afh_map;
struct eht_cap;
struct eht_oper;
struct fils_discovery;
struct hal_caps;
struct hal_tx_rate_stage;
struct he_cap;
struct he_oper;
struct ht_cap;
struct ht_oper;
struct reorder_buf;
struct rf_2a4m1_aes128_ctx;
struct rf_2a4m1_beacon_info;
struct rf_2a4m1_chan_def;
struct rf_2a4m1_edca_ac;
struct rf_2a4m1_edca_ac_rec;
struct rf_2a4m1_edca_param;
struct rf_2a4m1_ft_action_req;
struct rf_2a4m1_ft_action_resp;
struct rf_2a4m1_ft_fte;
struct rf_2a4m1_hal;
struct rf_2a4m1_hal_cfg;
struct rf_2a4m1_hal_ops;
struct rf_2a4m1_hal_tx_status;
struct rf_2a4m1_ie_reader;
struct rf_2a4m1_ie_writer;
struct rf_2a4m1_key;
struct rf_2a4m1_lmac_cfg;
struct rf_2a4m1_mgmt_hdr;
struct rf_2a4m1_mpdu;
struct rf_2a4m1_mt7601u_hal;
struct rf_2a4m1_pmf_igtk;
struct rf_2a4m1_pmf_oci;
struct rf_2a4m1_pmf_sa_query;
struct rf_2a4m1_pn_replay;
struct rf_2a4m1_qos_admission;
struct rf_2a4m1_qos_ctrl;
struct rf_2a4m1_rd;
struct rf_2a4m1_ric_rde;
struct rf_2a4m1_ric_request;
struct rf_2a4m1_ric_response;
struct rf_2a4m1_rxinfo;
struct rf_2a4m1_sae;
struct rf_2a4m1_sae_commit;
struct rf_2a4m1_sae_confirm;
struct rf_2a4m1_sha1_ctx;
struct rf_2a4m1_sha256_ctx;
struct rf_2a4m1_sme;
struct rf_2a4m1_sme_crypto;
struct rf_2a4m1_sta_cfg;
struct rf_2a4m1_tdls_ftie;
struct rf_2a4m1_tdls_link_id;
struct rf_2a4m1_tdls_peer;
struct rf_2a4m1_tdls_setup_confirm;
struct rf_2a4m1_tdls_setup_req;
struct rf_2a4m1_tdls_setup_resp;
struct rf_2a4m1_tdls_teardown;
struct rf_2a4m1_tim;
struct rf_2a4m1_ts_admission;
struct rf_2a4m1_ts_info;
struct rf_2a4m1_tspec;
struct rf_2a4m1_tx_params;
struct ric_descriptor;
struct rrm_beacon_report;
struct rrm_beacon_req;
struct rrm_link_report;
struct rrm_neighbor;
struct rrm_neighbor_table;
struct tdls_discovery_req;
struct tdls_discovery_resp;
struct twt_agreement;
struct vht_cap;
struct vht_oper;
struct wnm_gtk;
struct wnm_sleep_ap;
struct wnm_sleep_sta;

/* -- types -- */
typedef struct { uint8_t a[RF_2A4M1_ETH_ALEN]; } rf_2a4m1_mac_addr;
typedef enum {
	RF_2A4M1_S8021X_OK         =  0,
	RF_2A4M1_S8021X_ERR_INVAL  = -1,    
	S8021X_ERR_NOSPACE= -2,    
	RF_2A4M1_S8021X_ERR_STATE  = -3,    
	S8021X_ERR_NOTFOUND= -4,   
	RF_2A4M1_S8021X_ERR_REJECTED= -5,   
} rf_2a4m1_s8021x_status;
enum hal_gen { RF_2A4M1_HAL_GEN_LEGACY = 0, RF_2A4M1_HAL_GEN_HT = 4};
enum hal_phy_mode {
	RF_2A4M1_HAL_PHY_CCK = 0,    
	RF_2A4M1_HAL_PHY_OFDM,       
	RF_2A4M1_HAL_PHY_HT,         
	HAL_PHY_VHT,        
	HAL_PHY_HE,         
	HAL_PHY_EHT,        
};
enum rf_2a4m1_hal_cal { HAL_CAL_ALL = 0, HAL_CAL_RF, HAL_CAL_RX_DCOC, HAL_CAL_TX_LO, HAL_CAL_TEMP };
enum hal_opmode { RF_2A4M1_HAL_OPMODE_STA = 0, RF_2A4M1_HAL_OPMODE_AP, HAL_OPMODE_MONITOR, HAL_OPMODE_MESH };
struct hal_caps {
	uint8_t  max_gen;           
	uint8_t  bands;             
	uint16_t max_bw_mhz;        
	uint8_t  tx_chains, rx_chains;
	uint16_t max_mcs;           
	uint32_t feat;              
	uint8_t  n_links;           
	bool     lower_mac_does_acks;   
	bool     crypto_offload;        
	uint8_t  n_key_slots;           
	uint8_t  n_sta_slots;           
};
struct rf_2a4m1_chan_def {
	uint8_t  band;              
	uint16_t center_mhz;        
	uint16_t center2_mhz;       
	uint16_t width_mhz;         
	uint8_t  primary_idx;       
};
struct rf_2a4m1_key {
	uint8_t  type;              
	uint8_t  cipher;            
	uint8_t  key_id;            
	uint8_t  key_len;           
	uint8_t  material[32];      
	rf_2a4m1_mac_addr peer;              
};
struct rf_2a4m1_sta_cfg {
	rf_2a4m1_mac_addr addr;
	uint16_t aid;
	uint8_t  gen;               
	uint8_t  n_ss;              
	uint16_t max_mcs;
	uint16_t ampdu_len;         
	bool     qos, ht;
};
struct rf_2a4m1_edca_ac { uint16_t cw_min, cw_max; uint8_t aifsn; uint16_t txop; };
struct rf_2a4m1_lmac_cfg {
	struct rf_2a4m1_edca_ac ac[4];       
	uint16_t sifs_us, slot_us;
	bool     auto_resp;         
	uint8_t  rts_retry, data_retry;
};
struct rf_2a4m1_mpdu { uint8_t *data; uint16_t len; uint16_t cap; };
struct rf_2a4m1_tx_params {
	uint8_t  wcid;              
	uint8_t  ac;                
	uint8_t  tid;               
	uint8_t  gen;               
	uint8_t  phy_mode;          
	uint16_t mcs;               
	uint16_t bw_mhz;
	uint8_t  n_ss;
	 
	bool     short_gi, ldpc, stbc;
	bool     ampdu;             
	uint8_t  ba_size;           
	uint8_t  ampdu_density;     
	bool     hw_seqno;          
	int8_t   power_dbm;
	uint8_t  retries;
	bool     no_ack;            
	uint8_t  key_slot;          
	uint32_t iv, eiv;           
	bool     iv_valid;          
	uint8_t  link_id;           
	uint8_t  pktid;             
};
struct rf_2a4m1_rxinfo {
	const uint8_t *data; uint16_t len;
	uint8_t  gen;               
	uint8_t  phy_mode;          
	uint16_t mcs, bw_mhz;
	uint8_t  n_ss;
	bool     short_gi;
	bool     stbc;              
	int8_t   rssi, snr;         
	int8_t   rssi_chain[HAL_MAX_CHAINS];   
	int8_t   freq_off;          
	uint8_t  antenna;           
	 
	bool     fcs_ok;            
	bool     crc_err;           
	bool     phy_err;           
	bool     crypto_ok;         
	bool     ampdu;             
	 
	uint8_t  wcid;              
	uint8_t  bss_idx;           
	uint8_t  tid;               
	uint8_t  key_idx;           
	uint8_t  link_id;           
};
typedef void (*hal_rx_cb)(struct rf_2a4m1_hal *h, const struct rf_2a4m1_rxinfo *rx, void *ctx);
struct hal_tx_rate_stage {
	uint8_t  phy_mode;          
	uint16_t mcs;               
	uint16_t bw_mhz;
	uint8_t  tries;             
};
struct rf_2a4m1_hal_tx_status {
	 
	uint8_t  cookie;            
	uint8_t  wcid;              
	uint8_t  tid;               
	uint8_t  link_id;           
	 
	bool     valid;             
	bool     acked;             
	bool     failed;            
	bool     no_ack;            
	bool     ampdu;             
	bool     is_probe;          
	uint8_t  retries;           
	 
	uint8_t  phy_mode;          
	uint8_t  gen;               
	uint16_t mcs;               
	uint16_t bw_mhz;
	uint8_t  n_ss;
	bool     short_gi;
	bool     stbc;
	 
	uint8_t  n_stages;          
	struct hal_tx_rate_stage stage[HAL_TX_MAX_STAGES];
};
typedef void (*hal_tx_status_cb)(struct rf_2a4m1_hal *h, const struct rf_2a4m1_hal_tx_status *ts, void *ctx);
struct rf_2a4m1_hal_cfg {
	rf_2a4m1_mac_addr    addr;           
	uint8_t     opmode;         
	struct rf_2a4m1_chan_def chan;       
	hal_rx_cb   rx_cb;          
	void       *rx_ctx;         
};
struct rf_2a4m1_hal_ops {
	 
	int (*start)(struct rf_2a4m1_hal *h, const struct rf_2a4m1_hal_cfg *cfg);
	void (*stop)(struct rf_2a4m1_hal *h);
	int (*set_channel)(struct rf_2a4m1_hal *h, const struct rf_2a4m1_chan_def *c);      
	 
	int (*tx)(struct rf_2a4m1_hal *h, struct rf_2a4m1_mpdu *m, const struct rf_2a4m1_tx_params *tp);
	int (*set_key)(struct rf_2a4m1_hal *h, uint8_t slot, const struct rf_2a4m1_key *k);    
	int (*set_sta)(struct rf_2a4m1_hal *h, uint8_t wcid, const struct rf_2a4m1_sta_cfg *s); 
	int (*set_rx_filter)(struct rf_2a4m1_hal *h, uint32_t pass_mask);             
	int (*calibrate)(struct rf_2a4m1_hal *h, enum rf_2a4m1_hal_cal rf_2a4m1_what);                  
	int (*set_lower_mac)(struct rf_2a4m1_hal *h, const struct rf_2a4m1_lmac_cfg *cfg);     
};
struct rf_2a4m1_hal {
	const struct rf_2a4m1_hal_ops *ops;
	struct hal_caps       caps;
	void                 *priv;    
	hal_rx_cb             rx_cb;
	void                 *rx_ctx;
	hal_tx_status_cb      tx_status_cb;    
	void                 *tx_status_ctx;
};
struct rf_2a4m1_sha1_ctx {
	uint32_t h[5];
	uint64_t nbits;               
	uint8_t  buf[RF_2A4M1_SHA1_BLOCK_LEN];
	size_t   buf_len;
};
struct rf_2a4m1_aes128_ctx { uint32_t rk[44]; };
struct aes256_ctx { uint32_t rk[60]; };
enum {
	RF_2A4M1_CCMP_RX_E_CRYPTO = -1,   
	RF_2A4M1_CCMP_RX_E_REPLAY = -2,   
};
struct rf_2a4m1_sha256_ctx {
	uint32_t h[8];
	uint64_t nbits;
	uint8_t  buf[RF_2A4M1_SHA256_BLOCK_LEN];
	size_t   buf_len;
};
struct rf_2a4m1_sae {
	uint8_t pwe[RF_2A4M1_P256_POINT_LEN];             
	uint8_t rnd[RF_2A4M1_SAE_KEY_LEN];                
	uint8_t commit_scalar[RF_2A4M1_SAE_KEY_LEN];      
	uint8_t commit_element[RF_2A4M1_P256_POINT_LEN];  
	uint8_t peer_scalar[RF_2A4M1_SAE_KEY_LEN];
	uint8_t peer_element[RF_2A4M1_P256_POINT_LEN];
	uint8_t kck[RF_2A4M1_SAE_KEY_LEN];
	uint8_t pmk[RF_2A4M1_SAE_KEY_LEN];
	uint8_t pmkid[16];
	bool    have_keys;
};
enum sae_pwe_method { SAE_PWE_HNP = 0, SAE_PWE_H2E = 1 };
enum bip_status {
	RF_2A4M1_BIP_OK            =  0,
	RF_2A4M1_BIP_ERR_MIC       = -1,    
	RF_2A4M1_BIP_ERR_REPLAY    = -2,    
	RF_2A4M1_BIP_ERR_MALFORMED = -3,    
};
struct rf_2a4m1_pn_replay {
	uint64_t highest[RF_2A4M1_PN_REPLAY_TIDS];    
	bool     seen[RF_2A4M1_PN_REPLAY_TIDS];       
};
struct rf_2a4m1_ie_reader { const uint8_t *buf; size_t len; size_t pos; };
struct rf_2a4m1_ie_writer { uint8_t *buf; size_t cap; size_t pos; };
struct rf_2a4m1_mgmt_hdr {
	uint8_t  type;                 
	uint8_t  subtype;              
	uint16_t fc;                   
	rf_2a4m1_mac_addr addr1, addr2, addr3;  
	uint16_t seq_ctrl;
	const uint8_t *body;           
	uint16_t body_len;
};
struct rf_2a4m1_beacon_info {
	uint16_t cap;                      
	uint8_t  channel;                  
	uint8_t  ssid_len;                 
	uint8_t  ssid[RF_2A4M1_MGMT_SSID_MAX];
	bool     has_rsn;                  
};
enum qos_ac  { RF_2A4M1_QOS_AC_BK = 0, RF_2A4M1_QOS_AC_BE = 1, RF_2A4M1_QOS_AC_VI = 2, RF_2A4M1_QOS_AC_VO = 3 };
enum qos_aci { RF_2A4M1_QOS_ACI_BE = 0, RF_2A4M1_QOS_ACI_BK = 1, RF_2A4M1_QOS_ACI_VI = 2, RF_2A4M1_QOS_ACI_VO = 3 };
struct rf_2a4m1_edca_ac_rec {
	uint8_t  aci_aifsn;     
	uint8_t  ecw;           
	uint16_t txop_limit;    
};
struct rf_2a4m1_edca_param {
	uint8_t qos_info;
	struct rf_2a4m1_edca_ac_rec ac[RF_2A4M1_QOS_AC_COUNT];    
};
enum qos_ack_policy {
	RF_2A4M1_QOS_ACK_NORMAL      = 0,    
	QOS_ACK_NOACK       = 1,    
	QOS_ACK_NO_EXPLICIT = 2,    
	QOS_ACK_BLOCK       = 3,    
};
struct rf_2a4m1_qos_ctrl {
	uint8_t tid;           
	bool    eosp;          
	uint8_t ack_policy;    
	bool    amsdu;         
	uint8_t hi;            
};
struct rf_2a4m1_ts_info {
	bool    traffic_type;    
	uint8_t tsid;            
	uint8_t direction;       
	uint8_t access_policy;   
	bool    aggregation;     
	bool    apsd;            
	uint8_t user_priority;   
	uint8_t ack_policy;      
	bool    schedule;        
};
struct rf_2a4m1_tspec {
	struct rf_2a4m1_ts_info ts;
	uint16_t nominal_msdu, max_msdu;
	uint32_t min_si, max_si, inactivity, suspension, service_start;
	uint32_t min_rate, mean_rate, peak_rate, burst_size, delay_bound, min_phy_rate;
	uint16_t surplus_bw;     
	uint16_t medium_time;    
};
enum rf_2a4m1_qos_admit_result {
	RF_2A4M1_QOS_ADMIT_OK = 0,             
	RF_2A4M1_QOS_ADMIT_REFUSED_LIMIT,      
	RF_2A4M1_QOS_ADMIT_REFUSED_FULL,       
	RF_2A4M1_QOS_ADMIT_REFUSED_DUP,        
	RF_2A4M1_QOS_ADMIT_REFUSED_ARG,        
};
struct rf_2a4m1_ts_admission {
	bool     in_use;
	uint8_t  tsid;           
	uint8_t  ac;             
	uint8_t  direction;      
	uint16_t medium_time;    
};
struct rf_2a4m1_qos_admission {
	struct rf_2a4m1_ts_admission ts[RF_2A4M1_QOS_ADM_MAX_TS];
	uint32_t ac_total[RF_2A4M1_QOS_AC_COUNT];    
	uint32_t ac_limit[RF_2A4M1_QOS_AC_COUNT];    
};
struct rf_2a4m1_tim {
	uint8_t dtim_count;
	uint8_t dtim_period;
	uint8_t bitmap_control;    
	const uint8_t *pvb;        
	uint8_t pvb_len;
};
enum rf_2a4m1_pmf_mode {
	RF_2A4M1_PMF_MODE_DISABLED = 0,    
	RF_2A4M1_PMF_MODE_OPTIONAL = 1,    
	RF_2A4M1_PMF_MODE_REQUIRED = 2,    
};
enum pmf_protect_kind {
	PMF_PROTECT_NONE = 0,    
	PMF_PROTECT_BIP  = 1,    
	PMF_PROTECT_CCMP = 2,    
};
struct rf_2a4m1_pmf_igtk {
	uint8_t  rf_2a4m1_key[RF_2A4M1_BIP_IGTK_LEN];    
	uint16_t key_id;               
	uint64_t ipn_tx;               
	uint64_t ipn_rx;               
	bool     installed;
};
enum pmf_sa_query_state {
	RF_2A4M1_PMF_SA_QUERY_IDLE    = 0,    
	RF_2A4M1_PMF_SA_QUERY_PENDING = 1,    
	RF_2A4M1_PMF_SA_QUERY_ANSWERED= 2,    
	RF_2A4M1_PMF_SA_QUERY_TIMEOUT = 3,    
};
struct rf_2a4m1_pmf_sa_query {
	uint8_t  state;
	uint16_t trans_id;       
	uint16_t next_tid;       
	unsigned retries;        
	unsigned max_retries;    
	unsigned timeout_ticks;  
	unsigned elapsed;        
};
struct rf_2a4m1_pmf_oci { uint8_t op_class; uint8_t prim_chan; uint8_t seg1_freq; };
struct rrm_neighbor {
	rf_2a4m1_mac_addr bssid;
	uint32_t bssid_info;
	uint8_t  op_class;
	uint8_t  channel;
	uint8_t  phy_type;
	uint8_t  preference;    
	bool     has_preference;
};
struct rrm_beacon_req {
	uint8_t  token;               
	uint8_t  request_mode;        
	uint8_t  op_class;
	uint8_t  channel;             
	uint16_t rand_interval;       
	uint16_t duration;            
	uint8_t  measure_mode;        
	rf_2a4m1_mac_addr bssid;               
};
struct rrm_beacon_report {
	uint8_t  token;               
	uint8_t  report_mode;         
	uint8_t  op_class;
	uint8_t  channel;
	uint64_t start_time;          
	uint16_t duration;            
	uint8_t  frame_info;          
	uint8_t  rcpi;                
	uint8_t  rsni;                
	rf_2a4m1_mac_addr bssid;
	uint8_t  antenna_id;
	uint32_t parent_tsf;
};
struct rrm_link_report {
	uint8_t dialog_token;
	int8_t  tx_power, link_margin;
	uint8_t rx_antenna, tx_antenna, rcpi, rsni;
};
struct rrm_neighbor_table {
	struct rrm_neighbor n[RRM_NEIGHBOR_MAX];
	size_t count;
};
struct rf_2a4m1_ft_fte {
	uint8_t  element_count;              
	uint8_t  mic[RF_2A4M1_FT_FTE_MIC_LEN];
	uint8_t  anonce[RF_2A4M1_FT_NONCE_LEN];
	uint8_t  snonce[RF_2A4M1_FT_NONCE_LEN];
	rf_2a4m1_mac_addr r1kh_id;    bool has_r1kh;
	uint8_t  r0kh_id[48]; uint8_t r0kh_len; bool has_r0kh;
};
struct rf_2a4m1_ft_action_req {
	rf_2a4m1_mac_addr sta_addr;       
	rf_2a4m1_mac_addr target_addr;    
	const uint8_t *elems;    
	size_t   elems_len;
};
struct rf_2a4m1_ft_action_resp {
	rf_2a4m1_mac_addr sta_addr;
	rf_2a4m1_mac_addr target_addr;
	uint16_t status;
	const uint8_t *elems;
	size_t   elems_len;
};
struct rf_2a4m1_ric_rde { uint8_t rdie_id; uint8_t desc_count; uint16_t status; };
struct ric_descriptor { uint8_t resource_type; uint8_t var[8]; uint8_t var_len; };
struct rf_2a4m1_ric_request { uint8_t rdie_id; size_t count; struct rf_2a4m1_tspec rf_2a4m1_tspec[RF_2A4M1_RIC_MAX_RESOURCES]; };
struct rf_2a4m1_ric_response {
	uint8_t  rdie_id;
	size_t   count;
	uint16_t status[RF_2A4M1_RIC_MAX_RESOURCES];
	bool     granted[RF_2A4M1_RIC_MAX_RESOURCES];
	struct rf_2a4m1_tspec rf_2a4m1_tspec[RF_2A4M1_RIC_MAX_RESOURCES];    
};
struct rf_2a4m1_tdls_link_id {
	rf_2a4m1_mac_addr bssid;
	rf_2a4m1_mac_addr init_addr;    
	rf_2a4m1_mac_addr resp_addr;    
};
struct rf_2a4m1_tdls_ftie {
	uint8_t mic[RF_2A4M1_TDLS_MIC_LEN];
	uint8_t anonce[RF_2A4M1_TDLS_NONCE_LEN];
	uint8_t snonce[RF_2A4M1_TDLS_NONCE_LEN];
};
struct rf_2a4m1_tdls_setup_req {
	uint8_t  dialog_token;
	uint16_t cap_info;
	struct rf_2a4m1_tdls_ftie ftie;         
	uint32_t key_lifetime;
	struct rf_2a4m1_tdls_link_id lid;
};
struct rf_2a4m1_tdls_setup_resp {
	uint16_t status;
	uint8_t  dialog_token;
	uint16_t cap_info;
	struct rf_2a4m1_tdls_ftie ftie;         
	uint32_t key_lifetime;
	struct rf_2a4m1_tdls_link_id lid;
};
struct rf_2a4m1_tdls_setup_confirm {
	uint16_t status;
	uint8_t  dialog_token;
	struct rf_2a4m1_tdls_ftie ftie;         
	uint32_t key_lifetime;
	struct rf_2a4m1_tdls_link_id lid;
};
struct rf_2a4m1_tdls_teardown {
	uint16_t reason;
	bool     has_ftie;             
	struct rf_2a4m1_tdls_ftie ftie;
	struct rf_2a4m1_tdls_link_id lid;
};
struct tdls_discovery_req {
	uint8_t  dialog_token;
	struct rf_2a4m1_tdls_link_id lid;
};
struct tdls_discovery_resp {
	uint8_t  dialog_token;
	uint16_t cap_info;
	struct rf_2a4m1_tdls_link_id lid;
};
enum tdls_state {
	RF_2A4M1_TDLS_IDLE = 0,      
	RF_2A4M1_TDLS_REQ_SENT,      
	RF_2A4M1_TDLS_SETUP,         
	RF_2A4M1_TDLS_ESTABLISHED,   
	RF_2A4M1_TDLS_TEARDOWN,      
};
struct rf_2a4m1_tdls_peer {
	uint8_t  state;                
	bool     is_initiator;
	rf_2a4m1_mac_addr self;
	rf_2a4m1_mac_addr peer;
	rf_2a4m1_mac_addr bssid;
	struct rf_2a4m1_tdls_link_id lid;       
	uint8_t  snonce[RF_2A4M1_TDLS_NONCE_LEN];   
	uint8_t  anonce[RF_2A4M1_TDLS_NONCE_LEN];   
	uint8_t  tpk[RF_2A4M1_TDLS_TPK_LEN];
	bool     have_tpk;
	uint8_t  dialog_token;
	uint32_t key_lifetime;
};
struct wnm_gtk {
	uint8_t rf_2a4m1_key[WNM_GTK_LEN];
	uint8_t key_id;                     
	uint8_t len;                        
	uint8_t rsc[WNM_GTK_RSC_LEN];       
	bool    installed;
};
struct wnm_sleep_sta {
	uint8_t  state;
	uint8_t  dialog_token;      
	uint16_t interval;          
	uint8_t  last_status;       
	struct wnm_gtk  gtk;        
	struct rf_2a4m1_pmf_igtk igtk;       
	bool     keys_refreshed;    
};
struct wnm_sleep_ap {
	struct wnm_gtk gtk;                  
	uint8_t  igtk[RF_2A4M1_BIP_IGTK_LEN];         
	uint16_t igtk_key_id;                
	uint64_t igtk_ipn;                   
	bool     sta_sleeping;               
};
struct ht_cap {
	uint16_t cap_info;                  
	uint8_t  ampdu_params;              
	uint8_t  mcs_set[HT_MCS_SET_LEN];   
	uint16_t ext_cap;                   
	uint32_t txbf_cap;                  
	uint8_t  asel_cap;                  
};
struct ht_oper {
	uint8_t primary_chan;                  
	uint8_t rf_2a4m1_info[5];                       
	uint8_t basic_mcs_set[HT_MCS_SET_LEN]; 
};
enum ba_state {
	BA_IDLE = 0,       
	BA_PENDING,        
	BA_ESTABLISHED,    
};
struct ba_agreement {
	enum ba_state state;
	uint8_t  tid;
	uint8_t  dialog_token;
	bool     amsdu;
	bool     immediate;
	uint16_t buf_size;
	uint16_t timeout;
	uint16_t ssn;
};
struct reorder_buf {
	uint16_t win_start;                  
	uint16_t win_size;                   
	bool     present[BA_MAX_WIN];        
	uint16_t sn[BA_MAX_WIN];             
	void   (*deliver)(void *ctx, uint16_t sn);
	void    *ctx;
};
struct rf_2a4m1_rd {
	const uint8_t *p;
	size_t len, pos;
	bool err;
};
struct rf_2a4m1_sae_commit {
	uint16_t group;                       
	uint8_t  scalar[SAE_SCALAR_LEN];      
	uint8_t  element[SAE_ELEMENT_LEN];    
};
struct rf_2a4m1_sae_confirm {
	uint16_t send_confirm;                
	uint8_t  confirm[SAE_CONFIRM_LEN];    
};
enum rf_2a4m1_sme_role  { RF_2A4M1_SME_ROLE_STA = 0, RF_2A4M1_SME_ROLE_AP = 1 };
enum sme_state { RF_2A4M1_SME_INIT = 0, RF_2A4M1_SME_SCANNING, RF_2A4M1_SME_AUTHED, RF_2A4M1_SME_ASSOCED, RF_2A4M1_SME_4WAY, RF_2A4M1_SME_CONNECTED, RF_2A4M1_SME_FAILED };
enum sme_akm   { SME_AKM_PSK = 0, RF_2A4M1_SME_AKM_SAE = 1, RF_2A4M1_SME_AKM_OWE = 2 };
struct rf_2a4m1_sme_crypto {
	 
	void   (*gen_nonce)(uint8_t out[RF_2A4M1_SME_NONCE_LEN], const uint8_t *pmk,
	                    const rf_2a4m1_mac_addr *mac, uint64_t salt);
	 
	void   (*kdf)(uint8_t ptk[SME_PTK_LEN], const uint8_t *pmk,
	              const uint8_t anonce[RF_2A4M1_SME_NONCE_LEN], const uint8_t snonce[RF_2A4M1_SME_NONCE_LEN],
	              const rf_2a4m1_mac_addr *a, const rf_2a4m1_mac_addr *b);
	 
	void   (*mic)(uint8_t out[RF_2A4M1_SME_MIC_LEN], const uint8_t *kck,
	              const uint8_t *frame, size_t len);
	 
	size_t (*key_wrap)(uint8_t *out, size_t out_cap, const uint8_t *kek,
	                   const uint8_t *in, size_t in_len);
	size_t (*key_unwrap)(uint8_t *out, size_t out_cap, const uint8_t *kek,
	                     const uint8_t *in, size_t in_len);
};
struct rf_2a4m1_sme {
	uint8_t   role;
	uint8_t   state;
	rf_2a4m1_mac_addr  self, peer;
	uint8_t   pmk[RF_2A4M1_SME_PMK_LEN];
	const struct rf_2a4m1_sme_crypto *crypto;
	struct rf_2a4m1_hal *rf_2a4m1_hal;            
	uint8_t   anonce[RF_2A4M1_SME_NONCE_LEN], snonce[RF_2A4M1_SME_NONCE_LEN];
	uint8_t   ptk[SME_PTK_LEN], gtk[RF_2A4M1_SME_GTK_LEN];
	uint64_t  replay;           
	bool      ptk_installed, gtk_installed, connected;
	uint64_t  nonce_salt;
	 
	uint16_t  seq;              
	uint16_t  aid;              
	uint8_t   channel;          
	uint8_t   ssid_len;         
	uint8_t   ssid[RF_2A4M1_MGMT_SSID_MAX];
	unsigned  bad_mic;          
	 
	unsigned  eapol_m1_rx, eapol_m2_tx, eapol_m3_rx, eapol_m4_tx;
	unsigned  eapol_m1_early;   
	 
	bool      early_m1_valid;
	uint16_t  early_m1_len;
	uint8_t   early_m1[RF_2A4M1_EAPOL_FIXED_LEN + RF_2A4M1_SME_KDE_MAX];
	 
	unsigned  data_rx;
	uint16_t  rx_data_len;
	uint8_t   rf_2a4m1_rx_data[RF_2A4M1_SME_DATA_MAX];
	 
	uint8_t   akm;                  
	 
	uint8_t   sae_pw_len;
	uint8_t   sae_pw[64];
	struct rf_2a4m1_sae rf_2a4m1_sae;
	uint8_t   sae_phase;            
	bool      sae_ok;               
	unsigned  sae_commit_tx, sae_commit_rx, sae_confirm_tx, sae_confirm_rx;
	 
	uint8_t   owe_priv[RF_2A4M1_P256_SCALAR_LEN];
	uint8_t   owe_pub[RF_2A4M1_P256_POINT_LEN];  
	bool      owe_kp_ok;              
	bool      owe_ok;                 
	unsigned  owe_dh_tx, owe_dh_rx;
	 
	bool             qos_advertise;    
	bool             qos_enabled;      
	struct rf_2a4m1_edca_param edca;            
	struct rf_2a4m1_edca_param edca_advert;     
	bool             edca_advert_set;  
	uint8_t          rx_tid;           
	uint8_t          rx_ac;            
	 
	struct rf_2a4m1_qos_admission admission;    
	uint16_t         addts_last_status;     
	bool             addts_resp_received;   
	uint8_t          addts_dialog;          
	unsigned         addts_grants, addts_denials;   
	 
	uint8_t          rf_2a4m1_pmf_mode;         
	bool             mfp_active;       
	uint16_t         pmf_peer_caps;    
	struct rf_2a4m1_pmf_igtk  igtk;             
	 
	bool             beacon_prot_enabled;  
	struct rf_2a4m1_pmf_igtk  bigtk;            
	bool             ocv_enabled;      
	struct rf_2a4m1_pmf_oci   oci_local;        
	unsigned         ocv_fail;         
	bool             td_advertise;     
	uint8_t          td_advert_bitmap; 
	uint8_t          td_bitmap;        
	bool             td_received;      
	 
	struct rf_2a4m1_pmf_sa_query sa_query;      
	unsigned         sa_query_comebacks;   
	unsigned         sa_query_resolved;    
	unsigned         mgmt_unprot_dropped;  
	unsigned         mgmt_prot_badmic;     
	unsigned         mgmt_prot_honored;    
	 
	bool     ft_enabled;
	uint16_t ft_mdid;                           
	uint8_t  ft_ssid[32];   uint8_t ft_ssid_len;
	uint8_t  ft_r0kh_id[48]; uint8_t ft_r0kh_len;
	uint8_t  ft_pmk_r0[RF_2A4M1_FT_PMK_R0_LEN];
	uint8_t  ft_pmk_r0_name[RF_2A4M1_FT_KEY_NAME_LEN];
	bool     ft_r0_valid;                       
	uint8_t  ft_snonce[RF_2A4M1_FT_NONCE_LEN], ft_anonce[RF_2A4M1_FT_NONCE_LEN];
	uint8_t  ft_pmk_r1[RF_2A4M1_FT_PMK_R1_LEN], ft_pmk_r1_name[RF_2A4M1_FT_KEY_NAME_LEN];
	uint8_t  ft_ptk_name[RF_2A4M1_FT_KEY_NAME_LEN];
	rf_2a4m1_mac_addr ft_target;                         
	bool     ft_completed;                      
	unsigned ft_reassocs;                       
	 
	struct rf_2a4m1_sme *ft_ds_target;                   
	 
	bool     ric_request_on;                    
	struct rf_2a4m1_tspec ric_req;                       
	struct rf_2a4m1_tspec ric_granted_tspec;             
	bool     ric_granted;                       
	uint16_t ric_status;                        
	bool     ric_resp_received;                 
	bool     ric_ap_deny;                       
	unsigned ric_ap_grants, ric_ap_denials;     
	 
	bool           ht_advertise;        
	bool           ht_enabled;          
	struct ht_oper ht_oper;             
	struct ht_oper ht_oper_advert;      
	bool           ht_oper_advert_set;  
	struct ht_cap  ht_peer_cap;         
	bool           ht_peer_cap_set;
	 
	struct ba_agreement ba;             
	bool           ba_recipient;        
	uint8_t        ba_recip_tid;        
	uint16_t       ba_recip_bufsize;    
	 
	struct rf_2a4m1_tdls_peer tdls;         
	bool         tdls_active;      
	struct rf_2a4m1_sme  *tdls_relay;       
	unsigned     tdls_rx;          
	unsigned     tdls_relayed;     
	 
	struct wnm_sleep_sta wnm_sta;   
	struct wnm_sleep_ap  wnm_ap;    
	unsigned     wnm_rekeys;        
	 
	uint64_t         data_tx_pn;          
	uint64_t         mgmt_tx_pn;          
	struct rf_2a4m1_pn_replay data_rx_replay;      
	uint8_t          installed_tk[RF_2A4M1_SME_TK_LEN];  
	bool             data_key_valid;      
	unsigned         data_replay;         
	 
	unsigned         ba_tx_req, ba_rx_rsp, ba_ok;
	unsigned         saq_tx_req, saq_rx_rsp, saq_ok;
	uint16_t         saq_trans_id;
	 
	unsigned         deauth_tx;
	bool             deauth_protected;
};
struct rf_2a4m1_mt7601u_hal {
	struct rf_2a4m1_hal rf_2a4m1_hal;
	 
	uint8_t  last_txwi[RF_2A4M1_MT7601U_TXWI_SIZE];
	uint8_t  last_bulkout[MT7601U_USB_MAX_FRAME];  
	uint16_t last_bulkout_len;
	uint16_t last_frame_len;
	uint32_t tx_count;
};

/* -- function prototypes (public core surface) -- */
size_t rf_2a4m1_addts_req_build(uint8_t *out, size_t cap, uint8_t dialog, const struct rf_2a4m1_tspec *t);
bool rf_2a4m1_addts_req_parse(const uint8_t *in, size_t len, uint8_t *dialog, struct rf_2a4m1_tspec *out);
size_t rf_2a4m1_addts_rsp_build(uint8_t *out, size_t cap, uint8_t dialog, uint16_t status,
                       const struct rf_2a4m1_tspec *t);
bool rf_2a4m1_addts_rsp_parse(const uint8_t *in, size_t len, uint8_t *dialog, uint16_t *status,
                     struct rf_2a4m1_tspec *out);
void rf_2a4m1_aes128_cmac(const uint8_t rf_2a4m1_key[RF_2A4M1_AES128_KEY_LEN],
                 const uint8_t *msg, size_t msg_len, uint8_t mac[RF_2A4M1_AES_BLOCK_LEN]);
void rf_2a4m1_aes128_decrypt_block(const struct rf_2a4m1_aes128_ctx *c,
                          const uint8_t in[RF_2A4M1_AES_BLOCK_LEN], uint8_t out[RF_2A4M1_AES_BLOCK_LEN]);
void rf_2a4m1_aes128_decrypt_block_soft(const struct rf_2a4m1_aes128_ctx *c,
                          const uint8_t in[RF_2A4M1_AES_BLOCK_LEN], uint8_t out[RF_2A4M1_AES_BLOCK_LEN]);
void rf_2a4m1_aes128_encrypt_block(const struct rf_2a4m1_aes128_ctx *c,
                          const uint8_t in[RF_2A4M1_AES_BLOCK_LEN], uint8_t out[RF_2A4M1_AES_BLOCK_LEN]);
void rf_2a4m1_aes128_encrypt_block_soft(const struct rf_2a4m1_aes128_ctx *c,
                          const uint8_t in[RF_2A4M1_AES_BLOCK_LEN], uint8_t out[RF_2A4M1_AES_BLOCK_LEN]);
void rf_2a4m1_aes128_init(struct rf_2a4m1_aes128_ctx *c, const uint8_t rf_2a4m1_key[RF_2A4M1_AES128_KEY_LEN]);
size_t rf_2a4m1_aes_key_unwrap(const uint8_t kek[RF_2A4M1_AES128_KEY_LEN],
                      const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap);
size_t rf_2a4m1_aes_key_wrap(const uint8_t kek[RF_2A4M1_AES128_KEY_LEN],
                    const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap);
size_t rf_2a4m1_ba_build_addba_req(uint8_t *out, size_t cap, uint8_t dialog_token, uint8_t tid,
                          bool amsdu, bool immediate, uint16_t buf_size,
                          uint16_t timeout, uint16_t ssn);
size_t rf_2a4m1_ba_build_addba_rsp(uint8_t *out, size_t cap, uint8_t dialog_token, uint16_t status,
                          uint8_t tid, bool amsdu, bool immediate, uint16_t buf_size,
                          uint16_t timeout);
size_t rf_2a4m1_ba_originator_addba(struct ba_agreement *a, uint8_t *out, size_t cap,
                           uint8_t dialog_token, uint8_t tid, bool amsdu, bool immediate,
                           uint16_t buf_size, uint16_t timeout, uint16_t ssn);
rf_2a4m1_s8021x_status rf_2a4m1_ba_originator_on_addba_rsp(struct ba_agreement *a, const uint8_t *body,
                                         size_t len);
bool rf_2a4m1_ba_parse_addba_req(const uint8_t *body, size_t len, uint8_t *dialog_token,
                        uint8_t *tid, bool *amsdu, bool *immediate, uint16_t *buf_size,
                        uint16_t *timeout, uint16_t *ssn);
bool rf_2a4m1_ba_parse_addba_rsp(const uint8_t *body, size_t len, uint8_t *dialog_token,
                        uint16_t *status, uint8_t *tid, bool *amsdu, bool *immediate,
                        uint16_t *buf_size, uint16_t *timeout);
size_t rf_2a4m1_bip_mmie_protect(uint8_t *out, size_t out_cap,
                        const uint8_t *hdr, const uint8_t *body, size_t body_len,
                        const uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                        const uint8_t ipn[RF_2A4M1_BIP_IPN_LEN], uint16_t key_id);
int rf_2a4m1_bip_mmie_verify(const uint8_t *frame, size_t frame_len,
                    const uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                    uint64_t *replay_ctr, uint16_t *key_id_out);
int rf_2a4m1_ccmp_decrypt(const uint8_t *in, size_t in_len, size_t hdr_len,
                 const uint8_t tk[16], uint8_t *out, size_t out_cap, uint64_t *pn_out);
int rf_2a4m1_ccmp_decrypt_rx(const uint8_t *in, size_t in_len, size_t hdr_len,
                    const uint8_t tk[16], uint8_t *out, size_t out_cap,
                    uint64_t *pn_out, struct rf_2a4m1_pn_replay *rp);
size_t rf_2a4m1_ccmp_encrypt(uint8_t *out, size_t out_cap,
                    const uint8_t *hdr, size_t hdr_len,
                    const uint8_t *payload, size_t payload_len,
                    const uint8_t tk[16], uint8_t key_id, uint64_t pn);
bool rf_2a4m1_crypto_ct_eq(const uint8_t *a, const uint8_t *b, size_t len);
size_t rf_2a4m1_delts_build(uint8_t *out, size_t cap, const struct rf_2a4m1_ts_info *ts, uint16_t reason);
bool rf_2a4m1_delts_parse(const uint8_t *in, size_t len, struct rf_2a4m1_ts_info *ts, uint16_t *reason);
void rf_2a4m1_edca_apply_to_lmac(const struct rf_2a4m1_edca_param *p, struct rf_2a4m1_edca_ac *out);
void rf_2a4m1_edca_defaults(struct rf_2a4m1_edca_param *p);
void rf_2a4m1_edca_fill_lmac_defaults(struct rf_2a4m1_edca_ac *out);
bool rf_2a4m1_edca_param_parse(const uint8_t *body, uint8_t dlen, struct rf_2a4m1_edca_param *out);
size_t rf_2a4m1_ft_action_req_build(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *sta, const rf_2a4m1_mac_addr *target,
                           const uint8_t *elems, size_t elems_len);
bool rf_2a4m1_ft_action_req_parse(const uint8_t *in, size_t len, struct rf_2a4m1_ft_action_req *r);
size_t rf_2a4m1_ft_action_resp_build(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *sta, const rf_2a4m1_mac_addr *target,
                            uint16_t status, const uint8_t *elems, size_t elems_len);
bool rf_2a4m1_ft_action_resp_parse(const uint8_t *in, size_t len, struct rf_2a4m1_ft_action_resp *r);
bool rf_2a4m1_ft_derive_pmk_r0(const uint8_t *xxkey, size_t xxkey_len,
                      const uint8_t *ssid, size_t ssid_len,
                      const uint8_t mdid[RF_2A4M1_FT_MDID_LEN],
                      const uint8_t *r0kh_id, size_t r0kh_len,
                      const rf_2a4m1_mac_addr *s0kh_id,
                      uint8_t pmk_r0[RF_2A4M1_FT_PMK_R0_LEN], uint8_t pmk_r0_name[RF_2A4M1_FT_KEY_NAME_LEN]);
bool rf_2a4m1_ft_derive_pmk_r1(const uint8_t pmk_r0[RF_2A4M1_FT_PMK_R0_LEN],
                      const uint8_t pmk_r0_name[RF_2A4M1_FT_KEY_NAME_LEN],
                      const rf_2a4m1_mac_addr *r1kh_id, const rf_2a4m1_mac_addr *s1kh_id,
                      uint8_t pmk_r1[RF_2A4M1_FT_PMK_R1_LEN], uint8_t pmk_r1_name[RF_2A4M1_FT_KEY_NAME_LEN]);
bool rf_2a4m1_ft_derive_ptk(const uint8_t pmk_r1[RF_2A4M1_FT_PMK_R1_LEN],
                   const uint8_t pmk_r1_name[RF_2A4M1_FT_KEY_NAME_LEN],
                   const uint8_t snonce[RF_2A4M1_FT_NONCE_LEN], const uint8_t anonce[RF_2A4M1_FT_NONCE_LEN],
                   const rf_2a4m1_mac_addr *bssid, const rf_2a4m1_mac_addr *sta_addr,
                   uint8_t ptk[RF_2A4M1_FT_PTK_LEN], uint8_t ptk_name[RF_2A4M1_FT_KEY_NAME_LEN]);
size_t rf_2a4m1_ft_fte_build(uint8_t *out, size_t cap, const struct rf_2a4m1_ft_fte *f);
void rf_2a4m1_ft_fte_mic(const uint8_t kck[16], const rf_2a4m1_mac_addr *sta, const rf_2a4m1_mac_addr *ap, uint8_t seq,
                const uint8_t *mde, size_t mde_len,
                const uint8_t *fte, size_t fte_len,
                const uint8_t key_name[RF_2A4M1_FT_KEY_NAME_LEN],
                const uint8_t *extra, size_t extra_len, uint8_t mic[RF_2A4M1_FT_FTE_MIC_LEN]);
bool rf_2a4m1_ft_fte_parse(const uint8_t *ie, size_t len, struct rf_2a4m1_ft_fte *f);
size_t rf_2a4m1_ft_mde_build(uint8_t *out, size_t cap, uint16_t mdid, uint8_t ft_cap_policy);
bool rf_2a4m1_ft_mde_parse(const uint8_t *ie, size_t len, uint16_t *mdid, uint8_t *ft_cap_policy);
uint16_t rf_2a4m1_get_le16(const uint8_t *p);
uint32_t rf_2a4m1_get_le32(const uint8_t *p);
void rf_2a4m1_hkdf_sha256_expand(const uint8_t *prk, size_t prk_len,
                        const uint8_t *rf_2a4m1_info, size_t info_len,
                        uint8_t *out, size_t out_len);
void rf_2a4m1_hkdf_sha256_extract(const uint8_t *salt, size_t salt_len,
                         const uint8_t *ikm, size_t ikm_len, uint8_t prk[RF_2A4M1_SHA256_DIGEST_LEN]);
void rf_2a4m1_hmac_sha1(const uint8_t *rf_2a4m1_key, size_t key_len,
               const uint8_t *msg, size_t msg_len, uint8_t out[RF_2A4M1_SHA1_DIGEST_LEN]);
void rf_2a4m1_hmac_sha256(const uint8_t *rf_2a4m1_key, size_t key_len,
                 const uint8_t *msg, size_t msg_len, uint8_t out[RF_2A4M1_SHA256_DIGEST_LEN]);
size_t rf_2a4m1_ht_cap_build(uint8_t *out, size_t cap, const struct ht_cap *c);
bool rf_2a4m1_ht_cap_parse(const uint8_t *body, uint8_t dlen, struct ht_cap *out);
size_t rf_2a4m1_ht_oper_build(uint8_t *out, size_t cap, const struct ht_oper *o);
bool rf_2a4m1_ht_oper_parse(const uint8_t *body, uint8_t dlen, struct ht_oper *out);
bool rf_2a4m1_ie_read_next(struct rf_2a4m1_ie_reader *r, uint8_t *id, const uint8_t **data, uint8_t *dlen);
bool rf_2a4m1_ie_write(struct rf_2a4m1_ie_writer *w, uint8_t id, const uint8_t *data, uint8_t dlen);
void rf_2a4m1_ieee80211_kdf_length(const uint8_t *rf_2a4m1_key, size_t key_len,
                          const char *label, size_t label_len,
                          const uint8_t *ctx, size_t ctx_len,
                          uint8_t *out, size_t out_bits);
void rf_2a4m1_ieee80211_prf(const uint8_t *rf_2a4m1_key, size_t key_len,
                   const char *label, size_t label_len,
                   const uint8_t *data, size_t data_len,
                   uint8_t *out, size_t out_len);
size_t rf_2a4m1_mgmt_build_assoc_req(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *bssid,
                            const rf_2a4m1_mac_addr *sa, uint16_t seq_ctrl,
                            uint16_t cap_info, uint16_t listen_interval,
                            const uint8_t *ssid, uint8_t ssid_len,
                            const uint8_t *rates, uint8_t n_rates,
                            const uint8_t *rsn, uint8_t rsn_len,
                            const uint8_t *extra_ies, uint8_t extra_ies_len);
size_t rf_2a4m1_mgmt_build_assoc_resp(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *da,
                             const rf_2a4m1_mac_addr *bssid, uint16_t seq_ctrl, uint16_t cap_info,
                             uint16_t status, uint16_t aid,
                             const uint8_t *rates, uint8_t n_rates,
                             const uint8_t *extra_ies, uint8_t extra_ies_len);
size_t rf_2a4m1_mgmt_build_auth_open(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *bssid,
                            const rf_2a4m1_mac_addr *sa, uint16_t seq_ctrl);
size_t rf_2a4m1_mgmt_build_auth_resp(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *da,
                            const rf_2a4m1_mac_addr *bssid, uint16_t seq_ctrl,
                            uint16_t algo, uint16_t status);
size_t rf_2a4m1_mgmt_build_auth_sae_commit(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *bssid,
                                  const rf_2a4m1_mac_addr *sa, uint16_t seq_ctrl, uint16_t status,
                                  uint16_t group,
                                  const uint8_t *scalar, uint8_t scalar_len,
                                  const uint8_t *element, uint8_t element_len);
size_t rf_2a4m1_mgmt_build_auth_sae_confirm(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *bssid,
                                   const rf_2a4m1_mac_addr *sa, uint16_t seq_ctrl,
                                   uint16_t send_confirm,
                                   const uint8_t *confirm, uint8_t confirm_len);
size_t rf_2a4m1_mgmt_build_beacon(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *bssid,
                         uint16_t seq_ctrl, uint64_t timestamp, uint16_t beacon_int,
                         uint16_t cap_info, const uint8_t *ssid, uint8_t ssid_len,
                         uint8_t channel, const uint8_t *rates, uint8_t n_rates,
                         const uint8_t *rsn, uint8_t rsn_len);
size_t rf_2a4m1_mgmt_build_probe_req(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *da,
                            const rf_2a4m1_mac_addr *sa, const rf_2a4m1_mac_addr *bssid, uint16_t seq_ctrl,
                            const uint8_t *ssid, uint8_t ssid_len,
                            const uint8_t *rates, uint8_t n_rates);
size_t rf_2a4m1_mgmt_build_probe_resp(uint8_t *out, size_t cap, const rf_2a4m1_mac_addr *da,
                             const rf_2a4m1_mac_addr *bssid, uint16_t seq_ctrl, uint64_t timestamp,
                             uint16_t beacon_int, uint16_t cap_info,
                             const uint8_t *ssid, uint8_t ssid_len, uint8_t channel,
                             const uint8_t *rates, uint8_t n_rates,
                             const uint8_t *rsn, uint8_t rsn_len);
bool rf_2a4m1_mgmt_parse_assoc_resp(const uint8_t *body, uint16_t blen,
                           uint16_t *cap, uint16_t *status, uint16_t *aid);
bool rf_2a4m1_mgmt_parse_auth(const uint8_t *body, uint16_t blen,
                     uint16_t *algo, uint16_t *txn, uint16_t *status);
bool rf_2a4m1_mgmt_parse_auth_sae_commit(const uint8_t *body, uint16_t blen, uint16_t *group,
                                const uint8_t **scalar, const uint8_t **element);
bool rf_2a4m1_mgmt_parse_auth_sae_confirm(const uint8_t *body, uint16_t blen, uint16_t *send_confirm,
                                 const uint8_t **confirm);
bool rf_2a4m1_mgmt_parse_beacon(const uint8_t *body, uint16_t blen, struct rf_2a4m1_beacon_info *bi);
bool rf_2a4m1_mgmt_parse_hdr(const uint8_t *f, uint16_t len, struct rf_2a4m1_mgmt_hdr *out);
int rf_2a4m1_mt7601u_build_txwi(uint8_t out[RF_2A4M1_MT7601U_TXWI_SIZE],
                       const struct rf_2a4m1_tx_params *tp, const struct rf_2a4m1_mpdu *m);
void rf_2a4m1_mt7601u_hal_init(struct rf_2a4m1_mt7601u_hal *mh);
int rf_2a4m1_mt7601u_hal_rx_bulk_in(struct rf_2a4m1_mt7601u_hal *mh, const uint8_t *buf, uint16_t len);
int rf_2a4m1_mt7601u_hal_rx_submit(struct rf_2a4m1_mt7601u_hal *mh, const uint8_t *rxwi, uint16_t rxwi_len,
                          const uint8_t *frame, uint16_t frame_len);
int rf_2a4m1_mt7601u_hal_tx_status(struct rf_2a4m1_mt7601u_hal *mh, uint32_t fifo);
int rf_2a4m1_mt7601u_parse_rxwi(const uint8_t *rxwi, uint16_t rxwi_len,
                       const uint8_t *frame, uint16_t frame_len, struct rf_2a4m1_rxinfo *out);
int rf_2a4m1_mt7601u_parse_tx_status(uint32_t fifo, struct rf_2a4m1_hal_tx_status *out);
int rf_2a4m1_mt7601u_usb_rx_unwrap(const uint8_t *in, uint16_t in_len,
                          const uint8_t **rxwi, uint16_t *rxwi_len,
                          const uint8_t **payload, uint16_t *payload_len);
int rf_2a4m1_mt7601u_usb_rx_wrap(uint8_t *out, uint16_t out_cap,
                        const uint8_t *rxwi, uint16_t rxwi_len,
                        const uint8_t *payload, uint16_t payload_len,
                        uint16_t *out_len);
int rf_2a4m1_mt7601u_usb_tx_wrap(uint8_t *out, uint16_t out_cap,
                        const uint8_t *txwi, uint16_t txwi_len,
                        const uint8_t *payload, uint16_t payload_len,
                        uint8_t qsel, bool wiv, uint16_t *out_len);
bool rf_2a4m1_owe_derive(const uint8_t priv[RF_2A4M1_P256_SCALAR_LEN],
                const uint8_t own_pub[RF_2A4M1_P256_POINT_LEN], const uint8_t peer_pub[RF_2A4M1_P256_POINT_LEN],
                bool own_is_client, uint8_t pmk[RF_2A4M1_SHA256_DIGEST_LEN], uint8_t pmkid[16]);
bool rf_2a4m1_p256_ecdh(const uint8_t priv[RF_2A4M1_P256_SCALAR_LEN], const uint8_t peer_pub[RF_2A4M1_P256_POINT_LEN],
               uint8_t dhkey[RF_2A4M1_P256_COORD_LEN]);
bool rf_2a4m1_p256_point_add(const uint8_t a[64], const uint8_t b[64], uint8_t out[64]);
bool rf_2a4m1_p256_point_from_x(const uint8_t x_be[32], uint8_t out[64]);
bool rf_2a4m1_p256_point_mul(const uint8_t scalar[32], const uint8_t pt[64], uint8_t out[64]);
void rf_2a4m1_p256_point_negate(const uint8_t in[64], uint8_t out[64]);
bool rf_2a4m1_p256_point_valid(const uint8_t pt[64]);
bool rf_2a4m1_p256_reduce_mod_p(const uint8_t *in, size_t in_len, uint8_t out[32]);
void rf_2a4m1_p256_scalar_add_n(const uint8_t a[32], const uint8_t b[32], uint8_t out[32]);
void rf_2a4m1_p256_scalar_reduce_mod_nm1_plus1(const uint8_t in[32], uint8_t out[32]);
bool rf_2a4m1_p256_scalar_valid(const uint8_t s[32]);
bool rf_2a4m1_p256_sswu(const uint8_t u_be[32], uint8_t out[64]);
void rf_2a4m1_pbkdf2_hmac_sha1(const uint8_t *pw, size_t pw_len,
                      const uint8_t *salt, size_t salt_len,
                      uint32_t iter, uint8_t *out, size_t dk_len);
bool rf_2a4m1_pmf_action_is_robust(uint8_t category);
size_t rf_2a4m1_pmf_bigtk_kde_build(uint8_t *out, size_t cap, const uint8_t bigtk[RF_2A4M1_BIP_IGTK_LEN],
                           uint16_t key_id, uint64_t bipn);
bool rf_2a4m1_pmf_bigtk_kde_find(const uint8_t *kd, size_t len, uint8_t bigtk[RF_2A4M1_BIP_IGTK_LEN],
                        uint16_t *key_id, uint64_t *bipn);
size_t rf_2a4m1_pmf_build_protected_mgmt(struct rf_2a4m1_pmf_igtk *k, uint8_t *out, size_t cap, uint8_t subtype,
                                const rf_2a4m1_mac_addr *da, const rf_2a4m1_mac_addr *sa, const rf_2a4m1_mac_addr *bssid,
                                const uint8_t *body, size_t body_len, uint16_t seq_ctrl);
size_t rf_2a4m1_pmf_comeback_ie_build(uint8_t *out, size_t cap, uint32_t comeback_tu);
size_t rf_2a4m1_pmf_group_protect(struct rf_2a4m1_pmf_igtk *k, uint8_t *out, size_t cap,
                         const uint8_t *hdr, const uint8_t *body, size_t body_len);
int rf_2a4m1_pmf_group_verify(struct rf_2a4m1_pmf_igtk *k, const uint8_t *frame, size_t len);
void rf_2a4m1_pmf_igtk_install(struct rf_2a4m1_pmf_igtk *k, const uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                      uint16_t key_id, uint64_t ipn_start);
bool rf_2a4m1_pmf_is_robust_mgmt(uint16_t fc, const uint8_t *body, size_t body_len);
uint16_t rf_2a4m1_pmf_mode_caps(enum rf_2a4m1_pmf_mode mode);
uint16_t rf_2a4m1_pmf_negotiate(enum rf_2a4m1_pmf_mode local, uint16_t peer_rsn_caps, bool *mfp_active);
size_t rf_2a4m1_pmf_oci_element_build(uint8_t *out, size_t cap, const struct rf_2a4m1_pmf_oci *oci);
bool rf_2a4m1_pmf_oci_element_parse(const uint8_t *ie, size_t len, struct rf_2a4m1_pmf_oci *oci);
size_t rf_2a4m1_pmf_oci_kde_build(uint8_t *out, size_t cap, const struct rf_2a4m1_pmf_oci *oci);
bool rf_2a4m1_pmf_oci_kde_find(const uint8_t *kd, size_t len, struct rf_2a4m1_pmf_oci *oci);
bool rf_2a4m1_pmf_ocv_verify(const struct rf_2a4m1_pmf_oci *peer_oci, const struct rf_2a4m1_pmf_oci *local_chan);
size_t rf_2a4m1_pmf_rsne_build(uint8_t *out, size_t cap, uint16_t rsn_caps, uint8_t akm);
bool rf_2a4m1_pmf_rsne_parse(const uint8_t *ie, size_t len, uint16_t *rsn_caps, bool *has_group_mgmt);
bool rf_2a4m1_pmf_rx_unprotected_ok(uint16_t fc, const uint8_t *body, size_t body_len, bool mfp_active);
size_t rf_2a4m1_pmf_sa_query_build(uint8_t *out, size_t cap, uint8_t action, uint16_t trans_id);
void rf_2a4m1_pmf_sa_query_init(struct rf_2a4m1_pmf_sa_query *q, unsigned max_retries, unsigned timeout_ticks);
bool rf_2a4m1_pmf_sa_query_on_response(struct rf_2a4m1_pmf_sa_query *q, const uint8_t *in, size_t len);
bool rf_2a4m1_pmf_sa_query_parse(const uint8_t *in, size_t len, uint8_t *action, uint16_t *trans_id);
size_t rf_2a4m1_pmf_sa_query_respond(const uint8_t *in, size_t len, uint8_t *out, size_t cap);
size_t rf_2a4m1_pmf_sa_query_start(struct rf_2a4m1_pmf_sa_query *q, uint8_t *out, size_t cap);
size_t rf_2a4m1_pmf_sa_query_tick(struct rf_2a4m1_pmf_sa_query *q, uint8_t *out, size_t cap);
size_t rf_2a4m1_pmf_td_kde_build(uint8_t *out, size_t cap, uint8_t bitmap);
bool rf_2a4m1_pmf_td_kde_find(const uint8_t *kd, size_t len, uint8_t *bitmap);
void rf_2a4m1_put_le16(uint8_t *p, uint16_t v);
void rf_2a4m1_put_le32(uint8_t *p, uint32_t v);
enum rf_2a4m1_qos_admit_result rf_2a4m1_qos_admission_admit(struct rf_2a4m1_qos_admission *a, uint8_t tsid, uint8_t ac,
                                          uint8_t direction, uint16_t medium_time);
enum rf_2a4m1_qos_admit_result rf_2a4m1_qos_admission_admit_tspec(struct rf_2a4m1_qos_admission *a, const struct rf_2a4m1_tspec *granted);
void rf_2a4m1_qos_admission_init(struct rf_2a4m1_qos_admission *a);
bool rf_2a4m1_qos_admission_release(struct rf_2a4m1_qos_admission *a, uint8_t tsid);
bool rf_2a4m1_qos_admission_release_ts(struct rf_2a4m1_qos_admission *a, const struct rf_2a4m1_ts_info *ts);
void rf_2a4m1_qos_admission_set_limit(struct rf_2a4m1_qos_admission *a, uint8_t ac, uint32_t limit);
size_t rf_2a4m1_qos_ctrl_build(uint8_t *out, size_t cap, const struct rf_2a4m1_qos_ctrl *q);
uint16_t rf_2a4m1_qos_ctrl_pack(const struct rf_2a4m1_qos_ctrl *q);
bool rf_2a4m1_qos_ctrl_parse(const uint8_t *in, size_t len, struct rf_2a4m1_qos_ctrl *out);
void rf_2a4m1_qos_ctrl_unpack(uint16_t v, struct rf_2a4m1_qos_ctrl *out);
uint8_t rf_2a4m1_qos_up_to_ac(uint8_t up);
size_t rf_2a4m1_ric_rde_build(uint8_t *out, size_t cap, const struct rf_2a4m1_ric_rde *r);
bool rf_2a4m1_ric_rde_parse(const uint8_t *ie, size_t len, struct rf_2a4m1_ric_rde *r);
size_t rf_2a4m1_ric_request_build(uint8_t *out, size_t cap, uint8_t rdie_id,
                         const struct rf_2a4m1_tspec *list, size_t n);
bool rf_2a4m1_ric_request_parse(const uint8_t *in, size_t len, struct rf_2a4m1_ric_request *out);
size_t rf_2a4m1_ric_response_build(uint8_t *out, size_t cap, uint8_t rdie_id, const struct rf_2a4m1_tspec *list,
                          const uint16_t *status, size_t n);
bool rf_2a4m1_ric_response_parse(const uint8_t *in, size_t len, struct rf_2a4m1_ric_response *out, size_t *consumed);
bool rf_2a4m1_sae_commit(struct rf_2a4m1_sae *s, const uint8_t pt[RF_2A4M1_P256_POINT_LEN],
                const rf_2a4m1_mac_addr *mac_a, const rf_2a4m1_mac_addr *mac_b,
                const uint8_t rnd[RF_2A4M1_SAE_KEY_LEN], const uint8_t mask[RF_2A4M1_SAE_KEY_LEN]);
bool rf_2a4m1_sae_commit_pwe(struct rf_2a4m1_sae *s, const uint8_t pwe[RF_2A4M1_P256_POINT_LEN],
                    const uint8_t rnd[RF_2A4M1_SAE_KEY_LEN], const uint8_t mask[RF_2A4M1_SAE_KEY_LEN]);
void rf_2a4m1_sae_confirm(const struct rf_2a4m1_sae *s, uint16_t send_confirm, uint8_t out[RF_2A4M1_SAE_KEY_LEN]);
bool rf_2a4m1_sae_derive_pt_h2e(uint8_t pt[RF_2A4M1_P256_POINT_LEN],
                       const uint8_t *ssid, size_t ssid_len,
                       const uint8_t *password, size_t password_len,
                       const uint8_t *identifier, size_t identifier_len);
bool rf_2a4m1_sae_derive_pwe_from_pt(uint8_t pwe[RF_2A4M1_P256_POINT_LEN], const uint8_t pt[RF_2A4M1_P256_POINT_LEN],
                            const rf_2a4m1_mac_addr *mac_a, const rf_2a4m1_mac_addr *mac_b);
bool rf_2a4m1_sae_process_commit(struct rf_2a4m1_sae *s, const uint8_t peer_scalar[RF_2A4M1_SAE_KEY_LEN],
                        const uint8_t peer_element[RF_2A4M1_P256_POINT_LEN]);
bool rf_2a4m1_sae_verify_confirm(const struct rf_2a4m1_sae *s, uint16_t send_confirm,
                        const uint8_t peer_confirm[RF_2A4M1_SAE_KEY_LEN]);
void rf_2a4m1_sha1(const uint8_t *data, size_t len, uint8_t out[RF_2A4M1_SHA1_DIGEST_LEN]);
void rf_2a4m1_sha1_final(struct rf_2a4m1_sha1_ctx *c, uint8_t out[RF_2A4M1_SHA1_DIGEST_LEN]);
void rf_2a4m1_sha1_init(struct rf_2a4m1_sha1_ctx *c);
void rf_2a4m1_sha1_sw(const uint8_t *data, size_t len, uint8_t out[RF_2A4M1_SHA1_DIGEST_LEN]);
void rf_2a4m1_sha1_update(struct rf_2a4m1_sha1_ctx *c, const uint8_t *d, size_t n);
void rf_2a4m1_sha256(const uint8_t *data, size_t len, uint8_t out[RF_2A4M1_SHA256_DIGEST_LEN]);
void rf_2a4m1_sha256_final(struct rf_2a4m1_sha256_ctx *c, uint8_t out[RF_2A4M1_SHA256_DIGEST_LEN]);
void rf_2a4m1_sha256_init(struct rf_2a4m1_sha256_ctx *c);
void rf_2a4m1_sha256_sw(const uint8_t *data, size_t len, uint8_t out[RF_2A4M1_SHA256_DIGEST_LEN]);
void rf_2a4m1_sha256_update(struct rf_2a4m1_sha256_ctx *c, const uint8_t *d, size_t n);
rf_2a4m1_s8021x_status rf_2a4m1_sme_addts_request(struct rf_2a4m1_sme *s, uint8_t tsid, uint8_t ac,
                                uint8_t direction, uint16_t medium_time);
rf_2a4m1_s8021x_status rf_2a4m1_sme_ba_establish(struct rf_2a4m1_sme *s, uint8_t tid);
void rf_2a4m1_sme_bind_hal(struct rf_2a4m1_sme *s, struct rf_2a4m1_hal *rf_2a4m1_hal);
rf_2a4m1_s8021x_status rf_2a4m1_sme_connect_start(struct rf_2a4m1_sme *s);
rf_2a4m1_s8021x_status rf_2a4m1_sme_data_tx(struct rf_2a4m1_sme *s, const uint8_t *payload, uint16_t len);
rf_2a4m1_s8021x_status rf_2a4m1_sme_data_tx_qos(struct rf_2a4m1_sme *s, const uint8_t *payload, uint16_t len, uint8_t up);
rf_2a4m1_s8021x_status rf_2a4m1_sme_delts(struct rf_2a4m1_sme *s, uint8_t tsid);
bool rf_2a4m1_sme_eapol_key_decode(const uint8_t *in, size_t len, uint16_t *key_info,
                          uint64_t *replay, uint8_t nonce[RF_2A4M1_SME_NONCE_LEN],
                          uint8_t *key_data, uint16_t key_data_cap,
                          uint16_t *key_data_len);
size_t rf_2a4m1_sme_eapol_key_encode(uint8_t *out, size_t cap, uint16_t key_info,
                            uint16_t key_len, uint64_t replay,
                            const uint8_t nonce[RF_2A4M1_SME_NONCE_LEN],
                            const uint8_t *key_data, uint16_t key_data_len);
void rf_2a4m1_sme_ft_establish(struct rf_2a4m1_sme *s);
rf_2a4m1_s8021x_status rf_2a4m1_sme_ft_roam(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *target);
rf_2a4m1_s8021x_status rf_2a4m1_sme_ft_roam_over_ds(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *target);
void rf_2a4m1_sme_init(struct rf_2a4m1_sme *s, enum rf_2a4m1_sme_role role, const rf_2a4m1_mac_addr *self,
              const uint8_t pmk[RF_2A4M1_SME_PMK_LEN], const struct rf_2a4m1_sme_crypto *crypto,
              uint64_t nonce_salt);
void rf_2a4m1_sme_rx(struct rf_2a4m1_sme *s, const uint8_t *in, size_t len);
void rf_2a4m1_sme_rx_mgmt(struct rf_2a4m1_sme *s, const uint8_t *frame, size_t len);
void rf_2a4m1_sme_rx_spurious_reassoc(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *src);
void rf_2a4m1_sme_sa_query_tick(struct rf_2a4m1_sme *s);
void rf_2a4m1_sme_set_admission_limit(struct rf_2a4m1_sme *s, uint8_t ac, uint32_t limit);
void rf_2a4m1_sme_set_beacon_protection(struct rf_2a4m1_sme *s, bool on);
void rf_2a4m1_sme_set_edca_advert(struct rf_2a4m1_sme *s, const struct rf_2a4m1_edca_param *p);
void rf_2a4m1_sme_set_ft(struct rf_2a4m1_sme *s, uint16_t mdid, const uint8_t *ssid, size_t ssid_len,
                const uint8_t *r0kh_id, size_t r0kh_len);
void rf_2a4m1_sme_set_ht_advertise(struct rf_2a4m1_sme *s, bool on);
void rf_2a4m1_sme_set_ht_oper(struct rf_2a4m1_sme *s, const struct ht_oper *o);
void rf_2a4m1_sme_set_ocv(struct rf_2a4m1_sme *s, bool on, uint8_t op_class, uint8_t prim_chan, uint8_t seg1_freq);
void rf_2a4m1_sme_set_peer(struct rf_2a4m1_sme *s, const rf_2a4m1_mac_addr *peer);
void rf_2a4m1_sme_set_pmf(struct rf_2a4m1_sme *s, enum rf_2a4m1_pmf_mode mode);
void rf_2a4m1_sme_set_qos_advertise(struct rf_2a4m1_sme *s, bool on);
void rf_2a4m1_sme_set_ric_deny(struct rf_2a4m1_sme *s, bool deny);
void rf_2a4m1_sme_set_ric_request(struct rf_2a4m1_sme *s, const struct rf_2a4m1_tspec *t);
void rf_2a4m1_sme_set_transition_disable(struct rf_2a4m1_sme *s, uint8_t bitmap);
rf_2a4m1_s8021x_status rf_2a4m1_sme_tx_protected_deauth(struct rf_2a4m1_sme *s, uint16_t reason);
rf_2a4m1_s8021x_status rf_2a4m1_sme_tx_protected_disassoc(struct rf_2a4m1_sme *s, uint16_t reason);
void rf_2a4m1_tdls_derive_tpk(const uint8_t snonce[RF_2A4M1_TDLS_NONCE_LEN], const uint8_t anonce[RF_2A4M1_TDLS_NONCE_LEN],
                     const rf_2a4m1_mac_addr *mac_i, const rf_2a4m1_mac_addr *mac_r, const rf_2a4m1_mac_addr *bssid,
                     uint8_t tpk[RF_2A4M1_TDLS_TPK_LEN]);
size_t rf_2a4m1_tdls_ftie_build(uint8_t *out, size_t cap, const struct rf_2a4m1_tdls_ftie *f);
void rf_2a4m1_tdls_ftie_mic(const uint8_t kck[RF_2A4M1_TDLS_TPK_KCK_LEN],
                   const rf_2a4m1_mac_addr *init, const rf_2a4m1_mac_addr *resp, uint8_t trans_seq,
                   const uint8_t *lid, size_t lid_len,
                   const uint8_t *tie, size_t tie_len,
                   const uint8_t *ftie, size_t ftie_len,
                   uint8_t mic[RF_2A4M1_TDLS_MIC_LEN]);
bool rf_2a4m1_tdls_ftie_parse(const uint8_t *ie, size_t len, struct rf_2a4m1_tdls_ftie *f);
size_t rf_2a4m1_tdls_link_id_build(uint8_t *out, size_t cap, const struct rf_2a4m1_tdls_link_id *l);
bool rf_2a4m1_tdls_link_id_parse(const uint8_t *ie, size_t len, struct rf_2a4m1_tdls_link_id *l);
bool rf_2a4m1_tdls_on_setup_confirm(struct rf_2a4m1_tdls_peer *p, const uint8_t *in, size_t len);
size_t rf_2a4m1_tdls_on_setup_req(struct rf_2a4m1_tdls_peer *p, const uint8_t *in, size_t len,
                         const uint8_t anonce[RF_2A4M1_TDLS_NONCE_LEN], uint8_t *out, size_t cap);
size_t rf_2a4m1_tdls_on_setup_resp(struct rf_2a4m1_tdls_peer *p, const uint8_t *in, size_t len,
                          uint8_t *out, size_t cap);
bool rf_2a4m1_tdls_on_teardown(struct rf_2a4m1_tdls_peer *p, const uint8_t *in, size_t len);
void rf_2a4m1_tdls_peer_init(struct rf_2a4m1_tdls_peer *p, const rf_2a4m1_mac_addr *self, const rf_2a4m1_mac_addr *peer,
                    const rf_2a4m1_mac_addr *bssid, bool is_initiator,
                    uint8_t dialog_token, uint32_t key_lifetime);
size_t rf_2a4m1_tdls_setup_confirm_build(uint8_t *out, size_t cap, const struct rf_2a4m1_tdls_setup_confirm *r);
bool rf_2a4m1_tdls_setup_confirm_parse(const uint8_t *in, size_t len, struct rf_2a4m1_tdls_setup_confirm *r);
bool rf_2a4m1_tdls_setup_req_parse(const uint8_t *in, size_t len, struct rf_2a4m1_tdls_setup_req *r);
size_t rf_2a4m1_tdls_setup_resp_build(uint8_t *out, size_t cap, const struct rf_2a4m1_tdls_setup_resp *r);
bool rf_2a4m1_tdls_setup_resp_parse(const uint8_t *in, size_t len, struct rf_2a4m1_tdls_setup_resp *r);
bool rf_2a4m1_tdls_teardown_parse(const uint8_t *in, size_t len, struct rf_2a4m1_tdls_teardown *t);
size_t rf_2a4m1_tdls_tie_build(uint8_t *out, size_t cap, uint32_t key_lifetime);
bool rf_2a4m1_tdls_tie_parse(const uint8_t *ie, size_t len, uint32_t *key_lifetime);
uint32_t rf_2a4m1_ts_info_pack(const struct rf_2a4m1_ts_info *i);
void rf_2a4m1_ts_info_unpack(uint32_t v, struct rf_2a4m1_ts_info *out);
size_t rf_2a4m1_tspec_build(uint8_t *out, size_t cap, const struct rf_2a4m1_tspec *t);
bool rf_2a4m1_tspec_parse(const uint8_t *body, uint8_t dlen, struct rf_2a4m1_tspec *out);
size_t rf_2a4m1_wmm_info_build(uint8_t *out, size_t cap, uint8_t qos_info);
bool rf_2a4m1_wmm_info_parse(const uint8_t *body, uint8_t dlen, uint8_t *qos_info_out);
size_t rf_2a4m1_wmm_param_build(uint8_t *out, size_t cap, const struct rf_2a4m1_edca_param *p);
bool rf_2a4m1_wmm_param_parse(const uint8_t *body, uint8_t dlen, struct rf_2a4m1_edca_param *out);
void rf_2a4m1_wpa_derive_ptk(const uint8_t pmk[RF_2A4M1_WPA_PMK_LEN],
                    const rf_2a4m1_mac_addr *aa, const rf_2a4m1_mac_addr *spa,
                    const uint8_t anonce[RF_2A4M1_WPA_NONCE_LEN],
                    const uint8_t snonce[RF_2A4M1_WPA_NONCE_LEN],
                    uint8_t ptk[RF_2A4M1_WPA_PTK_LEN]);

/* -- inline helpers (lifted from header static-inline) -- */
static inline uint16_t rf_2a4m1_get_be16(const uint8_t *p);
static inline uint32_t rf_2a4m1_get_be32(const uint8_t *p);
static inline void rf_2a4m1_put_be16(uint8_t *p, uint16_t v);
static inline void rf_2a4m1_put_be32(uint8_t *p, uint32_t v);
static inline void rf_2a4m1_put_le64(uint8_t *p, uint64_t v);
static inline uint64_t rf_2a4m1_get_be64(const uint8_t *p);
static inline void rf_2a4m1_put_be64(uint8_t *p, uint64_t v);
static inline void rf_2a4m1_rd_init(struct rf_2a4m1_rd *r, const uint8_t *p, size_t len);
static inline bool rf_2a4m1_rd_ok(const struct rf_2a4m1_rd *r);
static inline size_t rf_2a4m1_rd_remaining(const struct rf_2a4m1_rd *r);
static inline const uint8_t *rf_2a4m1_rd_bytes(struct rf_2a4m1_rd *r, size_t n);
static inline uint8_t rf_2a4m1_rd_u8(struct rf_2a4m1_rd *r);
static inline uint16_t rf_2a4m1_rd_le16(struct rf_2a4m1_rd *r);
static inline uint32_t rf_2a4m1_rd_le32(struct rf_2a4m1_rd *r);
static inline bool rf_2a4m1_rd_skip(struct rf_2a4m1_rd *r, size_t n);
static inline void rf_2a4m1_crypto_wipe(void *p, size_t n);
static inline void rf_2a4m1_pn_replay_reset(struct rf_2a4m1_pn_replay *r);
static inline unsigned rf_2a4m1_pn_replay_tid(const uint8_t *hdr);
static inline bool rf_2a4m1_pn_replay_ok(const struct rf_2a4m1_pn_replay *r, unsigned tid, uint64_t pn);
static inline void rf_2a4m1_pn_replay_commit(struct rf_2a4m1_pn_replay *r, unsigned tid, uint64_t pn);
static inline void rf_2a4m1_ie_reader_init(struct rf_2a4m1_ie_reader *r, const uint8_t *buf, size_t len);
static inline void rf_2a4m1_ie_writer_init(struct rf_2a4m1_ie_writer *w, uint8_t *buf, size_t cap);
static inline size_t rf_2a4m1_ie_writer_len(const struct rf_2a4m1_ie_writer *w);
static inline int rf_2a4m1_hal_start(struct rf_2a4m1_hal *h, const struct rf_2a4m1_hal_cfg *cfg);
static inline int rf_2a4m1_hal_tx(struct rf_2a4m1_hal *h, struct rf_2a4m1_mpdu *m, const struct rf_2a4m1_tx_params *tp);
static inline int rf_2a4m1_hal_set_lower_mac(struct rf_2a4m1_hal *h, const struct rf_2a4m1_lmac_cfg *cfg);
static inline void rf_2a4m1_hal_deliver_rx(struct rf_2a4m1_hal *h, const struct rf_2a4m1_rxinfo *rx);
static inline void rf_2a4m1_hal_deliver_tx_status(struct rf_2a4m1_hal *h, const struct rf_2a4m1_hal_tx_status *ts);
static inline uint16_t rf_2a4m1_pmf_get_le16(const uint8_t *p);
static inline void rf_2a4m1_pmf_put_le16(uint8_t *p, uint16_t v);
static inline bool rf_2a4m1_pmf_caps_mfpc(uint16_t caps);
static inline bool rf_2a4m1_pmf_caps_mfpr(uint16_t caps);
static inline uint16_t rf_2a4m1_qos_get_le16(const uint8_t *p);
static inline void rf_2a4m1_qos_put_le16(uint8_t *p, uint16_t v);
static inline uint32_t rf_2a4m1_qos_get_le32(const uint8_t *p);
static inline void rf_2a4m1_qos_put_le32(uint8_t *p, uint32_t v);
static inline uint8_t rf_2a4m1_edca_ecwmin(uint8_t ecw);
static inline uint8_t rf_2a4m1_edca_ecwmax(uint8_t ecw);
static inline uint8_t rf_2a4m1_edca_ecw(uint8_t emin, uint8_t emax);
static inline uint16_t rf_2a4m1_edca_cw_from_ecw(uint8_t e);
static inline void rf_2a4m1_roam_put_le16(uint8_t *p, uint16_t v);
static inline void rf_2a4m1_tdls_put_le16(uint8_t *p, uint16_t v);
static inline void rf_2a4m1_tdls_put_le32(uint8_t *p, uint32_t v);
static inline bool rf_2a4m1_tdls_link_id_eq(const struct rf_2a4m1_tdls_link_id *a, const struct rf_2a4m1_tdls_link_id *b);
static inline bool rf_2a4m1_mac_eq(const rf_2a4m1_mac_addr *x, const rf_2a4m1_mac_addr *y);

static inline uint16_t rf_2a4m1_get_be16(const uint8_t *p)
{
	return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static inline uint32_t rf_2a4m1_get_be32(const uint8_t *p)
{
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
	       ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static inline void rf_2a4m1_put_be16(uint8_t *p, uint16_t v)
{
	p[0] = (uint8_t)(v >> 8);
	p[1] = (uint8_t)v;
}

static inline void rf_2a4m1_put_be32(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)(v >> 24);
	p[1] = (uint8_t)(v >> 16);
	p[2] = (uint8_t)(v >> 8);
	p[3] = (uint8_t)v;
}

static inline void rf_2a4m1_put_le64(uint8_t *p, uint64_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
	p[4] = (uint8_t)(v >> 32);
	p[5] = (uint8_t)(v >> 40);
	p[6] = (uint8_t)(v >> 48);
	p[7] = (uint8_t)(v >> 56);
}

static inline uint64_t rf_2a4m1_get_be64(const uint8_t *p)
{
	return ((uint64_t)rf_2a4m1_get_be32(p) << 32) | (uint64_t)rf_2a4m1_get_be32(p + 4);
}

static inline void rf_2a4m1_put_be64(uint8_t *p, uint64_t v)
{
	rf_2a4m1_put_be32(p, (uint32_t)(v >> 32));
	rf_2a4m1_put_be32(p + 4, (uint32_t)v);
}

static inline void rf_2a4m1_rd_init(struct rf_2a4m1_rd *r, const uint8_t *p, size_t len)
{
	r->p = p;
	r->len = len;
	r->pos = 0;
	r->err = false;
}

static inline bool rf_2a4m1_rd_ok(const struct rf_2a4m1_rd *r) { return !r->err; }

static inline size_t rf_2a4m1_rd_remaining(const struct rf_2a4m1_rd *r)
{
	return r->err ? 0 : r->len - r->pos;
}

static inline const uint8_t *rf_2a4m1_rd_bytes(struct rf_2a4m1_rd *r, size_t n)
{
	if (r->err || n > r->len - r->pos) {
		r->err = true;
		return (const uint8_t *)0;
	}
	const uint8_t *q = r->p + r->pos;
	r->pos += n;
	return q;
}

static inline uint8_t rf_2a4m1_rd_u8(struct rf_2a4m1_rd *r)
{
	const uint8_t *q = rf_2a4m1_rd_bytes(r, 1);
	return q ? q[0] : 0;
}

static inline uint16_t rf_2a4m1_rd_le16(struct rf_2a4m1_rd *r)
{
	const uint8_t *q = rf_2a4m1_rd_bytes(r, 2);
	return q ? rf_2a4m1_get_le16(q) : 0;
}

static inline uint32_t rf_2a4m1_rd_le32(struct rf_2a4m1_rd *r)
{
	const uint8_t *q = rf_2a4m1_rd_bytes(r, 4);
	return q ? rf_2a4m1_get_le32(q) : 0;
}

static inline bool rf_2a4m1_rd_skip(struct rf_2a4m1_rd *r, size_t n)
{
	return rf_2a4m1_rd_bytes(r, n) != (const uint8_t *)0;
}

static inline void rf_2a4m1_crypto_wipe(void *p, size_t n)
{
	volatile uint8_t *v = (volatile uint8_t *)p;
	while (n--)
		*v++ = 0;
}

static inline void rf_2a4m1_pn_replay_reset(struct rf_2a4m1_pn_replay *r)
{
	for (unsigned i = 0; i < RF_2A4M1_PN_REPLAY_TIDS; i++) {
		r->highest[i] = 0;
		r->seen[i]    = false;
	}
}

static inline unsigned rf_2a4m1_pn_replay_tid(const uint8_t *hdr)
{
	bool qos = ((hdr[0] & 0x0c) == 0x08) && (hdr[0] & 0x80);  /* type=Data + QoS subtype bit */
	if (!qos)
		return RF_2A4M1_PN_REPLAY_NONQOS;
	bool a4 = (hdr[1] & 0x03) == 0x03;                        /* ToDS && FromDS */
	const uint8_t *qc = hdr + (a4 ? 30 : 24);
	return (unsigned)(qc[0] & 0x0f);
}

static inline bool rf_2a4m1_pn_replay_ok(const struct rf_2a4m1_pn_replay *r, unsigned tid, uint64_t pn)
{
	if (tid >= RF_2A4M1_PN_REPLAY_TIDS)
		return false;
	if (pn > RF_2A4M1_PN_REPLAY_PN_MAX)
		return false;
	if (!r->seen[tid])
		return true;
	return pn > r->highest[tid];
}

static inline void rf_2a4m1_pn_replay_commit(struct rf_2a4m1_pn_replay *r, unsigned tid, uint64_t pn)
{
	if (tid >= RF_2A4M1_PN_REPLAY_TIDS)
		return;
	r->highest[tid] = pn;
	r->seen[tid]    = true;
}

static inline void rf_2a4m1_ie_reader_init(struct rf_2a4m1_ie_reader *r, const uint8_t *buf, size_t len)
{
	r->buf = buf;
	r->len = len;
	r->pos = 0;
}

static inline void rf_2a4m1_ie_writer_init(struct rf_2a4m1_ie_writer *w, uint8_t *buf, size_t cap)
{
	w->buf = buf;
	w->cap = cap;
	w->pos = 0;
}

static inline size_t rf_2a4m1_ie_writer_len(const struct rf_2a4m1_ie_writer *w) { return w->pos; }

static inline int rf_2a4m1_hal_start(struct rf_2a4m1_hal *h, const struct rf_2a4m1_hal_cfg *cfg)
{
	if (!h || !h->ops || !h->ops->start || !cfg) return -1;
	h->rx_cb = cfg->rx_cb; h->rx_ctx = cfg->rx_ctx;   /* remember for hal_deliver_rx() */
	return h->ops->start(h, cfg);
}

static inline int rf_2a4m1_hal_tx(struct rf_2a4m1_hal *h, struct rf_2a4m1_mpdu *m, const struct rf_2a4m1_tx_params *tp)
{ return (h && h->ops && h->ops->tx && m && tp) ? h->ops->tx(h, m, tp) : -1; }

static inline int rf_2a4m1_hal_set_lower_mac(struct rf_2a4m1_hal *h, const struct rf_2a4m1_lmac_cfg *cfg)
{ return (h && h->ops && h->ops->set_lower_mac && cfg) ? h->ops->set_lower_mac(h, cfg) : -1; }

static inline void rf_2a4m1_hal_deliver_rx(struct rf_2a4m1_hal *h, const struct rf_2a4m1_rxinfo *rx)
{ if (h && h->rx_cb && rx) h->rx_cb(h, rx, h->rx_ctx); }

static inline void rf_2a4m1_hal_deliver_tx_status(struct rf_2a4m1_hal *h, const struct rf_2a4m1_hal_tx_status *ts)
{ if (h && h->tx_status_cb && ts) h->tx_status_cb(h, ts, h->tx_status_ctx); }

static inline uint16_t rf_2a4m1_pmf_get_le16(const uint8_t *p)
{ return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8)); }

static inline void rf_2a4m1_pmf_put_le16(uint8_t *p, uint16_t v)
{ p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }

static inline bool rf_2a4m1_pmf_caps_mfpc(uint16_t caps) { return (caps & RF_2A4M1_PMF_RSN_CAP_MFPC) != 0; }

static inline bool rf_2a4m1_pmf_caps_mfpr(uint16_t caps) { return (caps & RF_2A4M1_PMF_RSN_CAP_MFPR) != 0; }

static inline uint16_t rf_2a4m1_qos_get_le16(const uint8_t *p)
{ return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8)); }

static inline void rf_2a4m1_qos_put_le16(uint8_t *p, uint16_t v)
{ p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }

static inline uint32_t rf_2a4m1_qos_get_le32(const uint8_t *p)
{ return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }

static inline void rf_2a4m1_qos_put_le32(uint8_t *p, uint32_t v)
{ p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

static inline uint8_t rf_2a4m1_edca_ecwmin(uint8_t ecw)   { return (uint8_t)(ecw & 0x0f); }

static inline uint8_t rf_2a4m1_edca_ecwmax(uint8_t ecw)   { return (uint8_t)(ecw >> 4); }

static inline uint8_t rf_2a4m1_edca_ecw(uint8_t emin, uint8_t emax)
{ return (uint8_t)(((emax & 0x0f) << 4) | (emin & 0x0f)); }

static inline uint16_t rf_2a4m1_edca_cw_from_ecw(uint8_t e) { return (uint16_t)((1u << e) - 1u); }

static inline void rf_2a4m1_roam_put_le16(uint8_t *p, uint16_t v)
{ p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }

static inline void rf_2a4m1_tdls_put_le16(uint8_t *p, uint16_t v)
{ p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }

static inline void rf_2a4m1_tdls_put_le32(uint8_t *p, uint32_t v)
{ p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8); p[2]=(uint8_t)(v>>16); p[3]=(uint8_t)(v>>24); }

static inline bool rf_2a4m1_tdls_link_id_eq(const struct rf_2a4m1_tdls_link_id *a, const struct rf_2a4m1_tdls_link_id *b)
{ return rf_2a4m1_mac_eq(&a->bssid, &b->bssid) && rf_2a4m1_mac_eq(&a->init_addr, &b->init_addr) &&
         rf_2a4m1_mac_eq(&a->resp_addr, &b->resp_addr); }

static inline bool rf_2a4m1_mac_eq(const rf_2a4m1_mac_addr *x, const rf_2a4m1_mac_addr *y)
{
	for (int i = 0; i < RF_2A4M1_ETH_ALEN; i++)
		if (x->a[i] != y->a[i])
			return false;
	return true;
}

#endif /* RF_2A4M1_CORE_H */
