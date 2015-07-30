#include "at_merg.h"

/**
  * @brief  Execution commad of get link status.
  * @param  id: commad id number
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_setupMerg(uint8_t id,char *pPara )
{

  esp_StoreType esp;

  #ifdef DEBUG
        uart0_sendStr("executing merg setip\n");
  #endif // DEBUG

  pPara++;
  if (setParamToEsp(pPara,id,&esp)==false){
  #ifdef DEBUG
            uart0_sendStr("failed to parse the command\n");
  #endif // DEBUG
    at_backError;
    return;
  }
    #ifdef DEBUG
    char temp[255];
    os_sprintf(temp, "merg command: ssid-\"%s\" passwd-\"%s\" cmdid-%d cmdsubid-%d ssidlen-%d passwdlen-%d cwmode-%d cwmux-%d port-%d wpa-%d channel-%d dhcpmode-%d dhcpen-%d servermode-%d timeout-%d state-%d\n",
        esp.ssid,
        esp.passwd,
        esp.cmdid,
        esp.cmdsubid,
        esp.ssidlen,
        esp.passwdlen,
        esp.cwmode,
        esp.cwmux,
        esp.port,
        esp.wpa,
        esp.channel,
        esp.dhcp_mode,
        esp.dhcp_enable,
        esp.server_mode,
        esp.timeout,
        esp.state);
    uart0_sendStr(temp);
    #endif // DEBUG

    setupAp(&esp);
    //os_delay_us(10000);
    setupServer(&esp);
    //os_delay_us(10000);
    at_backOk;
    //system_restart();
}

void ICACHE_FLASH_ATTR
setupAp(esp_StoreType *espdata ){

    #ifdef DEBUG
            uart0_sendStr("setting mode\n");
    #endif // DEBUG
    //set mode
    if (at_setupCmdCwmodeEsp(espdata->cwmode)!=0){
        #ifdef DEBUG
                uart0_sendStr("failed to set mode\n");
        #endif // DEBUG
        at_backError;
        return;
    }

    #ifdef DEBUG
            uart0_sendStr("setting dhcp\n");
    #endif // DEBUG
    //set dhcp
    if (at_setupCmdCwdhcpEsp(espdata->dhcp_mode,espdata->dhcp_enable)!=0){
        #ifdef DEBUG
            uart0_sendStr("failed to set dhcp\n");
        #endif // DEBUG
        at_backError;
        return;
    }

    //set ssid,passwd
    #ifdef DEBUG
            uart0_sendStr("setting ssi\n");
    #endif // DEBUG
    struct softap_config apConfig;
    os_bzero(&apConfig, sizeof(struct softap_config));
    wifi_softap_get_config(&apConfig);

    if (espdata->ssidlen>sizeof(espdata->ssid)){
        espdata->ssidlen=sizeof(espdata->ssid);
    }
    if (espdata->passwdlen>sizeof(espdata->passwd)){
        espdata->passwdlen=sizeof(espdata->passwd);
    }
    if (espdata->channel>11){
        espdata->channel=1;
    }
    if (espdata->wpa>3){
        espdata->wpa=0;
    }

    if (espdata->ssidlen>0){
        os_memcpy(apConfig.ssid,&espdata->ssid,espdata->ssidlen);
        #ifdef DEBUG
            uart0_sendStr("SSID:");
            uart0_sendStr(apConfig.ssid);
            uart0_sendStr("\n");
        #endif // DEBUG
    }else{
        apConfig.ssid[0]='\0';
    }
    if (espdata->passwdlen>0){
        os_memcpy(apConfig.password,&espdata->passwd,espdata->passwdlen);
        #ifdef DEBUG
            uart0_sendStr("PASSWORD:");
            uart0_sendStr(apConfig.password);
            uart0_sendStr("\n");
        #endif // DEBUG
    }else{
        apConfig.password[0]='\0';
    }
    apConfig.channel=espdata->channel;
    apConfig.authmode=espdata->wpa;


    if (at_setupCmdCwsapEsp(&apConfig,espdata->passwdlen)!=0){
        #ifdef DEBUG
            uart0_sendStr("failed to set ssi\n");
        #endif // DEBUG
        at_backError;
        return;
    }
    #ifdef DEBUG
            uart0_sendStr("ssi set\n");
    #endif // DEBUG


    #ifdef DEBUG
            char temp[50];
            os_sprintf(temp,"Saving parameters to memory  size:%d\n",sizeof(esp_StoreType));
            uart0_sendStr(temp);
    #endif // DEBUG

    espdata->state=1;
    espdata->saved=1;
    saveMergParams(espdata);

    //system_restart();


}

void ICACHE_FLASH_ATTR
setupServer(esp_StoreType *espdata ){
    //set server mode
        #ifdef DEBUG
            uart0_sendStr("setting server mode\n");
        #endif // DEBUG
    if (at_setupCmdCipmuxEsp(espdata->cwmux)!=0)
    {
        #ifdef DEBUG
            uart0_sendStr("failed to set server mode\n");
        #endif // DEBUG
        at_backError;
        return;
    }
    #ifdef DEBUG
            uart0_sendStr("setting port\n");
        #endif // DEBUG
    if (at_setupCmdCipserverEsp(espdata->server_mode,espdata->port,espdata->timeout)!=0){
        #ifdef DEBUG
            uart0_sendStr("failed to set port\n");
        #endif // DEBUG
        at_backError;
        return;
    }
    #ifdef DEBUG
        uart0_sendStr("server mode OK\n");
    #endif // DEBUG
    //print the ip
    #ifdef DEBUG
            uart0_sendStr("printing ip and status\n");
    #endif // DEBUG
    at_exeCmdCifsr(CMD_CIFSR);
    //print status
    at_exeCmdCipstatus(CMD_CIPSTATUS);
}

void saveMergParams(esp_StoreType *espdata){

    esp_StoreType temp;
    /*
    temp.baud=0;
    temp.channel=0;
    temp.cmdid=0;
    temp.cmdsubid=0;
    temp.cwmode=0;
    temp.cwmux=0;
    temp.dhcp_enable=0;
    temp.dhcp_mode=0;
    os_memset(temp.passwd,'\0',sizeof(temp.passwd));
    os_memset(temp.ssid,'\0',sizeof(temp.ssid));
    temp.passwdlen=16;
    temp.port=0;
    temp.saved=0;
    temp.server_mode=0;
    temp.ssidlen=16;
    temp.state=0;
    temp.tcp_udp_mode=0;
    temp.timeout=0;
    temp.wpa=0;
    user_esp_platform_save_param(&temp, sizeof(esp_StoreType));
    */
    #ifdef DEBUG
    char tempesp[255];
    os_sprintf(tempesp, "merg command: ssid-\"%s\" passwd-\"%s\" cmdid-%d cmdsubid-%d ssidlen-%d passwdlen-%d cwmode-%d cwmux-%d port-%d wpa-%d channel-%d dhcpmode-%d dhcpen-%d servermode-%d timeout-%d state-%d\n",
        espdata->ssid,
        espdata->passwd,
        espdata->cmdid,
        espdata->cmdsubid,
        espdata->ssidlen,
        espdata->passwdlen,
        espdata->cwmode,
        espdata->cwmux,
        espdata->port,
        espdata->wpa,
        espdata->channel,
        espdata->dhcp_mode,
        espdata->dhcp_enable,
        espdata->server_mode,
        espdata->timeout,
        espdata->state);
    uart0_sendStr(tempesp);
    #endif // DEBUG

    user_esp_platform_save_param(espdata, sizeof(esp_StoreType));

    //check if the data was really written
    #ifdef DEBUG
        user_esp_platform_load_param(&temp, sizeof(esp_StoreType));
        if (espdata->baud!=temp.baud || espdata->channel!=temp.channel || espdata->cmdid!=temp.cmdid ||
            espdata->cmdsubid!=temp.cmdsubid || espdata->cwmode!=temp.cwmode || espdata->cwmux!=temp.cwmux ||
            espdata->dhcp_enable!=temp.dhcp_enable||espdata->dhcp_mode!=temp.dhcp_mode||
            espdata->passwdlen!=temp.passwdlen||espdata->port!=temp.port||espdata->saved!=temp.saved||
            espdata->server_mode!=temp.server_mode||espdata->ssidlen!=temp.ssidlen||espdata->state!=temp.state||
            espdata->tcp_udp_mode!=temp.tcp_udp_mode||espdata->timeout!=temp.timeout||espdata->wpa!=temp.wpa){

            uart0_sendStr("data incorrect written\n");
        }
    #endif // DEBUG
}

