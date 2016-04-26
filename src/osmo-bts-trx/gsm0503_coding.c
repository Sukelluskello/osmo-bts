/* (C) 2013 by Andreas Eversberg <jolly@eversberg.eu>
 * (C) 2015 by Alexander Chemeris <Alexander.Chemeris@fairwaves.co>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <osmocom/core/utils.h>
#include <osmocom/core/bits.h>
#include <osmocom/core/conv.h>
#include <osmocom/core/crcgen.h>
#include <osmocom/codec/codec.h>

#include <osmo-bts/logging.h>
#include <osmo-bts/gsm_data.h>

#include "gsm0503_conv.h"
#include "gsm0503_parity.h"
#include "gsm0503_mapping.h"
#include "gsm0503_interleaving.h"
#include "gsm0503_tables.h"
#include "gsm0503_coding.h"

int osmo_conv_decode_ber(const struct osmo_conv_code *code,
	const sbit_t *input, ubit_t *output,
	int *n_errors, int *n_bits_total)
{
	int res, i;
	ubit_t recoded[1024]; /* TODO: We can do smaller, I guess */

	res = osmo_conv_decode(code, input, output);

	*n_bits_total = osmo_conv_encode(code, output, recoded);
	OSMO_ASSERT(sizeof(recoded)/sizeof(recoded[0]) >= *n_bits_total);

	/* Count bit errors */
	*n_errors = 0;
	for (i=0; i< *n_bits_total; i++) {
		if (! ((recoded[i] && input[i]<0) ||
		       (!recoded[i] && input[i]>0)) )
			*n_errors += 1;
	}

	return res;
}


static int _xcch_decode_cB(uint8_t *l2_data, sbit_t *cB,
	int *n_errors, int *n_bits_total)
{
	ubit_t conv[224];
	int rv;

	osmo_conv_decode_ber(&gsm0503_conv_xcch, cB, conv, n_errors, n_bits_total);

	rv = osmo_crc64gen_check_bits(&gsm0503_fire_crc40, conv, 184, conv+184);
	if (rv)
		return -1;

	osmo_ubit2pbit_ext(l2_data, 0, conv, 0, 184, 1);

	return 0;
}

static int _xcch_encode_cB(ubit_t *cB, uint8_t *l2_data)
{
	ubit_t conv[224];

	osmo_pbit2ubit_ext(conv, 0, l2_data, 0, 184, 1);

	osmo_crc64gen_set_bits(&gsm0503_fire_crc40, conv, 184, conv+184);

	osmo_conv_encode(&gsm0503_conv_xcch, conv, cB);

	return 0;
}


/*
 * GSM xCCH block transcoding
 */

int xcch_decode(uint8_t *l2_data, sbit_t *bursts,
	int *n_errors, int *n_bits_total)
{
	sbit_t iB[456], cB[456];
	int i;

	for (i=0; i<4; i++)
		gsm0503_xcch_burst_unmap(&iB[i * 114], &bursts[i * 116], NULL,
			NULL);

	gsm0503_xcch_deinterleave(cB, iB);

	return _xcch_decode_cB(l2_data, cB, n_errors, n_bits_total);
}

int xcch_encode(ubit_t *bursts, uint8_t *l2_data)
{
	ubit_t iB[456], cB[456], hl = 1, hn = 1;
	int i;

	_xcch_encode_cB(cB, l2_data);

	gsm0503_xcch_interleave(cB, iB);

	for (i=0; i<4; i++)
		gsm0503_xcch_burst_map(&iB[i * 114], &bursts[i * 116], &hl,
			&hn);

	return 0;
}


/*
 * GSM PDTCH block transcoding
 */

int pdtch_decode(uint8_t *l2_data, sbit_t *bursts, uint8_t *usf_p,
	int *n_errors, int *n_bits_total)
{
	sbit_t iB[456], cB[676], hl_hn[8];
	ubit_t conv[456];
	int i, j, k, rv, best = 0, cs = 0, usf = 0; /* make GCC happy */

	for (i=0; i<4; i++)
		gsm0503_xcch_burst_unmap(&iB[i * 114], &bursts[i * 116],
			hl_hn + i*2, hl_hn + i*2 + 1);

	for (i=0; i<4; i++) {
		for (j=0, k=0; j<8; j++)
			k += abs(((int)gsm0503_pdtch_hl_hn_sbit[i][j]) -
							((int)hl_hn[j]));
		if (i == 0 || k < best) {
			best = k;
			cs = i+1;
		}
	}

	gsm0503_xcch_deinterleave(cB, iB);

	switch (cs) {
	case 1:
		osmo_conv_decode_ber(&gsm0503_conv_xcch, cB, conv, n_errors, n_bits_total);

		rv = osmo_crc64gen_check_bits(&gsm0503_fire_crc40, conv, 184,
			conv+184);
		if (rv)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, conv, 0, 184, 1);

		return 23;
	case 2:
		for (i=587, j=455; i>=0; i--)
			if (!gsm0503_puncture_cs2[i])
				cB[i] = cB[j--];
			else
				cB[i] = 0;

		osmo_conv_decode_ber(&gsm0503_conv_cs2, cB, conv, n_errors, n_bits_total);

		for (i=0; i<8; i++) {
			for (j=0, k=0; j<6; j++)
				k += abs(((int)gsm0503_usf2six[i][j]) -
							((int)conv[j]));
			if (i == 0 || k < best) {
				best = k;
				usf = i;
			}
		}

		conv[3] = usf & 1;
		conv[4] = (usf >> 1) & 1;
		conv[5] = (usf >> 2) & 1;
		if (usf_p)
			*usf_p = usf;

		rv = osmo_crc16gen_check_bits(&gsm0503_cs234_crc16, conv+3, 271,
			conv+3+271);
		if (rv)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, conv, 3, 271, 1);

		return 34;
	case 3:
		for (i=675, j=455; i>=0; i--)
			if (!gsm0503_puncture_cs3[i])
				cB[i] = cB[j--];
			else
				cB[i] = 0;

		osmo_conv_decode_ber(&gsm0503_conv_cs3, cB, conv, n_errors, n_bits_total);

		for (i=0; i<8; i++) {
			for (j=0, k=0; j<6; j++)
				k += abs(((int)gsm0503_usf2six[i][j]) -
							((int)conv[j]));
			if (i == 0 || k < best) {
				best = k;
				usf = i;
			}
		}

		conv[3] = usf & 1;
		conv[4] = (usf >> 1) & 1;
		conv[5] = (usf >> 2) & 1;
		if (usf_p)
			*usf_p = usf;

		rv = osmo_crc16gen_check_bits(&gsm0503_cs234_crc16, conv+3, 315,
			conv+3+315);
		if (rv)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, conv, 3, 315, 1);

		return 40;
	case 4:
		for (i=12; i<456;i++)
			conv[i] = (cB[i] < 0) ? 1:0;

		for (i=0; i<8; i++) {
			for (j=0, k=0; j<12; j++)
				k += abs(((int)gsm0503_usf2twelve_sbit[i][j]) -
							((int)cB[j]));
			if (i == 0 || k < best) {
				best = k;
				usf = i;
			}
		}

		conv[9] = usf & 1;
		conv[10] = (usf >> 1) & 1;
		conv[11] = (usf >> 2) & 1;
		if (usf_p)
			*usf_p = usf;

		rv = osmo_crc16gen_check_bits(&gsm0503_cs234_crc16, conv+9, 431,
			conv+9+431);
		if (rv) {
			*n_bits_total = 456-12;
			*n_errors = *n_bits_total;
			return -1;
		}

		*n_bits_total = 456-12;
		*n_errors = 0;

		osmo_ubit2pbit_ext(l2_data, 0, conv, 9, 431, 1);

		return 54;
	default:
		*n_bits_total = 0;
		*n_errors = 0;
		break;
	}

	return -1;
}

static int pdtch_edge_mcs1_demap(sbit_t *bursts, sbit_t *hc, sbit_t *dc)
{
	int i;
	sbit_t iB[456], q[8];

	for (i=0; i<4; i++)
		gsm0503_xcch_burst_unmap(&iB[i * 114], &bursts[i * 116],
					q + i*2, q + i*2 + 1);

	gsm0503_mcs1_ul_deinterleave(hc, dc, iB);

	return 0;
}

