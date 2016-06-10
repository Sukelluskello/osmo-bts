#ifndef _GSM_DATA_H
#define _GSM_DATA_H

#include <osmocom/core/timer.h>
#include <osmocom/core/linuxlist.h>
#include <osmocom/gsm/lapdm.h>

#include <osmo-bts/paging.h>
#include <osmo-bts/tx_power.h>

#define GSM_FR_BITS	260
#define GSM_EFR_BITS	244

#define GSM_FR_BYTES	33	/* TS 101318 Chapter 5.1: 260 bits + 4bit sig */
#define GSM_HR_BYTES	14	/* TS 101318 Chapter 5.2: 112 bits, no sig */
#define GSM_EFR_BYTES	31	/* TS 101318 Chapter 5.3: 244 bits + 4bit sig */

#define GSM_SUPERFRAME	(26*51)			/* 1326 TDMA frames */
#define GSM_HYPERFRAME	(2048*GSM_SUPERFRAME)	/* GSM_HYPERFRAME frames */

#define GSM_BTS_AGCH_QUEUE_THRESH_LEVEL_DEFAULT 41
#define GSM_BTS_AGCH_QUEUE_THRESH_LEVEL_DISABLE 999999
#define GSM_BTS_AGCH_QUEUE_LOW_LEVEL_DEFAULT 41
#define GSM_BTS_AGCH_QUEUE_HIGH_LEVEL_DEFAULT 91

struct pcu_sock_state;
struct smscb_msg;

struct gsm_network {
	struct llist_head bts_list;
	unsigned int num_bts;
	uint16_t mcc, mnc;
	struct pcu_sock_state *pcu_state;
};

/* data structure for BTS related data specific to the BTS role */
struct gsm_bts_role_bts {
	struct {
		/* Interference Boundaries for OML */
		int16_t boundary[6];
		uint8_t intave;
	} interference;
	unsigned int t200_ms[7];
	unsigned int t3105_ms;
	struct {
		uint8_t overload_period;
		struct {
			/* Input parameters from OML */
			uint8_t load_ind_thresh;	/* percent */
			uint8_t load_ind_period;	/* seconds */
			/* Internal data */
			struct osmo_timer_list timer;
			unsigned int pch_total;
			unsigned int pch_used;
		} ccch;
		struct {
			/* Input parameters from OML */
			int16_t busy_thresh;		/* in dBm */
			uint16_t averaging_slots;
			/* Internal data */
			unsigned int total;	/* total nr */
			unsigned int busy;	/* above busy_thresh */
			unsigned int access;	/* access bursts */
		} rach;
	} load;
	uint8_t ny1;
	uint8_t max_ta;

	/* AGCH queuing */
	struct llist_head agch_queue;
	int agch_queue_length;
	int agch_max_queue_length;

	int agch_queue_thresh_level;	/* Cleanup threshold in percent of max len */
	int agch_queue_low_level;	/* Low water mark in percent of max len */
	int agch_queue_high_level;	/* High water mark in percent of max len */

	/* TODO: Use a rate counter group instead */
	uint64_t agch_queue_dropped_msgs;
	uint64_t agch_queue_merged_msgs;
	uint64_t agch_queue_rejected_msgs;
	uint64_t agch_queue_agch_msgs;
	uint64_t agch_queue_pch_msgs;

	struct paging_state *paging_state;
	char *bsc_oml_host;
	struct llist_head oml_queue;
	unsigned int rtp_jitter_buf_ms;
	struct {
		uint8_t ciphers;	/* flags A5/1==0x1, A5/2==0x2, A5/3==0x4 */
	} support;
	struct {
		uint8_t tc4_ctr;
	} si;
	struct gsm_time gsm_time;
	uint8_t radio_link_timeout;

	int ul_power_target;		/* Uplink Rx power target */

	/* used by the sysmoBTS to adjust band */
	uint8_t auto_band;

	struct {
		struct llist_head queue;	/* list of struct smscb_msg */
		struct smscb_msg *cur_msg;	/* current SMS-CB */
	} smscb_state;

	float min_qual_rach;	/* minimum quality for RACH bursts */
	float min_qual_norm;	/* minimum quality for normal daata */

	struct {
		char *sock_path;
	} pcu;
#ifdef ENABLE_LC15BTS
	/* specific to LC15 BTS */
	uint8_t max_cell_size;		/* 166 qbits */
	uint8_t diversity_mode;		/* 0: SISO A, 1: SISO B, 2: MRC */
	uint8_t pedestal_mode;		/* 0: unused TS is OFF, 1: unused TS is in minimum Tx power */
	uint8_t led_ctrl_mode;		/* 0: control by BTS, 1: not control by BTS */
	uint8_t pwr_red_step;   	/* Tx power reduction steps in 1 dB or 2 dB */
	uint8_t dsp_alive_period;	/* DSP alive timer period  */
	uint8_t tx_pwr_adj_mode;	/* 0: no auto adjust power, 1: auto adjust power using RMS detector */
	uint8_t tx_pwr_red_8psk;	/* 8-PSK maximum Tx power reduction level in dB */
#endif
};

enum lchan_ciph_state {
	LCHAN_CIPH_NONE,
	LCHAN_CIPH_RX_REQ,
	LCHAN_CIPH_RX_CONF,
	LCHAN_CIPH_RXTX_REQ,
	LCHAN_CIPH_RX_CONF_TX_REQ,
	LCHAN_CIPH_RXTX_CONF,
};

#define bts_role_bts(x)	((struct gsm_bts_role_bts *)(x)->role)

#include "openbsc/gsm_data_shared.h"

void lchan_set_state(struct gsm_lchan *lchan, enum gsm_lchan_state state);

/* cipher code */
#define CIPHER_A5(x) (1 << (x-1))

int bts_supports_cipher(struct gsm_bts_role_bts *bts, int rsl_cipher);


#endif /* _GSM_DATA_H */
