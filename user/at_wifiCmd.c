/*
 * File	: at_wifiCmd.c
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
#include "at_wifiCmd.h"

at_mdStateType mdState = m_unlink;

extern BOOL specialAtState;
extern at_stateType at_state;
extern at_funcationType at_fun[];

uint8_t at_wifiMode;
os_timer_t at_japDelayChack;
//struct_MSGType generalMSG;

/** @defgroup AT_WSIFICMD_Functions
  * @{
  */


/**
  * @brief  Query commad of set wifi mode.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_queryCmdCwmode(uint8_t id)
{
  char temp[32];

  at_wifiMode = wifi_get_opmode();
  #ifdef VERBOSE
    os_sprintf(temp, "%s:%d\n", at_fun[id].at_cmdName, at_wifiMode);
  #else
    os_sprintf(temp, "%c%c%d%c", CANWII_SOH, at_fun[id].at_cmdCode, at_wifiMode,CANWII_EOH);
  #endif // VERBOSE

  uart0_sendStr(temp);
  at_backOk;
}

/**
  * @brief  Setup commad of set wifi mode.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdCwmode(uint8_t id, char *pPara)
{
  uint8_t mode;
  pPara++;
  mode = atoi(pPara);
  if (at_setupCmdCwmodeEsp(mode)==0){
    at_backOk;
    return;
  }
  at_backError;
}

uint8_t ICACHE_FLASH_ATTR
at_setupCmdCwmodeEsp(uint8_t mode)
{
/*
  if(mode == at_wifiMode)
  {
    return 0;
  }
  */
  if((mode >= 1) && (mode <= 3))
  {
    ETS_UART_INTR_DISABLE();
    wifi_set_opmode(mode);
    ETS_UART_INTR_ENABLE();
    at_wifiMode = mode;
    return 0;
  }
  return 1;
}