static int pdtch_edge_mcs5_demap(sbit_t *bursts, sbit_t *hc, sbit_t *dc)
{
	int i;
	sbit_t hi[136], di[1248];

	for (i=0; i<4; i++)
		gsm0503_mcs5_ul_burst_unmap(di, &bursts[i * 348], hi, i);

	gsm0503_mcs5_ul_deinterleave(hc, dc, hi, di);

	return 0;
}

static int _pdtch_edge_mcs7_demap(sbit_t *bursts, sbit_t *hi, sbit_t *di)
{
	int i;

	for (i=0; i<4; i++)
		gsm0503_mcs7_ul_burst_unmap(di, &bursts[i * 348], hi, i);

	return 0;
}

static int pdtch_edge_mcs7_demap(sbit_t *bursts, sbit_t *hc,
				sbit_t *c1, sbit_t *c2)
{
	sbit_t hi[124], di[1224];

	_pdtch_edge_mcs7_demap(bursts, hi, di);
	gsm0503_mcs7_ul_deinterleave(hc, c1, c2, hi, di);

	return 0;
}

static int pdtch_edge_mcs8_demap(sbit_t *bursts, sbit_t *hc,
				sbit_t *c1, sbit_t *c2)
{
	sbit_t hi[124], di[1224];

	_pdtch_edge_mcs7_demap(bursts, hi, di);
	gsm0503_mcs8_ul_deinterleave(hc, c1, c2, hi, di);

	return 0;
}

static int pdtch_edge_mcs1_decode_hdr(sbit_t *hc)
{
	sbit_t C[117];
	ubit_t upp[39];
	int rc, i, j, n_errors, n_bits_total;

	for (i=116, j=79; i>=0; i--) {
		if (!gsm0503_puncture_mcs1_ul_hdr[i])
			C[i] = hc[j--];
		else
			C[i] = 0;
	}

	osmo_conv_decode_ber(&gsm0503_conv_mcs1_ul_hdr, C, upp,
			&n_errors, &n_bits_total);
	rc = osmo_crc8gen_check_bits(&gsm0503_mcs_crc8_hdr,
			upp, 31, upp+31);
	if (rc)
		return -1;

	return 0;
}

static int pdtch_edge_mcs5_decode_hdr(sbit_t *hc)
{
	ubit_t upp[45];
	int rc, n_errors, n_bits_total;

	osmo_conv_decode_ber(&gsm0503_conv_mcs5_ul_hdr, hc, upp,
			&n_errors, &n_bits_total);
	rc = osmo_crc8gen_check_bits(&gsm0503_mcs_crc8_hdr,
				upp, 37, upp+37);
	if (rc)
		return -1;

	return 0;
}

static int pdtch_edge_mcs7_decode_hdr(sbit_t *hc)
{
	sbit_t C[162];
	ubit_t upp[54];
	int rc, i, j, n_errors, n_bits_total;

	for (i=161, j=159; i>=0; i--) {
		if (!gsm0503_puncture_mcs7_ul_hdr[i])
			C[i] = hc[j--];
		else
			C[i] = 0;
	}

	osmo_conv_decode_ber(&gsm0503_conv_mcs7_ul_hdr, C, upp,
			&n_errors, &n_bits_total);
	rc = osmo_crc8gen_check_bits(&gsm0503_mcs_crc8_hdr,
			upp, 46, upp+46);
	if (rc)
		return -1;

	return 0;
}

