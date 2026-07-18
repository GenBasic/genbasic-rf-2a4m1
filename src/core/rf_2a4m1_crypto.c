// SPDX-License-Identifier: GPL-2.0
//
// rf-2a4m1 -- GPL Wi-Fi driver for MediaTek MT7601U USB silicon.
// WPA2/WPA3 crypto primitives (SHA/AES/CMAC/HKDF, P-256 ECDH/ECDSA, SAE/OWE, BIP)
//
// Copyright (c) GenBasic.
// Licensed under the GNU General Public License, version 2.
//
// This file is machine-generated. Do not hand-edit.


#include "rf_2a4m1_core.h"

/* file-scope constants + helper types */
#define RF_2A4M1_BIP_MAX_BODY 512
#define RF_2A4M1_BIP_AAD_LEN  20
#define RF_2A4M1_BIP_CMAC_MAX (RF_2A4M1_BIP_AAD_LEN + RF_2A4M1_BIP_MAX_BODY + RF_2A4M1_BIP_MMIE_LEN)
#define RF_2A4M1_CCM_NONCE_LEN 13
#define RF_2A4M1_CCM_M          8
#define RF_2A4M1_CCM_L          2
typedef uint32_t rf_2a4m1_fe[8];
struct rf_2a4m1_jac { rf_2a4m1_fe X, Y, Z; };
#define RF_2A4M1_SAE_HNP_MIN_ITER 40

/* file-local forward declarations */
static bool rf_2a4m1_bip_compute_mic(const uint8_t *hdr, const uint8_t *body, size_t body_len,
                            const uint8_t *mmie, const uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                            uint8_t mic[RF_2A4M1_BIP_MIC_LEN]);
static bool rf_2a4m1_fe_is_square(const rf_2a4m1_fe a);
static bool rf_2a4m1_jac_is_inf(const struct rf_2a4m1_jac *p);
static bool rf_2a4m1_jac_to_affine(rf_2a4m1_fe x, rf_2a4m1_fe y, const struct rf_2a4m1_jac *p);
static bool rf_2a4m1_load_point(struct rf_2a4m1_jac *P, const uint8_t pt[64]);
static bool rf_2a4m1_point_on_curve(const rf_2a4m1_fe xm, const rf_2a4m1_fe ym);
static bool rf_2a4m1_scalar_in_range(const uint8_t k[32]);
static bool rf_2a4m1_store_point(uint8_t out[64], const struct rf_2a4m1_jac *P);
static inline bool rf_2a4m1_ccmp_fc_has_a4(const uint8_t *hdr);
static inline bool rf_2a4m1_ccmp_fc_is_qos_data(const uint8_t *hdr);
static int rf_2a4m1_fe_sgn0(const rf_2a4m1_fe a);
static int rf_2a4m1_mac_lt(const rf_2a4m1_mac_addr *x, const rf_2a4m1_mac_addr *y);
static int rf_2a4m1_nonce_lt(const uint8_t *x, const uint8_t *y);
static int rf_2a4m1_u256_cmp(const uint32_t a[8], const uint32_t b[8]);
static size_t rf_2a4m1_build_aad_nonce(const uint8_t *hdr, size_t hdr_len, uint64_t pn,
                              uint8_t aad[30], uint8_t nonce[RF_2A4M1_CCM_NONCE_LEN]);
static uint32_t rf_2a4m1_fe_eq(const rf_2a4m1_fe a, const rf_2a4m1_fe b);
static uint32_t rf_2a4m1_fe_is_zero(const rf_2a4m1_fe a);
static uint32_t rf_2a4m1_rol32(uint32_t v, unsigned n);
static uint32_t rf_2a4m1_ror32(uint32_t x, unsigned n);
static uint32_t rf_2a4m1_u256_sub_borrow(uint32_t r[8], const uint32_t a[8], const uint32_t b[8]);
static uint8_t rf_2a4m1_gf_inv(uint8_t x);
static uint8_t rf_2a4m1_gf_mul(uint8_t a, uint8_t b);
static uint8_t rf_2a4m1_inv_sbox(uint8_t y);
static uint8_t rf_2a4m1_rotl8(uint8_t x, int n);
static uint8_t rf_2a4m1_sbox(uint8_t x);
static uint8_t rf_2a4m1_xtime(uint8_t x);
static void rf_2a4m1_add_round_key(uint8_t s[16], const uint32_t *rk);
static void rf_2a4m1_aes_ccm(const uint8_t rf_2a4m1_key[16], const uint8_t nonce[RF_2A4M1_CCM_NONCE_LEN],
                    const uint8_t *adata, size_t adata_len,
                    uint8_t *mdata, size_t mdata_len, bool encrypt,
                    uint8_t mic[RF_2A4M1_CCM_M]);
static void rf_2a4m1_be_to_fe(rf_2a4m1_fe r, const uint8_t b[32]);
static void rf_2a4m1_bip_aad(const uint8_t *hdr, uint8_t aad[RF_2A4M1_BIP_AAD_LEN]);
static void rf_2a4m1_fe_add(rf_2a4m1_fe r, const rf_2a4m1_fe a, const rf_2a4m1_fe b);
static void rf_2a4m1_fe_cmov(rf_2a4m1_fe r, const rf_2a4m1_fe a, uint32_t flag);
static void rf_2a4m1_fe_cond_sub_mod(uint32_t r[8], const uint32_t t[8], uint32_t extra_carry,
                            const uint32_t mod[8]);
static void rf_2a4m1_fe_from_mont(rf_2a4m1_fe r, const rf_2a4m1_fe a);
static void rf_2a4m1_fe_inv(rf_2a4m1_fe r, const rf_2a4m1_fe a);
static void rf_2a4m1_fe_mul(rf_2a4m1_fe r, const rf_2a4m1_fe a, const rf_2a4m1_fe b);
static void rf_2a4m1_fe_pow(rf_2a4m1_fe r, const rf_2a4m1_fe a, const uint32_t exp[8]);
static void rf_2a4m1_fe_reduce_once(rf_2a4m1_fe r, const rf_2a4m1_fe a);
static void rf_2a4m1_fe_set(rf_2a4m1_fe r, const uint32_t a[8]);
static void rf_2a4m1_fe_sqr(rf_2a4m1_fe r, const rf_2a4m1_fe a);
static void rf_2a4m1_fe_sqrt(rf_2a4m1_fe r, const rf_2a4m1_fe a);
static void rf_2a4m1_fe_sub(rf_2a4m1_fe r, const rf_2a4m1_fe a, const rf_2a4m1_fe b);
static void rf_2a4m1_fe_to_be(uint8_t b[32], const rf_2a4m1_fe a);
static void rf_2a4m1_fe_to_mont(rf_2a4m1_fe r, const rf_2a4m1_fe a);
static void rf_2a4m1_fe_zero(rf_2a4m1_fe r);
static void rf_2a4m1_jac_add(struct rf_2a4m1_jac *out, const struct rf_2a4m1_jac *a, const struct rf_2a4m1_jac *b);
static void rf_2a4m1_jac_cmov(struct rf_2a4m1_jac *r, const struct rf_2a4m1_jac *a, uint32_t flag);
static void rf_2a4m1_jac_double(struct rf_2a4m1_jac *out, const struct rf_2a4m1_jac *p);
static void rf_2a4m1_jac_mul(struct rf_2a4m1_jac *R, const uint8_t k[32], const struct rf_2a4m1_jac *P);
static void rf_2a4m1_jac_set_inf(struct rf_2a4m1_jac *p);
static void rf_2a4m1_lshift1(const uint8_t *in, uint8_t *out);
static void rf_2a4m1_sae_cn(const uint8_t kck[RF_2A4M1_SAE_KEY_LEN], uint16_t send_confirm,
                   const uint8_t *sc1, const uint8_t *el1,
                   const uint8_t *sc2, const uint8_t *el2, uint8_t out[RF_2A4M1_SAE_KEY_LEN]);
static void rf_2a4m1_sha1_block(struct rf_2a4m1_sha1_ctx *c, const uint8_t *p);
static void rf_2a4m1_sha1_sw_final(struct rf_2a4m1_sha1_ctx *c, uint8_t out[RF_2A4M1_SHA1_DIGEST_LEN]);
static void rf_2a4m1_sha1_sw_init(struct rf_2a4m1_sha1_ctx *c);
static void rf_2a4m1_sha1_sw_update(struct rf_2a4m1_sha1_ctx *c, const uint8_t *data, size_t len);
static void rf_2a4m1_sha256_block(struct rf_2a4m1_sha256_ctx *c, const uint8_t *p);
static void rf_2a4m1_sha256_sw_final(struct rf_2a4m1_sha256_ctx *c, uint8_t out[RF_2A4M1_SHA256_DIGEST_LEN]);
static void rf_2a4m1_sha256_sw_init(struct rf_2a4m1_sha256_ctx *c);
static void rf_2a4m1_sha256_sw_update(struct rf_2a4m1_sha256_ctx *c, const uint8_t *data, size_t len);

static uint8_t rf_2a4m1_gf_mul(uint8_t a, uint8_t b)
{
	uint8_t r = 0;
	for (int i = 0; i < 8; i++) {
		r ^= a & (uint8_t)(0 - (b & 1));          /* add a iff low bit of b set */
		uint8_t hi = (uint8_t)(0 - (a >> 7));     /* 0xff iff high bit of a set  */
		a = (uint8_t)((a << 1) ^ (hi & 0x1b));    /* xtime with reduction 0x1b   */
		b = (uint8_t)(b >> 1);
	}
	return r;
}

static uint8_t rf_2a4m1_gf_inv(uint8_t x)
{
	uint8_t r = 1, b = x;
	for (int i = 1; i < 8; i++) {
		b = rf_2a4m1_gf_mul(b, b);      /* b = x^(2^i) */
		r = rf_2a4m1_gf_mul(r, b);      /* accumulate the running product */
	}
	return r;
}

static uint8_t rf_2a4m1_rotl8(uint8_t x, int n) { return (uint8_t)((x << n) | (x >> (8 - n))); }

static uint8_t rf_2a4m1_sbox(uint8_t x)
{
	uint8_t s = rf_2a4m1_gf_inv(x);
	return (uint8_t)(s ^ rf_2a4m1_rotl8(s, 1) ^ rf_2a4m1_rotl8(s, 2) ^ rf_2a4m1_rotl8(s, 3) ^ rf_2a4m1_rotl8(s, 4) ^ 0x63);
}

static uint8_t rf_2a4m1_inv_sbox(uint8_t y)
{
	uint8_t a = (uint8_t)(rf_2a4m1_rotl8(y, 1) ^ rf_2a4m1_rotl8(y, 3) ^ rf_2a4m1_rotl8(y, 6) ^ 0x05);
	return rf_2a4m1_gf_inv(a);
}

static const uint8_t RF_2A4M1_RCON[10] = {
	0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36
};

void rf_2a4m1_aes128_init(struct rf_2a4m1_aes128_ctx *c, const uint8_t rf_2a4m1_key[RF_2A4M1_AES128_KEY_LEN])
{
	for (int i = 0; i < 4; i++)
		c->rk[i] = ((uint32_t)rf_2a4m1_key[4 * i] << 24) | ((uint32_t)rf_2a4m1_key[4 * i + 1] << 16) |
		           ((uint32_t)rf_2a4m1_key[4 * i + 2] << 8) | (uint32_t)rf_2a4m1_key[4 * i + 3];
	for (int i = 4; i < 44; i++) {
		uint32_t t = c->rk[i - 1];
		if (i % 4 == 0) {
			t = (t << 8) | (t >> 24);          /* RotWord */
			t = ((uint32_t)rf_2a4m1_sbox((t >> 24) & 0xff) << 24) |
			    ((uint32_t)rf_2a4m1_sbox((t >> 16) & 0xff) << 16) |
			    ((uint32_t)rf_2a4m1_sbox((t >> 8) & 0xff) << 8) |
			    (uint32_t)rf_2a4m1_sbox(t & 0xff);      /* SubWord */
			t ^= (uint32_t)RF_2A4M1_RCON[i / 4 - 1] << 24;
		}
		c->rk[i] = c->rk[i - 4] ^ t;
	}
}

static uint8_t rf_2a4m1_xtime(uint8_t x) { return (uint8_t)((x << 1) ^ ((x >> 7) * 0x1b)); }

static void rf_2a4m1_add_round_key(uint8_t s[16], const uint32_t *rk)
{
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			s[4 * c + r] ^= (uint8_t)(rk[c] >> (24 - 8 * r));
}

void rf_2a4m1_aes128_encrypt_block_soft(const struct rf_2a4m1_aes128_ctx *c,
                          const uint8_t in[RF_2A4M1_AES_BLOCK_LEN], uint8_t out[RF_2A4M1_AES_BLOCK_LEN])
{
	uint8_t s[16];
	memcpy(s, in, 16);
	rf_2a4m1_add_round_key(s, &c->rk[0]);
	for (int round = 1; round <= 10; round++) {
		/* SubBytes */
		for (int i = 0; i < 16; i++)
			s[i] = rf_2a4m1_sbox(s[i]);
		/* ShiftRows (row r shifts left by r; state[4c+r]) */
		uint8_t t[16];
		for (int col = 0; col < 4; col++)
			for (int r = 0; r < 4; r++)
				t[4 * col + r] = s[4 * ((col + r) & 3) + r];
		memcpy(s, t, 16);
		/* MixColumns (skip on last round) */
		if (round != 10) {
			for (int col = 0; col < 4; col++) {
				uint8_t *p = &s[4 * col];
				uint8_t a0 = p[0], a1 = p[1], a2 = p[2], a3 = p[3];
				p[0] = (uint8_t)(rf_2a4m1_xtime(a0) ^ (rf_2a4m1_xtime(a1) ^ a1) ^ a2 ^ a3);
				p[1] = (uint8_t)(a0 ^ rf_2a4m1_xtime(a1) ^ (rf_2a4m1_xtime(a2) ^ a2) ^ a3);
				p[2] = (uint8_t)(a0 ^ a1 ^ rf_2a4m1_xtime(a2) ^ (rf_2a4m1_xtime(a3) ^ a3));
				p[3] = (uint8_t)((rf_2a4m1_xtime(a0) ^ a0) ^ a1 ^ a2 ^ rf_2a4m1_xtime(a3));
			}
		}
		rf_2a4m1_add_round_key(s, &c->rk[4 * round]);
	}
	memcpy(out, s, 16);
}

