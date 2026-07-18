// SPDX-License-Identifier: GPL-2.0
//
// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
// USB HAL seam (TXWI/RXWI descriptors + USB framing + TX-status)
//
// Copyright (c) GenBasic.
// Licensed under the GNU General Public License, version 2.
//
// This file is machine-generated. Do not hand-edit.


#include "rf_2a4m1_core.h"

/* file-local forward declarations */
static inline uint32_t rf_2a4m1_round_up4(uint32_t x);
static int  rf_2a4m1_mt7601u_calibrate(struct rf_2a4m1_hal *h, enum rf_2a4m1_hal_cal rf_2a4m1_what);
static int  rf_2a4m1_mt7601u_set_channel(struct rf_2a4m1_hal *h, const struct rf_2a4m1_chan_def *c);
static int  rf_2a4m1_mt7601u_set_key(struct rf_2a4m1_hal *h, uint8_t slot, const struct rf_2a4m1_key *k);
static int  rf_2a4m1_mt7601u_set_lower_mac(struct rf_2a4m1_hal *h, const struct rf_2a4m1_lmac_cfg *cfg);
static int  rf_2a4m1_mt7601u_set_rx_filter(struct rf_2a4m1_hal *h, uint32_t mask);
static int  rf_2a4m1_mt7601u_set_sta(struct rf_2a4m1_hal *h, uint8_t wcid, const struct rf_2a4m1_sta_cfg *s);
static int  rf_2a4m1_mt7601u_start(struct rf_2a4m1_hal *h, const struct rf_2a4m1_hal_cfg *cfg);
static int rf_2a4m1_mt7601u_tx(struct rf_2a4m1_hal *h, struct rf_2a4m1_mpdu *m, const struct rf_2a4m1_tx_params *tp);
static uint16_t rf_2a4m1_mt7601u_phy_type(uint8_t phy_mode);
static uint16_t rf_2a4m1_mt7601u_rate_word(const struct rf_2a4m1_tx_params *tp);
static void rf_2a4m1_mt7601u_stop(struct rf_2a4m1_hal *h);

void rf_2a4m1_put_le16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }

void rf_2a4m1_put_le32(uint8_t *p, uint32_t v)
{ p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

uint16_t rf_2a4m1_get_le16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }

uint32_t rf_2a4m1_get_le32(const uint8_t *p)
{ return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }

static uint16_t rf_2a4m1_mt7601u_phy_type(uint8_t phy_mode)
{
	switch (phy_mode) {
	case RF_2A4M1_HAL_PHY_CCK:  return RF_2A4M1_MT_PHY_TYPE_CCK;
	case RF_2A4M1_HAL_PHY_OFDM: return RF_2A4M1_MT_PHY_TYPE_OFDM;
	default:           return RF_2A4M1_MT_PHY_TYPE_HT;   /* HT + any higher gen this PHY can't do */
	}
}

static uint16_t rf_2a4m1_mt7601u_rate_word(const struct rf_2a4m1_tx_params *tp)
{
	uint16_t r = (uint16_t)(tp->mcs & RF_2A4M1_MT_RATE_MCS_MASK);
	r |= (uint16_t)(rf_2a4m1_mt7601u_phy_type(tp->phy_mode) << RF_2A4M1_MT_RATE_PHY_SHIFT);
	if (tp->bw_mhz >= 40)
		r |= RF_2A4M1_MT_RATE_BW_40;
	if (tp->short_gi)
		r |= RF_2A4M1_MT_RATE_SGI;
	if (tp->stbc)
		r |= (uint16_t)(1u << RF_2A4M1_MT_RATE_STBC_SHIFT);
	return r;
}