int pdtch_edge_decode(uint8_t *l2_data, sbit_t *bursts, int mcs,
		      int *n_errors, int *n_bits_total)
{
	sbit_t hc[160], dc[1248], C[1836], c1[612], c2[612];
	ubit_t u[612];

	int rc, i, j;

	switch (mcs) {
	case 1:
		pdtch_edge_mcs1_demap(bursts, hc, dc);
		pdtch_edge_mcs1_decode_hdr(hc);

		for (i=587, j=371; i>=0; i--) {
			if (!gsm0503_puncture_mcs1_p1[i])
				C[i] = dc[j--];
			else
				C[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs1, C, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 178, u+178);
		if (rc)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, u, 0, 178, 1);
		return 27;
	case 2:
		pdtch_edge_mcs1_demap(bursts, hc, dc);
		pdtch_edge_mcs1_decode_hdr(hc);

		for (i=731, j=371; i>=0; i--) {
			if (!gsm0503_puncture_mcs2_p1[i])
				dc[i] = dc[j--];
			else
				dc[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs2, dc, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 226, u+226);
		if (rc)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, u, 3, 226, 1);
		return 33;
	case 3:
		pdtch_edge_mcs1_demap(bursts, hc, dc);
		pdtch_edge_mcs1_decode_hdr(hc);

		for (i=947, j=371; i>=0; i--) {
			if (!gsm0503_puncture_mcs3_p1[i])
				dc[i] = dc[j--];
			else
				dc[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs3, dc, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 298, u+298);
		if (rc)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, u, 3, 298, 1);
		return 42;
	case 4:
		pdtch_edge_mcs1_demap(bursts, hc, dc);
		pdtch_edge_mcs1_decode_hdr(hc);

		for (i=1115, j=371; i>=0; i--) {
			if (!gsm0503_puncture_mcs4_p1[i])
				dc[i] = dc[j--];
			else
				dc[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs4, dc, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 354, u+354);
		if (rc)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, u, 3, 354, 1);
		return 49;
	case 5:
		pdtch_edge_mcs5_demap(bursts, hc, dc);
		pdtch_edge_mcs5_decode_hdr(hc);

		for (i=1403, j=1247; i>=0; i--) {
			if (!gsm0503_puncture_mcs5_p1[i])
				dc[i] = dc[j--];
			else
				dc[i] = 0;
		}
		osmo_conv_decode_ber(&gsm0503_conv_mcs5, dc, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 450, u+450);
		if (rc)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, u, 3, 450, 1);
		return 61;
	case 6:
		pdtch_edge_mcs5_demap(bursts, hc, dc);
		pdtch_edge_mcs5_decode_hdr(hc);

		for (i=1835, j=1247; i>=0; i--) {
			if (!gsm0503_puncture_mcs6_p1[i])
				dc[i] = dc[j--];
			else
				dc[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs6, dc, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 594, u+594);
		if (rc)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, u, 3, 594, 1);
		return 79;
	case 7:
		pdtch_edge_mcs7_demap(bursts, hc, c1, c2);
		pdtch_edge_mcs7_decode_hdr(hc);

		/* Block 1 */
		for (i=1403, j=611; i>=0; i--) {
			if (!gsm0503_puncture_mcs7_p1[i])
				c1[i] = c1[j--];
			else
				c1[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs7, c1, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 450, u+450);
		if (rc)
			return -1;

		/* Block 2 */
		for (i=1403, j=611; i>=0; i--) {
			if (!gsm0503_puncture_mcs7_p1[i])
				c2[i] = c2[j--];
			else
				c2[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs7, c2, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 450, u+450);
		if (rc)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, u, 3, 450 + 450, 1);
		return 119;
	case 8:
		pdtch_edge_mcs8_demap(bursts, hc, c1, c2);
		pdtch_edge_mcs7_decode_hdr(hc);

		/* Block 1 */
		for (i=1691, j=611; i>=0; i--) {
			if (!gsm0503_puncture_mcs8_p1[i])
				c1[i] = c1[j--];
			else
				c1[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs8, c1, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 546, u+546);
		if (rc)
			return -1;

		/* Block 2 */
		for (i=1691, j=611; i>=0; i--) {
			if (!gsm0503_puncture_mcs8_p1[i])
				c2[i] = c2[j--];
			else
				c2[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs8, c2, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 546, u+546);
		if (rc)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, u, 3, 546 + 546, 1);
		return 143;
	case 9:
		pdtch_edge_mcs8_demap(bursts, hc, c1, c2);
		pdtch_edge_mcs7_decode_hdr(hc);

		/* Block 1 */
		for (i=1835, j=611; i>=0; i--) {
			if (!gsm0503_puncture_mcs9_p1[i])
				c1[i] = c1[j--];
			else
				c1[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs9, c1, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 594, u+594);
		if (rc)
			return -1;

		/* Block 2 */
		for (i=1835, j=611; i>=0; i--) {
			if (!gsm0503_puncture_mcs9_p1[i])
				c2[i] = c2[j--];
			else
				c2[i] = 0;
		}

		osmo_conv_decode_ber(&gsm0503_conv_mcs9, c2, u,
				n_errors, n_bits_total);
		rc = osmo_crc16gen_check_bits(&gsm0503_mcs_crc12,
				u, 594, u+594);
		if (rc)
			return -1;

		osmo_ubit2pbit_ext(l2_data, 0, u, 3, 594 + 594, 1);
		return 155;
	default:
		*n_bits_total = 0;
		*n_errors = 0;
		break;
	}

	return -1;
}

static int pdtch_gprs_interleave(ubit_t *bursts, ubit_t *cB,
				const ubit_t *hl_hn)
{
	int i;
	ubit_t iB[456];

	gsm0503_xcch_interleave(cB, iB);

	for (i=0; i<4; i++)
		gsm0503_xcch_burst_map(&iB[i * 114], &bursts[i * 116],
			hl_hn + i*2, hl_hn + i*2 + 1);

	return 0;
}

static int pdtch_edge_mcs1_interleave(ubit_t *bursts, ubit_t *hc,
				ubit_t *dc, int usf, const ubit_t *hl_hn)
{
	int i;
	ubit_t iB[456];

	gsm0503_mcs1_dl_interleave(gsm0503_usf2six[usf], hc, dc, iB);

	for (i=0; i<4; i++)
		gsm0503_xcch_burst_map(&iB[i * 114], &bursts[i * 116],
			hl_hn + i*2, hl_hn + i*2 + 1);

	return 0;
}

static int pdtch_edge_mcs5_interleave(ubit_t *bursts, ubit_t *hc,
					ubit_t *dc, ubit_t *up)
{
	int i;
	ubit_t hi[100], di[1248];

	gsm0503_mcs5_dl_interleave(hc, dc, hi, di);
	for (i=0; i<4; i++)
		gsm0503_mcs5_dl_burst_map(di, &bursts[i*348], hi, up, i);

	return 0;
}

static int pdtch_edge_mcs7_map(ubit_t *bursts, ubit_t *hi,
				ubit_t *di, ubit_t *up)
{
	int i;

	for (i=0; i<4; i++)
		gsm0503_mcs7_dl_burst_map(di, &bursts[i*348], hi, up, i);

	return 0;
}

static int pdtch_edge_mcs7_interleave(ubit_t *bursts, ubit_t *hc,
					ubit_t *c1, ubit_t *c2, ubit_t *up)
{
	ubit_t hi[124], di[1224];

	gsm0503_mcs7_dl_interleave(hc, c1, c2, hi, di);
	pdtch_edge_mcs7_map(bursts, hi, di, up);

	return 0;
}

static int pdtch_edge_mcs8_interleave(ubit_t *bursts, ubit_t *hc,
					ubit_t *c1, ubit_t *c2, ubit_t *up)
{
	ubit_t hi[124], di[1224];

	gsm0503_mcs8_dl_interleave(hc, c1, c2, hi, di);
	pdtch_edge_mcs7_map(bursts, hi, di, up);

	return 0;
}

static void pdtch_mcs5_usf_precode(ubit_t *up, int usf)
{
        memcpy(up, gsm0503_mcs5_usf_precode_table[usf], 36 * sizeof(ubit_t));
}

static int pdtch_edge_mcs1_hdr_encode(ubit_t *hc, uint8_t *l2_data)
{
	int i, j;
	ubit_t upp[36], C[108];

	/* Header */
	osmo_pbit2ubit_ext(upp, 0, l2_data, 3, 28, 1);
	osmo_crc8gen_set_bits(&gsm0503_mcs_crc8_hdr, upp, 28, upp+28);
	osmo_conv_encode(&gsm0503_conv_mcs1_dl_hdr, upp, C);

	for (i=0, j=0; i<108; i++)
		if (!gsm0503_puncture_mcs1_dl_hdr[i])
			hc[j++] = C[i];
	return 0;
}

static int pdtch_edge_mcs5_hdr_encode(ubit_t *hc, uint8_t *l2_data)
{
	int usf;
	ubit_t up[36], upp[33];

	usf = l2_data[0] & 0x7;
	pdtch_mcs5_usf_precode(up, usf);

	osmo_pbit2ubit_ext(upp, 0, l2_data, 3, 25, 1);
	osmo_crc8gen_set_bits(&gsm0503_mcs_crc8_hdr, upp, 25, upp+25);
	osmo_conv_encode(&gsm0503_conv_mcs5_dl_hdr, upp, hc);

	hc[99] = hc[98];

	return 0;
}

static int pdtch_edge_mcs7_hdr_encode(ubit_t *hc, uint8_t *l2_data)
{
	int i, j, usf;
	ubit_t up[36], upp[45], C[135];

	usf = l2_data[0] & 0x7;
	pdtch_mcs5_usf_precode(up, usf);

	osmo_pbit2ubit_ext(upp, 0, l2_data, 3, 37, 1);
	osmo_crc8gen_set_bits(&gsm0503_mcs_crc8_hdr, upp, 37, upp+37);
	osmo_conv_encode(&gsm0503_conv_mcs7_dl_hdr, upp, C);

	for (i=0, j=0; i<135; i++)
		if (!gsm0503_puncture_mcs7_dl_hdr[i])
			hc[j++] = C[i];
	return 0;
}

int pdtch_encode(ubit_t *bursts, uint8_t *l2_data, uint8_t l2_len)
{
	ubit_t cB[676];
	ubit_t up[36], hc[124];
	ubit_t u[612], C[1836], dc[1248], c1[612], c2[612];
	const ubit_t *hl_hn;
	int i, j, usf;

	usf = l2_data[0] & 0x7;

	switch (l2_len) {
	case 23:
		/* GPRS CS-1 */
		osmo_pbit2ubit_ext(C, 0, l2_data, 0, 184, 1);

		osmo_crc64gen_set_bits(&gsm0503_fire_crc40, C, 184, C+184);

		osmo_conv_encode(&gsm0503_conv_xcch, C, cB);

		hl_hn = gsm0503_pdtch_hl_hn_ubit[0];
		pdtch_gprs_interleave(bursts, cB, hl_hn);
		break;
	case 34:
		/* GPRS CS-2 */
		osmo_pbit2ubit_ext(C, 3, l2_data, 0, 271, 1);

		osmo_crc16gen_set_bits(&gsm0503_cs234_crc16, C+3, 271, C+3+271);

		memcpy(C, gsm0503_usf2six[usf], 6);

		osmo_conv_encode(&gsm0503_conv_cs2, C, cB);

		for (i=0, j=0; i<588; i++)
			if (!gsm0503_puncture_cs2[i])
				cB[j++] = cB[i];

		hl_hn = gsm0503_pdtch_hl_hn_ubit[1];
		pdtch_gprs_interleave(bursts, cB, hl_hn);
		break;
	case 40:
		/* GPRS CS-3 */
		osmo_pbit2ubit_ext(C, 3, l2_data, 0, 315, 1);

		osmo_crc16gen_set_bits(&gsm0503_cs234_crc16, C+3, 315, C+3+315);

		memcpy(C, gsm0503_usf2six[usf], 6);

		osmo_conv_encode(&gsm0503_conv_cs3, C, cB);

		for (i=0, j=0; i<676; i++)
			if (!gsm0503_puncture_cs3[i])
				cB[j++] = cB[i];

		hl_hn = gsm0503_pdtch_hl_hn_ubit[2];
		pdtch_gprs_interleave(bursts, cB, hl_hn);
		break;
	case 54:
		/* GPRS CS-4 */
		osmo_pbit2ubit_ext(cB, 9, l2_data, 0, 431, 1);

		osmo_crc16gen_set_bits(&gsm0503_cs234_crc16, cB+9, 431,
			cB+9+431);

		memcpy(cB, gsm0503_usf2twelve_ubit[usf], 12);

		hl_hn = gsm0503_pdtch_hl_hn_ubit[3];
		pdtch_gprs_interleave(bursts, cB, hl_hn);
		break;
	case 27:
		/* EDGE MCS-1 DL */
		pdtch_edge_mcs1_hdr_encode(hc, l2_data);

		/* Data */
		osmo_pbit2ubit_ext(u, 0, l2_data, 31, 178, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 178, u+178);
		osmo_conv_encode(&gsm0503_conv_mcs1, u, C);

		for (i=0, j=0; i<588; i++)
			if (!gsm0503_puncture_mcs1_p1[i])
				dc[j++] = C[i];

		hl_hn = gsm0503_pdtch_hl_hn_ubit[3];
		pdtch_edge_mcs1_interleave(bursts, hc, dc, usf, hl_hn);
		break;
	case 33:
		/* EDGE MCS-2 DL */
		pdtch_edge_mcs1_hdr_encode(hc, l2_data);

		osmo_pbit2ubit_ext(u, 0, l2_data, 31, 226, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 226, u+226);
		osmo_conv_encode(&gsm0503_conv_mcs2, u, C);

		for (i=0, j=0; i<732; i++)
			if (!gsm0503_puncture_mcs2_p1[i])
				dc[j++] = C[i];

		hl_hn = gsm0503_pdtch_hl_hn_ubit[3];
		pdtch_edge_mcs1_interleave(bursts, hc, dc, usf, hl_hn);
		break;
	case 42:
		/* EDGE MCS-3 DL */
		pdtch_edge_mcs1_hdr_encode(hc, l2_data);

		osmo_pbit2ubit_ext(u, 0, l2_data, 31, 298, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 298, u+298);
		osmo_conv_encode(&gsm0503_conv_mcs3, u, C);

		for (i=0, j=0; i<948; i++)
			if (!gsm0503_puncture_mcs3_p1[i])
				dc[j++] = C[i];

		hl_hn = gsm0503_pdtch_hl_hn_ubit[3];
		pdtch_edge_mcs1_interleave(bursts, hc, dc, usf, hl_hn);
		break;
	case 49:
		/* EDGE MCS-4 DL */
		pdtch_edge_mcs1_hdr_encode(hc, l2_data);

		for (i=0, j=0; i<108; i++)
			if (!gsm0503_puncture_mcs1_dl_hdr[i])
				hc[j++] = C[i];

		osmo_pbit2ubit_ext(u, 0, l2_data, 31, 354, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 354, u+354);
		osmo_conv_encode(&gsm0503_conv_mcs4, u, C);

		for (i=0, j=0; i<1116; i++)
			if (!gsm0503_puncture_mcs4_p1[i])
				dc[j++] = C[i];

		hl_hn = gsm0503_pdtch_hl_hn_ubit[3];
		pdtch_edge_mcs1_interleave(bursts, hc, dc, usf, hl_hn);
		break;
	case 60:
		/* EDGE MCS-5 DL bits*/
		pdtch_edge_mcs5_hdr_encode(hc, l2_data);

		osmo_pbit2ubit_ext(u, 0, l2_data, 28, 450, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 450, u+450);
		osmo_conv_encode(&gsm0503_conv_mcs5, u, C);

		for (i=0, j=0; i<1404; i++)
			if (!gsm0503_puncture_mcs5_p1[i])
				dc[j++] = C[i];

		pdtch_edge_mcs5_interleave(bursts, hc, dc, up);
		break;
	case 78:
		/* EDGE MCS-6 DL */
		pdtch_edge_mcs5_hdr_encode(hc, l2_data);

		osmo_pbit2ubit_ext(u, 0, l2_data, 3+25, 594, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 594, u+594);
		osmo_conv_encode(&gsm0503_conv_mcs6, u, C);

		for (i=0, j=0; i<1836; i++)
			if (!gsm0503_puncture_mcs6_p1[i])
				dc[j++] = C[i];

		pdtch_edge_mcs5_interleave(bursts, hc, dc, up);
		break;
	case 118:
		/* EDGE MCS-7 DL */
		pdtch_edge_mcs7_hdr_encode(hc, l2_data);

		osmo_pbit2ubit_ext(u, 0, l2_data, 3+37, 450, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 450, u+450);
		osmo_conv_encode(&gsm0503_conv_mcs7, u, C);

		for (i=0, j=0; i<1404; i++)
			if (!gsm0503_puncture_mcs7_p1[i])
				c1[j++] = C[i];

		osmo_pbit2ubit_ext(u, 0, l2_data, 3+37+450, 450, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 450, u+450);
		osmo_conv_encode(&gsm0503_conv_mcs7, u, C);

		for (i=0, j=0; i<1404; i++)
			if (!gsm0503_puncture_mcs7_p1[i])
				c2[j++] = C[i];

		pdtch_edge_mcs7_interleave(bursts, hc, c1, c2, up);
		break;
	case 142:
		/* EDGE MCS-8 DL */
		pdtch_edge_mcs7_hdr_encode(hc, l2_data);

		for (i=0, j=0; i<135; i++)
			if (!gsm0503_puncture_mcs7_dl_hdr[i])
				hc[j++] = C[i];

		osmo_pbit2ubit_ext(u, 0, l2_data, 40, 546, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 546, u+546);
		osmo_conv_encode(&gsm0503_conv_mcs8, u, C);

		for (i=0, j=0; i<1692; i++)
			if (!gsm0503_puncture_mcs8_p1[i])
				c1[j++] = C[i];

		osmo_pbit2ubit_ext(u, 0, l2_data, 40+546, 546, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 546, u+546);
		osmo_conv_encode(&gsm0503_conv_mcs8, u, C);

		for (i=0, j=0; i<1692; i++)
			if (!gsm0503_puncture_mcs8_p1[i])
				c2[j++] = C[i];

		pdtch_edge_mcs8_interleave(bursts, hc, c1, c2, up);
		break;
	case 154:
		/* EDGE MCS-9 DL */
		pdtch_edge_mcs7_hdr_encode(hc, l2_data);

		osmo_pbit2ubit_ext(u, 0, l2_data, 40, 594, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 594, u+594);
		osmo_conv_encode(&gsm0503_conv_mcs9, u, C);

		for (i=0, j=0; i<1836; i++)
			if (!gsm0503_puncture_mcs9_p1[i])
				c1[j++] = C[i];

		osmo_pbit2ubit_ext(u, 0, l2_data, 40+594, 594, 1);
		osmo_crc16gen_set_bits(&gsm0503_mcs_crc12, u, 594, u+594);
		osmo_conv_encode(&gsm0503_conv_mcs9, u, C);

		for (i=0, j=0; i<1836; i++)
			if (!gsm0503_puncture_mcs9_p1[i])
				c2[j++] = C[i];

		pdtch_edge_mcs8_interleave(bursts, hc, c1, c2, up);
		break;
	default:
		return -1;
	}

	return 0;
}

/*
 * GSM TCH/F FR/EFR transcoding
 */

static void tch_fr_reassemble(uint8_t *tch_data, ubit_t *b_bits, int net_order)
{
	int i, j, k, l, o;

	tch_data[0] = 0xd << 4;
	memset(tch_data + 1, 0, 32);

	if (net_order) {
		i = 0; /* counts bits */
		j = 4; /* counts output bits */
		while (i < 260) {
			tch_data[j>>3] |= (b_bits[i] << (7-(j&7)));
			i++;
			j++;
		}
		return;
	}

	/* reassemble d-bits */
	i = 0; /* counts bits */
	j = 4; /* counts output bits */
	k = gsm0503_gsm_fr_map[0]-1; /* current number bit in element */
	l = 0; /* counts element bits */
	o = 0; /* offset input bits */
	while (i < 260) {
		tch_data[j>>3] |= (b_bits[k+o] << (7-(j&7)));
		if (--k < 0) {
			o += gsm0503_gsm_fr_map[l];
			k = gsm0503_gsm_fr_map[++l]-1;
		}
		i++;
		j++;
	}
}

static void tch_fr_disassemble(ubit_t *b_bits, uint8_t *tch_data, int net_order)
{
	int i, j, k, l, o;

	if (net_order) {
		i = 0; /* counts bits */
		j = 4; /* counts output bits */
		while (i < 260) {
			b_bits[i] = (tch_data[j>>3] >> (7-(j&7))) & 1;
			i++;
			j++;
		}
		return;
	}

	i = 0; /* counts bits */
	j = 4; /* counts input bits */
	k = gsm0503_gsm_fr_map[0]-1; /* current number bit in element */
	l = 0; /* counts element bits */
	o = 0; /* offset output bits */
	while (i < 260) {
		b_bits[k+o] = (tch_data[j>>3] >> (7-(j&7))) & 1;
		if (--k < 0) {
			o += gsm0503_gsm_fr_map[l];
			k = gsm0503_gsm_fr_map[++l]-1;
		}
		i++;
		j++;
	}
}

static void tch_hr_reassemble(uint8_t *tch_data, ubit_t *b_bits)
{
	int i, j;

	tch_data[0] = 0x00; /* F = 0, FT = 000 */
	memset(tch_data + 1, 0, 14);

	i = 0; /* counts bits */
	j = 8; /* counts output bits */
	while (i < 112) {
		tch_data[j>>3] |= (b_bits[i] << (7-(j&7)));
		i++;
		j++;
	}
}

static void tch_hr_disassemble(ubit_t *b_bits, uint8_t *tch_data)
{
	int i, j;

	i = 0; /* counts bits */
	j = 8; /* counts output bits */
	while (i < 112) {
		b_bits[i] = (tch_data[j>>3] >> (7-(j&7))) & 1;
		i++;
		j++;
	}
}

static void tch_efr_reassemble(uint8_t *tch_data, ubit_t *b_bits)
{
	int i, j;

	tch_data[0] = 0xc << 4;
	memset(tch_data + 1, 0, 30);

	i = 0; /* counts bits */
	j = 4; /* counts output bits */
	while (i < 244) {
		tch_data[j>>3] |= (b_bits[i] << (7-(j&7)));
		i++;
		j++;
	}
}

static void tch_efr_disassemble(ubit_t *b_bits, uint8_t *tch_data)
{
	int i, j;

	i = 0; /* counts bits */
	j = 4; /* counts output bits */
	while (i < 244) {
		b_bits[i] = (tch_data[j>>3] >> (7-(j&7))) & 1;
		i++;
		j++;
	}
}

static void tch_amr_reassemble(uint8_t *tch_data, ubit_t *d_bits, int len)
{
	int i, j;

	memset(tch_data, 0, (len + 7) >> 3);

	i = 0; /* counts bits */
	j = 0; /* counts output bits */
	while (i < len) {
		tch_data[j>>3] |= (d_bits[i] << (7-(j&7)));
		i++;
		j++;
	}
}

static void tch_amr_disassemble(ubit_t *d_bits, uint8_t *tch_data, int len)
{
	int i, j;

	i = 0; /* counts bits */
	j = 0; /* counts output bits */
	while (i < len) {
		d_bits[i] = (tch_data[j>>3] >> (7-(j&7))) & 1;
		i++;
		j++;
	}
}

static void tch_fr_d_to_b(ubit_t *b_bits, ubit_t *d_bits)
{
	int i;

	for (i = 0; i < 260; i++)
		b_bits[gsm610_bitorder[i]] = d_bits[i];
}

static void tch_fr_b_to_d(ubit_t *d_bits, ubit_t *b_bits)
{
	int i;

	for (i = 0; i < 260; i++)
		d_bits[i] = b_bits[gsm610_bitorder[i]];
}

static void tch_hr_d_to_b(ubit_t *b_bits, ubit_t *d_bits)
{
	int i;

	const uint16_t *map;

	if (!d_bits[93] && !d_bits[94])
		map = gsm620_unvoiced_bitorder;
	else
		map = gsm620_voiced_bitorder;

	for (i = 0; i < 112; i++)
		b_bits[map[i]] = d_bits[i];
}

static void tch_hr_b_to_d(ubit_t *d_bits, ubit_t *b_bits)
{
	int i;
	const uint16_t *map;

	if (!b_bits[34] && !b_bits[35])
		map = gsm620_unvoiced_bitorder;
	else
		map = gsm620_voiced_bitorder;

	for (i = 0; i < 112; i++)
		d_bits[i] = b_bits[map[i]];
}

static void tch_efr_d_to_w(ubit_t *b_bits, ubit_t *d_bits)
{
	int i;

	for (i = 0; i < 260; i++)
		b_bits[gsm660_bitorder[i]] = d_bits[i];
}

static void tch_efr_w_to_d(ubit_t *d_bits, ubit_t *b_bits)
{
	int i;

	for (i = 0; i < 260; i++)
		d_bits[i] = b_bits[gsm660_bitorder[i]];
}

static void tch_efr_protected(ubit_t *s_bits, ubit_t *b_bits)
{
	int i;

	for (i = 0; i < 65; i++)
		b_bits[i] = s_bits[gsm0503_gsm_efr_protected_bits[i]-1];
}

static void tch_fr_unreorder(ubit_t *d, ubit_t *p, ubit_t *u)
{
	int i;

	for (i=0; i<91; i++) {
		d[i<<1] = u[i];
		d[(i<<1)+1] = u[184-i];
	}
	for (i=0; i<3; i++)
		p[i] = u[91+i];
}

static void tch_fr_reorder(ubit_t *u, ubit_t *d, ubit_t *p)
{
	int i;

	for (i=0; i<91; i++) {
		u[i] = d[i<<1];
		u[184-i] = d[(i<<1)+1];
	}
	for (i=0; i<3; i++)
		u[91+i] = p[i];
}

static void tch_hr_unreorder(ubit_t *d, ubit_t *p, ubit_t *u)
{
	memcpy(d, u, 95);
	memcpy(p, u+95, 3);
}

static void tch_hr_reorder(ubit_t *u, ubit_t *d, ubit_t *p)
{
	memcpy(u, d, 95);
	memcpy(u+95, p, 3);
}

static void tch_efr_reorder(ubit_t *w, ubit_t *s, ubit_t *p)
{
	memcpy(w, s, 71);
	w[71] = w[72] = s[69];
	memcpy(w+73, s+71, 50);
	w[123] = w[124] = s[119];
	memcpy(w+125, s+121, 53);
	w[178] = w[179] = s[172];
	memcpy(w+180, s+174, 50);
	w[230] = w[231] = s[222];
	memcpy(w+232, s+224, 20);
	memcpy(w+252, p, 8);
}

static void tch_efr_unreorder(ubit_t *s, ubit_t *p, ubit_t *w)
{
	int sum;

	memcpy(s, w, 71);
	sum = s[69] + w[71] + w[72];
	s[69] = (sum > 2);
	memcpy(s+71, w+73, 50);
	sum = s[119] + w[123] + w[124];
	s[119] = (sum > 2);
	memcpy(s+121, w+125, 53);
	sum = s[172] + w[178] + w[179];
	s[172] = (sum > 2);
	memcpy(s+174, w+180, 50);
	sum = s[220] + w[230] + w[231];
	s[222] = (sum > 2);
	memcpy(s+224, w+232, 20);
	memcpy(p, w+252, 8);
}

static void tch_amr_merge(ubit_t *u, ubit_t *d, ubit_t *p, int len, int prot)
{
	memcpy(u, d, prot);
	memcpy(u+prot, p, 6);
	memcpy(u+prot+6, d+prot, len-prot);
}

static void tch_amr_unmerge(ubit_t *d, ubit_t *p, ubit_t *u, int len, int prot)
{
	memcpy(d, u, prot);
	memcpy(p, u+prot, 6);
	memcpy(d+prot, u+prot+6, len-prot);
}

int tch_fr_decode(uint8_t *tch_data, sbit_t *bursts, int net_order, int efr,
	int *n_errors, int *n_bits_total)
{
	sbit_t iB[912], cB[456], h;
	ubit_t conv[185], s[244], w[260], b[65], d[260], p[8];
	int i, rv, len, steal = 0;

	for (i=0; i<8; i++) {
		gsm0503_tch_burst_unmap(&iB[i * 114], &bursts[i * 116], &h,
			i>>2);
		steal -= h;
	}

	gsm0503_tch_fr_deinterleave(cB, iB);

	if (steal > 0) {
		rv = _xcch_decode_cB(tch_data, cB, n_errors, n_bits_total);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_fr_decode(): error decoding  FACCH frame (%d/%d bits)\n", *n_errors, *n_bits_total);
			return -1;
		}

		return 23;
	}

	osmo_conv_decode_ber(&gsm0503_conv_tch_fr, cB, conv, n_errors, n_bits_total);

	tch_fr_unreorder(d, p, conv);

	for (i=0; i<78; i++)
		d[i+182] = (cB[i+378] < 0) ? 1:0;

	rv = osmo_crc8gen_check_bits(&gsm0503_tch_fr_crc3, d, 50, p);
	if (rv) {
		LOGP(DL1C, LOGL_NOTICE, "tch_fr_decode(): error checking CRC8 for the FR part of an %s frame\n", efr?"EFR":"FR");
		return -1;
	}


	if (efr) {
		tch_efr_d_to_w(w, d);

		tch_efr_unreorder(s, p, w);

		tch_efr_protected(s, b);

		rv = osmo_crc8gen_check_bits(&gsm0503_tch_efr_crc8, b,
			65, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_fr_decode(): error checking CRC8 for the EFR part of an EFR frame\n");
			return -1;
		}

		tch_efr_reassemble(tch_data, s);

		len = GSM_EFR_BYTES;
	} else {
		tch_fr_d_to_b(w, d);

		tch_fr_reassemble(tch_data, w, net_order);

		len = GSM_FR_BYTES;
	}

	return len;
}

int tch_fr_encode(ubit_t *bursts, uint8_t *tch_data, int len, int net_order)
{
	ubit_t iB[912], cB[456], h;
	ubit_t conv[185], w[260], b[65], s[244], d[260], p[8];
	int i;

	switch (len) {
	case GSM_EFR_BYTES: /* TCH EFR */

		tch_efr_disassemble(s, tch_data);

		tch_efr_protected(s, b);

		osmo_crc8gen_set_bits(&gsm0503_tch_efr_crc8, b, 65, p);

		tch_efr_reorder(w, s, p);

		tch_efr_w_to_d(d, w);

		goto coding_efr_fr;
	case GSM_FR_BYTES: /* TCH FR */
		tch_fr_disassemble(w, tch_data, net_order);

		tch_fr_b_to_d(d, w);

coding_efr_fr:
		osmo_crc8gen_set_bits(&gsm0503_tch_fr_crc3, d, 50, p);

		tch_fr_reorder(conv, d, p);

		memcpy(cB+378, d+182, 78);

		osmo_conv_encode(&gsm0503_conv_tch_fr, conv, cB);

		h = 0;

		break;
	case GSM_MACBLOCK_LEN: /* FACCH */
		_xcch_encode_cB(cB, tch_data);

		h = 1;

		break;
	default:
		return -1;
	}

	gsm0503_tch_fr_interleave(cB, iB);

	for (i=0; i<8; i++)
		gsm0503_tch_burst_map(&iB[i * 114], &bursts[i * 116], &h, i>>2);

	return 0;
}

int tch_hr_decode(uint8_t *tch_data, sbit_t *bursts, int odd,
	int *n_errors, int *n_bits_total)
{
	sbit_t iB[912], cB[456], h;
	ubit_t conv[98], b[112], d[112], p[3];
	int i, rv, steal = 0;

	/* only unmap the stealing bits */
	if (!odd) {
		for (i=0; i<4; i++) {
			gsm0503_tch_burst_unmap(NULL, &bursts[i * 116], &h, 0);
			steal -= h;
		}
		for (i=2; i<5; i++) {
			gsm0503_tch_burst_unmap(NULL, &bursts[i * 116], &h, 1);
			steal -= h;
		}
	}

	/* if we found a stole FACCH, but only at correct alignment */
	if (steal > 0) {
		for (i=0; i<6; i++)
			gsm0503_tch_burst_unmap(&iB[i * 114], &bursts[i * 116],
				NULL, i>>2);
		for (i=2; i<4; i++)
			gsm0503_tch_burst_unmap(&iB[i * 114 + 456],
				&bursts[i * 116], NULL, 1);

		gsm0503_tch_fr_deinterleave(cB, iB);

		rv = _xcch_decode_cB(tch_data, cB, n_errors, n_bits_total);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_hr_decode(): error decoding  FACCH frame (%d/%d bits)\n", *n_errors, *n_bits_total);
			return -1;
		}

		return GSM_MACBLOCK_LEN;
	}

	for (i=0; i<4; i++)
		gsm0503_tch_burst_unmap(&iB[i * 114], &bursts[i * 116], NULL,
			i>>1);

	gsm0503_tch_hr_deinterleave(cB, iB);

	osmo_conv_decode_ber(&gsm0503_conv_tch_hr, cB, conv, n_errors, n_bits_total);

	tch_hr_unreorder(d, p, conv);

	for (i=0; i<17; i++)
		d[i+95] = (cB[i+211] < 0) ? 1:0;

	rv = osmo_crc8gen_check_bits(&gsm0503_tch_fr_crc3, d + 73, 22, p);
	if (rv) {
		LOGP(DL1C, LOGL_NOTICE, "tch_fr_decode(): error checking CRC8 for an HR frame\n");
		return -1;
	}

	tch_hr_d_to_b(b, d);

	tch_hr_reassemble(tch_data, b);

	return 15;
}

