/*
 * File	: at_ipCmd.c
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
#include "at_ipCmd.h"

extern at_mdStateType mdState;
extern BOOL specialAtState;
extern at_stateType at_state;
extern at_funcationType at_fun[];
extern uint8_t *pDataLine;
extern uint8_t at_dataLine[];
extern uint8_t at_wifiMode;
extern int8_t at_dataStrCpy(void *pDest, const void *pSrc, int8_t maxLen);

uint16_t at_sendLen; //now is 256
uint16_t at_tranLen; //now is 256
os_timer_t at_delayCheck;
BOOL IPMODE;
uint8_t ipDataSendFlag = 0;
//struct_MSGType generalMSG;


static BOOL at_ipMux = FALSE;
static BOOL disAllFlag = FALSE;

static at_linkConType pLink[at_linkMax];
static uint8_t sendingID;
static BOOL serverEn = FALSE;



static uint16_t server_timeover = TCP_SERVER_TIMEOUT;
static struct espconn *pTcpServer;
static struct espconn *pUdpServer;

/** @defgroup AT_IPCMD_Functions
  * @{
  */

static void at_tcpclient_discon_cb(void *arg);

/**
  * @brief  Test commad of at_testQueryCmdCwsap.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_testCmdGeneric(uint8_t id)
{
    char temp[32];
    #ifdef VERBOSE
        os_sprintf(temp, "%s:(1-3)\n", at_fun[id].at_cmdName);
    #else
        os_sprintf(temp, "%c%c%c",CANWII_SOH, at_fun[id].at_cmdCode,CANWII_EOH);
    #endif // VERBOSE
    uart0_sendStr(temp);
    at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCifsr(uint8_t id, char *pPara)
{
  struct ip_info pTempIp;
  int8_t len;
  char temp[64];

  if(at_wifiMode == STATION_MODE)
  {
    at_backError;
    return;
  }
  pPara = strchr(pPara, '\"');
  len = at_dataStrCpy(temp, pPara, 32);
  if(len == -1)
  {

    #ifdef VERBOSE
        uart0_sendStr("IP ERROR\n");
    #else
        os_sprintf(temp, "%c%d%c",CANWII_SOH, RSP_IP_ERROR,CANWII_EOH);
    #endif // VERBOSE
    uart0_sendStr(temp);

    return;
  }

  wifi_get_ip_info(0x01, &pTempIp);
  pTempIp.ip.addr = ipaddr_addr(temp);

  //os_printf("%d.%d.%d.%d\n",
  //               IP2STR(&pTempIp.ip));

  if(!wifi_set_ip_info(0x01, &pTempIp))
  {
    at_backError;
    return;
  }
  at_backOk;
  return;
}

/**
  * @brief  Execution commad of get module ip.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdCifsr(uint8_t id)//add get station ip and ap ip
{
    #ifdef DEBUG
       uart0_sendStr("showing IP\n");
    #endif // DEBUG
  if((at_wifiMode == SOFTAP_MODE)||(at_wifiMode == STATIONAP_MODE))
  {
        getShowIP_MAC(0x01,SOFTAP_IF,false,id);

  }
  if((at_wifiMode == STATION_MODE)||(at_wifiMode == STATIONAP_MODE))
  {
    getShowIP_MAC(0x00,STATION_IF,true,id);
  }

  mdState = m_gotip;
  at_backOk;
}

void ICACHE_FLASH_ATTR
 getShowIP_MAC(uint8_t p_ip_param,uint8_t p_mac_param,bool msg,uint8_t id){

    struct ip_info pTempIp;
    char temp[64];
    char ip[15];
    char mac[24];
    uint8 bssid[6];

    #ifdef DEBUG
        char log[50];
       uart0_sendStr("\ngetting the ip and mac\n");
    #endif // DEBUG
    wifi_get_ip_info(p_ip_param, &pTempIp);
    wifi_get_macaddr(p_mac_param, bssid);
    #ifdef VERBOSE
        if(msg)
        {
            os_sprintf(temp, "%s:STAIP,", at_fun[id].at_cmdName);
            uart0_sendStr(temp);

            os_sprintf(temp, "\"%d.%d.%d.%d\"\n",
                       IP2STR(&pTempIp.ip));
            uart0_sendStr(temp);

            os_sprintf(temp, "%s:STAMAC,", at_fun[id].at_cmdName);
            uart0_sendStr(temp);


            os_sprintf(temp, "\""MACSTR"\"\n",
                       MAC2STR(bssid));
            uart0_sendStr(temp);
        }
        else{
            os_sprintf(temp, "%s:APIP,", at_fun[id].at_cmdName);
            uart0_sendStr(temp);

            os_sprintf(temp, "\"%d.%d.%d.%d\"\n",
                       IP2STR(&pTempIp.ip));
            uart0_sendStr(temp);

            os_sprintf(temp, "%s:APMAC,", at_fun[id].at_cmdName);
            uart0_sendStr(temp);

            os_sprintf(temp, "\""MACSTR"\"\n%s\n",MAC2STR(bssid));
            //os_sprintf(temp, "\""MACSTR"\"\n%s\n",bssid);
            uart0_sendStr(temp);

        }

    #else
        #ifdef DEBUG

            if (msg){
                os_sprintf(log, "IP ST:\"%d.%d.%d.%d\"\n",IP2STR(&pTempIp.ip));
                uart0_sendStr(log);
                os_sprintf(log, "MAC:\"%d:%d:%d:%d:%d:%d\"\n",bssid[0],bssid[1],bssid[2],
                           bssid[3],bssid[4],bssid[5]);
                uart0_sendStr(log);
            }
            else{
                os_sprintf(log, "IP-S:\"%d.%d.%d.%d\"\n",IP2STR(&pTempIp.ip));
                uart0_sendStr(log);
                os_sprintf(log, "MAC:\"%d:%d:%d:%d:%d:%d\"\n",bssid[0],bssid[1],bssid[2],
                           bssid[3],bssid[4],bssid[5]);
                uart0_sendStr(log);
            }

        #endif // DEBUG
        os_sprintf(mac, "%d:%d:%d:%d:%d:%d",bssid[0],bssid[1],bssid[2],
                       bssid[3],bssid[4],bssid[5]);

        os_sprintf(ip, "%d.%d.%d.%d",IP2STR(&pTempIp.ip));
        os_sprintf(temp, "%c%c%s,%s%c",CANWII_SOH, at_fun[id].at_cmdCode,ip,mac,CANWII_EOH);
        uart0_sendStr(temp);
    #endif // VERBOSE


}

/**
  * @brief  Execution commad of get link status.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdCipstatus(uint8_t id)
{
  char temp[64];
  uint8_t i;


    #ifdef VERBOSE
        os_sprintf(temp, "STATUS:%d\n",
         mdState);
    #else
        //os_sprintf(temp, "%c%c\n",CANWII_SOH, CMD_CIPSTATUS);
    #endif // VERBOSE

    #ifdef DEBUG
       uart0_sendStr("getting status\n");
    #endif // DEBUG
//TODO
  if(serverEn)
  {

  }

  for(i=0; i<at_linkMax; i++)
  {
    if(pLink[i].linkEn)
    {
      if(pLink[i].pCon->type == ESPCONN_TCP)
      {

        #ifdef VERBOSE
        os_sprintf(temp, "%s:%d,\"TCP\",\"%d.%d.%d.%d\",%d,%d\n",
                   at_fun[id].at_cmdName,
                   pLink[i].linkId,
                   IP2STR(pLink[i].pCon->proto.tcp->remote_ip),
                   pLink[i].pCon->proto.tcp->remote_port,
                   pLink[i].teType);
        #else
            os_sprintf(temp, "%c%c%d,%c,%d.%d.%d.%d,%d,%d%c",
                   CANWII_SOH, CMD_CIPSTATUS,
                   pLink[i].linkId,
                   CANWII_TCP,
                   IP2STR(pLink[i].pCon->proto.tcp->remote_ip),
                   pLink[i].pCon->proto.tcp->remote_port,
                   pLink[i].teType,CANWII_EOH);
            uart0_sendStr(temp);
        #endif // VERBOSE
      }
      else
      {

        #ifdef VERBOSE
        os_sprintf(temp, "%s:%d,\"UDP\",\"%d.%d.%d.%d\",%d,%d,%d\n",
                   at_fun[id].at_cmdName,
                   pLink[i].linkId,
                   IP2STR(pLink[i].pCon->proto.udp->remote_ip),
                   pLink[i].pCon->proto.udp->remote_port,
                   pLink[i].pCon->proto.udp->local_port,
                   pLink[i].teType);
        #else
            os_sprintf(temp, "%c%c%d,%d,%d.%d.%d.%d,%d,%d%c",
                   CANWII_SOH, CMD_CIPSTATUS,
                   pLink[i].linkId,
                   CANWII_UDP,
                   IP2STR(pLink[i].pCon->proto.tcp->remote_ip),
                   pLink[i].pCon->proto.tcp->remote_port,
                   pLink[i].teType,CANWII_EOH);
            uart0_sendStr(temp);

            #ifdef DEBUG
                os_sprintf(temp,"link %d set\n",i);
                uart0_sendStr(temp);
            #endif // DEBUG

        #endif // VERBOSE

      }

    }
    else {
        #ifdef VERBOSE
        //os_sprintf(temp, "STATUS:%d\n",
         //mdState);
        #else
            os_sprintf(temp, "%c%c%c",CANWII_SOH, CMD_CIPSTATUS,CANWII_EOH);
            uart0_sendStr(temp);
        #endif // VERBOSE
        #ifdef DEBUG
            os_sprintf(temp,"link %d empty\n",i);
            uart0_sendStr(temp);
        #endif // DEBUG
    }
  }
  //uart_tx_one_char(CANWII_EOH);
  at_backOk;
}

/**
  * @brief  Test commad of start client.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_testCmdCipstart(uint8_t id)
{
  char temp[64];

  if(at_ipMux)
  {
    os_sprintf(temp, "%s:(\"type\"),(\"ip address\"),(port)\n",
               at_fun[id].at_cmdName);
    uart0_sendStr(temp);
    os_sprintf(temp, "%s:(\"type\"),(\"domain name\"),(port)\n",
               at_fun[id].at_cmdName);
    uart0_sendStr(temp);
  }
  else
  {
    os_sprintf(temp, "%s:(id)(\"type\"),(\"ip address\"),(port)\n",
               at_fun[id].at_cmdName);
    uart0_sendStr(temp);
    os_sprintf(temp, "%s:((id)\"type\"),(\"domain name\"),(port)\n",
               at_fun[id].at_cmdName);
    uart0_sendStr(temp);
  }
  at_backOk;
}

/**
  * @brief  Client received callback function.
  * @param  arg: contain the ip link information
  * @param  pdata: received data
  * @param  len: the lenght of received data
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_tcpclient_recv(void *arg, char *pdata, unsigned short len)
{
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;
  at_sendData(pdata,len,linkTemp->linkId);
}

/**
  * @brief  Client received callback function.
  * @param  arg: contain the ip link information
  * @param  pdata: received data
  * @param  len: the lenght of received data
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_udpclient_recv(void *arg, char *pdata, unsigned short len)
{
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;

  //os_printf("recv\n");
  if(linkTemp->changType == 0) //if when sending, receive data???
  {
    os_memcpy(pespconn->proto.udp->remote_ip, linkTemp->remoteIp, 4);
    pespconn->proto.udp->remote_port = linkTemp->remotePort;
  }
  else if(linkTemp->changType == 1)
  {
    os_memcpy(linkTemp->remoteIp, pespconn->proto.udp->remote_ip, 4);
    linkTemp->remotePort = pespconn->proto.udp->remote_port;
    linkTemp->changType = 0;
  }

  at_sendData(pdata,len,linkTemp->linkId);
}

/**
  * @brief  Send data back to client connected to esp
  * @param  pdata: received data
  * @param  len: the lenght of received data
  * @param  linkid: id of the link where the message was received
  * @retval None
*/
void ICACHE_FLASH_ATTR
at_sendData(char *pdata, unsigned short len,uint8_t linkId)
{

#ifdef VERBOSE
      char temp[32];
      //os_printf("recv\n");
      if(at_ipMux)
      {
        os_sprintf(temp, "\n+IPD,%d,%d:",linkId, len);
        uart0_sendStr(temp);
        uart0_tx_buffer(pdata, len);
      }
      else if(IPMODE == FALSE)
      {
        os_sprintf(temp, "\n+IPD,%d:", len);
        uart0_sendStr(temp);
        uart0_tx_buffer(pdata, len);
      }
      else
      {
        uart0_tx_buffer(pdata, len);
        return;
      }
      at_backOk;
    #else
      //os_printf("recv\n");
      if(at_ipMux)
      {
            sendData(pdata,len,linkId);
      }
      else if(IPMODE == FALSE)
      {
            sendData(pdata,len,255);
      }
      else
      {
            sendData(pdata,len,255);
      }
      //at_backOk;
    #endif // VERBOSE
}