/**
  * @brief  Transparent data through ip.
  * @param  arg: no used
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_japChack(void *arg)
{
  static uint8_t chackTime = 0;
  uint8_t japState;
  char temp[32];
  struct_MSGType generalMSG;

  os_timer_disarm(&at_japDelayChack);
  chackTime++;
  japState = wifi_station_get_connect_status();
  if(japState == STATION_GOT_IP)
  {
    chackTime = 0;
    at_backOk;
    specialAtState = TRUE;
    at_state = at_statIdle;
    return;
  }
  else if(chackTime >= 7)
  {
    wifi_station_disconnect();
    chackTime = 0;
    #ifdef VERBOSE
        os_sprintf(temp,"+CWJAP:%d\n",japState);
    #else
        os_sprintf(temp,"%c%c%d%c",CANWII_SOH,CMD_CWJAP,japState,CANWII_EOH);
    #endif // VERBOSE

    uart0_sendStr(temp);

    generalMSG.msgid=MSG_FAIL;
    generalMSG.param0=NULLPARAM;
    sendGeneralMsg(generalMSG);
    specialAtState = TRUE;
    at_state = at_statIdle;
    return;
  }
  os_timer_arm(&at_japDelayChack, 2000, 0);

}

/**
  * @brief  Query commad of module as wifi ap.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_queryCmdCwsap(uint8_t id)
{
  struct softap_config apConfig;
  char temp[128];

  if(at_wifiMode == STATION_MODE)
  {
    at_backError;
    return;
  }
  wifi_softap_get_config(&apConfig);
  #ifdef VERBOSE
    os_sprintf(temp,"%s:\"%s\",\"%s\",%d,%d\n",
             at_fun[id].at_cmdName,
             apConfig.ssid,
             apConfig.password,
             apConfig.channel,
             apConfig.authmode);
  #else
    os_sprintf(temp,"%c%c%s,%s,%d,%d%c",CANWII_SOH,
             at_fun[id].at_cmdCode,
             apConfig.ssid,
             apConfig.password,
             apConfig.channel,
             apConfig.authmode,CANWII_EOH);
  #endif // VERBOSE

  uart0_sendStr(temp);
  at_backOk;
}

/**
  * @brief  Setup commad of module as wifi ap.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdCwsap(uint8_t id, char *pPara)
{
  int8_t len,passLen;
  struct softap_config apConfig;

  os_bzero(&apConfig, sizeof(struct softap_config));
  wifi_softap_get_config(&apConfig);

  pPara++;
  len = at_dataStrCpy(apConfig.ssid, pPara, 32);
  apConfig.ssid_len = len;

  if(len < 1)
  {
    at_backError;
    return;
  }
  pPara += (len+3);
  passLen = at_dataStrCpy(apConfig.password, pPara, 64);
  if(passLen == -1 )
  {
    at_backError;
    return;
  }
  pPara += (passLen+3);
  apConfig.channel = atoi(pPara);

  pPara++;
  pPara = strchr(pPara, ',');
  pPara++;
  apConfig.authmode = atoi(pPara);

  if (at_setupCmdCwsapEsp(&apConfig,passLen)==0){
    at_backOk;
    return;
  }
  at_backError;

}

uint8_t ICACHE_FLASH_ATTR
at_setupCmdCwsapEsp(struct softap_config *apConfig,uint8_t passwdlen)
{
    bool ret;

    if(at_wifiMode == STATION_MODE)
    {
        #ifdef DEBUG
            uart0_sendStr("at_setupCmdCwsapEsp wifmode in STATION_MODE\n");
        #endif // DEBUG
        return 1;
    }

    if(apConfig->ssid_len < 1 || passwdlen==-1)
    {
        #ifdef DEBUG
            uart0_sendStr("at_setupCmdCwsapEsp invalid psswd len\n");
        #endif // DEBUG
        return 1;
    }
    if(apConfig->channel<1 || apConfig->channel>13)
    {
        #ifdef DEBUG
            uart0_sendStr("at_setupCmdCwsapEsp invalid channel\n");
        #endif // DEBUG
        return 1;
    }
    if(apConfig->authmode >= 5)
    {
        #ifdef DEBUG
            uart0_sendStr("at_setupCmdCwsapEsp authmode\n");
        #endif // DEBUG
        return 1;
    }
    if((apConfig->authmode != 0)&&(passwdlen < 5))
    {
        #ifdef DEBUG
            uart0_sendStr("at_setupCmdCwsapEsp password len should be bigger than 5\n");
        #endif // DEBUG
        return 1;
    }
    ETS_UART_INTR_DISABLE();
    ret=wifi_softap_set_config(apConfig);
    ETS_UART_INTR_ENABLE();
    return ((ret==true)?0:1);

}
//get the ip of stations connected to the AP
void ICACHE_FLASH_ATTR
at_exeCmdCwlif(uint8_t id)
{
  struct station_info *station;
  struct station_info *next_station;
  char temp[128];

  if(at_wifiMode == STATION_MODE)
  {
    at_backError;
    return;
  }
  station = wifi_softap_get_station_info();
  while(station)
  {
    #ifdef VERBOSE
        os_sprintf(temp, "%d.%d.%d.%d,"MACSTR"\n",
               IP2STR(&station->ip), MAC2STR(station->bssid));
    #else
        os_sprintf(temp, "%c%c%d,%d%c",CANWII_SOH,CMD_CWLIF,
               IP2STR(&station->ip), MAC2STR(station->bssid),CANWII_EOH);
    #endif // VERBOSE


    uart0_sendStr(temp);
    next_station = STAILQ_NEXT(station, next);
    os_free(station);
    station = next_station;
  }
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_queryCmdCwdhcp(uint8_t id)
{
	//char temp[32];
    at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCwdhcp(uint8_t id, char *pPara)
{
	uint8_t mode,opt;
	pPara++;
	mode = 0;
	mode = atoi(pPara);
	pPara ++;
	pPara = strchr(pPara, ',');
	pPara++;
	opt = atoi(pPara);
    if (at_setupCmdCwdhcpEsp(mode,opt)!=0){
        at_backError;
    }
	at_backOk;
	return;
}

uint8_t ICACHE_FLASH_ATTR
at_setupCmdCwdhcpEsp(uint8_t mode, uint8_t opt)
{
    int8_t ret = 0;
    switch (mode)
	{
	case 0:
	  if(opt==0)
	  {
	  	ret = wifi_softap_dhcps_start();
	  }
	  else
	  {
	  	ret = wifi_softap_dhcps_stop();
	  }
		break;

	case 1:
		if(opt==0)
	  {
	  	ret = wifi_station_dhcpc_start();
	  }
	  else
	  {
	  	ret = wifi_station_dhcpc_stop();
	  }
		break;

	case 2:
		if(opt==0)
	  {
	  	ret = wifi_softap_dhcps_start();
	  	ret |= wifi_station_dhcpc_start();
	  }
	  else
	  {
	  	ret = wifi_softap_dhcps_stop();
	  	ret |= wifi_station_dhcpc_stop();
	  }
		break;

	default:

		break;
	}
	if(ret)
	{
	  return 0;
	}

	return 1;
}

void ICACHE_FLASH_ATTR
at_queryCmdCipstamac(uint8_t id)
{
	char temp[64];
  uint8 bssid[6];

  //os_sprintf(temp, "%s:", at_fun[id].at_cmdName);
  //uart0_sendStr(temp);

  wifi_get_macaddr(STATION_IF, bssid);
  #ifdef VERBOSE
    os_sprintf(temp,"%s:\""MACSTR"\" %s\n", at_fun[id].at_cmdName, MAC2STR(bssid));
  #else
    os_sprintf(temp, "%c%c%d%c",CANWII_SOH,at_fun[id].at_cmdCode, MAC2STR(bssid),CANWII_EOH);
  #endif // VERBOSE

  uart0_sendStr(temp);
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCipstamac(uint8_t id, char *pPara)
{
	int8_t len,i;
  uint8 bssid[6];
  char temp[64];

	pPara++;

  len = at_dataStrCpy(temp, pPara, 32);
  if(len != 17)
  {
    at_backError;
    return;
  }

  pPara++;

  for(i=0;i<6;i++)
  {
    bssid[i] = strtol(pPara,&pPara,16);
    pPara += 1;
  }

  //os_printf(MACSTR"\n", MAC2STR(bssid));
  wifi_set_macaddr(STATION_IF, bssid);
	at_backOk;
}


void ICACHE_FLASH_ATTR
at_queryCmdCipapmac(uint8_t id)
{
	char temp[64];
  uint8 bssid[6];

  //os_sprintf(temp, "%s:", at_fun[id].at_cmdName);
  //uart0_sendStr(temp);

  wifi_get_macaddr(SOFTAP_IF, bssid);

  #ifdef VERBOSE
    os_sprintf(temp,"%s:\""MACSTR"\" %s\n", at_fun[id].at_cmdName, MAC2STR(bssid));
  #else
    os_sprintf(temp, "%c%c%d%c",CANWII_SOH,at_fun[id].at_cmdCode, MAC2STR(bssid),CANWII_EOH);
  #endif // VERBOSE

  uart0_sendStr(temp);
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCipapmac(uint8_t id, char *pPara)
{
  int8_t len,i;
  uint8 bssid[6];
  char temp[64];

	pPara++;

  len = at_dataStrCpy(temp, pPara, 32);
  if(len != 17)
  {
    at_backError;
    return;
  }

  pPara++;

  for(i=0;i<6;i++)
  {
    bssid[i] = strtol(pPara,&pPara,16);
    pPara += 1;
  }

  //os_printf(MACSTR"\n", MAC2STR(bssid));
  wifi_set_macaddr(SOFTAP_IF, bssid);
	at_backOk;
}

void ICACHE_FLASH_ATTR
at_queryCmdCipsta(uint8_t id)
{
	struct ip_info pTempIp;
  char temp[64];

  wifi_get_ip_info(0x00, &pTempIp);
  //os_sprintf(temp, "%s:", at_fun[id].at_cmdName);
  //uart0_sendStr(temp);
  #ifdef VERBOSE
    os_sprintf(temp, "%s:\"%d.%d.%d.%d\"\n",at_fun[id].at_cmdName, IP2STR(&pTempIp.ip));
  #else
    os_sprintf(temp, "%c%c%c.%c.%c.%c%c",CANWII_SOH,at_fun[id].at_cmdCode, IP2STR(&pTempIp.ip),CANWII_EOH);
  #endif // VERBOSE

  uart0_sendStr(temp);
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCipsta(uint8_t id, char *pPara)
{
	struct ip_info pTempIp;
  int8_t len;
  char temp[64];

  wifi_station_dhcpc_stop();

  pPara++;

  len = at_dataStrCpy(temp, pPara, 32);
  if(len == -1)
  {
    at_backError;
    return;
  }
  pPara++;
  wifi_get_ip_info(0x00, &pTempIp);
  pTempIp.ip.addr = ipaddr_addr(temp);

  /*os_printf("%c.%c.%d.%d\n",
                 IP2STR(&pTempIp.ip));
  */
  if(!wifi_set_ip_info(0x00, &pTempIp))
  {
    at_backError;
    wifi_station_dhcpc_start();
    return;
  }
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_queryCmdCipap(uint8_t id)
{
	struct ip_info pTempIp;
  char temp[64];

  wifi_get_ip_info(0x01, &pTempIp);

  #ifdef VERBOSE
    os_sprintf(temp, "%s:\"%d.%d.%d.%d\"\n",at_fun[id].at_cmdName, IP2STR(&pTempIp.ip));
  #else
    os_sprintf(temp, "%c%c%c.%c.%c.%c%c",CANWII_SOH,at_fun[id].at_cmdCode, IP2STR(&pTempIp.ip),CANWII_EOH);
  #endif // VERBOSE

  uart0_sendStr(temp);
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCipap(uint8_t id, char *pPara)
{
	struct ip_info pTempIp;
  int8_t len;
  char temp[64];

  wifi_softap_dhcps_stop();

  pPara++;

  len = at_dataStrCpy(temp, pPara, 32);
  if(len == -1)
  {
    at_backError;
    return;
  }
  pPara++;
  wifi_get_ip_info(0x01, &pTempIp);
  pTempIp.ip.addr = ipaddr_addr(temp);

  /*os_printf("%c.%c.%c.%c\n",
                 IP2STR(&pTempIp.ip));
  */
  if(!wifi_set_ip_info(0x01, &pTempIp))
  {
    at_backError;
    wifi_softap_dhcps_start();
    return;
  }
  wifi_softap_dhcps_start();
  at_backOk;
}



/**
  * @}
  */
