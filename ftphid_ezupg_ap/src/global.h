/********************************************************************* 
 FocalTech HID Forcehpad Upgrade Tool 
 Copyright (c) 2022, FocalTech Systems Co.,Ltd.
 Author: Wayne  <shihwei.huang@focaltech-electronics.com>
*********************************************************************/ 



/********************************************************************* 

global.cpp 
global variant

***********************************************************************/

//////////////////////////////////////////////////////////////////////
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

 
#include <stdio.h>
#include <stdlib.h>
#include <time.h> //<ctime>
#include <sys/time.h> // struct timeval & gettimeofday()
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <semaphore.h>	/* semaphore */
#include <getopt.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>     /* syslog */
#include <sys/select.h>         /* select */
#include <fcntl.h>      /* open */
#include <unistd.h>     /* close */
#include <sys/ioctl.h>  /* ioctl */
#include <dirent.h>         // opendir, readdir, closedir
#include <linux/hidraw.h>	// hidraw
#include <linux/input.h>	// BUS_TYPE




//////////////////////////////////////////////////////////////////////
// Definitions
//////////////////////////////////////////////////////////////////////
#define SleepMS(ms)	usleep((ms)*1000)
// Max Length of Full Path Name
#ifndef PATH_LEN_MAX
#define PATH_LEN_MAX        260
#endif //PATH_LEN_MAX

// Typedef
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;


#ifndef DEFAULT_LOG_FILENAME
#define DEFAULT_LOG_FILENAME	"/tmp/tp_log.txt"
#endif //DEFAULT_LOG_FILENAME


/* FOCALTECH IICHID Buffer Size */
const int FTP_IICHID_OUTPUT_BUFFER_SIZE = 64; 
const int FTP_IICHID_INPUT_BUFFER_SIZE  = 64;

#ifndef FOCAL_USB_VID
#define FOCAL_USB_VID					0x2808
#endif 

#ifndef _SAVE_LOG_
#define _SAVE_LOG_ 						1
#endif


#ifndef HIDIOCSOUTPUT
//#warning Please have your distro update the userspace kernel headers
#define HIDIOCSOUTPUT(len)   _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x0B, len)
#define HIDIOCGINPUT(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x0A, len)
#endif


/*********************************************************
Define Program Code
**********************************************************/
#define PROGRAM_CODE_OK							0x00
#define	PROGRAM_CODE_INVALID_PARAM				0x01
#define	PROGRAM_CODE_ALLOCATE_BUFFER_ERROR		0x02
#define	PROGRAM_CODE_ERASE_FLASH_ERROR			0x03
#define	PROGRAM_CODE_WRITE_FLASH_ERROR			0x04
#define	PROGRAM_CODE_READ_FLASH_ERROR			0x05
#define	PROGRAM_CODE_CHECK_DATA_ERROR			0x06//Download
#define	PROGRAM_CODE_CHECKSUM_ERROR				0x07//Upgrade
#define	PROGRAM_CODE_CHIP_ID_ERROR				0x08
#define	PROGRAM_CODE_ENTER_DEBUG_MODE_ERROR		0x09
#define	PROGRAM_CODE_WRITE_ENABLE_ERROR			0x0a
#define	PROGRAM_CODE_RESET_SYSTEM_ERROR			0x0b
#define	PROGRAM_CODE_IIC_BYTE_DELAY_ERROR		0x0c
#define	PROGRAM_CODE_CHECK_BLANK_ERROR			0x0d
#define	PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR	0x0e
#define	PROGRAM_CODE_COMM_ERROR					0x0f


/*********************************************************
Define COMM_HID
**********************************************************/
#define COMM_HID_OK							    0x00
#define COMM_HID_INVLID_PARAM			        0x01
#define COMM_HID_INVLID_DEVICE                  0x02
#define COMM_HID_INVALID_BUFFER_SIZE            0x03
#define COMM_HID_WRITE_USB_ERROR                0x04
#define COMM_HID_READ_USB_ERROR                 0x05
#define COMM_HID_FIND_NO_DEVICE                 0x06    
#define COMM_HID_PACKET_COMMAND_ERROR           0x07
#define COMM_HID_PACKET_DATA_ERROR              0x08
#define COMM_HID_PACKET_CHECKSUM_ERROR          0x09