int tch_hr_encode(ubit_t *bursts, uint8_t *tch_data, int len)
{
	ubit_t iB[912], cB[456], h;
	ubit_t conv[98], b[112], d[112], p[3];
	int i;

	switch (len) {
	case 15: /* TCH HR */
		tch_hr_disassemble(b, tch_data);

		tch_hr_b_to_d(d, b);

		osmo_crc8gen_set_bits(&gsm0503_tch_fr_crc3, d + 73, 22, p);

		tch_hr_reorder(conv, d, p);

		osmo_conv_encode(&gsm0503_conv_tch_hr, conv, cB);

		memcpy(cB+211, d+95, 17);

		h = 0;

		gsm0503_tch_hr_interleave(cB, iB);

		for (i=0; i<4; i++)
			gsm0503_tch_burst_map(&iB[i * 114], &bursts[i * 116],
				&h, i>>1);

		break;
	case GSM_MACBLOCK_LEN: /* FACCH */
		_xcch_encode_cB(cB, tch_data);

		h = 1;

		gsm0503_tch_fr_interleave(cB, iB);

		for (i=0; i<6; i++)
			gsm0503_tch_burst_map(&iB[i * 114], &bursts[i * 116],
				&h, i>>2);
		for (i=2; i<4; i++)
			gsm0503_tch_burst_map(&iB[i * 114 + 456],
				&bursts[i * 116], &h, 1);

		break;
	default:
		return -1;
	}

	return 0;
}

