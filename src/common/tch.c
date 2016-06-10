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

#include <stdbool.h>

#include <osmocom/codec/codec.h>
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

static inline bool fn_chk(uint8_t *t, uint32_t fn)
{
	uint8_t i;
	for (i = 0; i < ARRAY_SIZE(t); i++)
		if (fn % 104 == t[i])
			return false;
	return true;
}

static bool dtx_sched_optional(struct gsm_lchan *lchan, uint32_t fn)
{
	/* 3GPP TS 45.008 § 8.3 */
	uint8_t f[] = { 52, 53, 54, 55, 56, 57, 58, 59 },
		h0[] = { 0, 2, 4, 6, 52, 54, 56, 58 },
		h1[] = { 14, 16, 18, 20, 66, 68, 70, 72 };
	if (lchan->tch_mode == GSM48_CMODE_SPEECH_V1) {
		if (lchan->type == GSM_LCHAN_TCH_F)
			return fn_chk(f, fn);
		else
			return fn_chk(lchan->nr ? h1 : h0, fn);
	}
	return false;
}

static bool repeat_last_sid(struct gsm_lchan *lchan, struct msgb *msg)
{
	uint8_t *l1_payload = get_payload_addr(msg);
	if (lchan->tch.last_sid.len) {
		memcpy(l1_payload, lchan->tch.last_sid.buf,
		       lchan->tch.last_sid.len);
		set_payload_size(msg, lchan->tch.last_sid.len + 1);
		return true;
	}
	return false;
}

/* store the last SID frame in lchan context */
void save_last_sid(struct gsm_lchan *lchan, uint8_t *l1_payload, size_t length,
		   uint32_t fn, bool update)
{
	size_t copy_len = OSMO_MIN(length + 1,
				   ARRAY_SIZE(lchan->tch.last_sid.buf));

	lchan->tch.last_sid.len = copy_len;
	lchan->tch.last_sid.fn = fn;
	lchan->tch.last_sid.is_update = update;

	memcpy(lchan->tch.last_sid.buf, l1_payload, copy_len);
}

struct msgb *gen_empty_tch_msg(struct gsm_lchan *lchan, uint32_t fn)
{
	struct msgb *msg;

	msg = l1p_msgb_alloc();
	if (!msg)
		return NULL;

	switch (lchan->tch_mode) {
	case GSM48_CMODE_SPEECH_AMR:
		/* according to 3GPP TS 26.093 A.5.1.1: */
		if (lchan->tch.last_sid.is_update) {
			/* SID UPDATE should be repeated every 8th frame */
			if (fn - lchan->tch.last_sid.fn < 7) {
				msgb_free(msg);
				return NULL;
			}
		} else {
			/* 3rd frame after SID FIRST should be SID UPDATE */
			if (fn - lchan->tch.last_sid.fn < 3) {
				msgb_free(msg);
				return NULL;
			}
		}
		if (repeat_last_sid(lchan, msg))
			return msg;
		else {
			LOGP(DL1C, LOGL_NOTICE, "Have to send AMR frame on TCH "
			     "(FN=%u) but SID buffer is empty - sent NO_DATA\n",
			     fn);
			osmo_amr_rtp_enc(get_payload_addr(msg), 0, AMR_NO_DATA,
					 AMR_GOOD);
			return msg;
		}
		break;
	case GSM48_CMODE_SPEECH_V1:
		/* unlike AMR, FR & HR schedued based on absolute FN value */
		if (dtx_sched_optional(lchan, fn)) {
			msgb_free(msg);
			return NULL;
		}
		if (repeat_last_sid(lchan, msg))
			return msg;
		else {
			LOGP(DL1C, LOGL_NOTICE, "Have to send V1 frame on TCH "
			     "(FN=%u) but SID buffer is empty - sent nothing\n",
			     fn);
			return NULL;
		}
		break;
	case GSM48_CMODE_SPEECH_EFR:
		if (dtx_sched_optional(lchan, fn)) {
			msgb_free(msg);
			return NULL;
		}
		if (repeat_last_sid(lchan, msg))
			return msg;
		else {
			LOGP(DL1C, LOGL_NOTICE, "Have to send EFR frame on TCH "
			     "(FN=%u) but SID buffer is empty - sent nothing\n",
			     fn);
			return NULL;
		}
		break;
	default:
		msgb_free(msg);
		msg = NULL;
		break;
	}

	return msg;
}