/*********************************************************
Define process assignement 
**********************************************************/
#define PROCESS_ERR_OK                          0x0000 
#define PROCESS_ERR_MEM_FAIL                    0x0001 
#define PROCESS_ERR_INVLID_PARAM                0x0002
#define PROCESS_ERR_INVLID_FILE                 0x0003
#define PROCESS_ERR_IO_ERROR                    0x0004
#define PROCESS_ERR_GET_DATA_FAIL               0x0005            


/*********************************************************
Define HID CMD
**********************************************************/
#define		CMD_ENTER_UPGRADE_MODE			0x40
#define		CMD_CHECK_CURRENT_STATE			0x41
#define		CMD_READY_FOR_UPGRADE			0x42
#define		CMD_SEND_DATA					0x43
#define		CMD_UPGRADE_CHECKSUM			0x44
#define		CMD_EXIT_UPRADE_MODE			0x45
#define		CMD_USB_READ_UPGRADE_ID			0x46
#define		CMD_USB_ERASE_FLASH 			0x47
#define		CMD_USB_BOOT_READ 		    	0x48
#define		CMD_USB_BOOT_BOOTLOADERVERSION  0x49

#define		CMD_READ_REGISTER			0x50
#define		CMD_WRITE_REGISTER			0x51
#define		CMD_ACK						0xf0
#define		CMD_NACK					0xff
#define		REPORT_SIZE					64
#define 	MAX_USB_PACKET_SIZE			56
#define		FIRST_PACKET				0x00
#define		SERIAL_PACKET				0x01
#define		MID_PACKET					0x01
#define		END_PACKET					0x02

#define     FTS_READ_PACKET_OFFSET 		6
#define     FTS_HID_PACKET_MAX	        56


enum USB_UPGRADE_STEP{
    USB_UPGRADE_ENTRY_BOOTLOADER=0,
    USB_UPGRADE_ENTRY_UPGRADE,
    USB_UPGRADE_ERASE_FLASH,
	USB_UPGRADE_CHECK_ERASE_READY,
	USB_UPGRADE_SEND_DATA,
	USB_UPGRADE_CHECK_SUM,
	USB_UPGRADE_EXIT,	
	USB_UPGRADE_END
};

#ifndef LOG_BUF_SIZE
#define LOG_BUF_SIZE 4096
#endif //LOG_BUF_SIZE

//////////////////////////////////////////////////////////////////////
// Variable
//////////////////////////////////////////////////////////////////////
extern bool g_bEnableDebug;
extern bool g_bEnableOutputBufferDebug;
extern bool g_bEnableErrorMsg;
extern int g_firmware_fd;
extern int g_firmware_size;
extern int m_nHidrawFd;
extern int m_nHidrawFd;  
// Assign initial values to chip data
extern u16 m_usVID;
extern u16 m_usPID;
extern u16 m_usVersion;
// Assign Initial values to buffers
extern u8* m_inBuf;
extern u8* m_outBuf;
extern u8 m_inBufSize;
extern u8 m_outBufSize;
extern sem_t m_ioMutex;
extern u8 m_szOutputBuf[64];// Command Raw Buffer
extern u8 m_szInputBuf[64]; // Data Raw Buffer

#ifdef __cplusplus
  extern "C" {
#endif
void ErrorLogFormat(const char *pszFormat, ...);
void global_init();
u8 GetDeviceHandle(int nVID, int nPID);    
u8 SendData(unsigned char* sBuf, int sLen);
u8 ReadData(unsigned char* rBuf, int rLen);
u8 GetDevVidPid(unsigned int* p_nVid, unsigned int* p_nPid);
void CloseDevice(void);
void global_free(void);
void Run_Test(void);
void WriteLog(const char *pszFormat, ...);
#ifdef __cplusplus
}  
#endif

#endif //ndef __GLOBAL_H__