int rf_2a4m1_mt7601u_build_txwi(uint8_t out[RF_2A4M1_MT7601U_TXWI_SIZE],
                       const struct rf_2a4m1_tx_params *tp, const struct rf_2a4m1_mpdu *m)
{
	if (!out || !tp || !m)
		return RF_2A4M1_S8021X_ERR_INVAL;
	if (m->len > RF_2A4M1_MT_TXWI_LEN_BYTE_MASK)   /* 12-bit MPDU byte count */
		return RF_2A4M1_S8021X_ERR_INVAL;

	memset(out, 0, RF_2A4M1_MT7601U_TXWI_SIZE);

	/* flags @0: AMPDU bit iff the core marks this MPDU an A-MPDU subframe (else 0). When set,
	 * mirror the driver's mt7601u_push_txwi() A-MPDU path (tx.c): also carry the MPDU_DENSITY
	 * (mac.h MT_TXWI_FLAGS_MPDU_DENSITY = GENMASK(7,5)) so the chip's hardware aggregation logic
	 * honours the min MPDU start-spacing the peer's HT cap negotiated. */
	uint16_t flags = 0;
	if (tp->ampdu) {
		flags = RF_2A4M1_MT_TXWI_FLAGS_AMPDU;
		flags |= (uint16_t)((tp->ampdu_density & 0x7) << 5);   /* MT_TXWI_FLAGS_MPDU_DENSITY */
	}
	rf_2a4m1_put_le16(out + RF_2A4M1_MT_TXWI_OFF_FLAGS, flags);

	/* rate_ctl @2: PHY mode / MCS / BW / SGI / STBC. */
	rf_2a4m1_put_le16(out + RF_2A4M1_MT_TXWI_OFF_RATE, rf_2a4m1_mt7601u_rate_word(tp));

	/* ack_ctl @4: request ACK unless No-Ack / broadcast; NSEQ asks the HW to assign the
	 * 802.11 sequence number (seqno offload). For an A-MPDU subframe, also program the Block-Ack
	 * window (mac.h MT_TXWI_ACK_CTL_BA_WINDOW = GENMASK(7,2)) — the chip's agg engine packs up to
	 * ba_size consecutive same-WCID/same-TID MPDUs into one PPDU (tx.c mt7601u_push_txwi). */
	out[RF_2A4M1_MT_TXWI_OFF_ACK_CTL] = (uint8_t)((tp->no_ack ? 0 : RF_2A4M1_MT_TXWI_ACK_CTL_REQ) |
	                                     (tp->hw_seqno ? RF_2A4M1_MT_TXWI_ACK_CTL_NSEQ : 0) |
	                                     (tp->ampdu ? ((tp->ba_size & 0x3f) << 2) : 0));

	/* wcid @5: the peer/WCID slot (0xff = no peer / mgmt broadcast wcid). */
	out[RF_2A4M1_MT_TXWI_OFF_WCID] = tp->wcid;

	/* len_ctl @6: the MPDU byte count (12-bit) + the PKTID nibble (15:12) — the TX-status
	 * correlation cookie the MAC echoes back in the MT_TX_STAT_FIFO PID field (0 => the chip
	 * disables status reporting for this frame, mirroring mt7601u_tx_pktid_enc). */
	rf_2a4m1_put_le16(out + RF_2A4M1_MT_TXWI_OFF_LEN,
	         (uint16_t)((m->len & RF_2A4M1_MT_TXWI_LEN_BYTE_MASK) |
	                    ((uint16_t)(tp->pktid << RF_2A4M1_MT_TXWI_LEN_PKTID_SHIFT) & RF_2A4M1_MT_TXWI_LEN_PKTID_MASK)));

	/* iv @8 / eiv @12: staged cipher IV/EIV when the core supplies them for HW crypto
	 * (iv_valid). Otherwise left zero — the IV then rides in the frame body (the WIV path).
	 * aid @16 / txstream @17 / ctl @18: left zero (power_dbm/aid/ampdu-density are not part
	 * of this HAL's normal-frame path — the driver's data path leaves them zero too). */
	if (tp->iv_valid) {
		rf_2a4m1_put_le32(out + RF_2A4M1_MT_TXWI_OFF_IV,  tp->iv);
		rf_2a4m1_put_le32(out + RF_2A4M1_MT_TXWI_OFF_EIV, tp->eiv);
	}
	return RF_2A4M1_S8021X_OK;
}

