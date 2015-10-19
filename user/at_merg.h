#ifndef __AT_MERG_H
#define __AT_MERG_H

#include "at.h"
#include "canwii.h"
#include "at_wifiCmd.h"
#include "at_ipCmd.h"
#include "at_baseCmd.h"
#include "user_interface.h"
#include "osapi.h"
#include <stdlib.h>

void at_setupMerg(uint8_t id,char *pPara );
void at_merg_query_setup(uint8_t id);
void at_merg_status(uint8_t id);
void setupServer(esp_StoreType *espdata );
void setupAp(esp_StoreType *espdata,bool save);
void saveMergParams(esp_StoreType *espdata);
void merg_version(uint8_t id);
#endif
