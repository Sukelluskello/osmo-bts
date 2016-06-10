/* OsmoBTS common TCH code */

/* (C) 2011-2012 by Harald Welte <laforge@gnumonks.org>
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

#include <osmo-bts/gsm_data.h>
#include <osmo-bts/logging.h>
#include <osmo-bts/l1sap.h>

extern struct msgb *l1p_msgb_alloc(void);
extern uint8_t *get_payload_addr(struct msgb *msg);
extern void set_payload_type(struct msgb *msg, struct gsm_lchan *lchan);
extern void set_payload_size(struct msgb *msg, uint8_t size);

int add_l1sap_header(struct gsm_lchan *lchan, struct msgb *rmsg,
		     struct gsm_bts_trx *trx, uint8_t chan_nr, uint32_t fn)
{
	struct osmo_phsap_prim *l1sap;

	LOGP(DL1C, LOGL_DEBUG, "%s Rx -> RTP: %s\n",
	     gsm_lchan_name(lchan), osmo_hexdump(rmsg->data, rmsg->len));

	rmsg->l2h = rmsg->data;
	msgb_push(rmsg, sizeof(*l1sap));
	rmsg->l1h = rmsg->data;
	l1sap = msgb_l1sap_prim(rmsg);
	osmo_prim_init(&l1sap->oph, SAP_GSM_PH, PRIM_TCH, PRIM_OP_INDICATION,
		       rmsg);
	l1sap->u.tch.chan_nr = chan_nr;
	l1sap->u.tch.fn = fn;

	return l1sap_up(trx, l1sap);
}

struct msgb *gen_empty_tch_msg(struct gsm_lchan *lchan)
{
	struct msgb *msg;
	uint8_t *l1_payload;

	msg = l1p_msgb_alloc();
	if (!msg)
		return NULL;

	l1_payload = get_payload_addr(msg);

	switch (lchan->tch_mode) {
	case GSM48_CMODE_SPEECH_AMR:
		set_payload_type(msg, lchan);
		if (lchan->tch.last_sid.len) {
			memcpy(l1_payload, lchan->tch.last_sid.buf,
				lchan->tch.last_sid.len);
			set_payload_size(msg, lchan->tch.last_sid.len + 1);
		} else {
			/* FIXME: decide if we should send SPEECH_BAD or
			 * SID_BAD */
#if 0
			*payload_type = GsmL1_TchPlType_Amr_SidBad;
			memset(l1_payload, 0xFF, 5);
			msu_param->u8Size = 5 + 3;
#else
			/* send an all-zero SID */
			set_payload_size(msg, 8);
#endif
		}
		break;
	default:
		msgb_free(msg);
		msg = NULL;
		break;
	}

	return msg;
}