void ICACHE_FLASH_ATTR
sendData(char *pdata, unsigned short len,uint8_t linkId){
    if (len<=255){
        char temp[4];
        os_sprintf(temp, "%c%c%d%c",CANWII_SOH,CMD_IPD,linkId, len);
        uart0_tx_buffer(temp, 4);
        uart0_tx_buffer(pdata, len);
        uart_tx_one_char(CANWII_EOH);
    }
    else{
        //calculate how many messages
        int n=len/255;
        int m=len%255;
        int l=len;
        int i;
        n=m>0?n++:n;
        for (i=0;i<n;i++){
            l=(i==n-1)?m:255;
            sendData(pdata,l,linkId);
        }
    }
}

/**
  * @brief  Client send over callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_tcpclient_sent_cb(void *arg)
{

    struct espconn *pespconn = (struct espconn *)arg;
    at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;

  if(IPMODE == TRUE)
  {
    ipDataSendFlag = 0;
  	os_timer_disarm(&at_delayCheck);
  	os_timer_arm(&at_delayCheck, 20, 0);
  	system_os_post(at_recvTaskPrio, 0, 0);
    ETS_UART_INTR_ENABLE();
    return;
  }
  specialAtState = TRUE;
  at_state = at_statIdle;

  generalMSG.msgid=MSG_SEND;
  generalMSG.param0=linkTemp->linkId;
  sendGeneralMsg(generalMSG);

  at_backOk;

}

/**
  * @brief  Tcp client connect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_tcpclient_connect_cb(void *arg)
{
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;
  char temp[16];

  //os_printf("tcp client connect\n");
  //os_printf("pespconn %p\n", pespconn);

  linkTemp->linkEn = TRUE;
  linkTemp->teType = teClient;
  linkTemp->repeaTime = 0;
  espconn_regist_disconcb(pespconn, at_tcpclient_discon_cb);
  espconn_regist_recvcb(pespconn, at_tcpclient_recv);
  espconn_regist_sentcb(pespconn, at_tcpclient_sent_cb);

  mdState = m_linked;
  if(at_state == at_statIpTraning)
 	{
 		return;
  }
  if(at_ipMux)
  {

    generalMSG.msgid=MSG_CONNECT;
    generalMSG.param0=linkTemp->linkId;
    sendGeneralMsg(generalMSG);

  }
  else
  {

    generalMSG.msgid=MSG_CONNECT;
    generalMSG.param0=-1;
    sendGeneralMsg(generalMSG);

  }
  at_backOk;

  specialAtState = TRUE;
  at_state = at_statIdle;
}

/**
  * @brief  Tcp client connect repeat callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_tcpclient_recon_cb(void *arg, sint8 errType)
{
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;
  struct ip_info ipconfig;
  os_timer_t sta_timer;
  char temp[16];


  if(at_state == at_statIpTraning)
  {
  	linkTemp->repeaTime++;
    ETS_UART_INTR_ENABLE();
    //os_printf("Traning recon\n");
    if(linkTemp->repeaTime > 10)
    {
    	linkTemp->repeaTime = 10;
    }
    os_delay_us(linkTemp->repeaTime * 10000);
    pespconn->proto.tcp->local_port = espconn_port();
    espconn_connect(pespconn);
    return;
  }

    generalMSG.msgid=MSG_CLOSED;
    generalMSG.param0=linkTemp->linkId;
    sendGeneralMsg(generalMSG);

  if(linkTemp->teToff == TRUE)
  {
    linkTemp->teToff = FALSE;
    linkTemp->repeaTime = 0;
    if(pespconn->proto.tcp != NULL)
    {
      os_free(pespconn->proto.tcp);
    }
    os_free(pespconn);
    linkTemp->linkEn = false;
    at_linkNum--;
    if(at_linkNum == 0)
    {
      at_backOk;
      mdState = m_unlink;
      disAllFlag = false;
      specialAtState = TRUE;
      at_state = at_statIdle;
    }
  }
  else
  {
    linkTemp->repeaTime++;
    if(linkTemp->repeaTime >= 1)
    {
      //os_printf("repeat over %d\n", linkTemp->repeaTime);
      linkTemp->repeaTime = 0;
      if(errType == ESPCONN_CLSD)
      {
        at_backOk;
      }
      else
      {
        at_backError;
      }
      if(pespconn->proto.tcp != NULL)
      {
        os_free(pespconn->proto.tcp);
      }
      os_free(pespconn);
      linkTemp->linkEn = false;
      //os_printf("disconnect\n");
      at_linkNum--;
      if (at_linkNum == 0)
      {
        mdState = m_unlink;
        disAllFlag = false;
      }
      ETS_UART_INTR_ENABLE();
      specialAtState = true;
      at_state = at_statIdle;
      return;
    }

    specialAtState = true;
    at_state = at_statIdle;
    //os_printf("link repeat %d\n", linkTemp->repeaTime);
    pespconn->proto.tcp->local_port = espconn_port();
    espconn_connect(pespconn);
  }
}


static ip_addr_t host_ip;
/******************************************************************************
 * FunctionName : user_esp_platform_dns_found
 * Description  : dns found callback
 * Parameters   : name -- pointer to the name that was looked up.
 *                ipaddr -- pointer to an ip_addr_t containing the IP address of
 *                the hostname, or NULL if the name could not be found (or on any
 *                other error).
 *                callback_arg -- a user-specified callback argument passed to
 *                dns_gethostbyname
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
at_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
  struct espconn *pespconn = (struct espconn *) arg;
  at_linkConType *linkTemp = (at_linkConType *) pespconn->reverse;
  char temp[16];

  if(ipaddr == NULL)
  {
    linkTemp->linkEn = FALSE;

    generalMSG.msgid=MSG_DNS_FAIL;
    generalMSG.param0=NULLPARAM;
    sendGeneralMsg(generalMSG);

    specialAtState = TRUE;
    at_state = at_statIdle;
    return;
  }

  /*os_printf("DNS found: %d.%d.%d.%d\n",
           *((uint8 *) &ipaddr->addr),
            *((uint8 *) &ipaddr->addr + 1),
            *((uint8 *) &ipaddr->addr + 2),
            *((uint8 *) &ipaddr->addr + 3));
  */

  if(host_ip.addr == 0 && ipaddr->addr != 0)
  {
    if(pespconn->type == ESPCONN_TCP)
    {
      os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);
      espconn_connect(pespconn);
      at_linkNum++;
    }
    else
    {
      os_memcpy(pespconn->proto.udp->remote_ip, &ipaddr->addr, 4);
      os_memcpy(linkTemp->remoteIp, &ipaddr->addr, 4);
      espconn_connect(pespconn);
      specialAtState = TRUE;
      at_state = at_statIdle;
      at_linkNum++;

      generalMSG.msgid=MSG_CONNECT;
      generalMSG.param0=linkTemp->linkId;
      sendGeneralMsg(generalMSG);

      at_backOk;
    }
  }
}



