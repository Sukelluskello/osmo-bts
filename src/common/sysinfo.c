/* (C) 2011 by Harald Welte <laforge@gnumonks.org>
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

#include <stdint.h>

#include <osmocom/gsm/gsm_utils.h>
#include <osmocom/gsm/sysinfo.h>

#include <osmo-bts/gsm_data.h>

#define BTS_HAS_SI(bts, sinum)	((bts)->si_valid & (1 << sinum))

/* Apply the rules from 05.02 6.3.1.3 Mapping of BCCH Data */
uint8_t *bts_sysinfo_get(struct gsm_bts *bts, struct gsm_time *g_time)
{
	struct gsm_bts_role_bts *btsb = bts_role_bts(bts);
	unsigned int tc4_cnt = 0;
	unsigned int tc4_sub[4];

	/* System information type 2 bis or 2 ter messages are sent if
	 * needed, as determined by the system operator.  If only one of
	 * them is needed, it is sent when TC = 5.  If both are needed,
	 * 2bis is sent when TC = 5 and 2ter is sent at least once
	 * within any of 4 consecutive occurrences of TC = 4.  */
	/* System information type 2 quater is sent if needed, as
	 * determined by the system operator. If sent on BCCH Norm, it
	 * shall be sent when TC = 5 if neither of 2bis and 2ter are
	 * used, otherwise it shall be sent at least once within any of
	 * 4 consecutive occurrences of TC = 4. If sent on BCCH Ext, it
	 * is sent at least once within any of 4 consecutive occurrences
	 * of TC = 5. */
	/* System Information type 9 is sent in those blocks with
	 * TC = 4 which are specified in system information type 3 as
	 * defined in 3GPP TS 04.08.  */
	/* System Information Type 13 need only be sent if GPRS support
	 * is indicated in one or more of System Information Type 3 or 4
	 * or 7 or 8 messages. These messages also indicate if the
	 * message is sent on the BCCH Norm or if the message is
	 * transmitted on the BCCH Ext. In the case that the message is
	 * sent on the BCCH Norm, it is sent at least once within any of
	 * 4 consecutive occurrences of TC = 4. */

	/* We only implement BCCH Norm at this time */
	switch (g_time->tc) {
	case 0:
		/* System Information Type 1 need only be sent if
		 * frequency hopping is in use or when the NCH is
		 * present in a cell. If the MS finds another message
		 * when TC = 0, it can assume that System Information
		 * Type 1 is not in use.  */
		return GSM_BTS_SI(bts, SYSINFO_TYPE_1);
	case 1:
		/* A SI 2 message will be sent at least every time TC = 1. */
		return GSM_BTS_SI(bts, SYSINFO_TYPE_2);
	case 2:
		return GSM_BTS_SI(bts, SYSINFO_TYPE_3);
	case 3:
		return GSM_BTS_SI(bts, SYSINFO_TYPE_4);
	case 4:
		/* iterate over 2ter, 2quater, 9, 13 */
		/* determine how many SI we need to send on TC=4,
		 * and which of them we send when */
		if (BTS_HAS_SI(bts, SYSINFO_TYPE_2ter) &&
		    BTS_HAS_SI(bts, SYSINFO_TYPE_2bis)) {
			tc4_sub[tc4_cnt] = SYSINFO_TYPE_2ter;
			tc4_cnt += 1;
		}
		if (BTS_HAS_SI(bts, SYSINFO_TYPE_2quater) &&
		    (BTS_HAS_SI(bts, SYSINFO_TYPE_2bis) ||
		     BTS_HAS_SI(bts, SYSINFO_TYPE_2ter))) {
			tc4_sub[tc4_cnt] = SYSINFO_TYPE_2quater;
			tc4_cnt += 1;
		}
		if (BTS_HAS_SI(bts, SYSINFO_TYPE_13)) {
			tc4_sub[tc4_cnt] = SYSINFO_TYPE_13;
			tc4_cnt += 1;
		}
		if (BTS_HAS_SI(bts, SYSINFO_TYPE_9)) {
			/* FIXME: check SI3 scheduling info! */
			tc4_sub[tc4_cnt] = SYSINFO_TYPE_9;
			tc4_cnt += 1;
		}
		/* simply send SI2 if we have nothing else to send */
		if (tc4_cnt == 0)
			return GSM_BTS_SI(bts, SYSINFO_TYPE_2);
		else {
			/* increment static counter by one, modulo count */
			btsb->si.tc4_ctr = (btsb->si.tc4_ctr + 1) % tc4_cnt;
			return GSM_BTS_SI(bts, tc4_sub[btsb->si.tc4_ctr]);
		}
	case 5:
		/* 2bis, 2ter, 2quater */
		if (BTS_HAS_SI(bts, SYSINFO_TYPE_2bis) &&
		    !BTS_HAS_SI(bts, SYSINFO_TYPE_2ter))
			return GSM_BTS_SI(bts, SYSINFO_TYPE_2bis);

		else if (BTS_HAS_SI(bts, SYSINFO_TYPE_2ter) &&
			 !BTS_HAS_SI(bts, SYSINFO_TYPE_2bis))
			return GSM_BTS_SI(bts, SYSINFO_TYPE_2ter);

		else if (BTS_HAS_SI(bts, SYSINFO_TYPE_2bis) &&
			 BTS_HAS_SI(bts, SYSINFO_TYPE_2ter))
			return GSM_BTS_SI(bts, SYSINFO_TYPE_2bis);

		else if (BTS_HAS_SI(bts, SYSINFO_TYPE_2quater) &&
			 !BTS_HAS_SI(bts, SYSINFO_TYPE_2bis) &&
			 !BTS_HAS_SI(bts, SYSINFO_TYPE_2ter))
			return GSM_BTS_SI(bts, SYSINFO_TYPE_2quater);
		break;
	case 6:
		return GSM_BTS_SI(bts, SYSINFO_TYPE_3);
	case 7:
		return GSM_BTS_SI(bts, SYSINFO_TYPE_4);
	}

	return NULL;
}

uint8_t *lchan_sacch_get(struct gsm_lchan *lchan)
{
	uint32_t tmp;

	for (tmp = lchan->si.last + 1; tmp != lchan->si.last; tmp = (tmp + 1) % _MAX_SYSINFO_TYPE) {
		if (lchan->si.valid & (1 << tmp)) {
			lchan->si.last = tmp;
			return lchan->si.buf[tmp];
		}
	}
	return NULL;
}