int rf_2a4m1_mt7601u_parse_rxwi(const uint8_t *rxwi, uint16_t rxwi_len,
                       const uint8_t *frame, uint16_t frame_len, struct rf_2a4m1_rxinfo *out)
{
	if (!rxwi || !out || rxwi_len < RF_2A4M1_MT7601U_RXWI_SIZE)
		return RF_2A4M1_S8021X_ERR_INVAL;

	uint32_t rf_2a4m1_rxinfo = rf_2a4m1_get_le32(rxwi + RF_2A4M1_MT_RXWI_OFF_RXINFO);
	uint32_t ctl    = rf_2a4m1_get_le32(rxwi + RF_2A4M1_MT_RXWI_OFF_CTL);
	uint16_t rate   = rf_2a4m1_get_le16(rxwi + RF_2A4M1_MT_RXWI_OFF_RATE);

	uint16_t mpdu_len = (uint16_t)((ctl & RF_2A4M1_MT_RXWI_CTL_MPDU_LEN_MASK) >> RF_2A4M1_MT_RXWI_CTL_MPDU_LEN_SHIFT);
	if (mpdu_len < 10)                    /* the driver drops sub-header frames */
		return RF_2A4M1_S8021X_ERR_INVAL;

	memset(out, 0, sizeof *out);
	out->data = frame;
	/* Prefer the caller's actual buffer length; fall back to the descriptor's MPDU_LEN. */
	out->len  = frame ? frame_len : mpdu_len;

	/* rate word -> PHY modulation + MCS + BW + SGI + STBC. */
	uint8_t phy = (uint8_t)((rate & RF_2A4M1_MT_RATE_PHY_MASK) >> RF_2A4M1_MT_RATE_PHY_SHIFT);
	out->mcs      = (uint16_t)(rate & RF_2A4M1_MT_RATE_MCS_MASK);
	out->bw_mhz   = (rate & RF_2A4M1_MT_RATE_BW_40) ? 40 : 20;
	out->short_gi = (rate & RF_2A4M1_MT_RATE_SGI) != 0;
	out->stbc     = (rate & RF_2A4M1_MT_RATE_STBC_MASK) != 0;
	/* N3 gap 1 (CLOSED): the real PHY type now maps onto the per-frame phy_mode discriminator,
	 * and gen is HT only for an HT/HT-GF frame — a legacy CCK/OFDM frame reports HAL_GEN_LEGACY
	 * (no longer forced to HT), with phy_mode telling CCK vs OFDM. */
	switch (phy) {
	case RF_2A4M1_MT_PHY_TYPE_CCK:   out->phy_mode = RF_2A4M1_HAL_PHY_CCK;  out->gen = RF_2A4M1_HAL_GEN_LEGACY; break;
	case RF_2A4M1_MT_PHY_TYPE_OFDM:  out->phy_mode = RF_2A4M1_HAL_PHY_OFDM; out->gen = RF_2A4M1_HAL_GEN_LEGACY; break;
	case RF_2A4M1_MT_PHY_TYPE_HT:
	case RF_2A4M1_MT_PHY_TYPE_HT_GF:
	default:                out->phy_mode = RF_2A4M1_HAL_PHY_HT;   out->gen = RF_2A4M1_HAL_GEN_HT;     break;
	}
	out->n_ss  = 1;                       /* 1T1R silicon */
	out->link_id = 0;                     /* single link */

	/* N3 gap 4 (CLOSED): source/peer identity + queue from the RXWI ctl word. */
	out->wcid    = (uint8_t)(ctl & RF_2A4M1_MT_RXWI_CTL_WCID_MASK);
	out->key_idx = (uint8_t)((ctl & RF_2A4M1_MT_RXWI_CTL_KEY_IDX_MASK) >> RF_2A4M1_MT_RXWI_CTL_KEY_IDX_SHIFT);
	out->bss_idx = (uint8_t)((ctl & RF_2A4M1_MT_RXWI_CTL_BSS_IDX_MASK) >> RF_2A4M1_MT_RXWI_CTL_BSS_IDX_SHIFT);
	out->tid     = (uint8_t)((ctl & RF_2A4M1_MT_RXWI_CTL_TID_MASK) >> RF_2A4M1_MT_RXWI_CTL_TID_SHIFT);

	/* N3 gap 2 (CLOSED): per-frame FCS/CRC status — the KEY monitor-mode gap. The MT7601U
	 * RXWI flags a CRC error (bad FCS) in rxinfo; it has no separate PHY-decode-error bit at
	 * the descriptor layer, so phy_err stays false (documented, not forced). */
	out->crc_err = (rf_2a4m1_rxinfo & RF_2A4M1_MT_RXINFO_CRCERR) != 0;
	out->fcs_ok  = !out->crc_err;
	out->phy_err = false;

	/* decrypt / integrity status: HW decrypt OK iff DECRYPT set and no ICV/MIC error. */
	out->crypto_ok = (rf_2a4m1_rxinfo & RF_2A4M1_MT_RXINFO_DECRYPT) &&
	                 !(rf_2a4m1_rxinfo & (RF_2A4M1_MT_RXINFO_ICVERR | RF_2A4M1_MT_RXINFO_MICERR));
	out->ampdu = (rf_2a4m1_rxinfo & RF_2A4M1_MT_RXINFO_AMPDU) != 0;

	/* N3 gap 3 (CLOSED, best-effort): RSSI as a signed dBm value + a per-chain array. The gain
	 * byte carries a raw 6-bit code + a 2-bit LNA path id. A byte-exact absolute dBm needs the
	 * per-chip RSSI-offset calibration (EEPROM/RF driver, outside the descriptor layer), so the
	 * dBm here is the negated code — a signed, dBm-DOMAIN best effort, not a calibrated value.
	 * 1T1R silicon populates chain 0 only. SNR/freq_off are the raw snr/freq_off bytes. */
	uint8_t gain = rxwi[RF_2A4M1_MT_RXWI_OFF_GAIN];
	out->rssi    = (int8_t)-(int)(gain & RF_2A4M1_MT_RXWI_GAIN_RSSI_VAL_MASK);
	out->rssi_chain[0] = out->rssi;       /* 1T1R: chain 0 carries the whole estimate */
	out->snr     = (int8_t)rxwi[RF_2A4M1_MT_RXWI_OFF_SNR];
	out->freq_off = (int8_t)rxwi[RF_2A4M1_MT_RXWI_OFF_FREQ_OFF];   /* N3 gap 5: frequency offset */
	out->antenna = (uint8_t)((gain >> RF_2A4M1_MT_RXWI_GAIN_LNA_ID_SHIFT) & 0x3);

	return RF_2A4M1_S8021X_OK;
}

