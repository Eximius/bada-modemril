/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
 *
 * libmocha-ipc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libmocha-ipc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libmocha-ipc.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __LBS_H__
#define __LBS_H__

#include <types.h>
#include <radio.h>

typedef enum
{
	LBS_PKT_INIT=0,              //0
	LBS_PKT_GET_POSITION,
	LBS_PKT_CANCEL_POSITION,
	LBS_PKT_SET_PARAMETER,
	LBS_PKT_GET_PARAMETER,
	LBS_PKT_DELETE_GPS_DATA,
	LBS_PKT_REQ_RSSI,
	LBS_PKT_SUPL_RESPONSE_MT,
	LBS_PKT_SUPL_RECEIVE_WAPPUSH,
	LBS_PKT_SUPL_SEND_COMPLETE,

	LBS_PKT_SUPL_DATA_RECEIVED,  //10
	LBS_PKT_SUPL_NETWORK_ERROR,
	LBS_PKT_SUPL_CHANGE_NETWORK_STATE,
	LBS_PKT_XTRA_INJECT_TIME_INFO,
	LBS_PKT_XTRA_INITIATE_DOWNLOAD,
	LBS_PKT_XTRA_INJECT_DATA,
	LBS_PKT_XTRA_INJECT_DATA_ERROR,
	LBS_PKT_XTRA_QUERY_DATA_VILIDITY,
	LBS_PKT_XTRA_SET_AUTO_DOWN_INTERVAL,
	LBS_PKT_BASEBAND_GET_LOCATION_ID, //19

	LBS_PKT_INIT_IND=21, //21
	LBS_PKT_GET_POSITION_IND,
	LBS_PKT_CANCEL_POSITION_IND,
	LBS_PKT_SET_PARAMETER_IND,
	LBS_PKT_GET_PARAMETER_IND,
	LBS_PKT_RECEIVED_MT_LR,
	LBS_PKT_ERROR_IND,
	LBS_PKT_NMEA_DATA_IND,
	LBS_PKT_STATE_IND,

	LBS_PKT_RADIO_INFO_IND, //30
	LBS_PKT_SUPL_HASH_IND,
	LBS_PKT_SUPL_OPEN_CONN_IND,
	LBS_PKT_SUPL_SEND_DATA_IND,
	LBS_PKT_SUPL_RECEIVE_COMPLETE,
	LBS_PKT_SUPL_CLOSE_CONN_IND,
	LBS_PKT_XTRA_OPERATION_IND,
	LBS_PKT_XTRA_QUERY_DATA_VALIDITY_IND,
	LBS_PKT_XTRA_INJECT_TIME_INFO_IND,
	LBS_PKT_XTRA_INJECT_DATA_IND

} LBS_PacketType;

struct lbsPacketHeader {
	uint32_t type;
	uint32_t size;
	uint32_t subType; //seems to be always 1
} __attribute__((__packed__));

void lbs_init(void);
void lbs_send_init(uint32_t var);
void lbs_send_packet(uint32_t type, uint32_t size, uint32_t subType, void* buf);

#endif