void rf_2a4m1_aes128_decrypt_block_soft(const struct rf_2a4m1_aes128_ctx *c,
                          const uint8_t in[RF_2A4M1_AES_BLOCK_LEN], uint8_t out[RF_2A4M1_AES_BLOCK_LEN])
{
	uint8_t s[16];
	memcpy(s, in, 16);
	rf_2a4m1_add_round_key(s, &c->rk[40]);
	for (int round = 9; round >= 0; round--) {
		/* InvShiftRows (row r shifts right by r) */
		uint8_t t[16];
		for (int col = 0; col < 4; col++)
			for (int r = 0; r < 4; r++)
				t[4 * col + r] = s[4 * ((col - r) & 3) + r];
		memcpy(s, t, 16);
		/* InvSubBytes */
		for (int i = 0; i < 16; i++)
			s[i] = rf_2a4m1_inv_sbox(s[i]);
		rf_2a4m1_add_round_key(s, &c->rk[4 * round]);
		/* InvMixColumns (skip on the final decrypt round, round 0) */
		if (round != 0) {
			for (int col = 0; col < 4; col++) {
				uint8_t *p = &s[4 * col];
				uint8_t a0 = p[0], a1 = p[1], a2 = p[2], a3 = p[3];
				p[0] = (uint8_t)(rf_2a4m1_gf_mul(a0, 14) ^ rf_2a4m1_gf_mul(a1, 11) ^ rf_2a4m1_gf_mul(a2, 13) ^ rf_2a4m1_gf_mul(a3, 9));
				p[1] = (uint8_t)(rf_2a4m1_gf_mul(a0, 9) ^ rf_2a4m1_gf_mul(a1, 14) ^ rf_2a4m1_gf_mul(a2, 11) ^ rf_2a4m1_gf_mul(a3, 13));
				p[2] = (uint8_t)(rf_2a4m1_gf_mul(a0, 13) ^ rf_2a4m1_gf_mul(a1, 9) ^ rf_2a4m1_gf_mul(a2, 14) ^ rf_2a4m1_gf_mul(a3, 11));
				p[3] = (uint8_t)(rf_2a4m1_gf_mul(a0, 11) ^ rf_2a4m1_gf_mul(a1, 13) ^ rf_2a4m1_gf_mul(a2, 9) ^ rf_2a4m1_gf_mul(a3, 14));
			}
		}
	}
	memcpy(out, s, 16);
}

void rf_2a4m1_aes128_encrypt_block(const struct rf_2a4m1_aes128_ctx *c,
                          const uint8_t in[RF_2A4M1_AES_BLOCK_LEN], uint8_t out[RF_2A4M1_AES_BLOCK_LEN])
{
	rf_2a4m1_aes128_encrypt_block_soft(c, in, out);
}

void rf_2a4m1_aes128_decrypt_block(const struct rf_2a4m1_aes128_ctx *c,
                          const uint8_t in[RF_2A4M1_AES_BLOCK_LEN], uint8_t out[RF_2A4M1_AES_BLOCK_LEN])
{
	rf_2a4m1_aes128_decrypt_block_soft(c, in, out);
}

static void rf_2a4m1_lshift1(const uint8_t *in, uint8_t *out)
{
	uint8_t carry = 0;
	for (int i = 15; i >= 0; i--) {
		uint8_t v = in[i];
		out[i] = (uint8_t)((v << 1) | carry);
		carry = (v >> 7) & 1;
	}
}

void rf_2a4m1_aes128_cmac(const uint8_t rf_2a4m1_key[RF_2A4M1_AES128_KEY_LEN],
                 const uint8_t *msg, size_t msg_len, uint8_t mac[RF_2A4M1_AES_BLOCK_LEN])
{
	struct rf_2a4m1_aes128_ctx c;
	uint8_t l[16], k1[16], k2[16], x[16], y[16];
	rf_2a4m1_aes128_init(&c, rf_2a4m1_key);

	memset(x, 0, 16);
	rf_2a4m1_aes128_encrypt_block(&c, x, l);           /* L = AES_K(0^128) */
	rf_2a4m1_lshift1(l, k1);
	if (l[0] & 0x80)
		k1[15] ^= 0x87;
	rf_2a4m1_lshift1(k1, k2);
	if (k1[0] & 0x80)
		k2[15] ^= 0x87;

	size_t n = (msg_len + 15) / 16;
	bool complete;
	if (n == 0) {
		n = 1;
		complete = false;
	} else {
		complete = (msg_len % 16) == 0;
	}

	uint8_t last[16];
	size_t last_off = (n - 1) * 16;
	if (complete) {
		for (int i = 0; i < 16; i++)
			last[i] = msg[last_off + i] ^ k1[i];
	} else {
		size_t rem = msg_len - last_off;
		for (size_t i = 0; i < 16; i++) {
			uint8_t m;
			if (i < rem)      m = msg[last_off + i];
			else if (i == rem) m = 0x80;
			else               m = 0x00;
			last[i] = m ^ k2[i];
		}
	}

	memset(x, 0, 16);
	for (size_t i = 0; i < n - 1; i++) {
		for (int j = 0; j < 16; j++)
			y[j] = x[j] ^ msg[i * 16 + j];
		rf_2a4m1_aes128_encrypt_block(&c, y, x);
	}
	for (int j = 0; j < 16; j++)
		y[j] = x[j] ^ last[j];
	rf_2a4m1_aes128_encrypt_block(&c, y, mac);
}

static const uint8_t RF_2A4M1_KW_IV[8] = { 0xA6,0xA6,0xA6,0xA6,0xA6,0xA6,0xA6,0xA6 };

size_t rf_2a4m1_aes_key_wrap(const uint8_t kek[RF_2A4M1_AES128_KEY_LEN],
                    const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap)
{
	if (in_len < 16 || (in_len % 8) != 0)
		return 0;
	if (out_cap < in_len + 8)
		return 0;
	size_t nblk = in_len / 8;              /* number of 64-bit key blocks */
	struct rf_2a4m1_aes128_ctx c;
	rf_2a4m1_aes128_init(&c, kek);

	uint8_t a[8];
	memcpy(a, RF_2A4M1_KW_IV, 8);
	memcpy(out + 8, in, in_len);           /* R[1..n] laid out after the 8-byte A */

	for (int j = 0; j <= 5; j++) {
		for (size_t i = 1; i <= nblk; i++) {
			uint8_t blk[16], enc[16];
			memcpy(blk, a, 8);
			memcpy(blk + 8, out + 8 * i, 8);
			rf_2a4m1_aes128_encrypt_block(&c, blk, enc);
			uint64_t t = (uint64_t)nblk * (uint64_t)j + i;
			memcpy(a, enc, 8);
			for (int k = 0; k < 8; k++)
				a[7 - k] ^= (uint8_t)(t >> (8 * k));   /* A ^= t */
			memcpy(out + 8 * i, enc + 8, 8);           /* R[i] = MSB? -> low 8 */
		}
	}
	memcpy(out, a, 8);
	return in_len + 8;
}

size_t rf_2a4m1_aes_key_unwrap(const uint8_t kek[RF_2A4M1_AES128_KEY_LEN],
                      const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap)
{
	if (in_len < 24 || (in_len % 8) != 0)
		return 0;
	size_t nblk = in_len / 8 - 1;
	if (out_cap < nblk * 8)
		return 0;
	struct rf_2a4m1_aes128_ctx c;
	rf_2a4m1_aes128_init(&c, kek);

	uint8_t a[8];
	memcpy(a, in, 8);
	memcpy(out, in + 8, nblk * 8);         /* R[1..n] */

	for (int j = 5; j >= 0; j--) {
		for (size_t i = nblk; i >= 1; i--) {
			uint8_t blk[16], dec[16];
			uint64_t t = (uint64_t)nblk * (uint64_t)j + i;
			memcpy(blk, a, 8);
			for (int k = 0; k < 8; k++)
				blk[7 - k] ^= (uint8_t)(t >> (8 * k));   /* A ^= t */
			memcpy(blk + 8, out + 8 * (i - 1), 8);
			rf_2a4m1_aes128_decrypt_block(&c, blk, dec);
			memcpy(a, dec, 8);
			memcpy(out + 8 * (i - 1), dec + 8, 8);
		}
	}
	if (!rf_2a4m1_crypto_ct_eq(a, RF_2A4M1_KW_IV, 8))
		return 0;                          /* integrity check failed */
	return nblk * 8;
}

bool rf_2a4m1_crypto_ct_eq(const uint8_t *a, const uint8_t *b, size_t len)
{
	uint8_t diff = 0;
	for (size_t i = 0; i < len; i++)
		diff |= (uint8_t)(a[i] ^ b[i]);
	return diff == 0;
}

static void rf_2a4m1_bip_aad(const uint8_t *hdr, uint8_t aad[RF_2A4M1_BIP_AAD_LEN])
{
	aad[0] = hdr[0];                      /* FC[0] — version/type/subtype, unmasked */
	aad[1] = (uint8_t)(hdr[1] & 0xC7);    /* FC[1] — mask Retry/PwrMgt/MoreData (bits 11-13) */
	memcpy(&aad[2],  hdr + 4,  6);        /* A1 (header: FC(2) Dur(2) then A1) */
	memcpy(&aad[8],  hdr + 10, 6);        /* A2 */
	memcpy(&aad[14], hdr + 16, 6);        /* A3 */
}

static bool rf_2a4m1_bip_compute_mic(const uint8_t *hdr, const uint8_t *body, size_t body_len,
                            const uint8_t *mmie, const uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                            uint8_t mic[RF_2A4M1_BIP_MIC_LEN])
{
	if (body_len > RF_2A4M1_BIP_MAX_BODY)
		return false;

	uint8_t msg[RF_2A4M1_BIP_CMAC_MAX];
	size_t o = 0;
	rf_2a4m1_bip_aad(hdr, msg);                                    o += RF_2A4M1_BIP_AAD_LEN;
	memcpy(&msg[o], body, body_len);                     o += body_len;
	/* MMIE, MIC field zeroed: the EID|Len|KeyID|IPN (10 octets) are authenticated, */
	memcpy(&msg[o], mmie, 2 + RF_2A4M1_BIP_KEYID_LEN + RF_2A4M1_BIP_IPN_LEN);
	o += 2 + RF_2A4M1_BIP_KEYID_LEN + RF_2A4M1_BIP_IPN_LEN;
	memset(&msg[o], 0, RF_2A4M1_BIP_MIC_LEN);                     o += RF_2A4M1_BIP_MIC_LEN;  /* the 8 MIC octets = 0 */

	uint8_t full[RF_2A4M1_AES_BLOCK_LEN];
	rf_2a4m1_aes128_cmac(igtk, msg, o, full);                     /* AES-128-CMAC (RFC 4493) */
	memcpy(mic, full, RF_2A4M1_BIP_MIC_LEN);                      /* BIP-CMAC-128: truncate to 64 bits */

	rf_2a4m1_crypto_wipe(msg, sizeof msg);
	rf_2a4m1_crypto_wipe(full, sizeof full);
	return true;
}

size_t rf_2a4m1_bip_mmie_protect(uint8_t *out, size_t out_cap,
                        const uint8_t *hdr, const uint8_t *body, size_t body_len,
                        const uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                        const uint8_t ipn[RF_2A4M1_BIP_IPN_LEN], uint16_t key_id)
{
	size_t total = RF_2A4M1_BIP_MGMT_HDR_LEN + body_len + RF_2A4M1_BIP_MMIE_LEN;
	if (body_len > RF_2A4M1_BIP_MAX_BODY || total > out_cap)
		return 0;

	memcpy(out, hdr, RF_2A4M1_BIP_MGMT_HDR_LEN);
	memcpy(out + RF_2A4M1_BIP_MGMT_HDR_LEN, body, body_len);

	/* Build the MMIE with the MIC field zeroed, then fill the MIC. */
	uint8_t *mmie = out + RF_2A4M1_BIP_MGMT_HDR_LEN + body_len;
	mmie[0] = RF_2A4M1_BIP_MMIE_EID;
	mmie[1] = RF_2A4M1_BIP_MMIE_BODY;                              /* Length = 16 */
	mmie[2] = (uint8_t)(key_id & 0xff);                  /* KeyID, little-endian */
	mmie[3] = (uint8_t)(key_id >> 8);
	memcpy(&mmie[4], ipn, RF_2A4M1_BIP_IPN_LEN);                  /* IPN (6 octets, as supplied) */
	memset(&mmie[4 + RF_2A4M1_BIP_IPN_LEN], 0, RF_2A4M1_BIP_MIC_LEN);

	uint8_t mic[RF_2A4M1_BIP_MIC_LEN];
	if (!rf_2a4m1_bip_compute_mic(out, body, body_len, mmie, igtk, mic))
		return 0;
	memcpy(&mmie[4 + RF_2A4M1_BIP_IPN_LEN], mic, RF_2A4M1_BIP_MIC_LEN);
	rf_2a4m1_crypto_wipe(mic, sizeof mic);
	return total;
}

int rf_2a4m1_bip_mmie_verify(const uint8_t *frame, size_t frame_len,
                    const uint8_t igtk[RF_2A4M1_BIP_IGTK_LEN],
                    uint64_t *replay_ctr, uint16_t *key_id_out)
{
	if (frame_len < RF_2A4M1_BIP_MGMT_HDR_LEN + RF_2A4M1_BIP_MMIE_LEN)
		return RF_2A4M1_BIP_ERR_MALFORMED;
	size_t body_len = frame_len - RF_2A4M1_BIP_MGMT_HDR_LEN - RF_2A4M1_BIP_MMIE_LEN;
	if (body_len > RF_2A4M1_BIP_MAX_BODY)
		return RF_2A4M1_BIP_ERR_MALFORMED;

	const uint8_t *hdr  = frame;
	const uint8_t *body = frame + RF_2A4M1_BIP_MGMT_HDR_LEN;
	const uint8_t *mmie = frame + RF_2A4M1_BIP_MGMT_HDR_LEN + body_len;

	if (mmie[0] != RF_2A4M1_BIP_MMIE_EID || mmie[1] != RF_2A4M1_BIP_MMIE_BODY)
		return RF_2A4M1_BIP_ERR_MALFORMED;

	if (key_id_out)
		*key_id_out = (uint16_t)(mmie[2] | ((uint16_t)mmie[3] << 8));

	/* 1) Authenticity FIRST: recompute the MIC over AAD ‖ body ‖ MMIE(MIC=0), compare
	 *    constant-time. A forged/corrupted frame stops here (BIP_ERR_MIC) and never
	 *    touches the replay state. */
	uint8_t mic[RF_2A4M1_BIP_MIC_LEN];
	if (!rf_2a4m1_bip_compute_mic(hdr, body, body_len, mmie, igtk, mic))
		return RF_2A4M1_BIP_ERR_MALFORMED;
	bool ok = rf_2a4m1_crypto_ct_eq(mic, &mmie[4 + RF_2A4M1_BIP_IPN_LEN], RF_2A4M1_BIP_MIC_LEN);
	rf_2a4m1_crypto_wipe(mic, sizeof mic);
	if (!ok)
		return RF_2A4M1_BIP_ERR_MIC;

	/* 2) Replay: the IPN (48-bit big-endian, IPN[0] = MSB) must STRICTLY exceed the
	 *    stored per-IGTK counter; a genuine but replayed frame (valid MIC, old IPN)
	 *    is rejected here (BIP_ERR_REPLAY). On success, advance the counter. */
	const uint8_t *ipn = &mmie[4];
	uint64_t rx_ipn = 0;
	for (int i = 0; i < RF_2A4M1_BIP_IPN_LEN; i++)
		rx_ipn = (rx_ipn << 8) | ipn[i];
	if (replay_ctr) {
		if (rx_ipn <= *replay_ctr)
			return RF_2A4M1_BIP_ERR_REPLAY;
		*replay_ctr = rx_ipn;
	}
	return RF_2A4M1_BIP_OK;
}

