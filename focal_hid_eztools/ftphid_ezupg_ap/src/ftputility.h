/********************************************************************* 
 FocalTech HID Forcehpad Upgrade Tool 
 Copyright (c) 2022, FocalTech Systems Co.,Ltd.
 Author: Wayne  <shihwei.huang@focaltech-electronics.com>
*********************************************************************/ 


#ifndef _FTP_UTILITY_H_
#define _FTP_UTILITY_H_

/********************************************************************* 

 ftputility.h
open file, read file data  

***********************************************************************/
/***************************************************
 * Definitions
 ***************************************************/



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
 * Extern Variables Declaration
 ******************************************/


/*******************************************
 * Function Prototype
 ******************************************/
#ifdef __cplusplus
	  extern "C" {
#endif
int open_firmware_file(char *filename, size_t filename_len, int *fd);
int close_firmware_file(int fd);
int retrieve_data_from_firmware(unsigned char *data, int data_size);
int get_firmware_size(int fd, int *firmware_size);
// Remark ID
int get_remark_id_from_firmware(unsigned short *p_remark_id);
#ifdef __cplusplus
 }
#endif

#endif //_FTP_UTILITY_H_