static inline uint32_t rf_2a4m1_round_up4(uint32_t x) { return (x + 3u) & ~3u; }

int rf_2a4m1_mt7601u_usb_tx_wrap(uint8_t *out, uint16_t out_cap,
                        const uint8_t *txwi, uint16_t txwi_len,
                        const uint8_t *payload, uint16_t payload_len,
                        uint8_t qsel, bool wiv, uint16_t *out_len)
{
	if (!out || !txwi || (!payload && payload_len))
		return RF_2A4M1_S8021X_ERR_INVAL;

	uint32_t body   = (uint32_t)txwi_len + payload_len;   /* skb->len at info-compute */
	uint32_t padded = rf_2a4m1_round_up4(body);                    /* == TXINFO.LEN field       */
	uint32_t total  = RF_2A4M1_MT_DMA_HDR_LEN + padded + RF_2A4M1_MT_DMA_FCE_LEN;
	if (padded > RF_2A4M1_MT_TXD_INFO_LEN_MASK)                    /* 16-bit LEN field           */
		return RF_2A4M1_S8021X_ERR_INVAL;
	if (total > out_cap)
		return RF_2A4M1_S8021X_ERR_INVAL;

	uint32_t rf_2a4m1_info = RF_2A4M1_MT_TXD_PKT_INFO_80211
	              | (padded & RF_2A4M1_MT_TXD_INFO_LEN_MASK)
	              | ((uint32_t)(qsel & 0x3u) << RF_2A4M1_MT_TXD_PKT_INFO_QSEL_SHIFT)
	              | ((uint32_t)RF_2A4M1_MT_MSG_PORT_WLAN    << RF_2A4M1_MT_TXD_INFO_DPORT_SHIFT)
	              | ((uint32_t)RF_2A4M1_MT_INFO_TYPE_PACKET << RF_2A4M1_MT_TXD_INFO_TYPE_SHIFT);
	if (wiv)
		rf_2a4m1_info |= RF_2A4M1_MT_TXD_PKT_INFO_WIV;

	memset(out, 0, total);                                /* align pad + trailing zero  */
	rf_2a4m1_put_le32(out, rf_2a4m1_info);
	memcpy(out + RF_2A4M1_MT_DMA_HDR_LEN, txwi, txwi_len);
	if (payload_len)
		memcpy(out + RF_2A4M1_MT_DMA_HDR_LEN + txwi_len, payload, payload_len);
	if (out_len)
		*out_len = (uint16_t)total;
	return RF_2A4M1_S8021X_OK;
}