static void rf_2a4m1_aes_ccm(const uint8_t rf_2a4m1_key[16], const uint8_t nonce[RF_2A4M1_CCM_NONCE_LEN],
                    const uint8_t *adata, size_t adata_len,
                    uint8_t *mdata, size_t mdata_len, bool encrypt,
                    uint8_t mic[RF_2A4M1_CCM_M])
{
	struct rf_2a4m1_aes128_ctx c;
	uint8_t block[16], ctr[16], ctrpad[16];
	const uint8_t nonce_len = RF_2A4M1_CCM_NONCE_LEN;
	const uint8_t L = RF_2A4M1_CCM_L;

	rf_2a4m1_aes128_init(&c, rf_2a4m1_key);

	/* B0 = flags ‖ nonce ‖ big-endian m-data length. flags = (adata?1:0)<<6 | M'<<3 | L-1,
	 * with M' = (M-2)/2 = 3 for M=8. */
	block[0] = (uint8_t)(((adata_len != 0) << 6) | (((RF_2A4M1_CCM_M - 2) >> 1) << 3) | (L - 1));
	memcpy(&block[1], nonce, nonce_len);
	{
		size_t len = mdata_len;
		for (int i = 15; i > nonce_len; i--) {
			block[i] = (uint8_t)(len & 0xff);
			len >>= 8;
		}
	}
	rf_2a4m1_aes128_encrypt_block(&c, block, block);                /* X_1 = E(B0) */

	/* CBC-MAC over the a-data (2-octet length prefix + bytes, zero-padded per block). */
	uint8_t blen = 0;
	if (adata_len > 0) {
		block[blen++] ^= (uint8_t)(adata_len >> 8);
		block[blen++] ^= (uint8_t)(adata_len & 0xff);
		for (size_t i = 0; i < adata_len; i++) {
			if (blen == 16) { rf_2a4m1_aes128_encrypt_block(&c, block, block); blen = 0; }
			block[blen++] ^= adata[i];
		}
		if (blen != 0) { rf_2a4m1_aes128_encrypt_block(&c, block, block); blen = 0; }
	}

	/* CTR init: A_0 = (L-1) ‖ nonce ‖ 0…0 . First payload byte bumps it to A_1. */
	ctr[0] = (uint8_t)(L - 1);
	memcpy(&ctr[1], nonce, nonce_len);
	memset(&ctr[nonce_len + 1], 0, (size_t)(16 - nonce_len - 1));
	uint8_t ctrlen = 16;                                   /* force the first counter step */

	for (size_t i = 0; i < mdata_len; i++) {
		if (ctrlen == 16) {
			for (int j = 15; j > nonce_len; j--)
				if (++ctr[j]) break;
			rf_2a4m1_aes128_encrypt_block(&c, ctr, ctrpad);
			ctrlen = 0;
		}
		uint8_t pbyte;
		if (encrypt) {
			pbyte = mdata[i];
			mdata[i] = (uint8_t)(pbyte ^ ctrpad[ctrlen++]);
		} else {
			pbyte = (uint8_t)(mdata[i] ^ ctrpad[ctrlen++]);
			mdata[i] = pbyte;
		}
		if (blen == 16) { rf_2a4m1_aes128_encrypt_block(&c, block, block); blen = 0; }
		block[blen++] ^= pbyte;                            /* CBC-MAC over the PLAINTEXT */
	}
	if (blen != 0)
		rf_2a4m1_aes128_encrypt_block(&c, block, block);

	/* tag = CBC-MAC ⊕ E(A_0). Reset the counter to A_0 first. */
	memset(&ctr[nonce_len + 1], 0, (size_t)(16 - nonce_len - 1));
	rf_2a4m1_aes128_encrypt_block(&c, ctr, ctrpad);
	for (uint8_t i = 0; i < RF_2A4M1_CCM_M; i++)
		mic[i] = (uint8_t)(block[i] ^ ctrpad[i]);

	rf_2a4m1_crypto_wipe(block, sizeof block);
	rf_2a4m1_crypto_wipe(ctrpad, sizeof ctrpad);
	rf_2a4m1_crypto_wipe(&c, sizeof c);
}

static inline bool rf_2a4m1_ccmp_fc_is_qos_data(const uint8_t *hdr)
{
	/* type == data (bits 2-3 = 10b) and subtype QoS bit (bit 3 of subtype = FC bit 7). */
	return ((hdr[0] & 0x0c) == 0x08) && (hdr[0] & 0x80);
}

static inline bool rf_2a4m1_ccmp_fc_has_a4(const uint8_t *hdr)
{
	return (hdr[1] & 0x03) == 0x03;                        /* ToDS && FromDS */
}

static size_t rf_2a4m1_build_aad_nonce(const uint8_t *hdr, size_t hdr_len, uint64_t pn,
                              uint8_t aad[30], uint8_t nonce[RF_2A4M1_CCM_NONCE_LEN])
{
	const bool qos = rf_2a4m1_ccmp_fc_is_qos_data(hdr);
	const bool a4  = rf_2a4m1_ccmp_fc_has_a4(hdr);
	const bool mgmt = (hdr[0] & 0x0c) == 0x00;             /* FC type == Management (individual robust mgmt frame under PMF) */
	const uint8_t *a2 = hdr + 10;

	/* nonce: flags (priority = QoS TID else 0; §12.5.3.3.4 Nonce-Flags bit 4 = Management) ‖ A2 ‖ PN. */
	uint8_t prio = 0;
	if (qos) {
		const uint8_t *qc = hdr + (a4 ? 30 : 24);          /* QoS Control follows A4 (if any) */
		prio = (uint8_t)(qc[0] & 0x0f);
	}
	nonce[0] = (uint8_t)(prio | (mgmt ? 0x10 : 0));        /* §12.5.3.3.4: Management bit (0x10) set for mgmt, 0 for data */
	memcpy(&nonce[1], a2, 6);
	nonce[7]  = (uint8_t)(pn >> 40);
	nonce[8]  = (uint8_t)(pn >> 32);
	nonce[9]  = (uint8_t)(pn >> 24);
	nonce[10] = (uint8_t)(pn >> 16);
	nonce[11] = (uint8_t)(pn >> 8);
	nonce[12] = (uint8_t)(pn);

	/* AAD. */
	size_t o = 0;
	aad[o++] = mgmt ? hdr[0] : (uint8_t)(hdr[0] & 0x8f);   /* §12.5.3.3.3 FC[0]: subtype bits 4-6 masked for DATA only, KEPT for mgmt */
	aad[o++] = (uint8_t)(hdr[1] & 0xc7);                   /* FC[1]: mask Retry/PwrMgt/MoreData */
	memcpy(&aad[o], hdr + 4, 6);  o += 6;                  /* A1 */
	memcpy(&aad[o], hdr + 10, 6); o += 6;                  /* A2 */
	memcpy(&aad[o], hdr + 16, 6); o += 6;                  /* A3 */
	aad[o++] = (uint8_t)(hdr[22] & 0x0f);                  /* SC: keep frag number, mask seq */
	aad[o++] = 0x00;
	if (a4) { memcpy(&aad[o], hdr + 24, 6); o += 6; }      /* A4 (only when ToDS&&FromDS) */
	if (qos) {
		const uint8_t *qc = hdr + (a4 ? 30 : 24);
		aad[o++] = (uint8_t)(qc[0] & 0x0f);               /* QoS: TID only */
		aad[o++] = 0x00;
	}
	(void)hdr_len;
	return o;
}

size_t rf_2a4m1_ccmp_encrypt(uint8_t *out, size_t out_cap,
                    const uint8_t *hdr, size_t hdr_len,
                    const uint8_t *payload, size_t payload_len,
                    const uint8_t tk[16], uint8_t key_id, uint64_t pn)
{
	size_t total = hdr_len + RF_2A4M1_CCMP_HDR_LEN + payload_len + RF_2A4M1_CCMP_MIC_LEN;
	if (total > out_cap || hdr_len < 24)
		return 0;

	/* copy the header, force the Protected bit (FC bit 14 = FC[1] bit 6). */
	memcpy(out, hdr, hdr_len);
	out[1] |= 0x40;

	/* CCMP header (8 octets): PN0 PN1 rsvd (ExtIV|KeyID) PN2 PN3 PN4 PN5. */
	uint8_t *ch = out + hdr_len;
	ch[0] = (uint8_t)(pn);
	ch[1] = (uint8_t)(pn >> 8);
	ch[2] = 0x00;
	ch[3] = (uint8_t)(0x20 | ((key_id & 0x3) << 6));       /* ExtIV=1 */
	ch[4] = (uint8_t)(pn >> 16);
	ch[5] = (uint8_t)(pn >> 24);
	ch[6] = (uint8_t)(pn >> 32);
	ch[7] = (uint8_t)(pn >> 40);

	/* the AAD/nonce come from the (protected) header. */
	uint8_t aad[30], nonce[RF_2A4M1_CCM_NONCE_LEN];
	size_t aad_len = rf_2a4m1_build_aad_nonce(out, hdr_len, pn, aad, nonce);

	/* body = plaintext copied into place, then ciphered in-place; MIC appended. */
	uint8_t *body = out + hdr_len + RF_2A4M1_CCMP_HDR_LEN;
	memcpy(body, payload, payload_len);
	uint8_t mic[RF_2A4M1_CCM_M];
	rf_2a4m1_aes_ccm(tk, nonce, aad, aad_len, body, payload_len, true, mic);
	memcpy(body + payload_len, mic, RF_2A4M1_CCM_M);

	rf_2a4m1_crypto_wipe(aad, sizeof aad);
	rf_2a4m1_crypto_wipe(nonce, sizeof nonce);
	rf_2a4m1_crypto_wipe(mic, sizeof mic);
	return total;
}

int rf_2a4m1_ccmp_decrypt(const uint8_t *in, size_t in_len, size_t hdr_len,
                 const uint8_t tk[16], uint8_t *out, size_t out_cap, uint64_t *pn_out)
{
	if (hdr_len < 24 || in_len < hdr_len + RF_2A4M1_CCMP_HDR_LEN + RF_2A4M1_CCMP_MIC_LEN)
		return -1;
	const uint8_t *ch = in + hdr_len;
	if (!(ch[3] & 0x20))                                   /* ExtIV must be set for CCMP */
		return -1;

	uint64_t pn = (uint64_t)ch[0] | ((uint64_t)ch[1] << 8) |
	              ((uint64_t)ch[4] << 16) | ((uint64_t)ch[5] << 24) |
	              ((uint64_t)ch[6] << 32) | ((uint64_t)ch[7] << 40);
	if (pn_out) *pn_out = pn;

	size_t body_len = in_len - hdr_len - RF_2A4M1_CCMP_HDR_LEN - RF_2A4M1_CCMP_MIC_LEN;
	if (body_len > out_cap)
		return -1;

	uint8_t aad[30], nonce[RF_2A4M1_CCM_NONCE_LEN];
	size_t aad_len = rf_2a4m1_build_aad_nonce(in, hdr_len, pn, aad, nonce);

	/* decipher the ciphertext body into `out`, recompute the tag, compare. */
	memcpy(out, in + hdr_len + RF_2A4M1_CCMP_HDR_LEN, body_len);
	uint8_t mic[RF_2A4M1_CCM_M];
	rf_2a4m1_aes_ccm(tk, nonce, aad, aad_len, out, body_len, false, mic);
	const uint8_t *rx_mic = in + hdr_len + RF_2A4M1_CCMP_HDR_LEN + body_len;
	bool ok = rf_2a4m1_crypto_ct_eq(mic, rx_mic, RF_2A4M1_CCM_M);

	rf_2a4m1_crypto_wipe(aad, sizeof aad);
	rf_2a4m1_crypto_wipe(nonce, sizeof nonce);
	rf_2a4m1_crypto_wipe(mic, sizeof mic);
	if (!ok) {
		rf_2a4m1_crypto_wipe(out, body_len);
		return -1;
	}
	return (int)body_len;
}

int rf_2a4m1_ccmp_decrypt_rx(const uint8_t *in, size_t in_len, size_t hdr_len,
                    const uint8_t tk[16], uint8_t *out, size_t out_cap,
                    uint64_t *pn_out, struct rf_2a4m1_pn_replay *rp)
{
	/* Crypto FIRST: ccmp_decrypt parses the CCMP header, verifies the AES-CCM MIC, and
	 * (on success) copies the plaintext body + reports the 48-bit PN. A malformed frame
	 * or a MIC failure returns < 0 here — BEFORE any replay state is consulted, so a
	 * forged frame can never masquerade as (or perturb) a replay (MIC-before-replay,
	 * matching bip_mmie_verify). */
	uint64_t pn = 0;
	int r = rf_2a4m1_ccmp_decrypt(in, in_len, hdr_len, tk, out, out_cap, &pn);
	if (pn_out)
		*pn_out = pn;
	if (r < 0)
		return RF_2A4M1_CCMP_RX_E_CRYPTO;            /* malformed / too short / MIC failure */

	/* MIC verified — now the per-TID replay check (§12.5.3.4.4). The frame's own QoS TID
	 * (or the shared non-QoS/mgmt slot) selects the counter; the PN must strictly exceed
	 * the highest so far accepted for it. A genuine but replayed / out-of-order-old frame
	 * (valid MIC, stale PN) is rejected HERE with a distinct code — never silently
	 * accepted. NULL rp = stateless verify (crypto only, no replay). */
	if (rp) {
		unsigned tid = rf_2a4m1_pn_replay_tid(in);
		if (!rf_2a4m1_pn_replay_ok(rp, tid, pn)) {
			rf_2a4m1_crypto_wipe(out, (size_t)r);
			return RF_2A4M1_CCMP_RX_E_REPLAY;
		}
		rf_2a4m1_pn_replay_commit(rp, tid, pn);
	}
	return r;                                  /* recovered body length */
}

static const uint32_t RF_2A4M1_P256_P[8]  = { 0xffffffffu, 0xffffffffu, 0xffffffffu, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000001u, 0xffffffffu };

static const uint32_t RF_2A4M1_P256_N[8]  = { 0xfc632551u, 0xf3b9cac2u, 0xa7179e84u, 0xbce6faadu, 0xffffffffu, 0xffffffffu, 0x00000000u, 0xffffffffu };

static const uint32_t RF_2A4M1_MONT_ONE[8]= { 0x00000001u, 0x00000000u, 0x00000000u, 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xfffffffeu, 0x00000000u };

static const uint32_t RF_2A4M1_MONT_R2[8] = { 0x00000003u, 0x00000000u, 0xffffffffu, 0xfffffffbu, 0xfffffffeu, 0xffffffffu, 0xfffffffdu, 0x00000004u };

static const uint32_t RF_2A4M1_MONT_B[8]  = { 0x29c4bddfu, 0xd89cdf62u, 0x78843090u, 0xacf005cdu, 0xf7212ed6u, 0xe5a220abu, 0x04874834u, 0xdc30061du };

static void rf_2a4m1_fe_set(rf_2a4m1_fe r, const uint32_t a[8]) { for (int i = 0; i < 8; i++) r[i] = a[i]; }

static void rf_2a4m1_fe_zero(rf_2a4m1_fe r) { for (int i = 0; i < 8; i++) r[i] = 0; }

static uint32_t rf_2a4m1_fe_is_zero(const rf_2a4m1_fe a) { uint32_t x = 0; for (int i = 0; i < 8; i++) x |= a[i]; return (uint32_t)(x == 0); }

static uint32_t rf_2a4m1_fe_eq(const rf_2a4m1_fe a, const rf_2a4m1_fe b) { uint32_t x = 0; for (int i = 0; i < 8; i++) x |= a[i] ^ b[i]; return (uint32_t)(x == 0); }

static uint32_t rf_2a4m1_u256_sub_borrow(uint32_t r[8], const uint32_t a[8], const uint32_t b[8])
{
	uint64_t br = 0;
	for (int i = 0; i < 8; i++) {
		uint64_t d = (uint64_t)a[i] - b[i] - br;
		r[i] = (uint32_t)d;
		br = (d >> 63) & 1;
	}
	return (uint32_t)br;
}