/**
  * @brief  Tcp client disconnect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_tcpclient_discon_cb(void *arg)
{
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;
  uint8_t idTemp;
  char temp[16];

  if(pespconn == NULL)
  {
    return;
  }
  if(at_state == at_statIpTraning)
  {
    ETS_UART_INTR_ENABLE();
    //os_printf("Traning nodiscon\n");
    pespconn->proto.tcp->local_port = espconn_port();
    espconn_connect(pespconn);
    return;
  }
  if(pespconn->proto.tcp != NULL)
  {
    os_free(pespconn->proto.tcp);
  }
  os_free(pespconn);

  linkTemp->linkEn = FALSE;
  if(at_ipMux)
  {

    generalMSG.msgid=MSG_CLOSED;
    generalMSG.param0=linkTemp->linkId;
    sendGeneralMsg(generalMSG);

  }
  else
  {

    generalMSG.msgid=MSG_CLOSED;
    generalMSG.param0 = NULLPARAM;
    sendGeneralMsg(generalMSG);

  }


  at_linkNum--;

  if(disAllFlag == FALSE)
  {
    at_backOk;
  }
  if(at_linkNum == 0)
  {
    mdState = m_unlink;
    if(disAllFlag)
    {
      at_backOk;
    }
    disAllFlag = FALSE;
  }

  if(disAllFlag)
  {
    idTemp = linkTemp->linkId + 1;
    for(; idTemp<at_linkMax; idTemp++)
    {
      if(pLink[idTemp].linkEn)
      {
        if(pLink[idTemp].teType == teServer)
        {
          continue;
        }
        if(pLink[idTemp].pCon->type == ESPCONN_TCP)
        {
        	specialAtState = FALSE;
          espconn_disconnect(pLink[idTemp].pCon);
        	break;
        }
        else
        {

          generalMSG.msgid=MSG_CLOSED;
          generalMSG.param0=pLink[idTemp].linkId;
          sendGeneralMsg(generalMSG);

          pLink[idTemp].linkEn = FALSE;
          espconn_delete(pLink[idTemp].pCon);
          os_free(pLink[idTemp].pCon->proto.udp);
          os_free(pLink[idTemp].pCon);
          at_linkNum--;
          if(at_linkNum == 0)
          {
            mdState = m_unlink;
            at_backOk;
            disAllFlag = FALSE;
          }
        }
      }
    }
  }
  ETS_UART_INTR_ENABLE();
  specialAtState = TRUE;
  at_state = at_statIdle;
}


/**
  * @brief  Setup commad of close ip link.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdCipclose(uint8_t id, char *pPara)
{
  char temp[64];
  uint8_t linkID;
  uint8_t i;

  pPara++;
  if(at_ipMux == 0)
  {

    generalMSG.msgid=MSG_MUX;
    generalMSG.param0=0;
    sendGeneralMsg(generalMSG);

    return;
  }
  linkID = atoi(pPara);
  if(linkID > at_linkMax)
  {
    at_backError;
    return;
  }
  if(linkID == at_linkMax)
  {
    if(serverEn)
    {
      /* restart */
      //TODO: change the message
      //generalMSG.msgid=MSG_RESTART;
      //generalMSG.param0=NULLPARAM;
      //sendGeneralMsg(generalMSG);
      //uart0_sendStr("we must restart\n");
      return;
    }
    for(linkID=0; linkID<at_linkMax; linkID++)
    {
      if(pLink[linkID].linkEn)
      {
        if(pLink[linkID].pCon->type == ESPCONN_TCP)
        {
          pLink[linkID].teToff = TRUE;
          specialAtState = FALSE;
          espconn_disconnect(pLink[linkID].pCon);
          disAllFlag = TRUE;

          break;
        }
        else
        {
          pLink[linkID].linkEn = FALSE;

          generalMSG.msgid=MSG_CLOSED;
          generalMSG.param0=linkID;
          sendGeneralMsg(generalMSG);

          uart0_sendStr(temp);
          espconn_delete(pLink[linkID].pCon);
          os_free(pLink[linkID].pCon->proto.udp);
          os_free(pLink[linkID].pCon);
          at_linkNum--;
          if(at_linkNum == 0)
          {
            mdState = m_unlink;
            at_backOk;
          }
        }
      }
    }
  }
  else
  {
    if(pLink[linkID].linkEn == FALSE)
    {

      generalMSG.msgid=MSG_LINK_SET_FAIL;
      generalMSG.param0=pLink[linkID].linkId;
      sendGeneralMsg(generalMSG);

      return;
    }
    if(pLink[linkID].teType == teServer)
    {
      if(pLink[linkID].pCon->type == ESPCONN_TCP)
      {
        pLink[linkID].teToff = TRUE;
        specialAtState = FALSE;
        espconn_disconnect(pLink[linkID].pCon);
      }
      else
      {
        pLink[linkID].linkEn = FALSE;

        generalMSG.msgid=MSG_CLOSED;
        generalMSG.param0=linkID;
        sendGeneralMsg(generalMSG);

        espconn_delete(pLink[linkID].pCon);
        at_linkNum--;
        at_backOk;
        if(at_linkNum == 0)
        {
          mdState = m_unlink;

        }
      }
    }
    else
    {
      if(pLink[linkID].pCon->type == ESPCONN_TCP)
      {
        pLink[linkID].teToff = TRUE;
        specialAtState = FALSE;
        espconn_disconnect(pLink[linkID].pCon);
      }
      else
      {
        pLink[linkID].linkEn = FALSE;

        generalMSG.msgid=MSG_CLOSED;
        generalMSG.param0=linkID;
        sendGeneralMsg(generalMSG);

        espconn_delete(pLink[linkID].pCon);
        os_free(pLink[linkID].pCon->proto.udp);
        os_free(pLink[linkID].pCon);
        at_linkNum--;
        at_backOk;
        if(at_linkNum == 0)
        {
          mdState = m_unlink;
        }
      }
    }
  }
}

