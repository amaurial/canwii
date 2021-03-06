/*
 * File	: user_main.c
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
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "at.h"
#include "at_merg.h"

extern uint8_t at_wifiMode;
extern void user_esp_platform_load_param(void *param, uint16 len);
void user_rf_pre_init(){};



void printEspParam(esp_StoreType *espP){

#ifdef DEBUG
        char temp[255];
        os_sprintf(temp, "esp config: state-%d saved-%d ssid-%s passwd-%s cmdid-%d cmdsubid-%d ssidlen-%d passwdlen-%d cwmode-%d cwmux-%d port-%d wpa-%d channel-%d dhcpmode-%d dhcpen-%d servermode-%d timeout-%d\n",
        espP->state,
        espP->saved,
        espP->ssid,
        espP->passwd,
        espP->cmdid,
        espP->cmdsubid,
        espP->ssidlen,
        espP->passwdlen,
        espP->cwmode,
        espP->cwmux,
        espP->port,
        espP->wpa,
        espP->channel,
        espP->dhcp_mode,
        espP->dhcp_enable,
        espP->server_mode,
        espP->timeout);
        uart0_sendStr(temp);

    #endif //DEBUG
}

void user_init(void)
{
  //global parameters
  //uart_div_modify(0,UART_CLK_FREQ / 115200);
  esp_StoreType espParam;

  uart_init(BIT_RATE_115200, BIT_RATE_115200);
  #ifdef DEBUG
    uart0_sendStr("INIT\n");
  #endif // DEBUG
  user_esp_platform_load_param(&espParam, sizeof(esp_StoreType));
  at_wifiMode = wifi_get_opmode();

  #ifdef DEBUG
    char temp[30];
    os_sprintf(temp, "wifi_mode: %d\n",at_wifiMode);
    uart0_sendStr(temp);
  #endif // DEBUG

    //TODO memory leak here
  //printEspParam(&espParam);

  //create the server
  if (espParam.state == 1){
    #ifdef DEBUG
        uart0_sendStr("STARTING SAVED STATE\n");
    #endif // DEBUG
    setupAp(&espParam,false);
    os_delay_us(10000);
    setupServer(&espParam);
    os_delay_us(10000);
    //print the ip
//    #ifdef DEBUG
//            uart0_sendStr("printing ip and status\n");
//    #endif // DEBUG
//    at_exeCmdCifsr(CMD_CIFSR);
    //print status
//    at_exeCmdCipstatus(CMD_CIPSTATUS);


  }
  at_backOk;

  //TODO Change message
    #ifdef DEBUG
        uart0_sendStr("ready\n");
    #endif //DEBUG

  at_init();
}