int tch_afs_decode(uint8_t *tch_data, sbit_t *bursts, int codec_mode_req,
	uint8_t *codec, int codecs, uint8_t *ft, uint8_t *cmr,
	int *n_errors, int *n_bits_total)
{
	sbit_t iB[912], cB[456], h;
	ubit_t d[244], p[6], conv[250];
	int i, j, k, best = 0, rv, len, steal = 0, id = 0;
	*n_errors = 0; *n_bits_total = 0;

	for (i=0; i<8; i++) {
		gsm0503_tch_burst_unmap(&iB[i * 114], &bursts[i * 116], &h,
			i>>2);
		steal -= h;
	}

	gsm0503_tch_fr_deinterleave(cB, iB);

	if (steal > 0) {
		rv = _xcch_decode_cB(tch_data, cB, n_errors, n_bits_total);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_afs_decode(): error decoding  FACCH frame (%d/%d bits)\n", *n_errors, *n_bits_total);
			return -1;
		}

		return GSM_MACBLOCK_LEN;
	}

	for (i=0; i<4; i++) {
		for (j=0, k=0; j<8; j++)
			k += abs(((int)gsm0503_afs_ic_sbit[i][j]) -
							((int)cB[j]));
		if (i == 0 || k < best) {
			best = k;
			id = i;
		}
	}

	/* check if indicated codec fits into range of codecs */
	if (id >= codecs) {
		/* codec mode out of range, return id */
		return id;
	}

	switch ((codec_mode_req) ? codec[*ft] : codec[id]) {
	case 7: /* TCH/AFS12.2 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_afs_12_2, cB+8, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 244, 81);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 81, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_afs_decode(): error checking CRC8 for an AMR 12.2 frame\n");
			return -1;
		}

		tch_amr_reassemble(tch_data, d, 244);

		len = 31;

		break;
	case 6: /* TCH/AFS10.2 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_afs_10_2, cB+8, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 204, 65);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 65, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_afs_decode(): error checking CRC8 for an AMR 10.2 frame\n");
			return -1;
		}

		tch_amr_reassemble(tch_data, d, 204);

		len = 26;

		break;
	case 5: /* TCH/AFS7.95 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_afs_7_95, cB+8, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 159, 75);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 75, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_afs_decode(): error checking CRC8 for an AMR 7.95 frame\n");
			return -1;
		}

		tch_amr_reassemble(tch_data, d, 159);

		len = 20;

		break;
	case 4: /* TCH/AFS7.4 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_afs_7_4, cB+8, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 148, 61);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 61, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_afs_decode(): error checking CRC8 for an AMR 7.4 frame\n");
			return -1;
		}

		tch_amr_reassemble(tch_data, d, 148);

		len = 19;

		break;
	case 3: /* TCH/AFS6.7 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_afs_6_7, cB+8, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 134, 55);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 55, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_afs_decode(): error checking CRC8 for an AMR 6.7 frame\n");
			return -1;
		}

		tch_amr_reassemble(tch_data, d, 134);

		len = 17;

		break;
	case 2: /* TCH/AFS5.9 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_afs_5_9, cB+8, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 118, 55);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 55, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_afs_decode(): error checking CRC8 for an AMR 5.9 frame\n");
			return -1;
		}

		tch_amr_reassemble(tch_data, d, 118);

		len = 15;

		break;
	case 1: /* TCH/AFS5.15 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_afs_5_15, cB+8, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 103, 49);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 49, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_afs_decode(): error checking CRC8 for an AMR 5.15 frame\n");
			return -1;
		}

		tch_amr_reassemble(tch_data, d, 103);

		len = 13;

		break;
	case 0: /* TCH/AFS4.75 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_afs_4_75, cB+8, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 95, 39);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 39, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_afs_decode(): error checking CRC8 for an AMR 4.75 frame\n");
			return -1;
		}

		tch_amr_reassemble(tch_data, d, 95);

		len = 12;

		break;
	default:
		LOGP(DL1C, LOGL_ERROR, "tch_afs_decode(): Unknown frame type\n");
		fprintf(stderr, "FIXME: FT %d not supported!\n", *ft);
		*n_bits_total = 448;
		*n_errors = *n_bits_total;
		return -1;
	}

	/* change codec request / indication, if frame is valid */
	if (codec_mode_req)
		*cmr = id;
	else
		*ft = id;

	return len;
}

