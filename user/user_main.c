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

void user_init(void)
{

  uart_init(BIT_RATE_115200, BIT_RATE_115200);
  #ifdef DEBUG
    uart0_sendStr("INIT\n");
  #endif // DEBUG
  user_esp_platform_load_param((uint32 *)&espParam, sizeof(esp_StoreType));
  at_wifiMode = wifi_get_opmode();

  //create the server
  if (espParam.state==1){
    #ifdef DEBUG
        char temp[255];
        os_sprintf(temp, "merg command: state-%d saved-%d ssid-%s passwd-%s cmdid-%d cmdsubid-%d ssidlen-%d passwdlen-%d cwmode-%d cwmux-%d port-%d wpa-%d channel-%d dhcpmode-%d dhcpen-%d servermode-%d timeout-%d\n",
        espParam.state,
        espParam.saved,
        espParam.ssid,
        espParam.passwd,
        espParam.cmdid,
        espParam.cmdsubid,
        espParam.ssidlen,
        espParam.passwdlen,
        espParam.cwmode,
        espParam.cwmux,
        espParam.port,
        espParam.wpa,
        espParam.channel,
        espParam.dhcp_mode,
        espParam.dhcp_enable,
        espParam.server_mode,
        espParam.timeout);
        uart0_sendStr(temp);
        uart0_sendStr("STARTING SAVED STATE\n");
    #endif //DEBUG

    setupServer(&espParam);
    espParam.state==1;
    espParam.saved==0;
    //user_esp_platform_save_param((uint32 *)&espParam, sizeof(esp_StoreType));
  }
  at_backOk;

  //TODO Change message
    #ifdef DEBUG
        os_printf("ready!!!\n");
        uart0_sendStr("ready\n");
    #endif //DEBUG

  at_init();
}
