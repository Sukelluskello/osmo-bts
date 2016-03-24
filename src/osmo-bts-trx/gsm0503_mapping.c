
#include <stdint.h>
#include <string.h>

#include <osmocom/core/bits.h>

#include "gsm0503_mapping.h"

void gsm0503_xcch_burst_unmap(sbit_t *iB, sbit_t *eB, sbit_t *hl, sbit_t *hn)
{
	memcpy(iB,    eB,    57);
	memcpy(iB+57, eB+59, 57);

	if (hl)
		*hl = eB[57];

	if (hn)
		*hn = eB[58];
}

void gsm0503_xcch_burst_map(ubit_t *iB, ubit_t *eB, const ubit_t *hl,
	const ubit_t *hn)
{
	memcpy(eB,    iB,    57);
	memcpy(eB+59, iB+57, 57);

	if (hl)
		eB[57] = *hl;
	if (hn)
		eB[58] = *hn;
}

void gsm0503_tch_burst_unmap(sbit_t *iB, sbit_t *eB, sbit_t *h, int odd)
{
	int i;

	/* brainfuck: only copy even or odd bits */
	if (iB) {
		for (i=odd; i<57; i+=2)
			iB[i] = eB[i];
		for (i=58-odd; i<114; i+=2)
			iB[i] = eB[i+2];
	}

	if (h) {
		if (!odd)
			*h = eB[58];
		else
			*h = eB[57];
	}
}

void gsm0503_tch_burst_map(ubit_t *iB, ubit_t *eB, const ubit_t *h, int odd)
{
	int i;

	/* brainfuck: only copy even or odd bits */
	if (eB) {
		for (i=odd; i<57; i+=2)
			eB[i] = iB[i];
		for (i=58-odd; i<114; i+=2)
			eB[i+2] = iB[i];
	}

	if (h) {
		if (!odd)
			eB[58] = *h;
		else
			eB[57] = *h;
	}
}

void gsm0503_mcs5_dl_burst_map(const ubit_t *di, ubit_t *eB,
			       const ubit_t *hi, const ubit_t *up, int B)
{
	int j;
	int q[8] = { 0, 0, 0, 0, 0, 0, 0, 0, };

	for (j=0; j<156; j++)
		eB[j] = di[312*B+j];
	for (j=156; j<168; j++)
		eB[j] = hi[25*B+j-156];
	for (j=168; j<174; j++)
		eB[j] = up[9*B+j-168];
	for (j=174; j<176; j++)
		eB[j] = q[2*B+j-174];
	for (j=176; j<179; j++)
		eB[j] = up[9*B+j-170];
	for (j=179; j<192; j++)
		eB[j] = hi[25*B+j-167];
	for (j=192; j<348; j++)
		eB[j] = di[312*B+j-36];
}

void gsm0503_mcs5_dl_burst_unmap(sbit_t *di, const sbit_t *eB,
			         sbit_t *hi, sbit_t *up, int B)
{
	int j;

	for (j=0; j<156; j++)
		di[312*B+j] = eB[j];
	for (j=156; j<168; j++)
		hi[25*B+j-156] = eB[j];
	for (j=168; j<174; j++)
		up[9*B+j-168] = eB[j];

	for (j=176; j<179; j++)
		up[9*B+j-170] = eB[j];
	for (j=179; j<192; j++)
		hi[25*B+j-167] = eB[j];
	for (j=192; j<348; j++)
		di[312*B+j-36] = eB[j];
}

void gsm0503_mcs5_ul_burst_map(const ubit_t *di, ubit_t *eB,
			       const ubit_t *hi, int B)
{
	int j;

	for (j=0; j<156; j++)
		eB[j] = di[312*B+j];
	for (j=156; j<174; j++)
		eB[j] = hi[34*B+j-156];
	for (j=174; j<176; j++)
		eB[j] = 0;
	for (j=176; j<192; j++)
		eB[j] = hi[34*B+j-158];
	for (j=192; j<348; j++)
		eB[j] = di[312*B+j-36];
}

void gsm0503_mcs5_ul_burst_unmap(sbit_t *di, const sbit_t *eB,
			         sbit_t *hi, int B)
{
	int j;

	for (j=0; j<156; j++)
		di[312*B+j] = eB[j];
	for (j=156; j<174; j++)
		hi[34*B+j-156] = eB[j];
	for (j=176; j<192; j++)
		hi[34*B+j-158] = eB[j];
	for (j=192; j<348; j++)
		di[312*B+j-36] = eB[j];
}

void gsm0503_mcs7_dl_burst_map(const ubit_t *di, ubit_t *eB,
			       const ubit_t *hi, const ubit_t *up, int B)
{
	int j;
	int q[8] = { 1, 1, 1, 0, 0, 1, 1, 1, };

	for (j=0; j<153; j++)
		eB[j] = di[306*B+j];
	for (j=153; j<168; j++)
		eB[j] = hi[31*B+j-153];
	for (j=168; j<174; j++)
		eB[j] = up[9*B+j-168];
	for (j=174; j<176; j++)
		eB[j] = q[2*B+j-174];
	for (j=176; j<179; j++)
		eB[j] = up[9*B+j-170];
	for (j=179; j<195; j++)
		eB[j] = hi[31*B+j-164];
	for (j=195; j<348; j++)
		eB[j] = di[306*B+j-42];
}

void gsm0503_mcs7_dl_burst_unmap(sbit_t *di, const sbit_t *eB,
			         sbit_t *hi, sbit_t *up, int B)
{
	int j;

	for (j=0; j<153; j++)
		di[306*B+j] = eB[j];
	for (j=153; j<168; j++)
		hi[31*B+j-153] = eB[j];
	for (j=168; j<174; j++)
		up[9*B+j-168] = eB[j];

	for (j=176; j<179; j++)
		up[9*B+j-170] = eB[j];
	for (j=179; j<195; j++)
		hi[31*B+j-164] = eB[j];
	for (j=195; j<348; j++)
		di[306*B+j-42] = eB[j];
}

void gsm0503_mcs7_ul_burst_map(const ubit_t *di, ubit_t *eB,
			       const ubit_t *hi, int B)
{
	int j;
	int q[8] = { 1, 1, 1, 0, 0, 1, 1, 1, };

	for (j=0; j<153; j++)
		eB[j] = di[306*B+j];
	for (j=153; j<174; j++)
		eB[j] = hi[40*B+j-153];
	for (j=174; j<176; j++)
		eB[j] = q[2*B+j-174];
	for (j=176; j<195; j++)
		eB[j] = hi[40*B+j-155];
	for (j=195; j<348; j++)
		eB[j] = di[306*B+j-42];
}

void gsm0503_mcs7_ul_burst_unmap(sbit_t *di, const sbit_t *eB,
			         sbit_t *hi, int B)
{
	int j;

	for (j=0; j<153; j++)
		di[306*B+j] = eB[j];
	for (j=153; j<174; j++)
		hi[40*B+j-153] = eB[j];

	for (j=176; j<195; j++)
		hi[40*B+j-155] = eB[j];
	for (j=195; j<348; j++)
		di[306*B+j-42] = eB[j];
}