int tch_afs_encode(ubit_t *bursts, uint8_t *tch_data, int len,
	int codec_mode_req, uint8_t *codec, int codecs, uint8_t ft,
	uint8_t cmr)
{
	ubit_t iB[912], cB[456], h;
	ubit_t d[244], p[6], conv[250];
	int i;
	uint8_t id;

	if (len == GSM_MACBLOCK_LEN) { /* FACCH */
		_xcch_encode_cB(cB, tch_data);

		h = 1;

		goto facch;
	}

	h = 0;

	if (codec_mode_req) {
		if (cmr >= codecs) {
			fprintf(stderr, "FIXME: CMR ID %d not in codec list!\n",
				cmr);
			return -1;
		}
		id = cmr;
	} else {
		if (ft >= codecs) {
			fprintf(stderr, "FIXME: FT ID %d not in codec list!\n",
				ft);
			return -1;
		}
		id = ft;
	}

	switch (codec[ft]) {
	case 7: /* TCH/AFS12.2 */
		if (len != 31) {
invalid_length:
			fprintf(stderr, "FIXME: payload length %d does not "
				"comply with codec type %d!\n", len, ft);
			return -1;
		}

		tch_amr_disassemble(d, tch_data, 244);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 81, p);

		tch_amr_merge(conv, d, p, 244, 81);

		osmo_conv_encode(&gsm0503_conv_tch_afs_12_2, conv, cB+8);

		break;
	case 6: /* TCH/AFS10.2 */
		if (len != 26)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 204);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 65, p);

		tch_amr_merge(conv, d, p, 204, 65);

		osmo_conv_encode(&gsm0503_conv_tch_afs_10_2, conv, cB+8);

		break;
	case 5: /* TCH/AFS7.95 */
		if (len != 20)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 159);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 75, p);

		tch_amr_merge(conv, d, p, 159, 75);

		osmo_conv_encode(&gsm0503_conv_tch_afs_7_95, conv, cB+8);

		break;
	case 4: /* TCH/AFS7.4 */
		if (len != 19)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 148);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 61, p);

		tch_amr_merge(conv, d, p, 148, 61);

		osmo_conv_encode(&gsm0503_conv_tch_afs_7_4, conv, cB+8);

		break;
	case 3: /* TCH/AFS6.7 */
		if (len != 17)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 134);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 55, p);

		tch_amr_merge(conv, d, p, 134, 55);

		osmo_conv_encode(&gsm0503_conv_tch_afs_6_7, conv, cB+8);

		break;
	case 2: /* TCH/AFS5.9 */
		if (len != 15)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 118);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 55, p);

		tch_amr_merge(conv, d, p, 118, 55);

		osmo_conv_encode(&gsm0503_conv_tch_afs_5_9, conv, cB+8);

		break;
	case 1: /* TCH/AFS5.15 */
		if (len != 13)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 103);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 49, p);

		tch_amr_merge(conv, d, p, 103, 49);

		osmo_conv_encode(&gsm0503_conv_tch_afs_5_15, conv, cB+8);

		break;
	case 0: /* TCH/AFS4.75 */
		if (len != 12)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 95);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 39, p);

		tch_amr_merge(conv, d, p, 95, 39);

		osmo_conv_encode(&gsm0503_conv_tch_afs_4_75, conv, cB+8);

		break;
	default:
		fprintf(stderr, "FIXME: FT %d not supported!\n", ft);

		return -1;
	}

	memcpy(cB, gsm0503_afs_ic_ubit[id], 8);

