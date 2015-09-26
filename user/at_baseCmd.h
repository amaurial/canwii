/*
 * File	: at_baseCmd.h
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
#ifndef __AT_BASECMD_H
#define __AT_BASECMD_H
#include <stdlib.h>
#include "osapi.h"
#include "c_types.h"
#include "at.h"
#include "at_utils.h"
#include "user_interface.h"
#include "at_version.h"
#include "driver/uart_register.h"
#include "spi_flash.h"

void at_exeCmdNull(uint8_t id);
void at_setupCmdE(uint8_t id, char *pPara);
void at_exeCmdRst(uint8_t id);
void at_exeCmdGmr(uint8_t id);
//void at_setupCmdGslp(uint8_t id, char *pPara);

#endif
