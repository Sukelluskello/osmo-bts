
#include <stdint.h>

#include <osmocom/core/conv.h>

#include "gsm0503_conv.h"

/*
 * GSM convolutional coding
 *
 * G_0 = 1 + x^3 + x^4
 * G_1 = 1 + x + x^3 + x^4
 */

static const uint8_t conv_xcch_next_output[][2] = {
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 3, 0 }, { 2, 1 }, { 3, 0 }, { 2, 1 },
	{ 3, 0 }, { 2, 1 }, { 3, 0 }, { 2, 1 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
};

static const uint8_t conv_xcch_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
};

static const uint8_t conv_mcs_next_output[][2] = {
	{ 0, 7 }, { 3, 4 }, { 6, 1 }, { 5, 2 },
	{ 6, 1 }, { 5, 2 }, { 0, 7 }, { 3, 4 },
	{ 1, 6 }, { 2, 5 }, { 7, 0 }, { 4, 3 },
	{ 7, 0 }, { 4, 3 }, { 1, 6 }, { 2, 5 },
	{ 4, 3 }, { 7, 0 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 4, 3 }, { 7, 0 },
	{ 5, 2 }, { 6, 1 }, { 3, 4 }, { 0, 7 },
	{ 3, 4 }, { 0, 7 }, { 5, 2 }, { 6, 1 },
	{ 7, 0 }, { 4, 3 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 7, 0 }, { 4, 3 },
	{ 6, 1 }, { 5, 2 }, { 0, 7 }, { 3, 4 },
	{ 0, 7 }, { 3, 4 }, { 6, 1 }, { 5, 2 },
	{ 3, 4 }, { 0, 7 }, { 5, 2 }, { 6, 1 },
	{ 5, 2 }, { 6, 1 }, { 3, 4 }, { 0, 7 },
	{ 2, 5 }, { 1, 6 }, { 4, 3 }, { 7, 0 },
	{ 4, 3 }, { 7, 0 }, { 2, 5 }, { 1, 6 },
};

static const uint8_t conv_mcs_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 28, 29 }, { 30, 31 },
	{ 32, 33 }, { 34, 35 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 60, 61 }, { 62, 63 },
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 28, 29 }, { 30, 31 },
	{ 32, 33 }, { 34, 35 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 60, 61 }, { 62, 63 },
};

const struct osmo_conv_code gsm0503_conv_xcch = {
	.N = 2,
	.K = 5,
	.len = 224,
	.next_output = conv_xcch_next_output,
	.next_state  = conv_xcch_next_state,
};


const struct osmo_conv_code gsm0503_conv_cs2 = {
	.N = 2,
	.K = 5,
	.len = 290,
	.next_output = conv_xcch_next_output,
	.next_state  = conv_xcch_next_state,
};