/**
  * @brief  Execution commad of close ip link.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdCipclose(uint8_t id)
{
  char temp[64];

  if(at_ipMux)
  {

    generalMSG.msgid=MSG_MUX;
    generalMSG.param0=1;
    sendGeneralMsg(generalMSG);

    return;
  }
  if(pLink[0].linkEn)
  {
    if(serverEn)
    {
      /* restart */
      //TODO: change the message
      //generalMSG.msgid=MSG_RESTART;
      //generalMSG.param0=NULLPARAM;
      //sendGeneralMsg(generalMSG);
      //uart0_sendStr("we must restart\n");
      return;
    }
    else
    {
      if(pLink[0].pCon->type == ESPCONN_TCP)
      {
        specialAtState = FALSE;
        pLink[0].teToff = TRUE;
        espconn_disconnect(pLink[0].pCon);
      }
      else
      {
        pLink[0].linkEn = FALSE;

        generalMSG.msgid=MSG_CLOSED;
        generalMSG.param0=NULLPARAM;
        sendGeneralMsg(generalMSG);

        espconn_delete(pLink[0].pCon);
        os_free(pLink[0].pCon->proto.udp);
        os_free(pLink[0].pCon);
        at_linkNum--;
        if(at_linkNum == 0)
        {
          mdState = m_unlink;
          at_backOk;
        }
      }
    }
  }
  else
  {
    at_backError;
  }
}

