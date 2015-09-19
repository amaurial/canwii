/*
 * File	: at.h
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
#ifndef __AT_H
#define __AT_H

#include "c_types.h"
#include "canwii.h"

#define at_recvTaskPrio        0
#define at_recvTaskQueueLen    64

#define CMD_BUFFER_SIZE 128
//#define DEBUG 1
//#define VERBOSE 1


#define at_procTaskPrio        1
#define at_procTaskQueueLen    1
#ifdef VERBOSE
    #define at_backOk        uart0_sendStr("\nOK\n")
    #define at_backError     uart0_sendStr("\nERROR\n")
    //#define at_backTeError   "+CTE ERROR: %d\n"
#else
    #define at_backOk        {uart_tx_one_char(CANWII_OK);}//;uart0_sendStr("\n");}
    #define at_backError     {uart_tx_one_char(CANWII_ERR);}//;uart0_sendStr("\n");}
    //#define at_backTeError   "%c" + CANWII_TE_ERR
#endif // VERBOSE

#define CMD_AT 0x0a
#define CMD_RST 0x0b
#define CMD_GMR 0x0c
#define CMD_GSLP 0x0d
#define CMD_CWMODE 0x0f
#define CMD_CWJAP 0x10
#define CMD_CWLAP 0x11
#define CMD_CWQAP 0x12
#define CMD_CWSAP 0x13
#define CMD_CWLIF 0x14
#define CMD_CWDHCP 0x15
#define CMD_CIFSR 0x16
#define CMD_CIPSTAMAC 0x17
#define CMD_CIPAPMAC 0x18
#define CMD_CIPSTA 0x19
#define CMD_CIPAP 0x1a
#define CMD_CIPSTATUS 0x1b
#define CMD_CIPSTART 0x1c
#define CMD_CIPCLOSE 0x1d
#define CMD_CIPSEND 0x1e
#define CMD_CIPMUX 0x1f
#define CMD_CIPSERVER 0x20
#define CMD_CIPMODE 0x21
#define CMD_CIPSTO 0x22
#define CMD_CIUPDATE 0x23
#define CMD_MPINFO 0x27
#define CMD_IPD 0x28
#define CMD_MERG_CONFIG_AP_EXT 0x29
#define CMD_MERG_CONFIG_AP 0x2a

#define CMD_QUERY '?'
#define CMD_EQUAL '='
#define CMD_TEST '!'

#define RSP_CONNECTED 0xA1
#define RSP_DISCONNECTED 0xA2
#define RSP_OK 0xA3
#define RSP_TCP_ERROR 0xA4
#define RSP_IP_ERROR 0xA5
#define RSP_NOAP 0xA6
#define RSP_FAIL_CONNECT 0xA7
#define RSP_MISS_PARAM_ERROR 0xA8
#define RSP_CLOSED 0xA9
#define RSP_SENT 0xAA
#define RSP_DNS_FAIL 0xAB
#define RSP_NOID_ERROR 0xAC
#define RSP_LINK_TYPE_ERROR 0xAD
#define RSP_BUSY_PROCESSING 0xAE
#define RSP_BUSY_SENDING 0xAF


#define TCP_SERVER_TIMEOUT 600//0 to 7200 seconds

//number of connections
#define at_linkMax 10
#define MSG_MAX_BUFFER_SIZE 2048

#define NULLPARAM 255

typedef enum{
  at_statIdle,
  at_statRecving,
  at_statProcess,
  at_statIpSending,
  at_statIpSended,
  at_statIpTraning
}at_stateType;

typedef enum{
  m_init,
  m_wact,
  m_gotip,
  m_linked,
  m_unlink,
  m_wdact
}at_mdStateType;

typedef struct
{
	char *at_cmdName;//command name
	int8_t at_cmdLen;//command size
	int8_t at_cmdCode;//command hexa code
  void (*at_testCmd)(uint8_t id);//test function
  void (*at_queryCmd)(uint8_t id);//query function
  void (*at_setupCmd)(uint8_t id, char *pPara);//setup function
  void (*at_exeCmd)(uint8_t id);//execute function
}at_funcationType;

typedef struct
{
  uint32_t baud;
  uint32_t saved;
  char ssid[16];
  uint8_t ssidlen;
  char passwd[16];
  uint8_t passwdlen;
  uint8_t channel;
  uint8_t wpa;
  uint8_t cwmode;
  uint8_t cwmux;
  uint8_t port;
  uint16_t timeout;//milliseconds
  uint8_t tcp_udp_mode; //0=TCP,1=UDP
  uint8_t dhcp_enable;//0=enable dhcp,1=disable
  uint8_t dhcp_mode;//0=ESP8266 softAP,1=ESP8266 station,2=both softAP and station
  uint8_t server_mode;//0=server,1=client
  uint8_t cmdid;
  uint8_t cmdsubid;
  uint8_t state;
}esp_StoreType;



typedef enum{
    MSG_MUX=11,
    MSG_RESTART=12,
    MSG_LINK_SET_FAIL=13,
    MSG_IP_MODE=14,
    MSG_TOO_LONG=15,
    MSG_TYPE_ERROR=16,
    MSG_LINK_DONE=17,
    MSG_NO_CHANGE=18,
    MSG_TCP_SERVER_FAIL=19,
    MSG_FAIL=20,
    MSG_NOAP=21,
    MSG_CLIENT_CONNECTED=22,
    MSG_CLIENT_DISCONNECTED=23,
    MSG_SEND=24,
    MSG_CONNECT=25,
    MSG_CLOSED=26,
    MSG_DNS_FAIL=27,
    MSG_ID_ERROR=28,
    MSG_LINK_TYPE_ERROR=29,
    MSG_IP_ERROR=30,
    MSG_ENTRY_ERROR=31,
    MSG_MISS_PARAM=32,
    MSG_ALREADY_CONNECT=33,
    MSG_CONNECT_FAIL=34,
    MSG_FAIL_READ_FLASH=35,
    MSG_FAIL_WRITE_FLASH=36,
    MSG_FAIL_TCP_MAX_CONN=37
}enum_msgType;


typedef struct
{
    enum_msgType msgid;
    uint8_t param0;
}struct_MSGType;

struct_MSGType generalMSG;


void at_init(void);
void at_cmdProcess(uint8_t *pAtRcvData);

#endif
