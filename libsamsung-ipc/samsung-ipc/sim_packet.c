/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
 *
 * Implemented as per the Mocha AP-CP protocol analysis done by Dominik Marszk
 *
 * libsamsung-ipc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libsamsung-ipc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libsamsung-ipc.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <getopt.h>

#include <radio.h>
#include <sim_packet.h>

#define LOG_TAG "RIL-SIM"
#include <utils/Log.h>

/*
 * TODO: Implement handling of all the SIM packets
 *
 */

static struct simDataRequest simData;
static uint16_t current_simDataType;
static uint32_t current_simDataCount;
static uint32_t current_simDataCounter;

void modem_response_sim(struct modem_io *resp)
{
	DEBUG_I("Entering\n");
	int32_t retval, count;
	uint32_t sid;
	struct simPacketHeader *simHeader;
	struct simPacket sim_packet;

	struct modem_io request;
    void *frame;
    uint8_t *payload;
    uint32_t frame_length;

    struct fifoPacketHeader *fifoHeader;

	DEBUG_I("Frame header = 0x%x\n Frame type = 0x%x\n Frame length = 0x%x\n", resp->magic, resp->cmd, resp->datasize);

	hexdump(resp->data, resp->datasize);

    simHeader = (struct simPacketHeader *)(resp->data);
    sim_packet.simBuf = (uint8_t *)(resp->data + sizeof(struct simPacketHeader));

	DEBUG_I("Sim Packet type = 0x%x\n Sim Packet sub-type = 0x%x\n Sim Packet length = 0x%x\n", simHeader->type, simHeader->subType, simHeader->bufLen);

	if(simHeader->type != 0)
	{
		switch (simHeader->subType)
		{
		case 0x00:
			DEBUG_I("SIM_PACKET OemSimAtkInjectDisplayTextInd rcvd\n");
			
			/*struct oemSimPacketHeader *oem_header;
			struct oemSimPacket oem_packet;

			oem_header = (struct oemSimPacketHeader *)(sim_packet.respBuf);
			oem_packet.oemBuf = (uint8_t *)(sim_packet.respBuf + sizeof(struct oemSimPacketHeader));

			DEBUG_I("Sim oem type = 0x%x\n Sim Packet sub-type = 0x%x\n Oem length = 0x%x\n", oem_header->oemType, oem_header->packetSubType, oem_header->oemBufLen);

			hexdump(oem_packet.oemBuf, oem_header->oemBufLen);
*/
			break;
		default :
			DEBUG_I("Unknown SIM subType %d\n", simHeader->subType);
			break;
		}
	}
	else
	{
		if(simHeader->subType >= SESSION_SUBTYPE_DIFF)
		{
			sid = simHeader->subType-SESSION_SUBTYPE_DIFF;
			if(sid < SIM_SESSION_COUNT)
			{
				DEBUG_I("In SIM OEM response, sid = %d\n", sid);
				switch(sid)
				{
					case 1:
					case 2: //in this session first packet with sim info is being send
						//TODO: these 2 sids are somewhat special - apps does switch some bool if they are used, not sure what way they are special.
						sim_parse_session_event(sim_packet.simBuf, simHeader->bufLen); //sid is stored in buf too
		        		sim_dummy1_to_modem(4);
		        		break;
					case 3:
						sim_data_request_to_modem(0x5, 0x6F42);
						current_simDataType = 0x6F42;
						simData.dataCounter = 0x00;
						break;
					case 4:
						if (simData.dataCounter == 0)
						{
							simData.simDataType = current_simDataType;
							simData.someType = 0x1C;
							simData.simInd1 = 0x02;
							simData.simInd2 = 0x01;
							simData.unk0 = 0x00;
							simData.unk1 = 0x00;
							simData.unk2 = 0x00;
							simData.unk3 = 0x00;
							simData.unk4 = 0x00;
							simData.unk5 = 0x00;
							simData.unk6 = 0x00;
							simData.unk7 = 0x00;
							simData.dataCounter = 0x01;
							memcpy(&current_simDataCount, (resp->data + 42), 4);
							sim_get_data_from_modem(0x5, &simData);
						}
						break;
					case 8:
						if(simData.dataCounter <= current_simDataCount)
						{
							DEBUG_I("Sent SIM Request type = 0x%x, packet no. %d, total packets = %d\n", simData.simDataType, simData.dataCounter, current_simDataCount);
							sim_get_data_from_modem(0x5, &simData);
						}
						else
							simData.dataCounter = 0;

						break;
					default:
						sim_parse_session_event(sim_packet.simBuf, simHeader->bufLen); //sid is stored in buf too
						break;
				}
			}
		}
		else
		{
			DEBUG_I("Unhandled SIM sub-type = %d\n", simHeader->subType);
			//sim_send_oem_req(sim_packet.simBuf, simHeader->bufLen); //bounceback packet
		}
	}

    DEBUG_I("leaving\n");

}

void sim_parse_session_event(uint8_t* buf, uint32_t bufLen)
{

}

