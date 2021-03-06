/* OsmoBTS lchan interface */

/* (C) 2012 by Holger Hans Peter Freyther
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

#include <osmocom/core/logging.h>
#include <osmo-bts/logging.h>
#include <osmo-bts/gsm_data.h>

void lchan_set_state(struct gsm_lchan *lchan, enum gsm_lchan_state state)
{
	DEBUGP(DL1C, "%s state %s -> %s\n",
	       gsm_lchan_name(lchan),
	       gsm_lchans_name(lchan->state),
	       gsm_lchans_name(state));
	lchan->state = state;
}