static int rf_2a4m1_u256_cmp(const uint32_t a[8], const uint32_t b[8])
{
	uint32_t s[8];
	uint32_t lt = rf_2a4m1_u256_sub_borrow(s, a, b);   /* a < b  => a-b borrows */
	uint32_t gt = rf_2a4m1_u256_sub_borrow(s, b, a);   /* a > b  => b-a borrows */
	return (int)gt - (int)lt;
}

static void rf_2a4m1_fe_cmov(rf_2a4m1_fe r, const rf_2a4m1_fe a, uint32_t flag)
{
	uint32_t mask = (uint32_t)0 - (flag & 1u);
	for (int i = 0; i < 8; i++)
		r[i] = (a[i] & mask) | (r[i] & ~mask);
}

static void rf_2a4m1_fe_cond_sub_mod(uint32_t r[8], const uint32_t t[8], uint32_t extra_carry,
                            const uint32_t mod[8])
{
	uint32_t s[8];
	uint32_t br = rf_2a4m1_u256_sub_borrow(s, t, mod);        /* t >= mod  <=>  br == 0 */
	uint32_t do_sub = (extra_carry & 1u) | (br ^ 1u);
	uint32_t mask = (uint32_t)0 - (do_sub & 1u);
	for (int i = 0; i < 8; i++)
		r[i] = (s[i] & mask) | (t[i] & ~mask);
}

static void rf_2a4m1_fe_add(rf_2a4m1_fe r, const rf_2a4m1_fe a, const rf_2a4m1_fe b)
{
	uint64_t c = 0;
	uint32_t t[8];
	for (int i = 0; i < 8; i++) {
		c += (uint64_t)a[i] + b[i];
		t[i] = (uint32_t)c;
		c >>= 32;
	}
	/* subtract p once if there was a carry out (>= 2^256) or t >= p — no branch */
	rf_2a4m1_fe_cond_sub_mod(r, t, (uint32_t)c, RF_2A4M1_P256_P);
}

static void rf_2a4m1_fe_sub(rf_2a4m1_fe r, const rf_2a4m1_fe a, const rf_2a4m1_fe b)
{
	uint32_t t[8];
	uint32_t borrow = rf_2a4m1_u256_sub_borrow(t, a, b);
	uint32_t s[8];
	uint64_t c = 0;
	for (int i = 0; i < 8; i++) {                 /* s = t + p (used only on underflow) */
		c += (uint64_t)t[i] + RF_2A4M1_P256_P[i];
		s[i] = (uint32_t)c;
		c >>= 32;
	}
	uint32_t mask = (uint32_t)0 - borrow;         /* underflow -> select t + p */
	for (int i = 0; i < 8; i++)
		r[i] = (s[i] & mask) | (t[i] & ~mask);
}

static void rf_2a4m1_fe_mul(rf_2a4m1_fe r, const rf_2a4m1_fe a, const rf_2a4m1_fe b)
{
	uint32_t t[10];
	for (int i = 0; i < 10; i++)
		t[i] = 0;

	for (int i = 0; i < 8; i++) {
		uint64_t c = 0;
		for (int j = 0; j < 8; j++) {
			uint64_t s = (uint64_t)t[j] + (uint64_t)a[j] * b[i] + c;
			t[j] = (uint32_t)s;
			c = s >> 32;
		}
		uint64_t s8 = (uint64_t)t[8] + c;
		t[8] = (uint32_t)s8;
		t[9] = (uint32_t)(s8 >> 32);

		uint32_t m = t[0];            /* * n0inv (=1) */
		uint64_t s0 = (uint64_t)t[0] + (uint64_t)m * RF_2A4M1_P256_P[0];
		c = s0 >> 32;
		for (int j = 1; j < 8; j++) {
			uint64_t s = (uint64_t)t[j] + (uint64_t)m * RF_2A4M1_P256_P[j] + c;
			t[j - 1] = (uint32_t)s;
			c = s >> 32;
		}
		uint64_t se = (uint64_t)t[8] + c;
		t[7] = (uint32_t)se;
		t[8] = t[9] + (uint32_t)(se >> 32);
	}

	/* result in t[0..8] (< 2p, so t[8] in {0,1}); reduce once — branchless. */
	rf_2a4m1_fe_cond_sub_mod(r, t, t[8], RF_2A4M1_P256_P);
}

static void rf_2a4m1_fe_sqr(rf_2a4m1_fe r, const rf_2a4m1_fe a) { rf_2a4m1_fe_mul(r, a, a); }

static void rf_2a4m1_fe_to_mont(rf_2a4m1_fe r, const rf_2a4m1_fe a) { rf_2a4m1_fe_mul(r, a, RF_2A4M1_MONT_R2); }

static void rf_2a4m1_fe_from_mont(rf_2a4m1_fe r, const rf_2a4m1_fe a)
{
	rf_2a4m1_fe one; rf_2a4m1_fe_zero(one); one[0] = 1;
	rf_2a4m1_fe_mul(r, a, one);
}

static void rf_2a4m1_fe_pow(rf_2a4m1_fe r, const rf_2a4m1_fe a, const uint32_t exp[8])
{
	rf_2a4m1_fe acc;
	rf_2a4m1_fe_set(acc, RF_2A4M1_MONT_ONE);
	for (int i = 255; i >= 0; i--) {
		rf_2a4m1_fe_sqr(acc, acc);
		if ((exp[i / 32] >> (i % 32)) & 1)
			rf_2a4m1_fe_mul(acc, acc, a);
	}
	rf_2a4m1_fe_set(r, acc);
}

static void rf_2a4m1_fe_inv(rf_2a4m1_fe r, const rf_2a4m1_fe a)
{
	static const uint32_t RF_2A4M1_P_MINUS_2[8] = {
		0xfffffffdu, 0xffffffffu, 0xffffffffu, 0x00000000u,
		0x00000000u, 0x00000000u, 0x00000001u, 0xffffffffu
	};
	rf_2a4m1_fe_pow(r, a, RF_2A4M1_P_MINUS_2);
}

static void rf_2a4m1_jac_set_inf(struct rf_2a4m1_jac *p) { rf_2a4m1_fe_zero(p->X); rf_2a4m1_fe_zero(p->Y); rf_2a4m1_fe_zero(p->Z); }

static bool rf_2a4m1_jac_is_inf(const struct rf_2a4m1_jac *p) { return rf_2a4m1_fe_is_zero(p->Z) != 0; }

static void rf_2a4m1_jac_cmov(struct rf_2a4m1_jac *r, const struct rf_2a4m1_jac *a, uint32_t flag)
{
	rf_2a4m1_fe_cmov(r->X, a->X, flag);
	rf_2a4m1_fe_cmov(r->Y, a->Y, flag);
	rf_2a4m1_fe_cmov(r->Z, a->Z, flag);
}

static void rf_2a4m1_jac_double(struct rf_2a4m1_jac *out, const struct rf_2a4m1_jac *p)
{
	rf_2a4m1_fe delta, gamma, beta, t1, t2, alpha, fourbeta, X3, Y3, Z3, tmp;
	rf_2a4m1_fe_sqr(delta, p->Z);
	rf_2a4m1_fe_sqr(gamma, p->Y);
	rf_2a4m1_fe_mul(beta, p->X, gamma);
	rf_2a4m1_fe_sub(t1, p->X, delta);
	rf_2a4m1_fe_add(t2, p->X, delta);
	rf_2a4m1_fe_mul(tmp, t1, t2);
	rf_2a4m1_fe_add(alpha, tmp, tmp);          /* 3*(X-δ)(X+δ) */
	rf_2a4m1_fe_add(alpha, alpha, tmp);
	rf_2a4m1_fe_add(t1, beta, beta);           /* 2β */
	rf_2a4m1_fe_add(fourbeta, t1, t1);         /* 4β */
	/* X3 = alpha^2 - 8*beta */
	rf_2a4m1_fe_sqr(X3, alpha);
	rf_2a4m1_fe_add(tmp, fourbeta, fourbeta);  /* 8β */
	rf_2a4m1_fe_sub(X3, X3, tmp);
	/* Z3 = (Y+Z)^2 - gamma - delta */
	rf_2a4m1_fe_add(tmp, p->Y, p->Z);
	rf_2a4m1_fe_sqr(Z3, tmp);
	rf_2a4m1_fe_sub(Z3, Z3, gamma);
	rf_2a4m1_fe_sub(Z3, Z3, delta);
	/* Y3 = alpha*(4*beta - X3) - 8*gamma^2 */
	rf_2a4m1_fe_sub(t2, fourbeta, X3);
	rf_2a4m1_fe_mul(Y3, alpha, t2);
	rf_2a4m1_fe_sqr(tmp, gamma);               /* γ^2 */
	rf_2a4m1_fe_add(tmp, tmp, tmp);            /* 2 */
	rf_2a4m1_fe_add(tmp, tmp, tmp);            /* 4 */
	rf_2a4m1_fe_add(tmp, tmp, tmp);            /* 8γ^2 */
	rf_2a4m1_fe_sub(Y3, Y3, tmp);
	rf_2a4m1_fe_set(out->X, X3); rf_2a4m1_fe_set(out->Y, Y3); rf_2a4m1_fe_set(out->Z, Z3);
}

static void rf_2a4m1_jac_add(struct rf_2a4m1_jac *out, const struct rf_2a4m1_jac *a, const struct rf_2a4m1_jac *b)
{
	rf_2a4m1_fe Z1Z1, Z2Z2, U1, U2, S1, S2, H, r, tmp, tmp2;
	rf_2a4m1_fe_sqr(Z1Z1, a->Z);
	rf_2a4m1_fe_sqr(Z2Z2, b->Z);
	rf_2a4m1_fe_mul(U1, a->X, Z2Z2);
	rf_2a4m1_fe_mul(U2, b->X, Z1Z1);
	rf_2a4m1_fe_mul(tmp, a->Y, b->Z);
	rf_2a4m1_fe_mul(S1, tmp, Z2Z2);
	rf_2a4m1_fe_mul(tmp, b->Y, a->Z);
	rf_2a4m1_fe_mul(S2, tmp, Z1Z1);

	/* generic add-2007-bl (valid when a,b finite and a != ±b) */
	rf_2a4m1_fe I, J, V, Xg, Yg, Zg;
	rf_2a4m1_fe_sub(H, U2, U1);
	rf_2a4m1_fe_add(tmp, H, H);                 /* 2H */
	rf_2a4m1_fe_sqr(I, tmp);                    /* I = (2H)^2 */
	rf_2a4m1_fe_mul(J, H, I);
	rf_2a4m1_fe_sub(tmp, S2, S1);
	rf_2a4m1_fe_add(r, tmp, tmp);               /* r = 2(S2-S1) */
	rf_2a4m1_fe_mul(V, U1, I);
	rf_2a4m1_fe_sqr(Xg, r);                     /* X3 = r^2 - J - 2V */
	rf_2a4m1_fe_sub(Xg, Xg, J);
	rf_2a4m1_fe_add(tmp, V, V);
	rf_2a4m1_fe_sub(Xg, Xg, tmp);
	rf_2a4m1_fe_sub(tmp, V, Xg);                /* Y3 = r*(V - X3) - 2*S1*J */
	rf_2a4m1_fe_mul(Yg, r, tmp);
	rf_2a4m1_fe_mul(tmp, S1, J);
	rf_2a4m1_fe_add(tmp, tmp, tmp);
	rf_2a4m1_fe_sub(Yg, Yg, tmp);
	rf_2a4m1_fe_add(tmp, a->Z, b->Z);           /* Z3 = ((Z1+Z2)^2 - Z1Z1 - Z2Z2) * H */
	rf_2a4m1_fe_sqr(tmp2, tmp);
	rf_2a4m1_fe_sub(tmp2, tmp2, Z1Z1);
	rf_2a4m1_fe_sub(tmp2, tmp2, Z2Z2);
	rf_2a4m1_fe_mul(Zg, tmp2, H);

	/* the doubling result (used when a == b) */
	struct rf_2a4m1_jac D;
	rf_2a4m1_jac_double(&D, a);

	/* constant-time predicates (fe_is_zero / fe_eq are branchless, 0/1) */
	uint32_t ia  = rf_2a4m1_fe_is_zero(a->Z);
	uint32_t ib  = rf_2a4m1_fe_is_zero(b->Z);
	uint32_t fin = (1u - ia) & (1u - ib);
	uint32_t xeq = rf_2a4m1_fe_eq(U1, U2);
	uint32_t yeq = rf_2a4m1_fe_eq(S1, S2);
	uint32_t is_dbl = fin & xeq & yeq;          /* a == b  -> use D */
	uint32_t is_neg = fin & xeq & (1u - yeq);   /* a == -b -> infinity */

	struct rf_2a4m1_jac res;
	rf_2a4m1_fe_set(res.X, Xg); rf_2a4m1_fe_set(res.Y, Yg); rf_2a4m1_fe_set(res.Z, Zg);   /* default: generic */
	rf_2a4m1_jac_cmov(&res, &D, is_dbl);
	struct rf_2a4m1_jac inf; rf_2a4m1_jac_set_inf(&inf);
	rf_2a4m1_jac_cmov(&res, &inf, is_neg);
	rf_2a4m1_jac_cmov(&res, a, ib);                       /* b at infinity -> a */
	rf_2a4m1_jac_cmov(&res, b, ia);                        /* a at infinity -> b (priority) */
	*out = res;
}

static void rf_2a4m1_jac_mul(struct rf_2a4m1_jac *R, const uint8_t k[32], const struct rf_2a4m1_jac *P)
{
	struct rf_2a4m1_jac acc, t;
	rf_2a4m1_jac_set_inf(&acc);
	for (int i = 0; i < 256; i++) {
		rf_2a4m1_jac_double(&acc, &acc);
		rf_2a4m1_jac_add(&t, &acc, P);
		uint32_t bit = (uint32_t)((k[i / 8] >> (7 - (i % 8))) & 1);
		rf_2a4m1_jac_cmov(&acc, &t, bit);
	}
	*R = acc;
}

static bool rf_2a4m1_jac_to_affine(rf_2a4m1_fe x, rf_2a4m1_fe y, const struct rf_2a4m1_jac *p)
{
	if (rf_2a4m1_jac_is_inf(p))
		return false;
	rf_2a4m1_fe zinv, zinv2, zinv3;
	rf_2a4m1_fe_inv(zinv, p->Z);
	rf_2a4m1_fe_sqr(zinv2, zinv);
	rf_2a4m1_fe_mul(zinv3, zinv2, zinv);
	rf_2a4m1_fe_mul(x, p->X, zinv2);
	rf_2a4m1_fe_mul(y, p->Y, zinv3);
	return true;
}

static void rf_2a4m1_be_to_fe(rf_2a4m1_fe r, const uint8_t b[32])
{
	for (int i = 0; i < 8; i++) {
		const uint8_t *q = &b[(7 - i) * 4];
		r[i] = ((uint32_t)q[0] << 24) | ((uint32_t)q[1] << 16) |
		       ((uint32_t)q[2] << 8) | (uint32_t)q[3];
	}
}

static void rf_2a4m1_fe_to_be(uint8_t b[32], const rf_2a4m1_fe a)
{
	for (int i = 0; i < 8; i++) {
		uint8_t *q = &b[(7 - i) * 4];
		q[0] = (uint8_t)(a[i] >> 24); q[1] = (uint8_t)(a[i] >> 16);
		q[2] = (uint8_t)(a[i] >> 8);  q[3] = (uint8_t)a[i];
	}
}