int rf_2a4m1_mt7601u_usb_rx_wrap(uint8_t *out, uint16_t out_cap,
                        const uint8_t *rxwi, uint16_t rxwi_len,
                        const uint8_t *payload, uint16_t payload_len,
                        uint16_t *out_len)
{
	if (!out || !rxwi || (!payload && payload_len))
		return RF_2A4M1_S8021X_ERR_INVAL;

	uint32_t dma_len = rf_2a4m1_round_up4((uint32_t)rxwi_len + payload_len);  /* RX DMA 4-pad */
	uint32_t total   = RF_2A4M1_MT_DMA_HDRS + dma_len;                        /* front(4)+dma+fce(4) */
	if (dma_len > RF_2A4M1_MT_RXD_INFO_LEN_MASK)                             /* 14-bit LEN field */
		return RF_2A4M1_S8021X_ERR_INVAL;
	if (total > out_cap)
		return RF_2A4M1_S8021X_ERR_INVAL;

	uint32_t rxd = RF_2A4M1_MT_RXD_PKT_INFO_80211 | (dma_len & RF_2A4M1_MT_RXD_INFO_LEN_MASK);
	memset(out, 0, total);                                          /* payload pad + FCE */
	rf_2a4m1_put_le32(out, rxd);
	memcpy(out + RF_2A4M1_MT_DMA_HDR_LEN, rxwi, rxwi_len);
	if (payload_len)
		memcpy(out + RF_2A4M1_MT_DMA_HDR_LEN + rxwi_len, payload, payload_len);
	if (out_len)
		*out_len = (uint16_t)total;
	return RF_2A4M1_S8021X_OK;
}

int rf_2a4m1_mt7601u_usb_rx_unwrap(const uint8_t *in, uint16_t in_len,
                          const uint8_t **rxwi, uint16_t *rxwi_len,
                          const uint8_t **payload, uint16_t *payload_len)
{
	if (!in)
		return RF_2A4M1_S8021X_ERR_INVAL;

	/* min_seg_len = MT_DMA_HDR_LEN + MT_RX_INFO_LEN + sizeof(rxwi) + MT_FCE_INFO_LEN
	 * = 4 + 4 + 28 + 4 = 40 (dma.c). Both data_len and dma_len must be >= it. */
	uint32_t min_seg = RF_2A4M1_MT_DMA_HDRS + RF_2A4M1_MT7601U_RXWI_SIZE + RF_2A4M1_MT_DMA_FCE_LEN;   /* 8+28+4=40 */
	if (in_len < min_seg)
		return RF_2A4M1_S8021X_ERR_INVAL;

	uint16_t dma_len = rf_2a4m1_get_le16(in);
	if (dma_len == 0 ||
	    (uint32_t)dma_len + RF_2A4M1_MT_DMA_HDRS > in_len ||   /* dma_len + 8 > data_len */
	    (dma_len & 0x3u) ||                           /* not 4-aligned          */
	    dma_len < min_seg)                            /* below the guard floor  */
		return RF_2A4M1_S8021X_ERR_INVAL;

	if (rxwi)        *rxwi        = in + RF_2A4M1_MT_DMA_HDR_LEN;
	if (rxwi_len)    *rxwi_len    = RF_2A4M1_MT7601U_RXWI_SIZE;
	if (payload)     *payload     = in + RF_2A4M1_MT_DMA_HDR_LEN + RF_2A4M1_MT7601U_RXWI_SIZE;
	if (payload_len) *payload_len = (uint16_t)(dma_len - RF_2A4M1_MT7601U_RXWI_SIZE);
	return RF_2A4M1_S8021X_OK;
}