char * ICACHE_FLASH_ATTR
at_checkLastNum(char *pPara, uint8_t maxLen)
{
  int8_t ret = -1;
  char *pTemp;
  uint8_t i;

  pTemp = pPara;
  for(i=0;i<maxLen;i++)
  {
    if((*pTemp > '9')||(*pTemp < '0'))
    {
      break;
    }
    pTemp++;
  }
  if(i == maxLen)
  {
    return NULL;
  }
  else
  {
    return pTemp;
  }
}
/**
  * @brief  Setup commad of send ip data.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdCipsend(uint8_t id, char *pPara)
{

  if(IPMODE == TRUE)
  {

    generalMSG.msgid=MSG_IP_MODE;
    generalMSG.param0=1;
    sendGeneralMsg(generalMSG);

    at_backError;
    return;
  }
  pPara++;
  if(at_ipMux)
  {
    sendingID = atoi(pPara);
    if(sendingID >= at_linkMax)
    {
      generalMSG.msgid=MSG_LINK_SET_FAIL;
      generalMSG.param0=pLink[sendingID].linkId;
      sendGeneralMsg(generalMSG);
      at_backError;
      return;
    }
    pPara++;
  }
  else
  {
    sendingID = 0;
  }
  if(pLink[sendingID].linkEn == FALSE)
  {
    generalMSG.msgid=MSG_LINK_SET_FAIL;
    generalMSG.param0=pLink[sendingID].linkId;
    sendGeneralMsg(generalMSG);
    at_backError;
    return;
  }

  //get the message until eoh
  char temp[MSG_MAX_BUFFER_SIZE];
  int i=0;
  for (i=0;i<(MSG_MAX_BUFFER_SIZE+1);i++){
    if (*pPara == CANWII_EOH ){
        break;
    }
    temp[i] = *pPara;
    pPara++;
  }
  at_sendLen=i;

  if(at_sendLen >= MSG_MAX_BUFFER_SIZE)
  {
    generalMSG.msgid=MSG_TOO_LONG;
    generalMSG.param0=at_sendLen;
    sendGeneralMsg(generalMSG);
    at_backError;
    return;
  }
    //send data
    //at_backOk;
    ipSendData(temp,sendingID,at_sendLen);
    at_state = at_statIdle;
}

/**
  * @brief  Send data through ip.
  * @param  pAtRcvData: point to data
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_ipDataSending(uint8_t *pAtRcvData)
{
  espconn_sent(pLink[sendingID].pCon, pAtRcvData, at_sendLen);
  //os_printf("id:%d,Len:%d,dp:%p\n",sendingID,at_sendLen,pAtRcvData);

  //bug if udp,send is ok
//  if(pLink[sendingID].pCon->type == ESPCONN_UDP)
//  {
//    uart0_sendStr("\r\nSEND OK\r\n");
//    specialAtState = TRUE;
//    at_state = at_statIdle;
//  }
}

/**
  * @brief  General send data through ip. Used to send data to remote host.
  * @param  pAtRcvData: point to data
  * @param  linkid: link where client is connected
  * @param  length: size of the buffer limited by MSG_MAX_BUFFER_SIZE
  * @retval None
  */
void ICACHE_FLASH_ATTR
ipSendData(uint8_t *pAtRcvData,uint8_t linkid,uint16_t length)
{

  //sanity check
  if (linkid>at_linkMax){
    generalMSG.msgid=MSG_LINK_SET_FAIL;
    generalMSG.param0=pLink[sendingID].linkId;
    sendGeneralMsg(generalMSG);
    return;
  }
  if (length>MSG_MAX_BUFFER_SIZE){
    generalMSG.msgid=MSG_TOO_LONG;
    generalMSG.param0=at_sendLen;
    sendGeneralMsg(generalMSG);
    return;
  }

  espconn_sent(pLink[linkid].pCon, pAtRcvData, length);

  //os_printf("id:%d,Len:%d,dp:%p\n",linkid,length,pAtRcvData);

  //bug if udp,send is ok
//  if(pLink[sendingID].pCon->type == ESPCONN_UDP)
//  {
//    uart0_sendStr("\r\nSEND OK\r\n");
//    specialAtState = TRUE;
//    at_state = at_statIdle;
//  }
}

