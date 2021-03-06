/*
 * File	: at_ipCmd.h
 * This file is part of Espressif's AT+ command set program.
 * Copyright (C) 2013 - 2016, Espressif Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __AT_IPCMD_H
#define __AT_IPCMD_H

#include "at.h"
#include "c_types.h"
#include "user_interface.h"
#include "at_version.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "driver/uart.h"
#include <stdlib.h>
#include "upgrade.h"


typedef enum
{
  teClient,
  teServer
}teType;

typedef struct
{
	BOOL linkEn;
  BOOL teToff;
	uint8_t linkId;
	teType teType;
	uint8_t repeaTime;
	uint8_t changType;
	uint8 remoteIp[4];
	int32_t remotePort;
	struct espconn *pCon;
}at_linkConType;





void at_testCmdGeneric(uint8_t id);

void at_setupCmdCifsr(uint8_t id, char *pPara);
void at_exeCmdCifsr(uint8_t id);
void getShowIP_MAC(uint8_t p_ip_param,uint8_t p_mac_param,bool msg,uint8_t id);

void at_exeCmdCipstatus(uint8_t id);

void at_testCmdCipstart(uint8_t id);
void at_setupCmdCipstart(uint8_t id, char *pPara);

void at_setupCmdCipclose(uint8_t id, char *pPara);
void at_exeCmdCipclose(uint8_t id);

void at_setupCmdCipsend(uint8_t id, char *pPara);
void at_exeCmdCipsend(uint8_t id);

void at_queryCmdCipmux(uint8_t id);
void at_setupCmdCipmux(uint8_t id, char *pPara);
uint8_t at_setupCmdCipmuxEsp(uint8_t mux);

void at_setupCmdCipserver(uint8_t id, char *pPara);
uint8_t at_setupCmdCipserverEsp(uint8_t mode, int32_t port,int16_t timeout);

void at_queryCmdCipmode(uint8_t id);
void at_setupCmdCipmode(uint8_t id, char *pPara);
uint8_t at_setupCmdCipmodeEsp(uint8_t mode);

void at_queryCmdCipsto(uint8_t id);
void at_setupCmdCipsto(uint8_t id, char *pPara);

void at_sendData(char *pdata, unsigned short len,uint8_t linkId);
void ipSendData(uint8_t *pAtRcvData,uint8_t linkid,uint16_t length);
void sendData(char *pdata, unsigned short len,uint8_t linkId);

#endif