static int rf_2a4m1_mt7601u_tx(struct rf_2a4m1_hal *h, struct rf_2a4m1_mpdu *m, const struct rf_2a4m1_tx_params *tp)
{
	struct rf_2a4m1_mt7601u_hal *mh = (struct rf_2a4m1_mt7601u_hal *)h;   /* hal is first member */
	int rc = rf_2a4m1_mt7601u_build_txwi(mh->last_txwi, tp, m);
	if (rc != RF_2A4M1_S8021X_OK)
		return rc;
	/* USB bulk-out framing: prepend the DMA TXINFO word, append the 4-byte round-up pad +
	 * trailing zero word (mt7601u_dma_skb_wrap_pkt). WIV mirrors hw_key_idx==0xff (no HW
	 * key => the IV is carried in the frame, not the TXWI). Data frames ride the EDCA qsel.
	 * The mock endpoint mh->last_bulkout is the byte sink the harness inspects; real URB
	 * submit/complete + endpoint queues remain stubbed. */
	bool wiv = (tp->key_slot == 0xff);
	rc = rf_2a4m1_mt7601u_usb_tx_wrap(mh->last_bulkout, sizeof mh->last_bulkout,
	                         mh->last_txwi, RF_2A4M1_MT7601U_TXWI_SIZE,
	                         m->data, m->len, RF_2A4M1_MT_QSEL_EDCA, wiv, &mh->last_bulkout_len);
	if (rc != RF_2A4M1_S8021X_OK)
		return rc;
	mh->last_frame_len = m->len;
	mh->tx_count++;
	return RF_2A4M1_S8021X_OK;
}

int rf_2a4m1_mt7601u_hal_rx_submit(struct rf_2a4m1_mt7601u_hal *mh, const uint8_t *rxwi, uint16_t rxwi_len,
                          const uint8_t *frame, uint16_t frame_len)
{
	struct rf_2a4m1_rxinfo rx;
	int rc;
	if (!mh)
		return RF_2A4M1_S8021X_ERR_INVAL;
	rc = rf_2a4m1_mt7601u_parse_rxwi(rxwi, rxwi_len, frame, frame_len, &rx);
	if (rc != RF_2A4M1_S8021X_OK)
		return rc;
	rf_2a4m1_hal_deliver_rx(&mh->rf_2a4m1_hal, &rx);
	return RF_2A4M1_S8021X_OK;
}