static bool rf_2a4m1_scalar_in_range(const uint8_t k[32])
{
	rf_2a4m1_fe s;
	rf_2a4m1_be_to_fe(s, k);
	if (rf_2a4m1_fe_is_zero(s))
		return false;
	return rf_2a4m1_u256_cmp(s, RF_2A4M1_P256_N) < 0;
}

static bool rf_2a4m1_point_on_curve(const rf_2a4m1_fe xm, const rf_2a4m1_fe ym)
{
	rf_2a4m1_fe y2, x3, t, rhs, three_x;
	rf_2a4m1_fe_sqr(y2, ym);
	rf_2a4m1_fe_sqr(x3, xm);
	rf_2a4m1_fe_mul(x3, x3, xm);
	rf_2a4m1_fe_add(three_x, xm, xm);
	rf_2a4m1_fe_add(three_x, three_x, xm);     /* 3x */
	rf_2a4m1_fe_sub(t, x3, three_x);           /* x^3 - 3x */
	rf_2a4m1_fe_add(rhs, t, RF_2A4M1_MONT_B);           /* + b */
	return rf_2a4m1_fe_eq(y2, rhs);
}

bool rf_2a4m1_p256_ecdh(const uint8_t priv[RF_2A4M1_P256_SCALAR_LEN], const uint8_t peer_pub[RF_2A4M1_P256_POINT_LEN],
               uint8_t dhkey[RF_2A4M1_P256_COORD_LEN])
{
	if (!rf_2a4m1_scalar_in_range(priv))
		return false;
	rf_2a4m1_fe px, py, pxm, pym;
	rf_2a4m1_be_to_fe(px, peer_pub);
	rf_2a4m1_be_to_fe(py, peer_pub + 32);
	/* coordinates must be canonical (< p) and the point on the curve, non-zero */
	if (rf_2a4m1_u256_cmp(px, RF_2A4M1_P256_P) >= 0 || rf_2a4m1_u256_cmp(py, RF_2A4M1_P256_P) >= 0)
		return false;
	if (rf_2a4m1_fe_is_zero(px) && rf_2a4m1_fe_is_zero(py))
		return false;
	rf_2a4m1_fe_to_mont(pxm, px);
	rf_2a4m1_fe_to_mont(pym, py);
	if (!rf_2a4m1_point_on_curve(pxm, pym))
		return false;
	struct rf_2a4m1_jac Q, R;
	rf_2a4m1_fe_set(Q.X, pxm); rf_2a4m1_fe_set(Q.Y, pym); rf_2a4m1_fe_set(Q.Z, RF_2A4M1_MONT_ONE);
	rf_2a4m1_jac_mul(&R, priv, &Q);
	rf_2a4m1_fe xm, x;
	rf_2a4m1_fe ym_unused;
	if (!rf_2a4m1_jac_to_affine(xm, ym_unused, &R))
		return false;                 /* shared point at infinity — invalid */
	rf_2a4m1_fe_from_mont(x, xm);
	rf_2a4m1_fe_to_be(dhkey, x);
	return true;
}

static const uint32_t RF_2A4M1_EXP_P_MINUS_1_HALF[8] = {
	0xffffffffu, 0xffffffffu, 0x7fffffffu, 0x00000000u,
	0x00000000u, 0x80000000u, 0x80000000u, 0x7fffffffu
};

static const uint32_t RF_2A4M1_EXP_P_PLUS_1_QUARTER[8] = {
	0x00000000u, 0x00000000u, 0x40000000u, 0x00000000u,
	0x00000000u, 0x40000000u, 0xc0000000u, 0x3fffffffu
};

static bool rf_2a4m1_fe_is_square(const rf_2a4m1_fe a)
{
	if (rf_2a4m1_fe_is_zero(a))
		return true;
	rf_2a4m1_fe t;
	rf_2a4m1_fe_pow(t, a, RF_2A4M1_EXP_P_MINUS_1_HALF);
	return rf_2a4m1_fe_eq(t, RF_2A4M1_MONT_ONE);
}

static void rf_2a4m1_fe_sqrt(rf_2a4m1_fe r, const rf_2a4m1_fe a) { rf_2a4m1_fe_pow(r, a, RF_2A4M1_EXP_P_PLUS_1_QUARTER); }

static int rf_2a4m1_fe_sgn0(const rf_2a4m1_fe a)
{
	rf_2a4m1_fe n;
	rf_2a4m1_fe_from_mont(n, a);
	return (int)(n[0] & 1u);
}

static bool rf_2a4m1_load_point(struct rf_2a4m1_jac *P, const uint8_t pt[64])
{
	rf_2a4m1_fe x, y;
	rf_2a4m1_be_to_fe(x, pt);
	rf_2a4m1_be_to_fe(y, pt + 32);
	if (rf_2a4m1_u256_cmp(x, RF_2A4M1_P256_P) >= 0 || rf_2a4m1_u256_cmp(y, RF_2A4M1_P256_P) >= 0)
		return false;
	rf_2a4m1_fe_to_mont(P->X, x);
	rf_2a4m1_fe_to_mont(P->Y, y);
	rf_2a4m1_fe_set(P->Z, RF_2A4M1_MONT_ONE);
	return true;
}

static bool rf_2a4m1_store_point(uint8_t out[64], const struct rf_2a4m1_jac *P)
{
	rf_2a4m1_fe xm, ym, x, y;
	if (!rf_2a4m1_jac_to_affine(xm, ym, P))
		return false;
	rf_2a4m1_fe_from_mont(x, xm);
	rf_2a4m1_fe_from_mont(y, ym);
	rf_2a4m1_fe_to_be(out, x);
	rf_2a4m1_fe_to_be(out + 32, y);
	return true;
}

bool rf_2a4m1_p256_scalar_valid(const uint8_t s[32]) { return rf_2a4m1_scalar_in_range(s); }

bool rf_2a4m1_p256_point_valid(const uint8_t pt[64])
{
	rf_2a4m1_fe x, y, xm, ym;
	rf_2a4m1_be_to_fe(x, pt);
	rf_2a4m1_be_to_fe(y, pt + 32);
	if (rf_2a4m1_u256_cmp(x, RF_2A4M1_P256_P) >= 0 || rf_2a4m1_u256_cmp(y, RF_2A4M1_P256_P) >= 0)
		return false;
	if (rf_2a4m1_fe_is_zero(x) && rf_2a4m1_fe_is_zero(y))
		return false;
	rf_2a4m1_fe_to_mont(xm, x);
	rf_2a4m1_fe_to_mont(ym, y);
	return rf_2a4m1_point_on_curve(xm, ym);
}

void rf_2a4m1_p256_point_negate(const uint8_t in[64], uint8_t out[64])
{
	rf_2a4m1_fe y, zero, ny;
	rf_2a4m1_be_to_fe(y, in + 32);
	rf_2a4m1_fe_zero(zero);
	rf_2a4m1_fe_sub(ny, zero, y);              /* (p - y) mod p ; 0 -> 0 */
	memcpy(out, in, 32);
	rf_2a4m1_fe_to_be(out + 32, ny);
}

bool rf_2a4m1_p256_point_mul(const uint8_t scalar[32], const uint8_t pt[64], uint8_t out[64])
{
	if (!rf_2a4m1_p256_point_valid(pt))
		return false;
	struct rf_2a4m1_jac P, R;
	if (!rf_2a4m1_load_point(&P, pt))
		return false;
	rf_2a4m1_jac_mul(&R, scalar, &P);
	return rf_2a4m1_store_point(out, &R);
}

bool rf_2a4m1_p256_point_add(const uint8_t a[64], const uint8_t b[64], uint8_t out[64])
{
	if (!rf_2a4m1_p256_point_valid(a) || !rf_2a4m1_p256_point_valid(b))
		return false;
	struct rf_2a4m1_jac A, B, R;
	if (!rf_2a4m1_load_point(&A, a) || !rf_2a4m1_load_point(&B, b))
		return false;
	rf_2a4m1_jac_add(&R, &A, &B);
	return rf_2a4m1_store_point(out, &R);
}

void rf_2a4m1_p256_scalar_reduce_mod_nm1_plus1(const uint8_t in[32], uint8_t out[32])
{
	static const uint32_t RF_2A4M1_N_MINUS_1[8] = {
		0xfc632550u, 0xf3b9cac2u, 0xa7179e84u, 0xbce6faadu,
		0xffffffffu, 0xffffffffu, 0x00000000u, 0xffffffffu
	};
	rf_2a4m1_fe v;
	rf_2a4m1_be_to_fe(v, in);
	rf_2a4m1_fe_cond_sub_mod(v, v, 0, RF_2A4M1_N_MINUS_1);        /* v mod (n-1): one branchless sub */
	uint64_t c = 1;                             /* + 1 : v was <= n-2, so v+1 <= n-1 */
	for (int i = 0; i < 8; i++) {
		c += v[i];
		v[i] = (uint32_t)c;
		c >>= 32;
	}
	rf_2a4m1_fe_to_be(out, v);
}

void rf_2a4m1_p256_scalar_add_n(const uint8_t a[32], const uint8_t b[32], uint8_t out[32])
{
	/* be_to_fe / fe_to_be are pure byte packing (mod-independent), so reuse them. */
	rf_2a4m1_fe x, y, t;
	rf_2a4m1_be_to_fe(x, a);
	rf_2a4m1_be_to_fe(y, b);
	uint64_t c = 0;
	for (int i = 0; i < 8; i++) {
		c += (uint64_t)x[i] + y[i];
		t[i] = (uint32_t)c;
		c >>= 32;
	}
	/* a,b < n -> sum < 2n: one branchless conditional subtraction of n */
	rf_2a4m1_fe_cond_sub_mod(t, t, (uint32_t)c, RF_2A4M1_P256_N);
	rf_2a4m1_fe_to_be(out, t);
}

bool rf_2a4m1_p256_sswu(const uint8_t u_be[32], uint8_t out[64])
{
	rf_2a4m1_fe u, um, A, Z, zero, three, ten;
	rf_2a4m1_be_to_fe(u, u_be);
	if (rf_2a4m1_u256_cmp(u, RF_2A4M1_P256_P) >= 0)
		return false;
	rf_2a4m1_fe_to_mont(um, u);
	rf_2a4m1_fe_zero(zero);
	rf_2a4m1_fe_add(three, RF_2A4M1_MONT_ONE, RF_2A4M1_MONT_ONE); rf_2a4m1_fe_add(three, three, RF_2A4M1_MONT_ONE);   /* 3 */
	rf_2a4m1_fe_sub(A, zero, three);                                              /* A = -3 */
	rf_2a4m1_fe_add(ten, three, three); rf_2a4m1_fe_add(ten, ten, three); rf_2a4m1_fe_add(ten, ten, RF_2A4M1_MONT_ONE); /* 10 */
	rf_2a4m1_fe_sub(Z, zero, ten);                                               /* Z = -10 */

	rf_2a4m1_fe u2, u4, Zu2, Z2u4, denom, tv1;
	rf_2a4m1_fe_sqr(u2, um);
	rf_2a4m1_fe_sqr(u4, u2);
	rf_2a4m1_fe_mul(Zu2, Z, u2);
	rf_2a4m1_fe_mul(Z2u4, Z, Z); rf_2a4m1_fe_mul(Z2u4, Z2u4, u4);
	rf_2a4m1_fe_add(denom, Z2u4, Zu2);
	rf_2a4m1_fe_inv(tv1, denom);                 /* inv0: 0 -> 0 */

	rf_2a4m1_fe x1;
	if (rf_2a4m1_fe_is_zero(tv1)) {              /* denom == 0 -> x1 = B / (Z*A) */
		rf_2a4m1_fe za, iza;
		rf_2a4m1_fe_mul(za, Z, A);
		rf_2a4m1_fe_inv(iza, za);
		rf_2a4m1_fe_mul(x1, RF_2A4M1_MONT_B, iza);
	} else {
		rf_2a4m1_fe negB, iA, nBiA, onep;
		rf_2a4m1_fe_sub(negB, zero, RF_2A4M1_MONT_B);
		rf_2a4m1_fe_inv(iA, A);
		rf_2a4m1_fe_mul(nBiA, negB, iA);         /* -B/A */
		rf_2a4m1_fe_add(onep, RF_2A4M1_MONT_ONE, tv1);    /* 1 + tv1 */
		rf_2a4m1_fe_mul(x1, nBiA, onep);
	}

	rf_2a4m1_fe gx1, x1sq, x1cu, Ax1;
	rf_2a4m1_fe_sqr(x1sq, x1); rf_2a4m1_fe_mul(x1cu, x1sq, x1);
	rf_2a4m1_fe_mul(Ax1, A, x1);
	rf_2a4m1_fe_add(gx1, x1cu, Ax1); rf_2a4m1_fe_add(gx1, gx1, RF_2A4M1_MONT_B);

	rf_2a4m1_fe x2, gx2, x2sq, x2cu, Ax2;
	rf_2a4m1_fe_mul(x2, Zu2, x1);
	rf_2a4m1_fe_sqr(x2sq, x2); rf_2a4m1_fe_mul(x2cu, x2sq, x2);
	rf_2a4m1_fe_mul(Ax2, A, x2);
	rf_2a4m1_fe_add(gx2, x2cu, Ax2); rf_2a4m1_fe_add(gx2, gx2, RF_2A4M1_MONT_B);

	rf_2a4m1_fe x, y;
	if (rf_2a4m1_fe_is_square(gx1)) {
		rf_2a4m1_fe_set(x, x1);
		rf_2a4m1_fe_sqrt(y, gx1);
	} else {
		rf_2a4m1_fe_set(x, x2);
		rf_2a4m1_fe_sqrt(y, gx2);
	}
	if ((int)(u[0] & 1u) != rf_2a4m1_fe_sgn0(y)) {   /* sgn0(u): NORMAL-domain u LSB */
		rf_2a4m1_fe ny;
		rf_2a4m1_fe_sub(ny, zero, y);
		rf_2a4m1_fe_set(y, ny);
	}
	rf_2a4m1_fe xn, yn;
	rf_2a4m1_fe_from_mont(xn, x);
	rf_2a4m1_fe_from_mont(yn, y);
	rf_2a4m1_fe_to_be(out, xn);
	rf_2a4m1_fe_to_be(out + 32, yn);
	return true;
}

bool rf_2a4m1_p256_point_from_x(const uint8_t x_be[32], uint8_t out[64])
{
	rf_2a4m1_fe x, xm, x3, three_x, t, rhs, y;
	rf_2a4m1_be_to_fe(x, x_be);
	if (rf_2a4m1_u256_cmp(x, RF_2A4M1_P256_P) >= 0)
		return false;
	rf_2a4m1_fe_to_mont(xm, x);
	rf_2a4m1_fe_sqr(x3, xm); rf_2a4m1_fe_mul(x3, x3, xm);                       /* x^3          */
	rf_2a4m1_fe_add(three_x, xm, xm); rf_2a4m1_fe_add(three_x, three_x, xm);    /* 3x           */
	rf_2a4m1_fe_sub(t, x3, three_x);                                   /* x^3 - 3x     */
	rf_2a4m1_fe_add(rhs, t, RF_2A4M1_MONT_B);                                   /* x^3 - 3x + b */
	if (!rf_2a4m1_fe_is_square(rhs))
		return false;
	rf_2a4m1_fe_sqrt(y, rhs);
	if (rf_2a4m1_fe_sgn0(y) != 0) {                                    /* select the even root */
		rf_2a4m1_fe zero, ny;
		rf_2a4m1_fe_zero(zero);
		rf_2a4m1_fe_sub(ny, zero, y);
		rf_2a4m1_fe_set(y, ny);
	}
	rf_2a4m1_fe xn, yn;
	rf_2a4m1_fe_from_mont(xn, xm);
	rf_2a4m1_fe_from_mont(yn, y);
	rf_2a4m1_fe_to_be(out, xn);
	rf_2a4m1_fe_to_be(out + 32, yn);
	return true;
}

