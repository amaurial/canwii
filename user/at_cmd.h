/*
 * File	: at_cmd.h
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
#ifndef __AT_CMD_H
#define __AT_CMD_H

#include "at.h"
#include "at_wifiCmd.h"
#include "at_ipCmd.h"
#include "at_baseCmd.h"
#include "at_merg.h"
#include "user_interface.h"
#include "osapi.h"

#define at_cmdNum   24

at_funcationType at_fun[at_cmdNum]={
  {NULL,    0,      0x00,               NULL,               NULL, NULL,           at_exeCmdNull},
  {"+RST",  4,      CMD_RST,            at_testCmdGeneric,  NULL, NULL,           at_exeCmdRst},
  {"+GMR",  4,      CMD_GMR,            at_testCmdGeneric,  NULL, NULL,           at_exeCmdGmr},
  {"+CWMODE",   7,  CMD_CWMODE,         at_testCmdGeneric,  at_queryCmdCwmode,  at_setupCmdCwmode,      NULL},
  {"+CWSAP",    6,  CMD_CWSAP,          at_testCmdGeneric,  at_queryCmdCwsap,   at_setupCmdCwsap,       NULL},
  {"+CWLIF",    6,  CMD_CWLIF,          at_testCmdGeneric,  NULL,               NULL,                   at_exeCmdCwlif},
  {"+CWDHCP",   7,  CMD_CWDHCP,         at_testCmdGeneric,  at_queryCmdCwdhcp,  at_setupCmdCwdhcp,      NULL},
  {"+CIFSR",    6,  CMD_CIFSR,          at_testCmdGeneric,  NULL,               at_setupCmdCifsr,       at_exeCmdCifsr},
  {"+CIPSTAMAC",10, CMD_CIPSTAMAC,      at_testCmdGeneric,  at_queryCmdCipstamac,at_setupCmdCipstamac,  NULL},
  {"+CIPAPMAC", 9,  CMD_CIPAPMAC,       at_testCmdGeneric,  at_queryCmdCipapmac, at_setupCmdCipapmac,   NULL},
  {"+CIPSTA",   7,  CMD_CIPSTA,         at_testCmdGeneric,  at_queryCmdCipsta,  at_setupCmdCipsta,      NULL},
  {"+CIPAP",    6,  CMD_CIPAP,          at_testCmdGeneric,  at_queryCmdCipap,   at_setupCmdCipap,       NULL},
  {"+CIPSTATUS",10, CMD_CIPSTATUS,      at_testCmdGeneric,  NULL,               NULL,                   at_exeCmdCipstatus},
  {"+CIPCLOSE", 9,  CMD_CIPCLOSE,       at_testCmdGeneric,  NULL,               at_setupCmdCipclose,    at_exeCmdCipclose},
  {"+CIPSEND",  8,  CMD_CIPSEND,        at_testCmdGeneric,  NULL,               at_setupCmdCipsend,     at_exeCmdCipsend},
  {"+CIPMUX",   7,  CMD_CIPMUX,         at_testCmdGeneric,  at_queryCmdCipmux,  at_setupCmdCipmux,      NULL},
  {"+CIPSERVER",10, CMD_CIPSERVER,      at_testCmdGeneric,  NULL,               at_setupCmdCipserver,   NULL},
  {"+CIPMODE",  8,  CMD_CIPMODE,        at_testCmdGeneric,  at_queryCmdCipmode, at_setupCmdCipmode,     NULL},
  {"+CIPSTO",   7,  CMD_CIPSTO,         at_testCmdGeneric,  at_queryCmdCipsto,  at_setupCmdCipsto,      NULL},
  {"+MERG"    , 5,  CMD_MERG_CONFIG_AP_EXT,at_testCmdGeneric,  at_merg_query_setup,               at_setupMerg,           NULL},
  {"+MERGAP"  , 7,  CMD_MERG_CONFIG_AP, at_testCmdGeneric,  NULL,               at_setupMerg,           NULL},
  {"+AT",       3,  CMD_AT,             at_testCmdGeneric,  at_exeCmdNull,      NULL,                   at_exeCmdNull},
  {"+VERSION",  8,  CMD_MERG_VERSION,   at_testCmdGeneric,  NULL,       NULL,           merg_version},
  {"+STATUS",  7,   CMD_MERG_STATUS,    at_testCmdGeneric,  NULL,       NULL,           at_merg_status}
  };

#endif