facch:
	gsm0503_tch_fr_interleave(cB, iB);

	for (i=0; i<8; i++)
		gsm0503_tch_burst_map(&iB[i * 114], &bursts[i * 116], &h, i>>2);

	return 0;
}

int tch_ahs_decode(uint8_t *tch_data, sbit_t *bursts, int odd,
	int codec_mode_req, uint8_t *codec, int codecs, uint8_t *ft,
	uint8_t *cmr, int *n_errors, int *n_bits_total)
{
	sbit_t iB[912], cB[456], h;
	ubit_t d[244], p[6], conv[135];
	int i, j, k, best = 0, rv, len, steal = 0, id = 0;

	/* only unmap the stealing bits */
	if (!odd) {
		for (i=0; i<4; i++) {
			gsm0503_tch_burst_unmap(NULL, &bursts[i * 116], &h, 0);
			steal -= h;
		}
		for (i=2; i<5; i++) {
			gsm0503_tch_burst_unmap(NULL, &bursts[i * 116], &h, 1);
			steal -= h;
		}
	}

	/* if we found a stole FACCH, but only at correct alignment */
	if (steal > 0) {
		for (i=0; i<6; i++)
			gsm0503_tch_burst_unmap(&iB[i * 114], &bursts[i * 116],
				NULL, i>>2);
		for (i=2; i<4; i++)
			gsm0503_tch_burst_unmap(&iB[i * 114 + 456],
				&bursts[i * 116], NULL, 1);

		gsm0503_tch_fr_deinterleave(cB, iB);

		rv = _xcch_decode_cB(tch_data, cB, n_errors, n_bits_total);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_ahs_decode(): error decoding  FACCH frame (%d/%d bits)\n", *n_errors, *n_bits_total);
			return -1;
		}

		return GSM_MACBLOCK_LEN;
	}

	for (i=0; i<4; i++)
		gsm0503_tch_burst_unmap(&iB[i * 114], &bursts[i * 116], NULL,
			i>>1);

	gsm0503_tch_hr_deinterleave(cB, iB);

	for (i=0; i<4; i++) {
		for (j=0, k=0; j<4; j++)
			k += abs(((int)gsm0503_ahs_ic_sbit[i][j]) -
							((int)cB[j]));
		if (i == 0 || k < best) {
			best = k;
			id = i;
		}
	}

	/* check if indicated codec fits into range of codecs */
	if (id >= codecs) {
		/* codec mode out of range, return id */
		return id;
	}

	switch ((codec_mode_req) ? codec[*ft] : codec[id]) {
	case 5: /* TCH/AHS7.95 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_ahs_7_95, cB+4, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 123, 67);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 67, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_ahs_decode(): error checking CRC8 for an AMR 7.95 frame\n");
			return -1;
		}

		for (i=0; i<36;i++)
			d[i+123] = (cB[i+192] < 0) ? 1:0;

		tch_amr_reassemble(tch_data, d, 159);

		len = 20;

		break;
	case 4: /* TCH/AHS7.4 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_ahs_7_4, cB+4, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 120, 61);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 61, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_ahs_decode(): error checking CRC8 for an AMR 7.4 frame\n");
			return -1;
		}

		for (i=0; i<28;i++)
			d[i+120] = (cB[i+200] < 0) ? 1:0;

		tch_amr_reassemble(tch_data, d, 148);

		len = 19;

		break;
	case 3: /* TCH/AHS6.7 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_ahs_6_7, cB+4, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 110, 55);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 55, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_ahs_decode(): error checking CRC8 for an AMR 6.7 frame\n");
			return -1;
		}

		for (i=0; i<24;i++)
			d[i+110] = (cB[i+204] < 0) ? 1:0;

		tch_amr_reassemble(tch_data, d, 134);

		len = 17;

		break;
	case 2: /* TCH/AHS5.9 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_ahs_5_9, cB+4, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 102, 55);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 55, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_ahs_decode(): error checking CRC8 for an AMR 5.9 frame\n");
			return -1;
		}

		for (i=0; i<16;i++)
			d[i+102] = (cB[i+212] < 0) ? 1:0;

		tch_amr_reassemble(tch_data, d, 118);

		len = 15;

		break;
	case 1: /* TCH/AHS5.15 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_ahs_5_15, cB+4, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 91, 49);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 49, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_ahs_decode(): error checking CRC8 for an AMR 5.15 frame\n");
			return -1;
		}

		for (i=0; i<12;i++)
			d[i+91] = (cB[i+216] < 0) ? 1:0;

		tch_amr_reassemble(tch_data, d, 103);

		len = 13;

		break;
	case 0: /* TCH/AHS4.75 */
		osmo_conv_decode_ber(&gsm0503_conv_tch_ahs_4_75, cB+4, conv, n_errors, n_bits_total);

		tch_amr_unmerge(d, p, conv, 83, 39);

		rv = osmo_crc8gen_check_bits(&gsm0503_amr_crc6, d, 39, p);
		if (rv) {
			LOGP(DL1C, LOGL_NOTICE, "tch_ahs_decode(): error checking CRC8 for an AMR 4.75 frame\n");
			return -1;
		}

		for (i=0; i<12;i++)
			d[i+83] = (cB[i+216] < 0) ? 1:0;

		tch_amr_reassemble(tch_data, d, 95);

		len = 12;

		break;
	default:
		LOGP(DL1C, LOGL_ERROR, "tch_afs_decode(): Unknown frame type\n");
		fprintf(stderr, "FIXME: FT %d not supported!\n", *ft);
		*n_bits_total = 159;
		*n_errors = *n_bits_total;
		return -1;
	}

	/* change codec request / indication, if frame is valid */
	if (codec_mode_req)
		*cmr = id;
	else
		*ft = id;

	return len;
}