static void rf_2a4m1_fe_reduce_once(rf_2a4m1_fe r, const rf_2a4m1_fe a)
{
	rf_2a4m1_fe_cond_sub_mod(r, a, 0, RF_2A4M1_P256_P);
}

bool rf_2a4m1_p256_reduce_mod_p(const uint8_t *in, size_t in_len, uint8_t out[32])
{
	if (in_len == 0 || in_len > 48)
		return false;
	uint8_t pad[32];
	rf_2a4m1_fe lo, red;
	if (in_len <= 32) {
		memset(pad, 0, 32);
		memcpy(pad + (32 - in_len), in, in_len);   /* right-align big-endian */
		rf_2a4m1_be_to_fe(lo, pad);
		rf_2a4m1_fe_reduce_once(red, lo);
		rf_2a4m1_fe_to_be(out, red);
		return true;
	}
	size_t hlen = in_len - 32;                     /* 1..16: hi part < 2^128 < p */
	uint8_t hpad[32];
	rf_2a4m1_fe hi, hiR, res;
	memset(hpad, 0, 32);
	memcpy(hpad + (32 - hlen), in, hlen);
	memcpy(pad, in + hlen, 32);
	rf_2a4m1_be_to_fe(hi, hpad);
	rf_2a4m1_be_to_fe(lo, pad);
	rf_2a4m1_fe_reduce_once(red, lo);
	rf_2a4m1_fe_to_mont(hiR, hi);                            /* hi * 2^256 mod p */
	rf_2a4m1_fe_add(res, hiR, red);
	rf_2a4m1_fe_to_be(out, res);
	return true;
}

bool rf_2a4m1_owe_derive(const uint8_t priv[RF_2A4M1_P256_SCALAR_LEN],
                const uint8_t own_pub[RF_2A4M1_P256_POINT_LEN], const uint8_t peer_pub[RF_2A4M1_P256_POINT_LEN],
                bool own_is_client, uint8_t pmk[RF_2A4M1_SHA256_DIGEST_LEN], uint8_t pmkid[16])
{
	uint8_t z[RF_2A4M1_P256_COORD_LEN];
	if (!rf_2a4m1_p256_ecdh(priv, peer_pub, z))
		return false;

	/* C = client(STA) public key, A = AP public key, in that order. On the wire (RFC
	 * 8110 / hostapd's crypto_ecdh_get_pubkey(ecdh,0)) an ECC OWE public value is the
	 * 32-byte X-COORDINATE only — so the salt and the PMKID hash key on C.x and A.x,
	 * the leading P256_COORD_LEN octets of each uncompressed X||Y point here. Keying on
	 * X alone is exact: the peer reconstructs ±point from X, and both the ECDH shared
	 * secret and these X-only inputs are invariant under the Y sign. */
	const uint8_t *C = own_is_client ? own_pub : peer_pub;
	const uint8_t *A = own_is_client ? peer_pub : own_pub;

	/* salt = C.x | A.x | group(19, little-endian 2 octets) — verified byte-exact against stock
	 * wpa_supplicant's PRK (offline HMAC over its captured z: HMAC(key=C.x|A.x|group_LE, msg=z)). */
	uint8_t salt[2 * RF_2A4M1_P256_COORD_LEN + 2];
	memcpy(salt, C, RF_2A4M1_P256_COORD_LEN);
	memcpy(salt + RF_2A4M1_P256_COORD_LEN, A, RF_2A4M1_P256_COORD_LEN);
	salt[2 * RF_2A4M1_P256_COORD_LEN]     = 19;    /* group 19, LE16 */
	salt[2 * RF_2A4M1_P256_COORD_LEN + 1] = 0;

	uint8_t prk[RF_2A4M1_SHA256_DIGEST_LEN];
	rf_2a4m1_hkdf_sha256_extract(salt, sizeof salt, z, RF_2A4M1_P256_COORD_LEN, prk);
	rf_2a4m1_hkdf_sha256_expand(prk, RF_2A4M1_SHA256_DIGEST_LEN,
	                   (const uint8_t *)"OWE Key Generation", 18, pmk, RF_2A4M1_SHA256_DIGEST_LEN);

	/* PMKID = Truncate-128(SHA-256(C.x | A.x)) */
	uint8_t hbuf[2 * RF_2A4M1_P256_COORD_LEN], hash[RF_2A4M1_SHA256_DIGEST_LEN];
	memcpy(hbuf, C, RF_2A4M1_P256_COORD_LEN);
	memcpy(hbuf + RF_2A4M1_P256_COORD_LEN, A, RF_2A4M1_P256_COORD_LEN);
	rf_2a4m1_sha256(hbuf, sizeof hbuf, hash);
	memcpy(pmkid, hash, 16);
	/* F12: wipe the ECDH shared secret + HKDF PRK (PMK already delivered to caller). */
	rf_2a4m1_crypto_wipe(z, sizeof z);
	rf_2a4m1_crypto_wipe(prk, sizeof prk);
	return true;
}

static const uint8_t RF_2A4M1_SAE_KEYSEED_SALT[RF_2A4M1_SAE_KEY_LEN] = { 0 };

bool rf_2a4m1_sae_derive_pt_h2e(uint8_t pt[RF_2A4M1_P256_POINT_LEN],
                       const uint8_t *ssid, size_t ssid_len,
                       const uint8_t *password, size_t password_len,
                       const uint8_t *identifier, size_t identifier_len)
{
	/* pwd-seed = HKDF-Extract(salt = SSID, IKM = password [|| identifier]) */
	uint8_t ikm[256];
	if (password_len > sizeof ikm || identifier_len > sizeof ikm - password_len)
		return false;   /* F11: overflow-safe bound (no size_t wrap) */
	memcpy(ikm, password, password_len);
	if (identifier_len)
		memcpy(ikm + password_len, identifier, identifier_len);
	uint8_t pwd_seed[RF_2A4M1_SHA256_DIGEST_LEN];
	rf_2a4m1_hkdf_sha256_extract(ssid, ssid_len, ikm, password_len + identifier_len, pwd_seed);
	rf_2a4m1_crypto_wipe(ikm, sizeof ikm);   /* F12: raw password material consumed */

	/* For j in {1,2}: pwd-value = HKDF-Expand(pwd-seed, "SAE Hash to Element uj Pj", L),
	 * L = ceil((ceil(log2 p) + 128)/8) = 48 for P-256; u = OS2IP(pwd-value) mod p;
	 * Pj = SSWU(u). PT = P1 + P2 (the MAC-independent password token). */
	static const char *rf_2a4m1_info[2] = {
		"SAE Hash to Element u1 P1", "SAE Hash to Element u2 P2"
	};
	uint8_t P1[RF_2A4M1_P256_POINT_LEN], P2[RF_2A4M1_P256_POINT_LEN];
	uint8_t *Ppt[2] = { P1, P2 };
	bool ok = true;
	for (int j = 0; j < 2; j++) {
		uint8_t pwd_value[48], u[RF_2A4M1_P256_COORD_LEN];
		rf_2a4m1_hkdf_sha256_expand(pwd_seed, RF_2A4M1_SHA256_DIGEST_LEN,
		                   (const uint8_t *)rf_2a4m1_info[j], 25, pwd_value, sizeof pwd_value);
		if (!rf_2a4m1_p256_reduce_mod_p(pwd_value, sizeof pwd_value, u) || !rf_2a4m1_p256_sswu(u, Ppt[j]))
			ok = false;
		rf_2a4m1_crypto_wipe(pwd_value, sizeof pwd_value);   /* F12: password-derived transients */
		rf_2a4m1_crypto_wipe(u, sizeof u);
		if (!ok)
			break;
	}
	rf_2a4m1_crypto_wipe(pwd_seed, sizeof pwd_seed);         /* F12: pwd-seed consumed */
	if (!ok)
		return false;
	return rf_2a4m1_p256_point_add(P1, P2, pt);
}

bool rf_2a4m1_sae_derive_pwe_from_pt(uint8_t pwe[RF_2A4M1_P256_POINT_LEN], const uint8_t pt[RF_2A4M1_P256_POINT_LEN],
                            const rf_2a4m1_mac_addr *mac_a, const rf_2a4m1_mac_addr *mac_b)
{
	if (!rf_2a4m1_p256_point_valid(pt))
		return false;
	/* val = H(<0>, MAX(A,B) || MIN(A,B)); the two 6-octet MACs ordered by value.
	 * HMAC-SHA256 with an all-zero key == HKDF-Extract(salt=0). (802.11 §12.4.4.3.3) */
	int a_ge_b = 1;                       /* MAX==A unless A < B */
	for (int i = 0; i < RF_2A4M1_ETH_ALEN; i++) {
		if (mac_a->a[i] != mac_b->a[i]) { a_ge_b = mac_a->a[i] > mac_b->a[i]; break; }
	}
	const rf_2a4m1_mac_addr *hi = a_ge_b ? mac_a : mac_b;
	const rf_2a4m1_mac_addr *lo = a_ge_b ? mac_b : mac_a;
	uint8_t maxmin[2 * RF_2A4M1_ETH_ALEN];
	memcpy(maxmin, hi->a, RF_2A4M1_ETH_ALEN);
	memcpy(maxmin + RF_2A4M1_ETH_ALEN, lo->a, RF_2A4M1_ETH_ALEN);
	uint8_t val[RF_2A4M1_SHA256_DIGEST_LEN];
	rf_2a4m1_hmac_sha256(RF_2A4M1_SAE_KEYSEED_SALT, sizeof RF_2A4M1_SAE_KEYSEED_SALT, maxmin, sizeof maxmin, val);

	/* val = (val mod (n-1)) + 1  ->  a scalar in [1, n-1]; PWE = val * PT. */
	uint8_t k[RF_2A4M1_SAE_KEY_LEN];
	rf_2a4m1_p256_scalar_reduce_mod_nm1_plus1(val, k);
	bool ok = rf_2a4m1_p256_point_mul(k, pt, pwe);
	rf_2a4m1_crypto_wipe(k, sizeof k);   /* F12: PWE-derivation scalar consumed */
	return ok;
}

bool rf_2a4m1_sae_commit_pwe(struct rf_2a4m1_sae *s, const uint8_t pwe[RF_2A4M1_P256_POINT_LEN],
                    const uint8_t rnd[RF_2A4M1_SAE_KEY_LEN], const uint8_t mask[RF_2A4M1_SAE_KEY_LEN])
{
	if (!rf_2a4m1_p256_point_valid(pwe) || !rf_2a4m1_p256_scalar_valid(rnd) || !rf_2a4m1_p256_scalar_valid(mask))
		return false;
	memcpy(s->pwe, pwe, RF_2A4M1_P256_POINT_LEN);
	memcpy(s->rnd, rnd, RF_2A4M1_SAE_KEY_LEN);
	/* commit-scalar = (rand + mask) mod n */
	rf_2a4m1_p256_scalar_add_n(rnd, mask, s->commit_scalar);
	if (!rf_2a4m1_p256_scalar_valid(s->commit_scalar))     /* reject the degenerate 0 */
		return false;
	{   /* F10: RFC 7664 §5.3 — commit-scalar must be > 1 */
		uint8_t one[RF_2A4M1_SAE_KEY_LEN] = { 0 };
		one[RF_2A4M1_SAE_KEY_LEN - 1] = 1;
		if (rf_2a4m1_crypto_ct_eq(s->commit_scalar, one, RF_2A4M1_SAE_KEY_LEN))
			return false;
	}
	/* COMMIT-ELEMENT = inverse(mask * PWE) = -(mask * PWE). `pwe` is caller-owned
	 * (H2E derives a local one in sae_commit; H&P passes its buffer) — not wiped here. */
	uint8_t me[RF_2A4M1_P256_POINT_LEN];
	if (!rf_2a4m1_p256_point_mul(mask, pwe, me))
		return false;
	rf_2a4m1_p256_point_negate(me, s->commit_element);
	rf_2a4m1_crypto_wipe(me, sizeof me);     /* F12: mask*PWE intermediate consumed */
	s->have_keys = false;
	return true;
}

bool rf_2a4m1_sae_commit(struct rf_2a4m1_sae *s, const uint8_t pt[RF_2A4M1_P256_POINT_LEN],
                const rf_2a4m1_mac_addr *mac_a, const rf_2a4m1_mac_addr *mac_b,
                const uint8_t rnd[RF_2A4M1_SAE_KEY_LEN], const uint8_t mask[RF_2A4M1_SAE_KEY_LEN])
{
	uint8_t pwe[RF_2A4M1_P256_POINT_LEN];
	if (!rf_2a4m1_sae_derive_pwe_from_pt(pwe, pt, mac_a, mac_b))
		return false;
	bool ok = rf_2a4m1_sae_commit_pwe(s, pwe, rnd, mask);
	rf_2a4m1_crypto_wipe(pwe, sizeof pwe);   /* F12: local H2E PWE copy (s->pwe retains it) */
	return ok;
}

bool rf_2a4m1_sae_process_commit(struct rf_2a4m1_sae *s, const uint8_t peer_scalar[RF_2A4M1_SAE_KEY_LEN],
                        const uint8_t peer_element[RF_2A4M1_P256_POINT_LEN])
{
	if (!rf_2a4m1_p256_scalar_valid(peer_scalar) || !rf_2a4m1_p256_point_valid(peer_element))
		return false;
	/* F5: reflection — reject a peer commit identical to our own (802.11 §12.4.5.4) */
	if (rf_2a4m1_crypto_ct_eq(peer_scalar, s->commit_scalar, RF_2A4M1_SAE_KEY_LEN) &&
	    rf_2a4m1_crypto_ct_eq(peer_element, s->commit_element, RF_2A4M1_P256_POINT_LEN))
		return false;
	/* K = rand * (peer-scalar * PWE + peer-element); the shared secret k = K.x. */
	uint8_t t[RF_2A4M1_P256_POINT_LEN], t2[RF_2A4M1_P256_POINT_LEN], K[RF_2A4M1_P256_POINT_LEN];
	if (!rf_2a4m1_p256_point_mul(peer_scalar, s->pwe, t))
		return false;
	if (!rf_2a4m1_p256_point_add(t, peer_element, t2))
		return false;
	if (!rf_2a4m1_p256_point_mul(s->rnd, t2, K))
		return false;
	const uint8_t *k = K;                          /* K.x = first 32 bytes */

	/* keyseed = HMAC-SHA256(<0>^32, k) */
	uint8_t keyseed[RF_2A4M1_SHA256_DIGEST_LEN];
	rf_2a4m1_hmac_sha256(RF_2A4M1_SAE_KEYSEED_SALT, sizeof RF_2A4M1_SAE_KEYSEED_SALT, k, RF_2A4M1_P256_COORD_LEN, keyseed);
	/* context = (commit-scalar + peer-commit-scalar) mod n */
	uint8_t ctx[RF_2A4M1_SAE_KEY_LEN];
	rf_2a4m1_p256_scalar_add_n(s->commit_scalar, peer_scalar, ctx);
	/* KCK || PMK = KDF-Length-512(keyseed, "SAE KCK and PMK", context) */
	uint8_t kck_pmk[2 * RF_2A4M1_SAE_KEY_LEN];
	rf_2a4m1_ieee80211_kdf_length(keyseed, RF_2A4M1_SHA256_DIGEST_LEN, "SAE KCK and PMK", 15,
	                     ctx, sizeof ctx, kck_pmk, 512);
	memcpy(s->kck, kck_pmk, RF_2A4M1_SAE_KEY_LEN);
	memcpy(s->pmk, kck_pmk + RF_2A4M1_SAE_KEY_LEN, RF_2A4M1_SAE_KEY_LEN);
	memcpy(s->pmkid, ctx, 16);                      /* PMKID = L(context, 0, 128) */

	memcpy(s->peer_scalar, peer_scalar, RF_2A4M1_SAE_KEY_LEN);
	memcpy(s->peer_element, peer_element, RF_2A4M1_P256_POINT_LEN);
	s->have_keys = true;
	/* F12: wipe the shared-secret + key-derivation transients (KCK/PMK now live in s). */
	rf_2a4m1_crypto_wipe(t, sizeof t);
	rf_2a4m1_crypto_wipe(t2, sizeof t2);
	rf_2a4m1_crypto_wipe(K, sizeof K);
	rf_2a4m1_crypto_wipe(keyseed, sizeof keyseed);
	rf_2a4m1_crypto_wipe(kck_pmk, sizeof kck_pmk);
	return true;
}