/**
  * @brief  Transparent data through ip.
  * @param  arg: no used
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_ipDataTransparent(void *arg)
{
	if(at_state != at_statIpTraning)
	{
		return;
	}

	os_timer_disarm(&at_delayCheck);
	if((at_tranLen == 3) && (os_memcmp(at_dataLine, "+++", 3) == 0))
	{
		specialAtState = TRUE;
        at_state = at_statIdle;
		return;
	}
	else if(at_tranLen)
	{
	  ETS_UART_INTR_DISABLE();
    espconn_sent(pLink[0].pCon, at_dataLine, at_tranLen);
    ipDataSendFlag = 1;
    pDataLine = at_dataLine;
  	at_tranLen = 0;
  	return;
  }
  os_timer_arm(&at_delayCheck, 20, 0);
}

/**
  * @brief  Send data through ip.
  * @param  pAtRcvData: point to data
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_ipDataSendNow(void)
{
  espconn_sent(pLink[0].pCon, at_dataLine, at_tranLen);
  ipDataSendFlag = 1;
  pDataLine = at_dataLine;
  at_tranLen = 0;
}

/**
  * @brief  Setup commad of send ip data.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdCipsend(uint8_t id)
{
	if((serverEn) || (IPMODE == FALSE))
	{
		at_backError;
		return;
	}
	if(pLink[0].linkEn == FALSE)
  {
	  at_backError;
	  return;
  }
	pDataLine = at_dataLine;
	at_tranLen = 0;
  specialAtState = FALSE;
  at_state = at_statIpTraning;
  os_timer_disarm(&at_delayCheck);
  os_timer_setfn(&at_delayCheck, (os_timer_func_t *)at_ipDataTransparent, NULL);
  os_timer_arm(&at_delayCheck, 20, 0);
  //TODO: change the message
  uart0_sendStr("\n>");
}

/**
  * @brief  Query commad of set multilink mode.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_queryCmdCipmux(uint8_t id)
{
  char temp[32];

  #ifdef VERBOSE
    os_sprintf(temp, "%s:%d\n",at_fun[id].at_cmdName, at_ipMux);
  #else
    os_sprintf(temp, "%c%c%d%c",CANWII_SOH,at_fun[id].at_cmdCode, at_ipMux,CANWII_EOH);
  #endif // VERBOSE

  uart0_sendStr(temp);
  at_backOk;
}

/**
  * @brief  Setup commad of set multilink mode.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdCipmux(uint8_t id, char *pPara)
{
  uint8_t muxTemp;
  pPara++;
  muxTemp = atoi(pPara);
  if (at_setupCmdCipmuxEsp(muxTemp)!=0)
  {
    at_backError;
    return;
  }
  at_backOk;
}

uint8_t ICACHE_FLASH_ATTR
at_setupCmdCipmuxEsp(uint8_t mux)
{
  if(mdState == m_linked)
  {

    generalMSG.msgid=MSG_LINK_DONE;
    generalMSG.param0=NULLPARAM;
    sendGeneralMsg(generalMSG);
    return 1;
  }
  if(mux == 1)
  {
    at_ipMux = TRUE;
  }
  else if(mux == 0)
  {
    at_ipMux = FALSE;
  }
  else
  {
    return 1;
  }
  return 0;
}

/**
  * @brief  Tcp server disconnect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_tcpserver_discon_cb(void *arg)
{
    struct espconn *pespconn = (struct espconn *) arg;
    at_linkConType *linkTemp = (at_linkConType *) pespconn->reverse;
    char temp[16];

    //os_printf("S conect C: %p\n", arg);

    linkTemp->linkEn = FALSE;
    linkTemp->pCon = NULL;

    generalMSG.msgid=MSG_CLIENT_DISCONNECTED;
    generalMSG.param0=linkTemp->linkId;
    sendGeneralMsg(generalMSG);

    if(linkTemp->teToff == TRUE)
    {
        linkTemp->teToff = FALSE;
        at_backOk;
    }

    //unset the ith bit
    server_info.which_clients &=  ~(0x01 << linkTemp->linkId);

    at_linkNum--;
    if (at_linkNum == 0)
    {
        mdState = m_unlink;
        //    uart0_sendStr("Unlink\r\n");
        disAllFlag = false;
    }
    ETS_UART_INTR_ENABLE();
    specialAtState = true;
    at_state = at_statIdle;
}

/**
  * @brief  Tcp server connect repeat callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_tcpserver_recon_cb(void *arg, sint8 errType)
{
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;
  char temp[16];

  //os_printf("S conect C: %p\n", arg);

  if(pespconn == NULL)
  {
    return;
  }

  linkTemp->linkEn = false;
  linkTemp->pCon = NULL;
  //os_printf("con EN? %d\n", linkTemp->linkId);
  at_linkNum--;
  if (at_linkNum == 0)
  {
    mdState = m_unlink;
  }


    generalMSG.msgid=MSG_CONNECT;
    generalMSG.param0=linkTemp->linkId;
    sendGeneralMsg(generalMSG);


  disAllFlag = false;
  if(linkTemp->teToff == TRUE)
  {
    linkTemp->teToff = FALSE;
    at_backOk;
  }
  ETS_UART_INTR_ENABLE();
  specialAtState = true;
  at_state = at_statIdle;
}

/**
  * @brief  Tcp server listend callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
LOCAL void ICACHE_FLASH_ATTR
at_tcpserver_listen(void *arg)
{
  struct espconn *pespconn = (struct espconn *)arg;
  uint8_t i;
  char temp[16];

  #ifdef DEBUG
        uart0_sendStr("at_tcpserver_listen got a client\n");
    #endif // DEBUG

  //os_printf("get tcpClient:\n");
  for(i=0;i<at_linkMax;i++)
  {
    //get the first linkid available
    if(pLink[i].linkEn == FALSE)
    {
      pLink[i].linkEn = TRUE;
      break;
    }
  }
  if(i>=at_linkMax)
  {
    #ifdef DEBUG
        uart0_sendStr("at_tcpserver_listen linkid > 5\n");
    #endif // DEBUG
    return;
  }

  #ifdef DEBUG
        uart0_sendStr("at_tcpserver_listen register callback functions\n");
    #endif // DEBUG

  pLink[i].teToff = FALSE;
  pLink[i].linkId = i;
  pLink[i].teType = teServer;
  pLink[i].repeaTime = 0;
  pLink[i].pCon = pespconn;
  mdState = m_linked;
  at_linkNum++;
  pespconn->reverse = &pLink[i];
  //set the ith bit
  server_info.which_clients |= (0x1 << i);

  espconn_regist_recvcb(pespconn, at_tcpclient_recv);
  espconn_regist_reconcb(pespconn, at_tcpserver_recon_cb);
  espconn_regist_disconcb(pespconn, at_tcpserver_discon_cb);
  espconn_regist_sentcb(pespconn, at_tcpclient_sent_cb);///////


    generalMSG.msgid=MSG_CLIENT_CONNECTED;
    generalMSG.param0=i;
    sendGeneralMsg(generalMSG);

}


/**
  * @brief  Setup commad of module as server.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdCipserver(uint8_t id, char *pPara)
{
  BOOL serverEnTemp;
  int32_t port;

  pPara++;
  serverEnTemp = atoi(pPara);
  pPara++;
  if(serverEnTemp == 0)
  {
    if(*pPara != CANWII_EOH)
    {
      at_backError;
      return;
    }
  }
  else if(serverEnTemp == 1)
  {
    if(*pPara == ',')
    {
      pPara++;
      port = atoi(pPara);
    }
    else
    {
      port = 0;
    }
  }
  else
  {
    at_backError;
    return;
  }

    if (at_setupCmdCipserverEsp(serverEnTemp,port,server_timeover)!=0){
        at_backError;
        return;
    }
  at_backOk;
}

uint8_t ICACHE_FLASH_ATTR
at_setupCmdCipserverEsp(uint8_t mode, int32_t port,int16_t timeout)
{
  int32_t tport;

  tport=port;
  if(at_ipMux == FALSE)
  {
    #ifdef DEBUG
        uart0_sendStr("at_setupCmdCipserverEsp wrong at_ipMux\n");
    #endif // DEBUG
    return 1;
  }

  if (mode>1 || mode <0){
    #ifdef DEBUG
        uart0_sendStr("at_setupCmdCipserverEsp wrong mode\n");
    #endif // DEBUG
    return 1;
  }

  if (port>0xff){
    #ifdef DEBUG
        uart0_sendStr("at_setupCmdCipserverEsp wrong port\n");
    #endif // DEBUG
    return 1;
  }

  if(mode == 0 && tport<=0)
  {
    #ifdef DEBUG
        uart0_sendStr("at_setupCmdCipserverEsp wrong mode/port\n");
    #endif // DEBUG
    return 1;
  }
  else if(mode == 1)
  {
    if(tport<=0){
      tport = 30;
    }
  }
  /*
  else
  {
    #ifdef DEBUG
        uart0_sendStr("at_setupCmdCipserverEsp mode wrong\n");
    #endif // DEBUG
    return 1;
  }*/