int rf_2a4m1_mt7601u_parse_tx_status(uint32_t fifo, struct rf_2a4m1_hal_tx_status *out)
{
	if (!out)
		return RF_2A4M1_S8021X_ERR_INVAL;

	memset(out, 0, sizeof *out);
	out->valid  = (fifo & RF_2A4M1_MT_TXS_FIFO_VALID) != 0;
	out->cookie = (uint8_t)((fifo & RF_2A4M1_MT_TXS_FIFO_PID_MASK) >> RF_2A4M1_MT_TXS_FIFO_PID_SHIFT);
	out->wcid   = (uint8_t)((fifo & RF_2A4M1_MT_TXS_FIFO_WCID_MASK) >> RF_2A4M1_MT_TXS_FIFO_WCID_SHIFT);
	out->ampdu  = (fifo & RF_2A4M1_MT_TXS_FIFO_AGGR) != 0;

	/* ack/no-ack/fail: mt76_mac_fill_tx_status() sets NO_ACK when !ack_req, else STAT_ACK on
	 * success. So acked = ack was required AND received; failed = required but not received. */
	bool ack_req = (fifo & RF_2A4M1_MT_TXS_FIFO_ACKREQ) != 0;
	bool success = (fifo & RF_2A4M1_MT_TXS_FIFO_SUCCESS) != 0;
	out->no_ack  = !ack_req;
	out->acked   = ack_req && success;
	out->failed  = ack_req && !success;

	/* The upper 16 bits are a rate word laid out exactly like the TXWI/RXWI rate_ctl
	 * (regs.h note + mt76_mac_process_tx_rate) — decode with the shared MT_RATE_* masks. */
	uint16_t rate = (uint16_t)((fifo & RF_2A4M1_MT_TXS_FIFO_RATE_MASK) >> RF_2A4M1_MT_TXS_FIFO_RATE_SHIFT);
	uint8_t phy   = (uint8_t)((rate & RF_2A4M1_MT_RATE_PHY_MASK) >> RF_2A4M1_MT_RATE_PHY_SHIFT);
	out->mcs      = (uint16_t)(rate & RF_2A4M1_MT_RATE_MCS_MASK);
	out->bw_mhz   = (rate & RF_2A4M1_MT_RATE_BW_40) ? 40 : 20;
	out->short_gi = (rate & RF_2A4M1_MT_RATE_SGI) != 0;
	out->stbc     = (rate & RF_2A4M1_MT_RATE_STBC_MASK) != 0;
	switch (phy) {
	case RF_2A4M1_MT_PHY_TYPE_CCK:   out->phy_mode = RF_2A4M1_HAL_PHY_CCK;  out->gen = RF_2A4M1_HAL_GEN_LEGACY; break;
	case RF_2A4M1_MT_PHY_TYPE_OFDM:  out->phy_mode = RF_2A4M1_HAL_PHY_OFDM; out->gen = RF_2A4M1_HAL_GEN_LEGACY; break;
	case RF_2A4M1_MT_PHY_TYPE_HT:
	case RF_2A4M1_MT_PHY_TYPE_HT_GF:
	default:                out->phy_mode = RF_2A4M1_HAL_PHY_HT;   out->gen = RF_2A4M1_HAL_GEN_HT;     break;
	}
	out->n_ss = 1;                        /* 1T1R silicon */

	/* Retry derivation (mt7601u_tx_pktid_dec): the MT7601U has no hardware retry field it
	 * uses (the EXT retry register is left unread) — instead the pktid encodes the REQUESTED
	 * rate (rate+1, +8 for a probe; MCS0/MCS7-probe share pktid 9), so retries = requested -
	 * effective. This is the chip's ONLY rate-fallback signal at the descriptor layer; a
	 * per-rate-stage ladder (hal_tx_status.stage[]) is not reported here, so n_stages stays 0
	 * and the rate controller reconstructs the intermediate stages from its submitted rates. */
	if (out->cookie) {                    /* pktid 0 disables status reporting */
		int req = (int)out->cookie - 1;
		uint8_t eff = (uint8_t)(out->mcs & 0x7);
		if (req > 7) {
			out->is_probe = true;
			req -= 8;
			if (req == 0 && eff)          /* MCS0 vs MCS7 share pktid 9 */
				req = 7;
		}
		int retry = req - (int)eff;
		out->retries = retry > 0 ? (uint8_t)retry : 0;
	}
	return RF_2A4M1_S8021X_OK;
}

int rf_2a4m1_mt7601u_hal_tx_status(struct rf_2a4m1_mt7601u_hal *mh, uint32_t fifo)
{
	struct rf_2a4m1_hal_tx_status ts;
	int rc;
	if (!mh)
		return RF_2A4M1_S8021X_ERR_INVAL;
	rc = rf_2a4m1_mt7601u_parse_tx_status(fifo, &ts);
	if (rc != RF_2A4M1_S8021X_OK)
		return rc;
	rf_2a4m1_hal_deliver_tx_status(&mh->rf_2a4m1_hal, &ts);
	return RF_2A4M1_S8021X_OK;
}

int rf_2a4m1_mt7601u_hal_rx_bulk_in(struct rf_2a4m1_mt7601u_hal *mh, const uint8_t *buf, uint16_t len)
{
	const uint8_t *rxwi, *payload;
	uint16_t rxwi_len, payload_len;
	int rc;
	if (!mh)
		return RF_2A4M1_S8021X_ERR_INVAL;

	rc = rf_2a4m1_mt7601u_usb_rx_unwrap(buf, len, &rxwi, &rxwi_len, &payload, &payload_len);
	if (rc != RF_2A4M1_S8021X_OK)
		return rc;

	/* dma_len is 4-aligned (RX DMA pad), so `payload_len` is the padded span. The true
	 * 802.11 length lives in the RXWI MPDU_LEN field (mt76_mac_process_rx derives true_len
	 * from the descriptor, not the DMA length); clamp the span to it. */
	uint32_t ctl = rf_2a4m1_get_le32(rxwi + RF_2A4M1_MT_RXWI_OFF_CTL);
	uint16_t mpdu_len = (uint16_t)((ctl & RF_2A4M1_MT_RXWI_CTL_MPDU_LEN_MASK) >> RF_2A4M1_MT_RXWI_CTL_MPDU_LEN_SHIFT);
	uint16_t frame_len = mpdu_len < payload_len ? mpdu_len : payload_len;
	return rf_2a4m1_mt7601u_hal_rx_submit(mh, rxwi, rxwi_len, payload, frame_len);
}

