#ifndef _OML_H
#define _OML_H
#include <osmo-bts/pcuif_proto.h>

struct gsm_bts;
struct gsm_abis_mo;
struct msgb;
struct gsm_lchan;

struct gsm_failure_evt_rep {
	uint8_t event_type;
	uint8_t event_serverity;
	uint8_t cause_type;
	uint16_t event_cause;
	char *add_text;
};

/* FIXME: can move to libosmocore */
enum abis_nm_msgtype_ipacc_appended {
	NM_MT_IPACC_START_MEAS_ACK 	= 0xde,
	NM_MT_IPACC_MEAS_RES_REQ_NACK	= 0xfc,
	NM_MT_IPACC_START_MEAS_NACK	= 0xfd,
	NM_MT_IPACC_STOP_MEAS_ACK 	= 0xdf,
	NM_MT_IPACC_STOP_MEAS_NACK	= 0xfe,
};

enum abis_mm_event_causes {
	/* Critical causes */
	NM_MM_EVT_CRIT_SW_FATAL		= 0x0000,
	NM_MM_EVT_CRIT_PROC_STOP	= 0x0002,
	NM_MM_EVT_CRIT_RTP_TOUT		= 0x032c,
	NM_MM_EVT_CRIT_BOOT_FAIL	= 0x0401,
	/* Major causes */
	NM_MM_EVT_MAJ_UKWN_MSG		= 0x0002,
	NM_MM_EVT_MAJ_RSL_FAIL		= 0x0309,
	NM_MM_EVT_MAJ_UNSUP_ATTR	= 0x0318,
	NM_MM_EVT_MAJ_NET_CONGEST	= 0x032b,
	/* Minor causes */
	NM_MM_EVT_MIN_PAG_TAB_FULL	= 0x0401,
	/* Warning causes */
	NM_MM_EVT_WARN_SW_WARN		= 0x0001,

};

int oml_init(void);
int down_oml(struct gsm_bts *bts, struct msgb *msg);

struct msgb *oml_msgb_alloc(void);
int oml_send_msg(struct msgb *msg, int is_mauf);
int oml_mo_send_msg(struct gsm_abis_mo *mo, struct msgb *msg, uint8_t msg_type);
int oml_mo_opstart_ack(struct gsm_abis_mo *mo);
int oml_mo_opstart_nack(struct gsm_abis_mo *mo, uint8_t nack_cause);
int oml_mo_statechg_ack(struct gsm_abis_mo *mo);
int oml_mo_statechg_nack(struct gsm_abis_mo *mo, uint8_t nack_cause);

/* Change the state and send STATE CHG REP */
int oml_mo_state_chg(struct gsm_abis_mo *mo, int op_state, int avail_state);

/* First initialization of MO, does _not_ generate state changes */
void oml_mo_state_init(struct gsm_abis_mo *mo, int op_state, int avail_state);

/* Update admin state and send ACK/NACK */
int oml_mo_rf_lock_chg(struct gsm_abis_mo *mo, uint8_t mute_state[8],
		       int success);

/* Transmit STATE CHG REP even if there was no state change */
int oml_tx_state_changed(struct gsm_abis_mo *mo);

int oml_mo_tx_sw_act_rep(struct gsm_abis_mo *mo);

int oml_fom_ack_nack(struct msgb *old_msg, uint8_t cause);

int oml_mo_fom_ack_nack(struct gsm_abis_mo *mo, uint8_t orig_msg_type,
			uint8_t cause);

/* Configure LAPDm T200 timers for this lchan according to OML */
int oml_set_lchan_t200(struct gsm_lchan *lchan);
extern const unsigned int oml_default_t200_ms[7];

/* Transmit failure event report */
int oml_tx_failure_event_rep(struct gsm_abis_mo *mo, struct gsm_failure_evt_rep failure_evt_rep);

/* Transmit start/stop/request measurement messages*/
int oml_tx_mm_start_meas_ack_nack(struct gsm_abis_mo *mo, uint8_t meas_id, uint8_t nack_cause);
int oml_tx_mm_meas_res_req_nack(struct gsm_abis_mo *mo, uint8_t meas_id, uint8_t nack_cause);
int oml_tx_mm_meas_res_resp(struct gsm_abis_mo *mo, struct gsm_pcu_if_meas_resp meas_resp);
int oml_tx_mm_stop_meas_ack_nack(struct gsm_abis_mo *mo, uint8_t meas_id, uint8_t nack_cause);


#endif // _OML_H */
