/********************************************************************* 
 FocalTech HID Forcehpad Upgrade Tool 
 Copyright (c) 2022, FocalTech Systems Co.,Ltd.
 Author: Wayne  <shihwei.huang@focaltech-electronics.com>
*********************************************************************/ 



/**********************************************************************
 
ftpprogram.h
head file, include get vrsion, upgrade function

**********************************************************************/

#ifndef _FTPPROGRAM_H_
#define _FTPPROGRAM_H_

#include "global.h"

/***************************************************
 * Definitions
**************************************************/

/*******************************************
 * Global Variables Declaration
 ******************************************/

// Debug
extern bool g_debug;

#ifndef DEBUG_PRINTF
#define DEBUG_PRINTF(fmt, argv...) if(g_debug) printf(fmt, ##argv)
#endif //DEBUG_PRINTF

#ifndef ERROR_PRINTF
#define ERROR_PRINTF(fmt, argv...) fprintf(stderr, fmt, ##argv)
#endif //ERROR_PRINTF


/*******************************************
 * Function Prototype
 ******************************************/
#ifdef __cplusplus
	  extern "C" {
#endif
u8 get_fw_version_data(u16 *p_fw_version);
u8 HID_Program_Upgrade();
void Run_Test(void);
#ifdef __cplusplus
 }
#endif

#endif //_FTPPROGRAM_H_