/*
  if(mode == serverEn)
  {
    #ifdef DEBUG
        uart0_sendStr("at_setupCmdCipserverEsp no change. force reset maybe\n");
    #endif // DEBUG
    espconn_disconnect();
    generalMSG.msgid=MSG_NO_CHANGE;
    generalMSG.param0=NULLPARAM;
    sendGeneralMsg(generalMSG);
    return 0;
  }
   */
   if(mode == serverEn){
        #ifdef DEBUG
        uart0_sendStr("at_setupCmdCipserverEsp delete previous tcp server\n");
        #endif // DEBUG
        int i;
        for(i=0;i<at_linkMax;i++)
          {
            espconn_delete(pLink[i].pCon);
            pLink[i].linkEn=FALSE;
            pLink[i].teToff = TRUE;
            pLink[i].pCon = NULL;
          }
        at_linkNum=0;
        mdState = m_unlink;
        disAllFlag = false;
   }

  if(mode==1)
  {
    #ifdef DEBUG
        uart0_sendStr("at_setupCmdCipserverEsp create a new tcp server\n");
    #endif // DEBUG
    pTcpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
    if (pTcpServer == NULL)
    {
      generalMSG.msgid=MSG_TCP_SERVER_FAIL;
      generalMSG.param0=NULLPARAM;
      sendGeneralMsg(generalMSG);
      #ifdef DEBUG
        uart0_sendStr("at_setupCmdCipserverEsp failed to set tcp server\n");
    #endif // DEBUG
      return 1;
    }
    //force a disconnect.

    pTcpServer->type = ESPCONN_TCP;
    pTcpServer->state = ESPCONN_LISTEN;
    pTcpServer->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    pTcpServer->proto.tcp->local_port = tport;
    //number of tcp connections
    /*
    if (espconn_tcp_set_max_con_allow(pTcpServer,at_linkMax) != 0){
        #ifdef DEBUG
            char temp[50];
            os_sprintf(temp,"Failed to set max tcp connections:%d\n",at_linkMax);
            uart0_sendStr(temp);
        #endif // DEBUG
        generalMSG.msgid=MSG_FAIL_TCP_MAX_CONN;
        generalMSG.param0=at_linkMax;
        sendGeneralMsg(generalMSG);
    }
    */
    espconn_regist_connectcb(pTcpServer, at_tcpserver_listen);
    espconn_accept(pTcpServer);
    espconn_regist_time(pTcpServer, timeout, 0);
  }
  else
  {
    /* restart */
    //TODO: change the message
    //generalMSG.msgid=MSG_RESTART;
    //generalMSG.param0=NULLPARAM;
    //sendGeneralMsg(generalMSG);
    #ifdef DEBUG
        char temp[50];
        os_sprintf(temp,"at_setupCmdCipserverEsp mode not expected:%d\n",mode);
        uart0_sendStr(temp);
    #endif // DEBUG
    //uart0_sendStr("we must restart\n");
    generalMSG.msgid=MSG_INVALID_MODE;
    generalMSG.param0=mode;
    sendGeneralMsg(generalMSG);
    return 1;
  }
  serverEn = mode;
  return 0;
}