const struct osmo_conv_code gsm0503_conv_cs3 = {
	.N = 2,
	.K = 5,
	.len = 334,
	.next_output = conv_xcch_next_output,
	.next_state  = conv_xcch_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs1_dl_hdr = {
	.N = 3,
	.K = 7,
	.len = 36,
	.term = CONV_TERM_TAIL_BITING,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs1_ul_hdr = {
	.N = 3,
	.K = 7,
	.len = 39,
	.term = CONV_TERM_TAIL_BITING,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs1 = {
	.N = 3,
	.K = 7,
	.len = 190,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs2 = {
	.N = 3,
	.K = 7,
	.len = 238,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs3 = {
	.N = 3,
	.K = 7,
	.len = 310,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs4 = {
	.N = 3,
	.K = 7,
	.len = 366,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs5_dl_hdr = {
	.N = 3,
	.K = 7,
	.len = 33,
	.term = CONV_TERM_TAIL_BITING,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs5_ul_hdr = {
	.N = 3,
	.K = 7,
	.len = 45,
	.term = CONV_TERM_TAIL_BITING,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs5 = {
	.N = 3,
	.K = 7,
	.len = 462,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs6 = {
	.N = 3,
	.K = 7,
	.len = 606,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs7_dl_hdr = {
	.N = 3,
	.K = 7,
	.len = 46,
	.term = CONV_TERM_TAIL_BITING,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs7_ul_hdr = {
	.N = 3,
	.K = 7,
	.len = 56,
	.term = CONV_TERM_TAIL_BITING,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs7 = {
	.N = 3,
	.K = 7,
	.len = 462,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs8 = {
	.N = 3,
	.K = 7,
	.len = 558,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_mcs9 = {
	.N = 3,
	.K = 7,
	.len = 606,
	.next_output = conv_mcs_next_output,
	.next_state  = conv_mcs_next_state,
};


const struct osmo_conv_code gsm0503_conv_rach = {
	.N = 2,
	.K = 5,
	.len = 14,
	.next_output = conv_xcch_next_output,
	.next_state  = conv_xcch_next_state,
};


const struct osmo_conv_code gsm0503_conv_sch = {
	.N = 2,
	.K = 5,
	.len = 35,
	.next_output = conv_xcch_next_output,
	.next_state  = conv_xcch_next_state,
};


const struct osmo_conv_code gsm0503_conv_tch_fr = {
	.N = 2,
	.K = 5,
	.len = 185,
	.next_output = conv_xcch_next_output,
	.next_state  = conv_xcch_next_state,
};


/*
 * GSM HR convolutional coding
 *
 * G_0 = 1 + x^2 + x^3 + x^5 + x^6
 * G_1 = 1 + x + x^4 + x^6
 * G_3 = 1 + x + x^2 + x^3 + x^4 + x^6
 */

static const uint8_t conv_tch_hr_next_output[][2] = {
	{ 0, 7 }, { 3, 4 }, { 5, 2 }, { 6, 1 },
	{ 5, 2 }, { 6, 1 }, { 0, 7 }, { 3, 4 },
	{ 3, 4 }, { 0, 7 }, { 6, 1 }, { 5, 2 },
	{ 6, 1 }, { 5, 2 }, { 3, 4 }, { 0, 7 },
	{ 4, 3 }, { 7, 0 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 4, 3 }, { 7, 0 },
	{ 7, 0 }, { 4, 3 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 7, 0 }, { 4, 3 },
	{ 7, 0 }, { 4, 3 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 7, 0 }, { 4, 3 },
	{ 4, 3 }, { 7, 0 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 4, 3 }, { 7, 0 },
	{ 3, 4 }, { 0, 7 }, { 6, 1 }, { 5, 2 },
	{ 6, 1 }, { 5, 2 }, { 3, 4 }, { 0, 7 },
	{ 0, 7 }, { 3, 4 }, { 5, 2 }, { 6, 1 },
	{ 5, 2 }, { 6, 1 }, { 0, 7 }, { 3, 4 },
};

static const uint8_t conv_tch_hr_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 28, 29 }, { 30, 31 },
	{ 32, 33 }, { 34, 35 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 60, 61 }, { 62, 63 },
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 28, 29 }, { 30, 31 },
	{ 32, 33 }, { 34, 35 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 60, 61 }, { 62, 63 },
};

static const int conv_tch_hr_puncture[] = {
	/* Class 1 bits */
	  1,   4,   7,  10,  13,  16,  19,  22,  25,  28,  31,  34,
	 37,  40,  43,  46,  49,  52,  55,  58,  61,  64,  67,  70,
	 73,  76,  79,  82,  85,  88,  91,  94,  97, 100, 103, 106,
	109, 112, 115, 118, 121, 124, 127, 130, 133, 136, 139, 142,
	145, 148, 151, 154, 157, 160, 163, 166, 169, 172, 175, 178,
	181, 184, 187, 190, 193, 196, 199, 202, 205, 208, 211, 214,
	217, 220, 223, 226, 229, 232, 235, 238, 241, 244, 247, 250,
	253, 256, 259, 262, 265, 268, 271, 274, 277, 280, 283,

	/* Tail bits */
	295, 298, 301, 304, 307, 310,

	/* End */
	-1,
};

const struct osmo_conv_code gsm0503_conv_tch_hr = {
	.N = 3,
	.K = 7,
	.len = 98,
	.next_output = conv_tch_hr_next_output,
	.next_state  = conv_tch_hr_next_state,
	.puncture    = conv_tch_hr_puncture,
};


/* TCH/AFS12.2 */
/* ----------- */

static const uint8_t conv_tch_afs_12_2_next_output[][2] = {
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
};

static const uint8_t conv_tch_afs_12_2_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  9,  8 }, { 11, 10 }, { 13, 12 }, { 15, 14 },
	{  1,  0 }, {  3,  2 }, {  5,  4 }, {  7,  6 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
};

static const uint8_t conv_tch_afs_12_2_next_term_output[] = {
	 0,  1,  0,  1,  3,  2,  3,  2,  3,  2,  3,  2,  0,  1,  0,  1,
};

static const uint8_t conv_tch_afs_12_2_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14,  0,  2,  4,  6,  8, 10, 12, 14,
};

static int conv_tch_afs_12_2_puncture[] = {
	321, 325, 329, 333, 337, 341, 345, 349, 353, 357, 361, 363,
	365, 369, 373, 377, 379, 381, 385, 389, 393, 395, 397, 401,
	405, 409, 411, 413, 417, 421, 425, 427, 429, 433, 437, 441,
	443, 445, 449, 453, 457, 459, 461, 465, 469, 473, 475, 477,
	481, 485, 489, 491, 493, 495, 497, 499, 501, 503, 505, 507,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_afs_12_2 = {
	.N = 2,
	.K = 5,
	.len = 250,
	.next_output      = conv_tch_afs_12_2_next_output,
	.next_state       = conv_tch_afs_12_2_next_state,
	.next_term_output = conv_tch_afs_12_2_next_term_output,
	.next_term_state  = conv_tch_afs_12_2_next_term_state,
	.puncture         = conv_tch_afs_12_2_puncture,
};


/* TCH/AFS10.2 */
/* ----------- */

static const uint8_t conv_tch_afs_10_2_next_output[][2] = {
	{ 0, 7 }, { 2, 5 }, { 4, 3 }, { 6, 1 },
	{ 2, 5 }, { 0, 7 }, { 6, 1 }, { 4, 3 },
	{ 0, 7 }, { 2, 5 }, { 4, 3 }, { 6, 1 },
	{ 2, 5 }, { 0, 7 }, { 6, 1 }, { 4, 3 },
};

static const uint8_t conv_tch_afs_10_2_next_state[][2] = {
	{  0,  1 }, {  3,  2 }, {  5,  4 }, {  6,  7 },
	{  9,  8 }, { 10, 11 }, { 12, 13 }, { 15, 14 },
	{  1,  0 }, {  2,  3 }, {  4,  5 }, {  7,  6 },
	{  8,  9 }, { 11, 10 }, { 13, 12 }, { 14, 15 },
};

static const uint8_t conv_tch_afs_10_2_next_term_output[] = {
	 0,  5,  3,  6,  5,  0,  6,  3,  7,  2,  4,  1,  2,  7,  1,  4,
};

static const uint8_t conv_tch_afs_10_2_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14,  0,  2,  4,  6,  8, 10, 12, 14,
};

static int conv_tch_afs_10_2_puncture[] = {
	  1,   4,   7,  10,  16,  19,  22,  28,  31,  34,  40,  43,
	 46,  52,  55,  58,  64,  67,  70,  76,  79,  82,  88,  91,
	 94, 100, 103, 106, 112, 115, 118, 124, 127, 130, 136, 139,
	142, 148, 151, 154, 160, 163, 166, 172, 175, 178, 184, 187,
	190, 196, 199, 202, 208, 211, 214, 220, 223, 226, 232, 235,
	238, 244, 247, 250, 256, 259, 262, 268, 271, 274, 280, 283,
	286, 292, 295, 298, 304, 307, 310, 316, 319, 322, 325, 328,
	331, 334, 337, 340, 343, 346, 349, 352, 355, 358, 361, 364,
	367, 370, 373, 376, 379, 382, 385, 388, 391, 394, 397, 400,
	403, 406, 409, 412, 415, 418, 421, 424, 427, 430, 433, 436,
	439, 442, 445, 448, 451, 454, 457, 460, 463, 466, 469, 472,
	475, 478, 481, 484, 487, 490, 493, 496, 499, 502, 505, 508,
	511, 514, 517, 520, 523, 526, 529, 532, 535, 538, 541, 544,
	547, 550, 553, 556, 559, 562, 565, 568, 571, 574, 577, 580,
	583, 586, 589, 592, 595, 598, 601, 604, 607, 609, 610, 613,
	616, 619, 621, 622, 625, 627, 628, 631, 633, 634, 636, 637,
	639, 640,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_afs_10_2 = {
	.N = 3,
	.K = 5,
	.len = 210,
	.next_output      = conv_tch_afs_10_2_next_output,
	.next_state       = conv_tch_afs_10_2_next_state,
	.next_term_output = conv_tch_afs_10_2_next_term_output,
	.next_term_state  = conv_tch_afs_10_2_next_term_state,
	.puncture         = conv_tch_afs_10_2_puncture,
};


/* TCH/AFS7.95 */
/* ----------- */

static const uint8_t conv_tch_afs_7_95_next_output[][2] = {
	{ 0, 7 }, { 3, 4 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 0, 7 }, { 3, 4 },
	{ 3, 4 }, { 0, 7 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 3, 4 }, { 0, 7 },
	{ 3, 4 }, { 0, 7 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 3, 4 }, { 0, 7 },
	{ 0, 7 }, { 3, 4 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 0, 7 }, { 3, 4 },
	{ 0, 7 }, { 3, 4 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 0, 7 }, { 3, 4 },
	{ 3, 4 }, { 0, 7 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 3, 4 }, { 0, 7 },
	{ 3, 4 }, { 0, 7 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 3, 4 }, { 0, 7 },
	{ 0, 7 }, { 3, 4 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 0, 7 }, { 3, 4 },
};

static const uint8_t conv_tch_afs_7_95_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  5,  4 }, {  7,  6 },
	{  9,  8 }, { 11, 10 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 21, 20 }, { 23, 22 },
	{ 25, 24 }, { 27, 26 }, { 28, 29 }, { 30, 31 },
	{ 33, 32 }, { 35, 34 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 45, 44 }, { 47, 46 },
	{ 49, 48 }, { 51, 50 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 61, 60 }, { 63, 62 },
	{  1,  0 }, {  3,  2 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 13, 12 }, { 15, 14 },
	{ 17, 16 }, { 19, 18 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 29, 28 }, { 31, 30 },
	{ 32, 33 }, { 34, 35 }, { 37, 36 }, { 39, 38 },
	{ 41, 40 }, { 43, 42 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 53, 52 }, { 55, 54 },
	{ 57, 56 }, { 59, 58 }, { 60, 61 }, { 62, 63 },
};

static const uint8_t conv_tch_afs_7_95_next_term_output[] = {
	 0,  3,  5,  6,  5,  6,  0,  3,  3,  0,  6,  5,  6,  5,  3,  0,
	 4,  7,  1,  2,  1,  2,  4,  7,  7,  4,  2,  1,  2,  1,  7,  4,
	 7,  4,  2,  1,  2,  1,  7,  4,  4,  7,  1,  2,  1,  2,  4,  7,
	 3,  0,  6,  5,  6,  5,  3,  0,  0,  3,  5,  6,  5,  6,  0,  3,
};

static const uint8_t conv_tch_afs_7_95_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
	 0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
};

static int conv_tch_afs_7_95_puncture[] = {
	  1,   2,   4,   5,   8,  22,  70, 118, 166, 214, 262, 310,
	317, 319, 325, 332, 334, 341, 343, 349, 356, 358, 365, 367,
	373, 380, 382, 385, 389, 391, 397, 404, 406, 409, 413, 415,
	421, 428, 430, 433, 437, 439, 445, 452, 454, 457, 461, 463,
	469, 476, 478, 481, 485, 487, 490, 493, 500, 502, 503, 505,
	506, 508, 509, 511, 512,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_afs_7_95 = {
	.N = 3,
	.K = 7,
	.len = 165,
	.next_output      = conv_tch_afs_7_95_next_output,
	.next_state       = conv_tch_afs_7_95_next_state,
	.next_term_output = conv_tch_afs_7_95_next_term_output,
	.next_term_state  = conv_tch_afs_7_95_next_term_state,
	.puncture         = conv_tch_afs_7_95_puncture,
};


/* TCH/AFS7.4 */
/* ---------- */

static const uint8_t conv_tch_afs_7_4_next_output[][2] = {
	{ 0, 7 }, { 2, 5 }, { 4, 3 }, { 6, 1 },
	{ 2, 5 }, { 0, 7 }, { 6, 1 }, { 4, 3 },
	{ 0, 7 }, { 2, 5 }, { 4, 3 }, { 6, 1 },
	{ 2, 5 }, { 0, 7 }, { 6, 1 }, { 4, 3 },
};

static const uint8_t conv_tch_afs_7_4_next_state[][2] = {
	{  0,  1 }, {  3,  2 }, {  5,  4 }, {  6,  7 },
	{  9,  8 }, { 10, 11 }, { 12, 13 }, { 15, 14 },
	{  1,  0 }, {  2,  3 }, {  4,  5 }, {  7,  6 },
	{  8,  9 }, { 11, 10 }, { 13, 12 }, { 14, 15 },
};

static const uint8_t conv_tch_afs_7_4_next_term_output[] = {
	 0,  5,  3,  6,  5,  0,  6,  3,  7,  2,  4,  1,  2,  7,  1,  4,
};

static const uint8_t conv_tch_afs_7_4_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14,  0,  2,  4,  6,  8, 10, 12, 14,
};

static int conv_tch_afs_7_4_puncture[] = {
	  0, 355, 361, 367, 373, 379, 385, 391, 397, 403, 409, 415,
	421, 427, 433, 439, 445, 451, 457, 460, 463, 466, 468, 469,
	471, 472,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_afs_7_4 = {
	.N = 3,
	.K = 5,
	.len = 154,
	.next_output      = conv_tch_afs_7_4_next_output,
	.next_state	  = conv_tch_afs_7_4_next_state,
	.next_term_output = conv_tch_afs_7_4_next_term_output,
	.next_term_state  = conv_tch_afs_7_4_next_term_state,
	.puncture         = conv_tch_afs_7_4_puncture,
};


/* TCH/AFS6.7 */
/* ---------- */

static const uint8_t conv_tch_afs_6_7_next_output[][2] = {
	{ 0, 15 }, { 4, 11 }, { 8, 7 }, { 12, 3 },
	{ 4, 11 }, { 0, 15 }, { 12, 3 }, { 8, 7 },
	{ 0, 15 }, { 4, 11 }, { 8, 7 }, { 12, 3 },
	{ 4, 11 }, { 0, 15 }, { 12, 3 }, { 8, 7 },
};

static const uint8_t conv_tch_afs_6_7_next_state[][2] = {
	{  0,  1 }, {  3,  2 }, {  5,  4 }, {  6,  7 },
	{  9,  8 }, { 10, 11 }, { 12, 13 }, { 15, 14 },
	{  1,  0 }, {  2,  3 }, {  4,  5 }, {  7,  6 },
	{  8,  9 }, { 11, 10 }, { 13, 12 }, { 14, 15 },
};

static int conv_tch_afs_6_7_puncture[] = {
	  1,   3,   7,  11,  15,  27,  39,  55,  67,  79,  95, 107,
	119, 135, 147, 159, 175, 187, 199, 215, 227, 239, 255, 267,
	279, 287, 291, 295, 299, 303, 307, 311, 315, 319, 323, 327,
	331, 335, 339, 343, 347, 351, 355, 359, 363, 367, 369, 371,
	375, 377, 379, 383, 385, 387, 391, 393, 395, 399, 401, 403,
	407, 409, 411, 415, 417, 419, 423, 425, 427, 431, 433, 435,
	439, 441, 443, 447, 449, 451, 455, 457, 459, 463, 465, 467,
	471, 473, 475, 479, 481, 483, 487, 489, 491, 495, 497, 499,
	503, 505, 507, 511, 513, 515, 519, 521, 523, 527, 529, 531,
	535, 537, 539, 543, 545, 547, 549, 551, 553, 555, 557, 559,
	561, 563, 565, 567, 569, 571, 573, 575,
	-1, /* end */
};

static const uint8_t conv_tch_afs_6_7_next_term_output[] = {
	 0, 11,  7, 12, 11,  0, 12,  7, 15,  4,  8,  3,  4, 15,  3,  8,
};

static const uint8_t conv_tch_afs_6_7_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14,  0,  2,  4,  6,  8, 10, 12, 14,
};

const struct osmo_conv_code gsm0503_conv_tch_afs_6_7 = {
	.N = 4,
	.K = 5,
	.len = 140,
	.next_output      = conv_tch_afs_6_7_next_output,
	.next_state       = conv_tch_afs_6_7_next_state,
	.next_term_output = conv_tch_afs_6_7_next_term_output,
	.next_term_state  = conv_tch_afs_6_7_next_term_state,
	.puncture         = conv_tch_afs_6_7_puncture,
};


/* TCH/AFS5.9 */
/* ---------- */

static const uint8_t conv_tch_afs_5_9_next_output[][2] = {
	{  0, 15 }, {  8,  7 }, {  4, 11 }, { 12,  3 },
	{  4, 11 }, { 12,  3 }, {  0, 15 }, {  8,  7 },
	{  8,  7 }, {  0, 15 }, { 12,  3 }, {  4, 11 },
	{ 12,  3 }, {  4, 11 }, {  8,  7 }, {  0, 15 },
	{  8,  7 }, {  0, 15 }, { 12,  3 }, {  4, 11 },
	{ 12,  3 }, {  4, 11 }, {  8,  7 }, {  0, 15 },
	{  0, 15 }, {  8,  7 }, {  4, 11 }, { 12,  3 },
	{  4, 11 }, { 12,  3 }, {  0, 15 }, {  8,  7 },
	{  0, 15 }, {  8,  7 }, {  4, 11 }, { 12,  3 },
	{  4, 11 }, { 12,  3 }, {  0, 15 }, {  8,  7 },
	{  8,  7 }, {  0, 15 }, { 12,  3 }, {  4, 11 },
	{ 12,  3 }, {  4, 11 }, {  8,  7 }, {  0, 15 },
	{  8,  7 }, {  0, 15 }, { 12,  3 }, {  4, 11 },
	{ 12,  3 }, {  4, 11 }, {  8,  7 }, {  0, 15 },
	{  0, 15 }, {  8,  7 }, {  4, 11 }, { 12,  3 },
	{  4, 11 }, { 12,  3 }, {  0, 15 }, {  8,  7 },
};

static const uint8_t conv_tch_afs_5_9_next_state[][2] = {
	{  0,  1 }, {  3,  2 }, {  5,  4 }, {  6,  7 },
	{  9,  8 }, { 10, 11 }, { 12, 13 }, { 15, 14 },
	{ 17, 16 }, { 18, 19 }, { 20, 21 }, { 23, 22 },
	{ 24, 25 }, { 27, 26 }, { 29, 28 }, { 30, 31 },
	{ 32, 33 }, { 35, 34 }, { 37, 36 }, { 38, 39 },
	{ 41, 40 }, { 42, 43 }, { 44, 45 }, { 47, 46 },
	{ 49, 48 }, { 50, 51 }, { 52, 53 }, { 55, 54 },
	{ 56, 57 }, { 59, 58 }, { 61, 60 }, { 62, 63 },
	{  1,  0 }, {  2,  3 }, {  4,  5 }, {  7,  6 },
	{  8,  9 }, { 11, 10 }, { 13, 12 }, { 14, 15 },
	{ 16, 17 }, { 19, 18 }, { 21, 20 }, { 22, 23 },
	{ 25, 24 }, { 26, 27 }, { 28, 29 }, { 31, 30 },
	{ 33, 32 }, { 34, 35 }, { 36, 37 }, { 39, 38 },
	{ 40, 41 }, { 43, 42 }, { 45, 44 }, { 46, 47 },
	{ 48, 49 }, { 51, 50 }, { 53, 52 }, { 54, 55 },
	{ 57, 56 }, { 58, 59 }, { 60, 61 }, { 63, 62 },
};

static const uint8_t conv_tch_afs_5_9_next_term_output[] = {
	 0,  7, 11, 12, 11, 12,  0,  7,  7,  0, 12, 11, 12, 11,  7,  0,
	 8, 15,  3,  4,  3,  4,  8, 15, 15,  8,  4,  3,  4,  3, 15,  8,
	15,  8,  4,  3,  4,  3, 15,  8,  8, 15,  3,  4,  3,  4,  8, 15,
	 7,  0, 12, 11, 12, 11,  7,  0,  0,  7, 11, 12, 11, 12,  0,  7,
};

static const uint8_t conv_tch_afs_5_9_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
	 0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
};

static int conv_tch_afs_5_9_puncture[] = {
	  0,   1,   3,   5,   7,  11,  15,  31,  47,  63,  79,  95,
	111, 127, 143, 159, 175, 191, 207, 223, 239, 255, 271, 287,
	303, 319, 327, 331, 335, 343, 347, 351, 359, 363, 367, 375,
	379, 383, 391, 395, 399, 407, 411, 415, 423, 427, 431, 439,
	443, 447, 455, 459, 463, 467, 471, 475, 479, 483, 487, 491,
	495, 499, 503, 507, 509, 511, 512, 513, 515, 516, 517, 519,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_afs_5_9 = {
	.N = 4,
	.K = 7,
	.len = 124,
	.next_output      = conv_tch_afs_5_9_next_output,
	.next_state       = conv_tch_afs_5_9_next_state,
	.next_term_output = conv_tch_afs_5_9_next_term_output,
	.next_term_state  = conv_tch_afs_5_9_next_term_state,
	.puncture         = conv_tch_afs_5_9_puncture,
};


/* TCH/AFS5.15 */
/* ----------- */

static const uint8_t conv_tch_afs_5_15_next_output[][2] = {
	{ 0, 31 }, { 4, 27 }, { 24, 7 }, { 28, 3 },
	{ 4, 27 }, { 0, 31 }, { 28, 3 }, { 24, 7 },
	{ 0, 31 }, { 4, 27 }, { 24, 7 }, { 28, 3 },
	{ 4, 27 }, { 0, 31 }, { 28, 3 }, { 24, 7 },
};

static const uint8_t conv_tch_afs_5_15_next_state[][2] = {
	{  0,  1 }, {  3,  2 }, {  5,  4 }, {  6,  7 },
	{  9,  8 }, { 10, 11 }, { 12, 13 }, { 15, 14 },
	{  1,  0 }, {  2,  3 }, {  4,  5 }, {  7,  6 },
	{  8,  9 }, { 11, 10 }, { 13, 12 }, { 14, 15 },
};

static const uint8_t conv_tch_afs_5_15_next_term_output[] = {
	 0, 27,  7, 28, 27,  0, 28,  7, 31,  4, 24,  3,  4, 31,  3, 24,
};

static const uint8_t conv_tch_afs_5_15_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14,  0,  2,  4,  6,  8, 10, 12, 14,
};

static int conv_tch_afs_5_15_puncture[] = {
	  0,   4,   5,   9,  10,  14,  15,  20,  25,  30,  35,  40,
	 50,  60,  70,  80,  90, 100, 110, 120, 130, 140, 150, 160,
	170, 180, 190, 200, 210, 220, 230, 240, 250, 260, 270, 280,
	290, 300, 310, 315, 320, 325, 330, 334, 335, 340, 344, 345,
	350, 354, 355, 360, 364, 365, 370, 374, 375, 380, 384, 385,
	390, 394, 395, 400, 404, 405, 410, 414, 415, 420, 424, 425,
	430, 434, 435, 440, 444, 445, 450, 454, 455, 460, 464, 465,
	470, 474, 475, 480, 484, 485, 490, 494, 495, 500, 504, 505,
	510, 514, 515, 520, 524, 525, 529, 530, 534, 535, 539, 540,
	544, 545, 549, 550, 554, 555, 559, 560, 564,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_afs_5_15 = {
	.N = 5,
	.K = 5,
	.len = 109,
	.next_output      = conv_tch_afs_5_15_next_output,
	.next_state       = conv_tch_afs_5_15_next_state,
	.next_term_output = conv_tch_afs_5_15_next_term_output,
	.next_term_state  = conv_tch_afs_5_15_next_term_state,
	.puncture         = conv_tch_afs_5_15_puncture,
};


/* TCH/AFS4.75 */
/* ----------- */

static const uint8_t conv_tch_afs_4_75_next_output[][2] = {
	{ 0, 31 }, { 24, 7 }, { 4, 27 }, { 28, 3 },
	{ 4, 27 }, { 28, 3 }, { 0, 31 }, { 24, 7 },
	{ 24, 7 }, { 0, 31 }, { 28, 3 }, { 4, 27 },
	{ 28, 3 }, { 4, 27 }, { 24, 7 }, { 0, 31 },
	{ 24, 7 }, { 0, 31 }, { 28, 3 }, { 4, 27 },
	{ 28, 3 }, { 4, 27 }, { 24, 7 }, { 0, 31 },
	{ 0, 31 }, { 24, 7 }, { 4, 27 }, { 28, 3 },
	{ 4, 27 }, { 28, 3 }, { 0, 31 }, { 24, 7 },
	{ 0, 31 }, { 24, 7 }, { 4, 27 }, { 28, 3 },
	{ 4, 27 }, { 28, 3 }, { 0, 31 }, { 24, 7 },
	{ 24, 7 }, { 0, 31 }, { 28, 3 }, { 4, 27 },
	{ 28, 3 }, { 4, 27 }, { 24, 7 }, { 0, 31 },
	{ 24, 7 }, { 0, 31 }, { 28, 3 }, { 4, 27 },
	{ 28, 3 }, { 4, 27 }, { 24, 7 }, { 0, 31 },
	{ 0, 31 }, { 24, 7 }, { 4, 27 }, { 28, 3 },
	{ 4, 27 }, { 28, 3 }, { 0, 31 }, { 24, 7 },
};

static const uint8_t conv_tch_afs_4_75_next_state[][2] = {
	{  0,  1 }, {  3,  2 }, {  5,  4 }, {  6,  7 },
	{  9,  8 }, { 10, 11 }, { 12, 13 }, { 15, 14 },
	{ 17, 16 }, { 18, 19 }, { 20, 21 }, { 23, 22 },
	{ 24, 25 }, { 27, 26 }, { 29, 28 }, { 30, 31 },
	{ 32, 33 }, { 35, 34 }, { 37, 36 }, { 38, 39 },
	{ 41, 40 }, { 42, 43 }, { 44, 45 }, { 47, 46 },
	{ 49, 48 }, { 50, 51 }, { 52, 53 }, { 55, 54 },
	{ 56, 57 }, { 59, 58 }, { 61, 60 }, { 62, 63 },
	{  1,  0 }, {  2,  3 }, {  4,  5 }, {  7,  6 },
	{  8,  9 }, { 11, 10 }, { 13, 12 }, { 14, 15 },
	{ 16, 17 }, { 19, 18 }, { 21, 20 }, { 22, 23 },
	{ 25, 24 }, { 26, 27 }, { 28, 29 }, { 31, 30 },
	{ 33, 32 }, { 34, 35 }, { 36, 37 }, { 39, 38 },
	{ 40, 41 }, { 43, 42 }, { 45, 44 }, { 46, 47 },
	{ 48, 49 }, { 51, 50 }, { 53, 52 }, { 54, 55 },
	{ 57, 56 }, { 58, 59 }, { 60, 61 }, { 63, 62 },
};

static const uint8_t conv_tch_afs_4_75_next_term_output[] = {
	 0,  7, 27, 28, 27, 28,  0,  7,  7,  0, 28, 27, 28, 27,  7,  0,
	24, 31,  3,  4,  3,  4, 24, 31, 31, 24,  4,  3,  4,  3, 31, 24,
	31, 24,  4,  3,  4,  3, 31, 24, 24, 31,  3,  4,  3,  4, 24, 31,
	 7,  0, 28, 27, 28, 27,  7,  0,  0,  7, 27, 28, 27, 28,  0,  7,
};

static const uint8_t conv_tch_afs_4_75_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
	 0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
};

static int conv_tch_afs_4_75_puncture[] = {
	  0,   1,   2,   4,   5,   7,   9,  15,  25,  35,  45,  55,
	 65,  75,  85,  95, 105, 115, 125, 135, 145, 155, 165, 175,
	185, 195, 205, 215, 225, 235, 245, 255, 265, 275, 285, 295,
	305, 315, 325, 335, 345, 355, 365, 375, 385, 395, 400, 405,
	410, 415, 420, 425, 430, 435, 440, 445, 450, 455, 459, 460,
	465, 470, 475, 479, 480, 485, 490, 495, 499, 500, 505, 509,
	510, 515, 517, 519, 520, 522, 524, 525, 526, 527, 529, 530,
	531, 532, 534,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_afs_4_75 = {
	.N = 5,
	.K = 7,
	.len = 101,
	.next_output      = conv_tch_afs_4_75_next_output,
	.next_state       = conv_tch_afs_4_75_next_state,
	.next_term_output = conv_tch_afs_4_75_next_term_output,
	.next_term_state  = conv_tch_afs_4_75_next_term_state,
	.puncture         = conv_tch_afs_4_75_puncture,
};


/* TCH/AHS7.95 */
/* ----------- */

static const uint8_t conv_tch_ahs_7_95_next_output[][2] = {
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
};

static const uint8_t conv_tch_ahs_7_95_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  9,  8 }, { 11, 10 }, { 13, 12 }, { 15, 14 },
	{  1,  0 }, {  3,  2 }, {  5,  4 }, {  7,  6 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
};

static const uint8_t conv_tch_ahs_7_95_next_term_output[] = {
	 0,  1,  0,  1,  3,  2,  3,  2,  3,  2,  3,  2,  0,  1,  0,  1,
};

static const uint8_t conv_tch_ahs_7_95_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14,  0,  2,  4,  6,  8, 10, 12, 14,
};

static int conv_tch_ahs_7_95_puncture[] = {
	  1,   3,   5,   7,  11,  15,  19,  23,  27,  31,  35,  43,
	 47,  51,  55,  59,  63,  67,  71,  79,  83,  87,  91,  95,
	 99, 103, 107, 115, 119, 123, 127, 131, 135, 139, 143, 151,
	155, 159, 163, 167, 171, 175, 177, 179, 183, 185, 187, 191,
	193, 195, 197, 199, 203, 205, 207, 211, 213, 215, 219, 221,
	223, 227, 229, 231, 233, 235, 239, 241, 243, 247, 249, 251,
	255, 257, 259, 261, 263, 265,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_ahs_7_95 = {
	.N = 2,
	.K = 5,
	.len = 129,
	.next_output      = conv_tch_ahs_7_95_next_output,
	.next_state       = conv_tch_ahs_7_95_next_state,
	.next_term_output = conv_tch_ahs_7_95_next_term_output,
	.next_term_state  = conv_tch_ahs_7_95_next_term_state,
	.puncture         = conv_tch_ahs_7_95_puncture,
};


/* TCH/AHS7.4 */
/* ---------- */

static const uint8_t conv_tch_ahs_7_4_next_output[][2] = {
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
};

static const uint8_t conv_tch_ahs_7_4_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  9,  8 }, { 11, 10 }, { 13, 12 }, { 15, 14 },
	{  1,  0 }, {  3,  2 }, {  5,  4 }, {  7,  6 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
};

static const uint8_t conv_tch_ahs_7_4_next_term_output[] = {
	 0,  1,  0,  1,  3,  2,  3,  2,  3,  2,  3,  2,  0,  1,  0,  1,
};

static const uint8_t conv_tch_ahs_7_4_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14,  0,  2,  4,  6,  8, 10, 12, 14,
};

static int conv_tch_ahs_7_4_puncture[] = {
	  1,   3,   7,  11,  19,  23,  27,  35,  39,  43,  51,  55,
	 59,  67,  71,  75,  83,  87,  91,  99, 103, 107, 115, 119,
	123, 131, 135, 139, 143, 147, 151, 155, 159, 163, 167, 171,
	175, 179, 183, 187, 191, 195, 199, 203, 207, 211, 215, 219,
	221, 223, 227, 229, 231, 235, 237, 239, 243, 245, 247, 251,
	253, 255, 257, 259,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_ahs_7_4 = {
	.N = 2,
	.K = 5,
	.len = 126,
	.next_output      = conv_tch_ahs_7_4_next_output,
	.next_state       = conv_tch_ahs_7_4_next_state,
	.next_term_output = conv_tch_ahs_7_4_next_term_output,
	.next_term_state  = conv_tch_ahs_7_4_next_term_state,
	.puncture         = conv_tch_ahs_7_4_puncture,
};


/* TCH/AHS6.7 */
/* ---------- */

static const uint8_t conv_tch_ahs_6_7_next_output[][2] = {
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
};

static const uint8_t conv_tch_ahs_6_7_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  9,  8 }, { 11, 10 }, { 13, 12 }, { 15, 14 },
	{  1,  0 }, {  3,  2 }, {  5,  4 }, {  7,  6 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
};

static const uint8_t conv_tch_ahs_6_7_next_term_output[] = {
	 0,  1,  0,  1,  3,  2,  3,  2,  3,  2,  3,  2,  0,  1,  0,  1,
};

static const uint8_t conv_tch_ahs_6_7_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14,  0,  2,  4,  6,  8, 10, 12, 14,
};

static int conv_tch_ahs_6_7_puncture[] = {
	  1,   3,   9,  19,  29,  39,  49,  59,  69,  79,  89,  99,
	109, 119, 129, 139, 149, 159, 167, 169, 177, 179, 187, 189,
	197, 199, 203, 207, 209, 213, 217, 219, 223, 227, 229, 231,
	233, 235, 237, 239,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_ahs_6_7 = {
	.N = 2,
	.K = 5,
	.len = 116,
	.next_output      = conv_tch_ahs_6_7_next_output,
	.next_state       = conv_tch_ahs_6_7_next_state,
	.next_term_output = conv_tch_ahs_6_7_next_term_output,
	.next_term_state  = conv_tch_ahs_6_7_next_term_state,
	.puncture         = conv_tch_ahs_6_7_puncture,
};


/* TCH/AHS5.9 */
/* ---------- */

static const uint8_t conv_tch_ahs_5_9_next_output[][2] = {
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
	{ 0, 3 }, { 1, 2 }, { 0, 3 }, { 1, 2 },
};

static const uint8_t conv_tch_ahs_5_9_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  4,  5 }, {  6,  7 },
	{  9,  8 }, { 11, 10 }, { 13, 12 }, { 15, 14 },
	{  1,  0 }, {  3,  2 }, {  5,  4 }, {  7,  6 },
	{  8,  9 }, { 10, 11 }, { 12, 13 }, { 14, 15 },
};

static const uint8_t conv_tch_ahs_5_9_next_term_output[] = {
	 0,  1,  0,  1,  3,  2,  3,  2,  3,  2,  3,  2,  0,  1,  0,  1,
};

static const uint8_t conv_tch_ahs_5_9_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14,  0,  2,  4,  6,  8, 10, 12, 14,
};

static int conv_tch_ahs_5_9_puncture[] = {
	  1,  15,  71, 127, 139, 151, 163, 175, 187, 195, 203, 211,
	215, 219, 221, 223,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_ahs_5_9 = {
	.N = 2,
	.K = 5,
	.len = 108,
	.next_output      = conv_tch_ahs_5_9_next_output,
	.next_state       = conv_tch_ahs_5_9_next_state,
	.next_term_output = conv_tch_ahs_5_9_next_term_output,
	.next_term_state  = conv_tch_ahs_5_9_next_term_state,
	.puncture         = conv_tch_ahs_5_9_puncture,
};


/* TCH/AHS5.15 */
/* ----------- */

static const uint8_t conv_tch_ahs_5_15_next_output[][2] = {
	{ 0, 7 }, { 2, 5 }, { 4, 3 }, { 6, 1 },
	{ 2, 5 }, { 0, 7 }, { 6, 1 }, { 4, 3 },
	{ 0, 7 }, { 2, 5 }, { 4, 3 }, { 6, 1 },
	{ 2, 5 }, { 0, 7 }, { 6, 1 }, { 4, 3 },
};

static const uint8_t conv_tch_ahs_5_15_next_state[][2] = {
	{  0,  1 }, {  3,  2 }, {  5,  4 }, {  6,  7 },
	{  9,  8 }, { 10, 11 }, { 12, 13 }, { 15, 14 },
	{  1,  0 }, {  2,  3 }, {  4,  5 }, {  7,  6 },
	{  8,  9 }, { 11, 10 }, { 13, 12 }, { 14, 15 },
};

static const uint8_t conv_tch_ahs_5_15_next_term_output[] = {
	 0,  5,  3,  6,  5,  0,  6,  3,  7,  2,  4,  1,  2,  7,  1,  4,
};

static const uint8_t conv_tch_ahs_5_15_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14,  0,  2,  4,  6,  8, 10, 12, 14,
};

static int conv_tch_ahs_5_15_puncture[] = {
	  0,   1,   3,   4,   6,   9,  12,  15,  18,  21,  27,  33,
	 39,  45,  51,  54, 57,  63,  69,  75,  81,  87,  90,  93,
	 99, 105, 111, 117, 123, 126, 129, 135, 141, 147, 153, 159,
	162, 165, 168, 171, 174, 177, 180, 183, 186, 189, 192, 195,
	198, 201, 204, 207, 210, 213, 216, 219, 222, 225, 228, 231,
	234, 237, 240, 243, 244, 246, 249, 252, 255, 256, 258, 261,
	264, 267, 268, 270, 273, 276, 279, 280, 282, 285, 288, 289,
	291, 294, 295, 297, 298, 300, 301,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_ahs_5_15 = {
	.N = 3,
	.K = 5,
	.len = 97,
	.next_output      = conv_tch_ahs_5_15_next_output,
	.next_state       = conv_tch_ahs_5_15_next_state,
	.next_term_output = conv_tch_ahs_5_15_next_term_output,
	.next_term_state  = conv_tch_ahs_5_15_next_term_state,
	.puncture         = conv_tch_ahs_5_15_puncture,
};


/* TCH/AHS4.75 */
/* ----------- */

static const uint8_t conv_tch_ahs_4_75_next_output[][2] = {
	{ 0, 7 }, { 3, 4 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 0, 7 }, { 3, 4 },
	{ 3, 4 }, { 0, 7 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 3, 4 }, { 0, 7 },
	{ 3, 4 }, { 0, 7 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 3, 4 }, { 0, 7 },
	{ 0, 7 }, { 3, 4 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 0, 7 }, { 3, 4 },
	{ 0, 7 }, { 3, 4 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 0, 7 }, { 3, 4 },
	{ 3, 4 }, { 0, 7 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 3, 4 }, { 0, 7 },
	{ 3, 4 }, { 0, 7 }, { 1, 6 }, { 2, 5 },
	{ 1, 6 }, { 2, 5 }, { 3, 4 }, { 0, 7 },
	{ 0, 7 }, { 3, 4 }, { 2, 5 }, { 1, 6 },
	{ 2, 5 }, { 1, 6 }, { 0, 7 }, { 3, 4 },
};

static const uint8_t conv_tch_ahs_4_75_next_state[][2] = {
	{  0,  1 }, {  2,  3 }, {  5,  4 }, {  7,  6 },
	{  9,  8 }, { 11, 10 }, { 12, 13 }, { 14, 15 },
	{ 16, 17 }, { 18, 19 }, { 21, 20 }, { 23, 22 },
	{ 25, 24 }, { 27, 26 }, { 28, 29 }, { 30, 31 },
	{ 33, 32 }, { 35, 34 }, { 36, 37 }, { 38, 39 },
	{ 40, 41 }, { 42, 43 }, { 45, 44 }, { 47, 46 },
	{ 49, 48 }, { 51, 50 }, { 52, 53 }, { 54, 55 },
	{ 56, 57 }, { 58, 59 }, { 61, 60 }, { 63, 62 },
	{  1,  0 }, {  3,  2 }, {  4,  5 }, {  6,  7 },
	{  8,  9 }, { 10, 11 }, { 13, 12 }, { 15, 14 },
	{ 17, 16 }, { 19, 18 }, { 20, 21 }, { 22, 23 },
	{ 24, 25 }, { 26, 27 }, { 29, 28 }, { 31, 30 },
	{ 32, 33 }, { 34, 35 }, { 37, 36 }, { 39, 38 },
	{ 41, 40 }, { 43, 42 }, { 44, 45 }, { 46, 47 },
	{ 48, 49 }, { 50, 51 }, { 53, 52 }, { 55, 54 },
	{ 57, 56 }, { 59, 58 }, { 60, 61 }, { 62, 63 },
};

static const uint8_t conv_tch_ahs_4_75_next_term_output[] = {
	 0,  3,  5,  6,  5,  6,  0,  3,  3,  0,  6,  5,  6,  5,  3,  0,
	 4,  7,  1,  2,  1,  2,  4,  7,  7,  4,  2,  1,  2,  1,  7,  4,
	 7,  4,  2,  1,  2,  1,  7,  4,  4,  7,  1,  2,  1,  2,  4,  7,
	 3,  0,  6,  5,  6,  5,  3,  0,  0,  3,  5,  6,  5,  6,  0,  3,
};

static const uint8_t conv_tch_ahs_4_75_next_term_state[] = {
	 0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
	 0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
};

static int conv_tch_ahs_4_75_puncture[] = {
	  1,   2,   4,   5,   7,   8,  10,  13,  16,  22,  28,  34,
	 40,  46,  52,  58, 64,  70,  76,  82,  88,  94, 100, 106,
	112, 118, 124, 130, 136, 142, 148, 151, 154, 160, 163, 166,
	172, 175, 178, 184, 187, 190, 196, 199, 202, 208, 211, 214,
	220, 223, 226, 232, 235, 238, 241, 244, 247, 250, 253, 256,
	259, 262, 265, 268, 271, 274, 275, 277, 278, 280, 281, 283,
	284,
	-1, /* end */
};

const struct osmo_conv_code gsm0503_conv_tch_ahs_4_75 = {
	.N = 3,
	.K = 7,
	.len = 89,
	.next_output      = conv_tch_ahs_4_75_next_output,
	.next_state       = conv_tch_ahs_4_75_next_state,
	.next_term_output = conv_tch_ahs_4_75_next_term_output,
	.next_term_state  = conv_tch_ahs_4_75_next_term_state,
	.puncture         = conv_tch_ahs_4_75_puncture,
};

