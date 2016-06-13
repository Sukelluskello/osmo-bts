#ifndef _PCU_IF_H
#define _PCU_IF_H

#define PCU_SOCK_DEFAULT	"/tmp/pcu_bts"

extern int pcu_direct;

int pcu_tx_info_ind(void);
int pcu_tx_rts_req(struct gsm_bts_trx_ts *ts, uint8_t is_ptcch, uint32_t fn,
	uint16_t arfcn, uint8_t block_nr);
int pcu_tx_data_ind(struct gsm_bts_trx_ts *ts, uint8_t is_ptcch, uint32_t fn,
	uint16_t arfcn, uint8_t block_nr, uint8_t *data, uint8_t len,
	int8_t rssi);
int pcu_tx_rach_ind(struct gsm_bts *bts, int16_t qta, uint8_t ra, uint32_t fn);
int pcu_tx_time_ind(uint32_t fn);
int pcu_tx_pag_req(const uint8_t *identity_lv, uint8_t chan_needed);
int pcu_tx_pch_data_cnf(uint32_t fn, uint8_t *data, uint8_t len);

int pcu_sock_init(const char *path);
void pcu_sock_exit(void);

int  pcu_tx_mm_start_meas(struct gsm_bts *bts, uint8_t meas_id);
int  pcu_tx_mm_meas_res_req(struct gsm_bts *bts, uint8_t meas_id);
int  pcu_tx_mm_stop_meas(struct gsm_bts *bts, uint8_t meas_id);

#endif /* _PCU_IF_H */