/**
  * @brief  Query commad of set transparent mode.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_queryCmdCipmode(uint8_t id)
{
	char temp[32];
  #ifdef VERBOSE
    os_sprintf(temp, "%s:%d\n", at_fun[id].at_cmdName, IPMODE);
  #else
    os_sprintf(temp, "%c%c%d%c", CANWII_SOH,at_fun[id].at_cmdCode, IPMODE,CANWII_EOH);
  #endif

  uart0_sendStr(temp);
  at_backOk;
}

/**
  * @brief  Setup commad of transparent.
  * @param  id: commad id number
  * @param  pPara: AT input param
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdCipmode(uint8_t id, char *pPara)
{
	uint8_t mode;
    pPara++;
    mode = atoi(pPara);
    if (at_setupCmdCipmodeEsp(mode)==0){
        at_backOk;
    }else {
        at_backError;
    }

}
uint8_t ICACHE_FLASH_ATTR
at_setupCmdCipmodeEsp(uint8_t mode )
{
  if((at_ipMux) || (serverEn))
  {
  	return 1;
  }
  if(mode > 1)
  {
  	return 1;
  }
  IPMODE = mode;
  return 0;
}

void ICACHE_FLASH_ATTR
at_queryCmdCipsto(uint8_t id)
{
  char temp[32];

  #ifdef VERBOSE
    os_sprintf(temp, "%s:%d\n",
             at_fun[id].at_cmdName, server_timeover);
  #else
    os_sprintf(temp, "%c%c%c%c\n", CANWII_SOH,at_fun[id].at_cmdCode, server_timeover,CANWII_EOH);
  #endif

  uart0_sendStr(temp);
  at_backOk;
}

void ICACHE_FLASH_ATTR
at_setupCmdCipsto(uint8_t id, char *pPara)
{

  uint16_t timeOver;

  if(serverEn == FALSE)
  {
    at_backError;
    return;
  }
  pPara++;
  timeOver = atoi(pPara);
  if(timeOver>7200)
  {
    at_backError;
    return;
  }
  if(timeOver != server_timeover)
  {
    server_timeover = timeOver;
    espconn_regist_time(pTcpServer, server_timeover, 0);
  }
  at_backOk;
  return;
}
#define KEY "39cdfe29a1863489e788efc339f514d78b78f0de"

#define ESP_PARAM_SAVE_SEC_0    1
#define ESP_PARAM_SAVE_SEC_1    2
#define ESP_PARAM_SEC_FLAG      3
#define UPGRADE_FRAME  "{\"path\": \"/v1/messages/\", \"method\": \"POST\", \"meta\": {\"Authorization\": \"token %s\"},\
\"get\":{\"action\":\"%s\"},\"body\":{\"pre_rom_version\":\"%s\",\"rom_version\":\"%s\"}}\n"
#define pheadbuffer "Connection: keep-alive\n\
Cache-Control: no-cache\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \n\
Accept: */*\n\
Authorization: token %s\n\
Accept-Encoding: gzip,deflate,sdch\n\
Accept-Language: zh-CN,zh;q=0.8\n\n"
#define pheadbuffer "Connection: keep-alive\n\
Cache-Control: no-cache\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \n\
Accept: */*\n\
Authorization: token %s\n\
Accept-Encoding: gzip,deflate,sdch\n\
Accept-Language: zh-CN,zh;q=0.8\n\n"


//TODO:add other params
struct espconn *pespconn;
struct upgrade_server_info *upServer = NULL;
//struct esp_platform_saved_param {
//    uint8 devkey[40];
//    uint8 token[40];
//    uint8 activeflag;
//    uint8 pad[3];
//};
struct esp_platform_sec_flag_param {
    uint8 flag;
    uint8 pad[3];
};


/******************************************************************************
 * FunctionName : user_esp_platform_upgrade_cb
 * Description  : Processing the downloaded data from the server
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
at_upDate_rsp(void *arg)
{
  struct upgrade_server_info *server = arg;


  if(server->upgrade_flag == true)
  {
    //os_printf("device_upgrade_success\n");
    at_backOk;
    system_upgrade_reboot();
  }
  else
  {
    //os_printf("device_upgrade_failed\n");
    at_backError;
  }

  os_free(server->url);
  server->url = NULL;
  os_free(server);
  server = NULL;

  specialAtState = TRUE;
  at_state = at_statIdle;
}

/**
  * @brief  Tcp client disconnect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_upDate_discon_cb(void *arg)
{
  struct espconn *pespconn = (struct espconn *)arg;
  uint8_t idTemp;

  if(pespconn->proto.tcp != NULL)
  {
    os_free(pespconn->proto.tcp);
  }
  if(pespconn != NULL)
  {
    os_free(pespconn);
  }

  //os_printf("disconnect\n");

  if(system_upgrade_start(upServer) == false)
  {
//    uart0_sendStr("+CIPUPDATE:0/r/n");
    at_backError;
    specialAtState = TRUE;
    at_state = at_statIdle;
  }
  else
  {
    //TODO: change the message
    uart0_sendStr("+CIPUPDATE:4\n");
  }
}

/**
  * @brief  Udp server receive data callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
LOCAL void ICACHE_FLASH_ATTR
at_upDate_recv(void *arg, char *pusrdata, unsigned short len)
{
  struct espconn *pespconn = (struct espconn *)arg;
  char temp[32];
  char *pTemp;
  uint8_t user_bin[9] = {0};
//  uint8_t devkey[41] = {0};
  uint8_t i;

  os_timer_disarm(&at_delayCheck);

//TODO: change the message
  uart0_sendStr("+CIPUPDATE:3\n");

//  os_printf("%s",pusrdata);
  pTemp = (char *)os_strstr(pusrdata,"rom_version\": ");
  if(pTemp == NULL)
  {
    return;
  }
  pTemp += sizeof("rom_version\": ");

//  user_esp_platform_load_param(&esp_param);

  upServer = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));
  os_memcpy(upServer->upgrade_version, pTemp, 5);
  upServer->upgrade_version[5] = '\0';
  os_sprintf(upServer->pre_version, "v%d.%d", AT_VERSION_main, AT_VERSION_sub);

  upServer->pespconn = pespconn;

//  os_memcpy(devkey, esp_param.devkey, 40);
  os_memcpy(upServer->ip, pespconn->proto.tcp->remote_ip, 4);

  upServer->port = 80;

  upServer->check_cb = at_upDate_rsp;
  upServer->check_times = 60000;

  if(upServer->url == NULL)
  {
    upServer->url = (uint8 *) os_zalloc(512);
  }

  if(system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
  {
    os_memcpy(user_bin, "user2.bin", 10);
  }
  else if(system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
  {
    os_memcpy(user_bin, "user1.bin", 10);
  }

  os_sprintf(upServer->url,
        "GET /v1/device/rom/?action=download_rom&version=%s&filename=%s HTTP/1.1\nHost: "IPSTR":%d\n"pheadbuffer"",
        upServer->upgrade_version, user_bin, IP2STR(upServer->ip),
        upServer->port, KEY);

}

LOCAL void ICACHE_FLASH_ATTR
at_upDate_wait(void *arg)
{
  struct espconn *pespconn = arg;
  os_timer_disarm(&at_delayCheck);
  if(pespconn != NULL)
  {
    espconn_disconnect(pespconn);
  }
  else
  {
    at_backError;
    specialAtState = TRUE;
    at_state = at_statIdle;
  }
}

/******************************************************************************
 * FunctionName : user_esp_platform_sent_cb
 * Description  : Data has been sent successfully and acknowledged by the remote host.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
at_upDate_sent_cb(void *arg)
{
  struct espconn *pespconn = arg;
  os_timer_disarm(&at_delayCheck);
  os_timer_setfn(&at_delayCheck, (os_timer_func_t *)at_upDate_wait, pespconn);
  os_timer_arm(&at_delayCheck, 5000, 0);
  //os_printf("at_upDate_sent_cb\n");
}

/**
  * @brief  Tcp client connect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_upDate_connect_cb(void *arg)
{
  struct espconn *pespconn = (struct espconn *)arg;
  uint8_t user_bin[9] = {0};

  char *temp;
  //TODO: change the message
  uart0_sendStr("+CIPUPDATE:2\n");


  espconn_regist_disconcb(pespconn, at_upDate_discon_cb);
  espconn_regist_recvcb(pespconn, at_upDate_recv);
  espconn_regist_sentcb(pespconn, at_upDate_sent_cb);

  temp = (uint8 *) os_zalloc(512);

  os_sprintf(temp,"GET /v1/device/rom/?is_format_simple=true HTTP/1.0\nHost: "IPSTR":%d\n"pheadbuffer"",
             IP2STR(pespconn->proto.tcp->remote_ip),
             80, KEY);

  espconn_sent(pespconn, temp, os_strlen(temp));
  os_free(temp);
}

/**
  * @brief  Tcp client connect repeat callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_upDate_recon_cb(void *arg, sint8 errType)
{
  struct espconn *pespconn = (struct espconn *)arg;

    at_backError;
    if(pespconn->proto.tcp != NULL)
    {
      os_free(pespconn->proto.tcp);
    }
    os_free(pespconn);
    //os_printf("disconnect\n");

    if(upServer != NULL)
    {
      os_free(upServer);
      upServer = NULL;
    }
    at_backError;
    specialAtState = TRUE;
    at_state = at_statIdle;
}

/******************************************************************************
 * FunctionName : upServer_dns_found
 * Description  : dns found callback
 * Parameters   : name -- pointer to the name that was looked up.
 *                ipaddr -- pointer to an ip_addr_t containing the IP address of
 *                the hostname, or NULL if the name could not be found (or on any
 *                other error).
 *                callback_arg -- a user-specified callback argument passed to
 *                dns_gethostbyname
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
upServer_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
  struct espconn *pespconn = (struct espconn *) arg;

  if(ipaddr == NULL)
  {
    at_backError;
    specialAtState = TRUE;
    at_state = at_statIdle;
    return;
  }
  //TODO: change the message
  uart0_sendStr("+CIPUPDATE:1\n");

  if(host_ip.addr == 0 && ipaddr->addr != 0)
  {
    if(pespconn->type == ESPCONN_TCP)
    {
      os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);
      espconn_regist_connectcb(pespconn, at_upDate_connect_cb);
      espconn_regist_reconcb(pespconn, at_upDate_recon_cb);
      espconn_connect(pespconn);
    }
  }
}






/**
  * @}
  */