static void rf_2a4m1_sae_cn(const uint8_t kck[RF_2A4M1_SAE_KEY_LEN], uint16_t send_confirm,
                   const uint8_t *sc1, const uint8_t *el1,
                   const uint8_t *sc2, const uint8_t *el2, uint8_t out[RF_2A4M1_SAE_KEY_LEN])
{
	uint8_t m[2 + RF_2A4M1_SAE_KEY_LEN + RF_2A4M1_P256_POINT_LEN + RF_2A4M1_SAE_KEY_LEN + RF_2A4M1_P256_POINT_LEN];
	size_t o = 0;
	m[o++] = (uint8_t)(send_confirm & 0xff);
	m[o++] = (uint8_t)((send_confirm >> 8) & 0xff);
	memcpy(m + o, sc1, RF_2A4M1_SAE_KEY_LEN);       o += RF_2A4M1_SAE_KEY_LEN;
	memcpy(m + o, el1, RF_2A4M1_P256_POINT_LEN);    o += RF_2A4M1_P256_POINT_LEN;
	memcpy(m + o, sc2, RF_2A4M1_SAE_KEY_LEN);       o += RF_2A4M1_SAE_KEY_LEN;
	memcpy(m + o, el2, RF_2A4M1_P256_POINT_LEN);    o += RF_2A4M1_P256_POINT_LEN;
	rf_2a4m1_hmac_sha256(kck, RF_2A4M1_SAE_KEY_LEN, m, o, out);
}

void rf_2a4m1_sae_confirm(const struct rf_2a4m1_sae *s, uint16_t send_confirm, uint8_t out[RF_2A4M1_SAE_KEY_LEN])
{
	rf_2a4m1_sae_cn(s->kck, send_confirm, s->commit_scalar, s->commit_element,
	       s->peer_scalar, s->peer_element, out);
}

bool rf_2a4m1_sae_verify_confirm(const struct rf_2a4m1_sae *s, uint16_t send_confirm,
                        const uint8_t peer_confirm[RF_2A4M1_SAE_KEY_LEN])
{
	/* A confirm can only be verified AFTER the peer commit was processed — that is
	 * what derives the KCK (sae_process_commit sets have_keys). Verifying earlier
	 * would key the MIC on the still-zero KCK of a freshly-inited struct, which an
	 * attacker can reproduce (KCK=0, peer scalar/element=0 all known) — accepting a
	 * confirm with no valid commit and no real PMK (802.11 §12.4: confirm follows a
	 * validated commit). Reject before the keys exist. */
	if (!s->have_keys)
		return false;
	uint8_t exp[RF_2A4M1_SAE_KEY_LEN];
	rf_2a4m1_sae_cn(s->kck, send_confirm, s->peer_scalar, s->peer_element,
	       s->commit_scalar, s->commit_element, exp);
	return rf_2a4m1_crypto_ct_eq(exp, peer_confirm, RF_2A4M1_SAE_KEY_LEN);
}

static uint32_t rf_2a4m1_rol32(uint32_t v, unsigned n) { return (v << n) | (v >> (32 - n)); }

static void rf_2a4m1_sha1_sw_init(struct rf_2a4m1_sha1_ctx *c)
{
	c->h[0] = 0x67452301u; c->h[1] = 0xEFCDAB89u; c->h[2] = 0x98BADCFEu;
	c->h[3] = 0x10325476u; c->h[4] = 0xC3D2E1F0u;
	c->nbits = 0;
	c->buf_len = 0;
}

static void rf_2a4m1_sha1_block(struct rf_2a4m1_sha1_ctx *c, const uint8_t *p)
{
	uint32_t w[80] = {0};   /* zero-init: the message schedule is fully written below;
	                         * the explicit init keeps -fanalyzer's partial-init model quiet */
	for (int i = 0; i < 16; i++)
		w[i] = ((uint32_t)p[i * 4] << 24) | ((uint32_t)p[i * 4 + 1] << 16) |
		       ((uint32_t)p[i * 4 + 2] << 8) | (uint32_t)p[i * 4 + 3];
	for (int i = 16; i < 80; i++)
		w[i] = rf_2a4m1_rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

	uint32_t a = c->h[0], b = c->h[1], d = c->h[2], e = c->h[3], f = c->h[4];
	for (int i = 0; i < 80; i++) {
		uint32_t k, t;
		if (i < 20)      { t = (b & d) | (~b & e);          k = 0x5A827999u; }
		else if (i < 40) { t = b ^ d ^ e;                   k = 0x6ED9EBA1u; }
		else if (i < 60) { t = (b & d) | (b & e) | (d & e); k = 0x8F1BBCDCu; }
		else             { t = b ^ d ^ e;                   k = 0xCA62C1D6u; }
		uint32_t tmp = rf_2a4m1_rol32(a, 5) + t + f + k + w[i];
		f = e; e = d; d = rf_2a4m1_rol32(b, 30); b = a; a = tmp;
	}
	c->h[0] += a; c->h[1] += b; c->h[2] += d; c->h[3] += e; c->h[4] += f;
}

static void rf_2a4m1_sha1_sw_update(struct rf_2a4m1_sha1_ctx *c, const uint8_t *data, size_t len)
{
	c->nbits += (uint64_t)len * 8;
	while (len > 0) {
		size_t take = RF_2A4M1_SHA1_BLOCK_LEN - c->buf_len;
		if (take > len)
			take = len;
		memcpy(c->buf + c->buf_len, data, take);
		c->buf_len += take;
		data += take;
		len -= take;
		if (c->buf_len == RF_2A4M1_SHA1_BLOCK_LEN) {
			rf_2a4m1_sha1_block(c, c->buf);
			c->buf_len = 0;
		}
	}
}

static void rf_2a4m1_sha1_sw_final(struct rf_2a4m1_sha1_ctx *c, uint8_t out[RF_2A4M1_SHA1_DIGEST_LEN])
{
	uint64_t nbits = c->nbits;
	uint8_t pad = 0x80;
	rf_2a4m1_sha1_sw_update(c, &pad, 1);
	pad = 0x00;
	while (c->buf_len != RF_2A4M1_SHA1_BLOCK_LEN - 8)
		rf_2a4m1_sha1_sw_update(c, &pad, 1);
	uint8_t lenb[8];
	for (int i = 0; i < 8; i++)
		lenb[i] = (uint8_t)(nbits >> (8 * (7 - i)));
	rf_2a4m1_sha1_sw_update(c, lenb, 8);     /* triggers the last block; buf_len -> 0 */
	for (int i = 0; i < 5; i++) {
		out[i * 4]     = (uint8_t)(c->h[i] >> 24);
		out[i * 4 + 1] = (uint8_t)(c->h[i] >> 16);
		out[i * 4 + 2] = (uint8_t)(c->h[i] >> 8);
		out[i * 4 + 3] = (uint8_t)c->h[i];
	}
}

void rf_2a4m1_sha1_sw(const uint8_t *data, size_t len, uint8_t out[RF_2A4M1_SHA1_DIGEST_LEN])
{
	struct rf_2a4m1_sha1_ctx c;
	rf_2a4m1_sha1_sw_init(&c);
	rf_2a4m1_sha1_sw_update(&c, data, len);
	rf_2a4m1_sha1_sw_final(&c, out);
}

void rf_2a4m1_sha1_init(struct rf_2a4m1_sha1_ctx *c)                              { rf_2a4m1_sha1_sw_init(c); }

void rf_2a4m1_sha1_update(struct rf_2a4m1_sha1_ctx *c, const uint8_t *d, size_t n) { rf_2a4m1_sha1_sw_update(c, d, n); }

void rf_2a4m1_sha1_final(struct rf_2a4m1_sha1_ctx *c, uint8_t out[RF_2A4M1_SHA1_DIGEST_LEN]) { rf_2a4m1_sha1_sw_final(c, out); }

void rf_2a4m1_sha1(const uint8_t *data, size_t len, uint8_t out[RF_2A4M1_SHA1_DIGEST_LEN]) { rf_2a4m1_sha1_sw(data, len, out); }

void rf_2a4m1_hmac_sha1(const uint8_t *rf_2a4m1_key, size_t key_len,
               const uint8_t *msg, size_t msg_len, uint8_t out[RF_2A4M1_SHA1_DIGEST_LEN])
{
	uint8_t k[RF_2A4M1_SHA1_BLOCK_LEN];
	uint8_t pad[RF_2A4M1_SHA1_BLOCK_LEN];
	uint8_t inner[RF_2A4M1_SHA1_DIGEST_LEN];
	struct rf_2a4m1_sha1_ctx c;

	memset(k, 0, sizeof k);
	if (key_len > RF_2A4M1_SHA1_BLOCK_LEN)
		rf_2a4m1_sha1(rf_2a4m1_key, key_len, k);          /* K' = H(K), then zero-padded */
	else
		memcpy(k, rf_2a4m1_key, key_len);

	for (int i = 0; i < RF_2A4M1_SHA1_BLOCK_LEN; i++)
		pad[i] = k[i] ^ 0x36u;
	rf_2a4m1_sha1_init(&c);
	rf_2a4m1_sha1_update(&c, pad, RF_2A4M1_SHA1_BLOCK_LEN);
	rf_2a4m1_sha1_update(&c, msg, msg_len);
	rf_2a4m1_sha1_final(&c, inner);

	for (int i = 0; i < RF_2A4M1_SHA1_BLOCK_LEN; i++)
		pad[i] = k[i] ^ 0x5Cu;
	rf_2a4m1_sha1_init(&c);
	rf_2a4m1_sha1_update(&c, pad, RF_2A4M1_SHA1_BLOCK_LEN);
	rf_2a4m1_sha1_update(&c, inner, RF_2A4M1_SHA1_DIGEST_LEN);
	rf_2a4m1_sha1_final(&c, out);
}

void rf_2a4m1_pbkdf2_hmac_sha1(const uint8_t *pw, size_t pw_len,
                      const uint8_t *salt, size_t salt_len,
                      uint32_t iter, uint8_t *out, size_t dk_len)
{
	uint8_t block[RF_2A4M1_SHA1_BLOCK_LEN + 4];   /* salt is short in WPA2 (SSID); guard below */
	uint32_t counter = 1;
	size_t done = 0;

	while (done < dk_len) {
		uint8_t u[RF_2A4M1_SHA1_DIGEST_LEN], t[RF_2A4M1_SHA1_DIGEST_LEN];
		/* U1 = PRF(pw, salt || INT(counter)) — salt+4 must fit `block`. WPA2 SSIDs
		 * are <= 32 octets, well within SHA1_BLOCK_LEN; a longer salt is rejected
		 * by clamping (the WPA2 caller never exceeds it). */
		size_t sl = salt_len;
		if (sl > RF_2A4M1_SHA1_BLOCK_LEN)
			sl = RF_2A4M1_SHA1_BLOCK_LEN;
		memcpy(block, salt, sl);
		block[sl]     = (uint8_t)(counter >> 24);
		block[sl + 1] = (uint8_t)(counter >> 16);
		block[sl + 2] = (uint8_t)(counter >> 8);
		block[sl + 3] = (uint8_t)counter;
		rf_2a4m1_hmac_sha1(pw, pw_len, block, sl + 4, u);
		memcpy(t, u, RF_2A4M1_SHA1_DIGEST_LEN);
		for (uint32_t j = 1; j < iter; j++) {
			rf_2a4m1_hmac_sha1(pw, pw_len, u, RF_2A4M1_SHA1_DIGEST_LEN, u);
			for (int k = 0; k < RF_2A4M1_SHA1_DIGEST_LEN; k++)
				t[k] ^= u[k];
		}
		size_t take = dk_len - done;
		if (take > RF_2A4M1_SHA1_DIGEST_LEN)
			take = RF_2A4M1_SHA1_DIGEST_LEN;
		memcpy(out + done, t, take);
		done += take;
		counter++;
	}
}

void rf_2a4m1_ieee80211_prf(const uint8_t *rf_2a4m1_key, size_t key_len,
                   const char *label, size_t label_len,
                   const uint8_t *data, size_t data_len,
                   uint8_t *out, size_t out_len)
{
	/* input = label || 0x00 || data || i. Bound the assembly buffer: WPA2 uses a
	 * 22-octet label + 76-octet data (2*6 MAC + 2*32 nonce), so 128 is ample. */
	uint8_t in[128];
	uint8_t r[RF_2A4M1_SHA1_DIGEST_LEN];
	size_t base = 0;
	uint8_t i = 0;
	size_t done = 0;

	memcpy(in, label, label_len);       base = label_len;
	in[base++] = 0x00;
	memcpy(in + base, data, data_len);  base += data_len;

	while (done < out_len) {
		in[base] = i;
		rf_2a4m1_hmac_sha1(rf_2a4m1_key, key_len, in, base + 1, r);
		size_t take = out_len - done;
		if (take > RF_2A4M1_SHA1_DIGEST_LEN)
			take = RF_2A4M1_SHA1_DIGEST_LEN;
		memcpy(out + done, r, take);
		done += take;
		i++;
	}
}

static int rf_2a4m1_mac_lt(const rf_2a4m1_mac_addr *x, const rf_2a4m1_mac_addr *y)
{
	for (int i = 0; i < RF_2A4M1_ETH_ALEN; i++) {
		if (x->a[i] < y->a[i]) return 1;
		if (x->a[i] > y->a[i]) return 0;
	}
	return 0;
}

static int rf_2a4m1_nonce_lt(const uint8_t *x, const uint8_t *y)
{
	for (int i = 0; i < RF_2A4M1_WPA_NONCE_LEN; i++) {
		if (x[i] < y[i]) return 1;
		if (x[i] > y[i]) return 0;
	}
	return 0;
}

