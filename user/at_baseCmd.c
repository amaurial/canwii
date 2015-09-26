/* 
 * File	: at_baseCmd.c
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
#include "at_baseCmd.h"

/** @defgroup AT_BASECMD_Functions
  * @{
  */

extern BOOL echoFlag;

typedef struct
{
    char flag;
    char reserve[3];
}updateFlagType;

/**
  * @brief  Execution commad of AT.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdNull(uint8_t id)
{
  at_backOk;
}

/**
  * @brief  Enable or disable Echo.
  * @param  id: command id number
  * @param  pPara:
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupCmdE(uint8_t id, char *pPara)
{
  if(*pPara == '0')
  {
    echoFlag = FALSE;
  }
  else if(*pPara == '1')
  {
    echoFlag = TRUE;
  }
  else
  {
    at_backError;
    return;
  }
  at_backOk;
}

/**
  * @brief  Execution commad of restart.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdRst(uint8_t id)
{
  at_backOk;
  system_restart();
}

/**
  * @brief  Execution commad of version.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_exeCmdGmr(uint8_t id)
{
  char temp[64];

  os_sprintf(temp, AT_VERSION);
  uart0_sendStr(temp);
  //TODO: change the message
  os_sprintf(temp,"%s\n", system_get_sdk_version());
  uart0_sendStr(temp);
  at_backOk;
}

//#define ESP_PARAM_START_SEC   0x3C
#define ESP_PARAM_START_SEC   0x3D
#define ESP_MEM_POS1 0x3D000//0xff00
#define ESP_MEM_POS2 0x3D000//0xff00
#define ESP_PARAM_SAVE_0    1
#define ESP_PARAM_SAVE_1    2
#define ESP_PARAM_FLAG      3
struct esp_platform_sec_flag_param {
    uint8 flag;
    uint8 pad[3];
};

/******************************************************************************
 * FunctionName : user_esp_platform_load_param
 * Description  : load parameter from flash, toggle use two sector by flag value.
 * Parameters   : param--the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_load_param(void *param, uint16 len)
{

        #ifdef DEBUG
            uart0_sendStr("reading on sector 0\n");
        #endif // DEBUG
        SpiFlashOpResult ret;
        ret=spi_flash_read(ESP_PARAM_START_SEC + ESP_MEM_POS1,(uint32 *)param, len);
        if (ret!=SPI_FLASH_RESULT_OK){
            #ifdef DEBUG
                uart0_sendStr("ERROR READING config.\n");
                if (ret==SPI_FLASH_RESULT_TIMEOUT){
                    uart0_sendStr("ERROR READING TIMEOUT.\n");
                }
                else{
                    uart0_sendStr("ERROR READING FAILED.\n");
                }
            #endif // DEBUG

            generalMSG.msgid=MSG_FAIL_READ_FLASH;
            generalMSG.param0=0;
            sendGeneralMsg(generalMSG);
        }
}
/*
void ICACHE_FLASH_ATTR
user_esp_platform_load_param(void *param, uint16 len)
{
    struct esp_platform_sec_flag_param flag;
    SpiFlashOpResult ret;

    //ret=spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
    //               (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));
    ret=spi_flash_read(ESP_MEM_POS1,(uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));

    if (ret!=SPI_FLASH_RESULT_OK){
        #ifdef DEBUG
            uart0_sendStr("ERROR READING param.\n");
        #endif // DEBUG
        }
    if (flag.flag == 0) {
        #ifdef DEBUG
            uart0_sendStr("reading on sector 0\n");
        #endif // DEBUG
        //ret=spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,(uint32 *)param, len);
        ret=spi_flash_read(ESP_MEM_POS1+sizeof(struct esp_platform_sec_flag_param),(uint32 *)param, len);
        if (ret!=SPI_FLASH_RESULT_OK){
            #ifdef DEBUG
            uart0_sendStr("ERROR READING config.\n");
        #endif // DEBUG
        }
    } else {
        #ifdef DEBUG
            uart0_sendStr("reading on sector 1\n");
        #endif // DEBUG
        //ret=spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,(uint32 *)param, len);
        ret=spi_flash_read(ESP_MEM_POS2+sizeof(struct esp_platform_sec_flag_param),(uint32 *)param, len);
        if (ret!=SPI_FLASH_RESULT_OK){
            #ifdef DEBUG
            uart0_sendStr("ERROR READING config.\n");
        #endif // DEBUG
        }
    }
}
*/
/******************************************************************************
 * FunctionName : user_esp_platform_save_param
 * Description  : toggle save param to two sector by flag value,
 *              : protect write and erase data while power off.
 * Parameters   : param -- the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_save_param(void *param, uint16 len)
{
        #ifdef DEBUG
            uart0_sendStr("saving on sector 0\n");
        #endif // DEBUG
        SpiFlashOpResult ret;
        spi_flash_erase_sector(ESP_PARAM_START_SEC);
        ret=spi_flash_write(ESP_PARAM_START_SEC + ESP_MEM_POS1,(uint32 *)param, len);
        if (ret!=SPI_FLASH_RESULT_OK){
            #ifdef DEBUG
                uart0_sendStr("ERROR WRITING config.\n");

                if (ret==SPI_FLASH_RESULT_TIMEOUT){
                    uart0_sendStr("ERROR WRITTING TIMEOUT.\n");
                }
                else{
                    uart0_sendStr("ERROR WRITTING FAILED.\n");
                }


                char temp[255];
                esp_StoreType *esptemp=(esp_StoreType *)param;
                os_sprintf(temp, "merg command: state-%d saved-%d ssid-%s passwd-%s cmdid-%d cmdsubid-%d ssidlen-%d passwdlen-%d cwmode-%d cwmux-%d port-%d wpa-%d channel-%d dhcpmode-%d dhcpen-%d servermode-%d timeout-%d\n",
                esptemp->state,
                esptemp->saved,
                esptemp->ssid,
                esptemp->passwd,
                esptemp->cmdid,
                esptemp->cmdsubid,
                esptemp->ssidlen,
                esptemp->passwdlen,
                esptemp->cwmode,
                esptemp->cwmux,
                esptemp->port,
                esptemp->wpa,
                esptemp->channel,
                esptemp->dhcp_mode,
                esptemp->dhcp_enable,
                esptemp->server_mode,
                esptemp->timeout);
                uart0_sendStr(temp);
            #endif // DEBUG
            generalMSG.msgid=MSG_FAIL_WRITE_FLASH;
            generalMSG.param0=0;
            sendGeneralMsg(generalMSG);
        }
}

/**
  * @}
  */