int tch_ahs_encode(ubit_t *bursts, uint8_t *tch_data, int len,
	int codec_mode_req, uint8_t *codec, int codecs, uint8_t ft,
	uint8_t cmr)
{
	ubit_t iB[912], cB[456], h;
	ubit_t d[244], p[6], conv[135];
	int i;
	uint8_t id;

	if (len == GSM_MACBLOCK_LEN) { /* FACCH */
		_xcch_encode_cB(cB, tch_data);

		h = 1;

		gsm0503_tch_fr_interleave(cB, iB);

		for (i=0; i<6; i++)
			gsm0503_tch_burst_map(&iB[i * 114], &bursts[i * 116],
				&h, i>>2);
		for (i=2; i<4; i++)
			gsm0503_tch_burst_map(&iB[i * 114 + 456],
				&bursts[i * 116], &h, 1);

		return 0;
	}

	h = 0;

	if (codec_mode_req) {
		if (cmr >= codecs) {
			fprintf(stderr, "FIXME: CMR ID %d not in codec list!\n",
				cmr);
			return -1;
		}
		id = cmr;
	} else {
		if (ft >= codecs) {
			fprintf(stderr, "FIXME: FT ID %d not in codec list!\n",
				ft);
			return -1;
		}
		id = ft;
	}

	switch (codec[ft]) {
	case 5: /* TCH/AHS7.95 */
		if (len != 20) {
invalid_length:
			fprintf(stderr, "FIXME: payload length %d does not "
				"comply with codec type %d!\n", len, ft);
			return -1;
		}

		tch_amr_disassemble(d, tch_data, 159);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 67, p);

		tch_amr_merge(conv, d, p, 123, 67);

		osmo_conv_encode(&gsm0503_conv_tch_ahs_7_95, conv, cB+4);

		memcpy(cB+192, d+123, 36);

		break;
	case 4: /* TCH/AHS7.4 */
		if (len != 19)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 148);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 61, p);

		tch_amr_merge(conv, d, p, 120, 61);

		osmo_conv_encode(&gsm0503_conv_tch_ahs_7_4, conv, cB+4);

		memcpy(cB+200, d+120, 28);

		break;
	case 3: /* TCH/AHS6.7 */
		if (len != 17)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 134);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 55, p);

		tch_amr_merge(conv, d, p, 110, 55);

		osmo_conv_encode(&gsm0503_conv_tch_ahs_6_7, conv, cB+4);

		memcpy(cB+204, d+110, 24);

		break;
	case 2: /* TCH/AHS5.9 */
		if (len != 15)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 118);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 55, p);

		tch_amr_merge(conv, d, p, 102, 55);

		osmo_conv_encode(&gsm0503_conv_tch_ahs_5_9, conv, cB+4);

		memcpy(cB+212, d+102, 16);

		break;
	case 1: /* TCH/AHS5.15 */
		if (len != 13)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 103);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 49, p);

		tch_amr_merge(conv, d, p, 91, 49);

		osmo_conv_encode(&gsm0503_conv_tch_ahs_5_15, conv, cB+4);

		memcpy(cB+216, d+91, 12);

		break;
	case 0: /* TCH/AHS4.75 */
		if (len != 12)
			goto invalid_length;

		tch_amr_disassemble(d, tch_data, 95);

		osmo_crc8gen_set_bits(&gsm0503_amr_crc6, d, 39, p);

		tch_amr_merge(conv, d, p, 83, 39);

		osmo_conv_encode(&gsm0503_conv_tch_ahs_4_75, conv, cB+4);

		memcpy(cB+216, d+83, 12);

		break;
	default:
		fprintf(stderr, "FIXME: FT %d not supported!\n", ft);

		return -1;
	}

	memcpy(cB, gsm0503_afs_ic_ubit[id], 4);

	gsm0503_tch_hr_interleave(cB, iB);

	for (i=0; i<4; i++)
		gsm0503_tch_burst_map(&iB[i * 114], &bursts[i * 116], &h, i>>1);

	return 0;
}

/*
 * GSM RACH transcoding
 */

/*
 * GSM RACH apply BSIC to parity
 *
 * p(j) = p(j) xor b(j)     j = 0, ..., 5
 * b(0) = MSB of PLMN colour code
 * b(5) = LSB of BS colour code
 */

static int rach_apply_bsic(ubit_t *d, uint8_t bsic)
{
	int i;

	/* Apply it */
	for (i=0; i<6; i++)
		d[8+i] ^= ((bsic >> (5-i)) & 1);

	return 0;
}

int rach_decode(uint8_t *ra, sbit_t *burst, uint8_t bsic)
{
	ubit_t conv[14];
	int rv;

	osmo_conv_decode(&gsm0503_conv_rach, burst, conv);

	rach_apply_bsic(conv, bsic);

	rv = osmo_crc8gen_check_bits(&gsm0503_rach_crc6, conv, 8, conv+8);
	if (rv)
		return -1;

	osmo_ubit2pbit_ext(ra, 0, conv, 0, 8, 1);

	return 0;
}

int rach_encode(ubit_t *burst, uint8_t *ra, uint8_t bsic)
{
	ubit_t conv[14];

	osmo_pbit2ubit_ext(conv, 0, ra, 0, 8, 1);

	osmo_crc8gen_set_bits(&gsm0503_rach_crc6, conv, 8, conv+8);

	rach_apply_bsic(conv, bsic);

	osmo_conv_encode(&gsm0503_conv_rach, conv, burst);

	return 0;
}


/*
 * GSM SCH transcoding
 */

int sch_decode(uint8_t *sb_info, sbit_t *burst)
{
	ubit_t conv[35];
	int rv;

	osmo_conv_decode(&gsm0503_conv_sch, burst, conv);

	rv = osmo_crc16gen_check_bits(&gsm0503_sch_crc10, conv, 25, conv+25);
	if (rv)
		return -1;

	osmo_ubit2pbit_ext(sb_info, 0, conv, 0, 25, 1);

	return 0;
}

int sch_encode(ubit_t *burst, uint8_t *sb_info)
{
	ubit_t conv[35];

	osmo_pbit2ubit_ext(conv, 0, sb_info, 0, 25, 1);

	osmo_crc16gen_set_bits(&gsm0503_sch_crc10, conv, 25, conv+25);

	osmo_conv_encode(&gsm0503_conv_sch, conv, burst);

	return 0;
}