void rf_2a4m1_wpa_derive_ptk(const uint8_t pmk[RF_2A4M1_WPA_PMK_LEN],
                    const rf_2a4m1_mac_addr *aa, const rf_2a4m1_mac_addr *spa,
                    const uint8_t anonce[RF_2A4M1_WPA_NONCE_LEN],
                    const uint8_t snonce[RF_2A4M1_WPA_NONCE_LEN],
                    uint8_t ptk[RF_2A4M1_WPA_PTK_LEN])
{
	uint8_t data[2 * RF_2A4M1_ETH_ALEN + 2 * RF_2A4M1_WPA_NONCE_LEN];
	const rf_2a4m1_mac_addr *lo_m = rf_2a4m1_mac_lt(aa, spa) ? aa : spa;
	const rf_2a4m1_mac_addr *hi_m = rf_2a4m1_mac_lt(aa, spa) ? spa : aa;
	const uint8_t *lo_n = rf_2a4m1_nonce_lt(anonce, snonce) ? anonce : snonce;
	const uint8_t *hi_n = rf_2a4m1_nonce_lt(anonce, snonce) ? snonce : anonce;
	size_t p = 0;
	memcpy(data + p, lo_m->a, RF_2A4M1_ETH_ALEN); p += RF_2A4M1_ETH_ALEN;
	memcpy(data + p, hi_m->a, RF_2A4M1_ETH_ALEN); p += RF_2A4M1_ETH_ALEN;
	memcpy(data + p, lo_n, RF_2A4M1_WPA_NONCE_LEN); p += RF_2A4M1_WPA_NONCE_LEN;
	memcpy(data + p, hi_n, RF_2A4M1_WPA_NONCE_LEN); p += RF_2A4M1_WPA_NONCE_LEN;

	rf_2a4m1_ieee80211_prf(pmk, RF_2A4M1_WPA_PMK_LEN, "Pairwise key expansion", 22,
	              data, p, ptk, RF_2A4M1_WPA_PTK_LEN);
}

static const uint32_t RF_2A4M1_K256[64] = {
	0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u,
	0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
	0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u, 0xe49b69c1u, 0xefbe4786u,
	0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
	0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u,
	0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
	0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u, 0xa2bfe8a1u, 0xa81a664bu,
	0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
	0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au,
	0x5b9cca4fu, 0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
	0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

static uint32_t rf_2a4m1_ror32(uint32_t x, unsigned n) { return (x >> n) | (x << (32 - n)); }

static void rf_2a4m1_sha256_block(struct rf_2a4m1_sha256_ctx *c, const uint8_t *p)
{
	uint32_t w[64];
	for (int i = 0; i < 16; i++)
		w[i] = ((uint32_t)p[4 * i] << 24) | ((uint32_t)p[4 * i + 1] << 16) |
		       ((uint32_t)p[4 * i + 2] << 8) | (uint32_t)p[4 * i + 3];
	for (int i = 16; i < 64; i++) {
		uint32_t s0 = rf_2a4m1_ror32(w[i - 15], 7) ^ rf_2a4m1_ror32(w[i - 15], 18) ^ (w[i - 15] >> 3);
		uint32_t s1 = rf_2a4m1_ror32(w[i - 2], 17) ^ rf_2a4m1_ror32(w[i - 2], 19) ^ (w[i - 2] >> 10);
		w[i] = w[i - 16] + s0 + w[i - 7] + s1;
	}
	uint32_t a = c->h[0], b = c->h[1], cc = c->h[2], d = c->h[3];
	uint32_t e = c->h[4], f = c->h[5], g = c->h[6], h = c->h[7];
	for (int i = 0; i < 64; i++) {
		uint32_t S1 = rf_2a4m1_ror32(e, 6) ^ rf_2a4m1_ror32(e, 11) ^ rf_2a4m1_ror32(e, 25);
		uint32_t ch = (e & f) ^ (~e & g);
		uint32_t t1 = h + S1 + ch + RF_2A4M1_K256[i] + w[i];
		uint32_t S0 = rf_2a4m1_ror32(a, 2) ^ rf_2a4m1_ror32(a, 13) ^ rf_2a4m1_ror32(a, 22);
		uint32_t maj = (a & b) ^ (a & cc) ^ (b & cc);
		uint32_t t2 = S0 + maj;
		h = g; g = f; f = e; e = d + t1; d = cc; cc = b; b = a; a = t1 + t2;
	}
	c->h[0] += a; c->h[1] += b; c->h[2] += cc; c->h[3] += d;
	c->h[4] += e; c->h[5] += f; c->h[6] += g; c->h[7] += h;
}

static void rf_2a4m1_sha256_sw_init(struct rf_2a4m1_sha256_ctx *c)
{
	c->h[0] = 0x6a09e667u; c->h[1] = 0xbb67ae85u; c->h[2] = 0x3c6ef372u;
	c->h[3] = 0xa54ff53au; c->h[4] = 0x510e527fu; c->h[5] = 0x9b05688cu;
	c->h[6] = 0x1f83d9abu; c->h[7] = 0x5be0cd19u;
	c->nbits = 0;
	c->buf_len = 0;
}

static void rf_2a4m1_sha256_sw_update(struct rf_2a4m1_sha256_ctx *c, const uint8_t *data, size_t len)
{
	c->nbits += (uint64_t)len * 8;
	while (len) {
		size_t take = RF_2A4M1_SHA256_BLOCK_LEN - c->buf_len;
		if (take > len)
			take = len;
		memcpy(c->buf + c->buf_len, data, take);
		c->buf_len += take;
		data += take;
		len -= take;
		if (c->buf_len == RF_2A4M1_SHA256_BLOCK_LEN) {
			rf_2a4m1_sha256_block(c, c->buf);
			c->buf_len = 0;
		}
	}
}

static void rf_2a4m1_sha256_sw_final(struct rf_2a4m1_sha256_ctx *c, uint8_t out[RF_2A4M1_SHA256_DIGEST_LEN])
{
	uint64_t nbits = c->nbits;
	uint8_t pad = 0x80;
	rf_2a4m1_sha256_sw_update(c, &pad, 1);
	uint8_t zero = 0;
	while (c->buf_len != 56)
		rf_2a4m1_sha256_sw_update(c, &zero, 1);
	uint8_t len_be[8];
	for (int i = 0; i < 8; i++)
		len_be[i] = (uint8_t)(nbits >> (8 * (7 - i)));
	/* feed the length WITHOUT re-counting it into nbits */
	memcpy(c->buf + c->buf_len, len_be, 8);
	rf_2a4m1_sha256_block(c, c->buf);
	for (int i = 0; i < 8; i++) {
		out[4 * i]     = (uint8_t)(c->h[i] >> 24);
		out[4 * i + 1] = (uint8_t)(c->h[i] >> 16);
		out[4 * i + 2] = (uint8_t)(c->h[i] >> 8);
		out[4 * i + 3] = (uint8_t)c->h[i];
	}
}

void rf_2a4m1_sha256_sw(const uint8_t *data, size_t len, uint8_t out[RF_2A4M1_SHA256_DIGEST_LEN])
{
	struct rf_2a4m1_sha256_ctx c;
	rf_2a4m1_sha256_sw_init(&c);
	rf_2a4m1_sha256_sw_update(&c, data, len);
	rf_2a4m1_sha256_sw_final(&c, out);
}

void rf_2a4m1_sha256_init(struct rf_2a4m1_sha256_ctx *c)                              { rf_2a4m1_sha256_sw_init(c); }

void rf_2a4m1_sha256_update(struct rf_2a4m1_sha256_ctx *c, const uint8_t *d, size_t n) { rf_2a4m1_sha256_sw_update(c, d, n); }

void rf_2a4m1_sha256_final(struct rf_2a4m1_sha256_ctx *c, uint8_t out[RF_2A4M1_SHA256_DIGEST_LEN]) { rf_2a4m1_sha256_sw_final(c, out); }

void rf_2a4m1_sha256(const uint8_t *data, size_t len, uint8_t out[RF_2A4M1_SHA256_DIGEST_LEN]) { rf_2a4m1_sha256_sw(data, len, out); }

void rf_2a4m1_hmac_sha256(const uint8_t *rf_2a4m1_key, size_t key_len,
                 const uint8_t *msg, size_t msg_len, uint8_t out[RF_2A4M1_SHA256_DIGEST_LEN])
{
	uint8_t k[RF_2A4M1_SHA256_BLOCK_LEN];
	uint8_t ipad[RF_2A4M1_SHA256_BLOCK_LEN], opad[RF_2A4M1_SHA256_BLOCK_LEN];
	uint8_t inner[RF_2A4M1_SHA256_DIGEST_LEN];
	struct rf_2a4m1_sha256_ctx c;

	memset(k, 0, sizeof k);
	if (key_len > RF_2A4M1_SHA256_BLOCK_LEN)
		rf_2a4m1_sha256(rf_2a4m1_key, key_len, k);          /* K' = H(K) when K longer than block */
	else
		memcpy(k, rf_2a4m1_key, key_len);

	for (int i = 0; i < RF_2A4M1_SHA256_BLOCK_LEN; i++) {
		ipad[i] = (uint8_t)(k[i] ^ 0x36);
		opad[i] = (uint8_t)(k[i] ^ 0x5c);
	}
	rf_2a4m1_sha256_init(&c);
	rf_2a4m1_sha256_update(&c, ipad, RF_2A4M1_SHA256_BLOCK_LEN);
	rf_2a4m1_sha256_update(&c, msg, msg_len);
	rf_2a4m1_sha256_final(&c, inner);

	rf_2a4m1_sha256_init(&c);
	rf_2a4m1_sha256_update(&c, opad, RF_2A4M1_SHA256_BLOCK_LEN);
	rf_2a4m1_sha256_update(&c, inner, RF_2A4M1_SHA256_DIGEST_LEN);
	rf_2a4m1_sha256_final(&c, out);
}

void rf_2a4m1_hkdf_sha256_extract(const uint8_t *salt, size_t salt_len,
                         const uint8_t *ikm, size_t ikm_len, uint8_t prk[RF_2A4M1_SHA256_DIGEST_LEN])
{
	uint8_t zero[RF_2A4M1_SHA256_DIGEST_LEN];
	if (salt == 0 || salt_len == 0) {
		memset(zero, 0, sizeof zero);     /* RFC 5869: absent salt = HashLen zeros */
		salt = zero;
		salt_len = RF_2A4M1_SHA256_DIGEST_LEN;
	}
	rf_2a4m1_hmac_sha256(salt, salt_len, ikm, ikm_len, prk);   /* PRK = HMAC(salt, IKM) */
}

void rf_2a4m1_hkdf_sha256_expand(const uint8_t *prk, size_t prk_len,
                        const uint8_t *rf_2a4m1_info, size_t info_len,
                        uint8_t *out, size_t out_len)
{
	uint8_t t[RF_2A4M1_SHA256_DIGEST_LEN];
	size_t t_len = 0;
	uint8_t counter = 1;
	size_t done = 0;
	while (done < out_len) {
		/* T(i) = HMAC(PRK, T(i-1) || info || i) */
		struct rf_2a4m1_sha256_ctx ictx, octx;
		uint8_t k[RF_2A4M1_SHA256_BLOCK_LEN], ipad[RF_2A4M1_SHA256_BLOCK_LEN], opad[RF_2A4M1_SHA256_BLOCK_LEN];
		uint8_t inner[RF_2A4M1_SHA256_DIGEST_LEN];
		memset(k, 0, sizeof k);
		if (prk_len > RF_2A4M1_SHA256_BLOCK_LEN)
			rf_2a4m1_sha256(prk, prk_len, k);
		else
			memcpy(k, prk, prk_len);
		for (int i = 0; i < RF_2A4M1_SHA256_BLOCK_LEN; i++) {
			ipad[i] = (uint8_t)(k[i] ^ 0x36);
			opad[i] = (uint8_t)(k[i] ^ 0x5c);
		}
		rf_2a4m1_sha256_init(&ictx);
		rf_2a4m1_sha256_update(&ictx, ipad, RF_2A4M1_SHA256_BLOCK_LEN);
		if (t_len)
			rf_2a4m1_sha256_update(&ictx, t, t_len);
		rf_2a4m1_sha256_update(&ictx, rf_2a4m1_info, info_len);
		rf_2a4m1_sha256_update(&ictx, &counter, 1);
		rf_2a4m1_sha256_final(&ictx, inner);
		rf_2a4m1_sha256_init(&octx);
		rf_2a4m1_sha256_update(&octx, opad, RF_2A4M1_SHA256_BLOCK_LEN);
		rf_2a4m1_sha256_update(&octx, inner, RF_2A4M1_SHA256_DIGEST_LEN);
		rf_2a4m1_sha256_final(&octx, t);
		t_len = RF_2A4M1_SHA256_DIGEST_LEN;
		size_t take = out_len - done;
		if (take > RF_2A4M1_SHA256_DIGEST_LEN)
			take = RF_2A4M1_SHA256_DIGEST_LEN;
		memcpy(out + done, t, take);
		done += take;
		counter++;
	}
}

void rf_2a4m1_ieee80211_kdf_length(const uint8_t *rf_2a4m1_key, size_t key_len,
                          const char *label, size_t label_len,
                          const uint8_t *ctx, size_t ctx_len,
                          uint8_t *out, size_t out_bits)
{
	size_t out_len = (out_bits + 7) / 8;
	uint8_t len_le[2] = { (uint8_t)(out_bits & 0xff), (uint8_t)((out_bits >> 8) & 0xff) };
	uint16_t i = 1;
	size_t done = 0;
	while (done < out_len) {
		uint8_t blk[RF_2A4M1_SHA256_DIGEST_LEN];
		uint8_t i_le[2] = { (uint8_t)(i & 0xff), (uint8_t)((i >> 8) & 0xff) };
		struct rf_2a4m1_sha256_ctx ictx, octx;
		uint8_t k[RF_2A4M1_SHA256_BLOCK_LEN], ipad[RF_2A4M1_SHA256_BLOCK_LEN], opad[RF_2A4M1_SHA256_BLOCK_LEN];
		uint8_t inner[RF_2A4M1_SHA256_DIGEST_LEN];
		memset(k, 0, sizeof k);
		if (key_len > RF_2A4M1_SHA256_BLOCK_LEN)
			rf_2a4m1_sha256(rf_2a4m1_key, key_len, k);
		else
			memcpy(k, rf_2a4m1_key, key_len);
		for (int j = 0; j < RF_2A4M1_SHA256_BLOCK_LEN; j++) {
			ipad[j] = (uint8_t)(k[j] ^ 0x36);
			opad[j] = (uint8_t)(k[j] ^ 0x5c);
		}
		rf_2a4m1_sha256_init(&ictx);
		rf_2a4m1_sha256_update(&ictx, ipad, RF_2A4M1_SHA256_BLOCK_LEN);
		rf_2a4m1_sha256_update(&ictx, i_le, 2);
		rf_2a4m1_sha256_update(&ictx, (const uint8_t *)label, label_len);
		rf_2a4m1_sha256_update(&ictx, ctx, ctx_len);
		rf_2a4m1_sha256_update(&ictx, len_le, 2);
		rf_2a4m1_sha256_final(&ictx, inner);
		rf_2a4m1_sha256_init(&octx);
		rf_2a4m1_sha256_update(&octx, opad, RF_2A4M1_SHA256_BLOCK_LEN);
		rf_2a4m1_sha256_update(&octx, inner, RF_2A4M1_SHA256_DIGEST_LEN);
		rf_2a4m1_sha256_final(&octx, blk);
		size_t take = out_len - done;
		if (take > RF_2A4M1_SHA256_DIGEST_LEN)
			take = RF_2A4M1_SHA256_DIGEST_LEN;
		memcpy(out + done, blk, take);
		done += take;
		i++;
	}
	/* mask trailing bits if out_bits is not a byte multiple (SAE uses 512, exact) */
	if (out_bits % 8) {
		unsigned rem = (unsigned)(out_bits % 8);
		out[out_len - 1] &= (uint8_t)(0xff << (8 - rem));
	}
}