static int  rf_2a4m1_mt7601u_start(struct rf_2a4m1_hal *h, const struct rf_2a4m1_hal_cfg *cfg)        { (void)h; (void)cfg; return 0; }

static void rf_2a4m1_mt7601u_stop(struct rf_2a4m1_hal *h)                                    { (void)h; }

static int  rf_2a4m1_mt7601u_set_channel(struct rf_2a4m1_hal *h, const struct rf_2a4m1_chan_def *c)   { (void)h; (void)c; return 0; }

static int  rf_2a4m1_mt7601u_set_key(struct rf_2a4m1_hal *h, uint8_t slot, const struct rf_2a4m1_key *k)    { (void)h; (void)slot; (void)k; return 0; }

static int  rf_2a4m1_mt7601u_set_sta(struct rf_2a4m1_hal *h, uint8_t wcid, const struct rf_2a4m1_sta_cfg *s){ (void)h; (void)wcid; (void)s; return 0; }

static int  rf_2a4m1_mt7601u_set_rx_filter(struct rf_2a4m1_hal *h, uint32_t mask)            { (void)h; (void)mask; return 0; }

static int  rf_2a4m1_mt7601u_calibrate(struct rf_2a4m1_hal *h, enum rf_2a4m1_hal_cal rf_2a4m1_what)            { (void)h; (void)rf_2a4m1_what; return 0; }

static int  rf_2a4m1_mt7601u_set_lower_mac(struct rf_2a4m1_hal *h, const struct rf_2a4m1_lmac_cfg *cfg){ (void)h; (void)cfg; return 0; }

static const struct rf_2a4m1_hal_ops rf_2a4m1_mt7601u_ops = {
	.start = rf_2a4m1_mt7601u_start, .stop = rf_2a4m1_mt7601u_stop, .set_channel = rf_2a4m1_mt7601u_set_channel,
	.tx = rf_2a4m1_mt7601u_tx, .set_key = rf_2a4m1_mt7601u_set_key, .set_sta = rf_2a4m1_mt7601u_set_sta,
	.set_rx_filter = rf_2a4m1_mt7601u_set_rx_filter, .calibrate = rf_2a4m1_mt7601u_calibrate,
	.set_lower_mac = rf_2a4m1_mt7601u_set_lower_mac,
};

void rf_2a4m1_mt7601u_hal_init(struct rf_2a4m1_mt7601u_hal *mh)
{
	if (!mh)
		return;
	memset(mh, 0, sizeof *mh);
	mh->rf_2a4m1_hal.ops  = &rf_2a4m1_mt7601u_ops;
	mh->rf_2a4m1_hal.priv = mh;
	/* MT7601U: single-chip USB 802.11n 1T1R 2.4 GHz. */
	mh->rf_2a4m1_hal.caps.max_gen   = RF_2A4M1_HAL_GEN_HT;      /* Wi-Fi 4 (HT) — the PHY ceiling */
	mh->rf_2a4m1_hal.caps.bands     = RF_2A4M1_HAL_BAND_2G4;    /* 2.4 GHz only */
	mh->rf_2a4m1_hal.caps.max_bw_mhz = 40;             /* HT20 / HT40 */
	mh->rf_2a4m1_hal.caps.tx_chains = 1;
	mh->rf_2a4m1_hal.caps.rx_chains = 1;
	mh->rf_2a4m1_hal.caps.max_mcs   = 7;               /* 1SS HT: MCS0..7 */
	mh->rf_2a4m1_hal.caps.feat      = 0;               
	mh->rf_2a4m1_hal.caps.n_links   = 1;
	mh->rf_2a4m1_hal.caps.lower_mac_does_acks = true;  /* the MAC/BB auto-ACKs */
	mh->rf_2a4m1_hal.caps.crypto_offload = true;       /* per-WCID HW key table (WEP/TKIP/CCMP) */
	mh->rf_2a4m1_hal.caps.n_key_slots = 4;             /* 4 shared keys per BSS */
	mh->rf_2a4m1_hal.caps.n_sta_slots = 8;             /* WCID table (subset used) */
}