int sim_send_oem_req(uint8_t* simBuf, uint8_t simBufLen)
{	
	//simBuf is expected to contain full oemPacket structure
	struct modem_io request;
	struct simPacket sim_packet;	
	sim_packet.header.type = 0;
	sim_packet.header.subType = ((struct oemSimPacketHeader *)(simBuf))->type;
	sim_packet.header.bufLen = simBufLen;
	sim_packet.simBuf = simBuf;
	
	uint32_t bufLen = sim_packet.header.bufLen + sizeof(struct simPacketHeader);
	uint8_t* fifobuf = malloc(bufLen);
	memcpy(fifobuf, &sim_packet.header, sizeof(struct simPacketHeader));
	memcpy(fifobuf + sizeof(struct simPacketHeader), sim_packet.simBuf, sim_packet.header.bufLen);

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_SIM;
	request.datasize = bufLen;

	request.data = fifobuf;

	hexdump(request.data, request.datasize);
	ipc_fmt_send(&request);

	free(fifobuf);
	//TODO: return nonzero in case of failure
	return 0;
}
int sim_send_other_req(void)
{
	//simBuf is expected to contain full oemPacket structure

	uint16_t dummy = 0;
	struct modem_io request;
	struct simPacket sim_packet;
	sim_packet.header.type = 1;
	sim_packet.header.subType = 0x31;
	sim_packet.header.bufLen = 2;
	sim_packet.simBuf = &dummy;

	uint32_t bufLen = sim_packet.header.bufLen + sizeof(struct simPacketHeader);
	uint8_t* fifobuf = malloc(bufLen);
	memcpy(fifobuf, &sim_packet.header, sizeof(struct simPacketHeader));
	memcpy(fifobuf + sizeof(struct simPacketHeader), sim_packet.simBuf, sim_packet.header.bufLen);

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_SIM;
	request.datasize = bufLen;

	request.data = fifobuf;

	ipc_fmt_send(&request);

	free(fifobuf);
	//TODO: return nonzero in case of failure
	return 0;
}

int sim_send_oem_data(uint8_t hSim, uint8_t packetType, uint8_t* dataBuf, uint32_t oemBufLen)
{	
	SIM_VALIDATE_SID(hSim);

	struct oemSimPacketHeader oem_header;	
	oem_header.type = packetType;
	oem_header.hSim = hSim; //session id
	oem_header.oemBufLen = oemBufLen;
	

	uint32_t simBufLen = oemBufLen + sizeof(struct oemSimPacketHeader);
	uint8_t* simBuf = malloc(simBufLen);
	memcpy(simBuf, &(oem_header), sizeof(struct oemSimPacketHeader));
	if(oemBufLen)
		memcpy(simBuf + sizeof(struct oemSimPacketHeader), dataBuf, oemBufLen);
	
	return sim_send_oem_req(simBuf, simBufLen);
	free(simBuf);

}

int sim_verify_chv(uint8_t hSim, uint8_t pinType, char* pin)
{	
	SIM_VALIDATE_SID(hSim);
	//TODO: obtain session context, check if session is busy, print exception if it is busy and return failure
	//TODO: if session is not busy, mark it busy
	uint8_t* packetBuf = malloc(10);	
	memset(packetBuf, 0x00, 10);

	packetBuf[0] = pinType;
	memcpy(packetBuf+1, pin, strlen(pin)); //max pin len is 9 digits
	if(sim_send_oem_data(hSim, 0xB, packetBuf, 10) != 0)
	{
		//TODO: mark session non-busy
		return -1;
	}
	free(packetBuf);
	return 0;
}

int sim_atk_open(uint32_t sid)
{
	//TODO: verify ATK session and create/open it and return handler to it?!
	DEBUG_I("Sending\n");
	if(sim_send_oem_data(0xA, 0x1B, NULL, 0) != 0) //0xA hSim is hardcoded in bada
		return -1;
	return 0;
}

int sim_open_to_modem(uint8_t hSim)
{
	//TODO: verify, create and initialize session, send real hSim
	DEBUG_I("Sending\n");
	if(sim_send_oem_data(0x04, 0x1, NULL, 0) != 0) //why it starts from 4? hell knows
		return -1;
	return 0;
}

int sim_dummy1_to_modem(uint8_t hSim)
{
	//TODO: verify, create and initialize session, send real hSim
	DEBUG_I("Sending\n");
	if(sim_send_oem_data(0x04, 0x2, NULL, 0) != 0) //why it starts from 4? hell knows
		return -1;
	return 0;
}

int sim_get_data_from_modem(uint8_t hSim, struct simDataRequest *simData)
{
	//TODO: verify, create and initialize session, send real hSim
	uint16_t dummy = 0x6F40;
	uint8_t *data;

	data = malloc(sizeof(struct simDataRequest));
	memcpy(data, simData,sizeof(struct simDataRequest));

	DEBUG_I("Sending\n");
	if(sim_send_oem_data(hSim, 0x7, data, sizeof(struct simDataRequest)) != 0) //why it starts from 4? hell knows
		return -1;
	simData->dataCounter += 1;
	return 0;
}

int sim_data_request_to_modem(uint8_t hSim, uint16_t simDataType)
{
	//TODO: verify, create and initialize session, send real hSim
	uint8_t *data;

	data = malloc(sizeof(simDataType));
	memcpy(data,&simDataType,sizeof(simDataType));

	DEBUG_I("Sending\n");
	if(sim_send_oem_data(hSim, 0x3, data, sizeof(simDataType)) != 0) //why it starts from 4? hell knows
		return -1;
	free(data);
	return 0;
}
