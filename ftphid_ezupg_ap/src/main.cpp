/********************************************************************* 
 FocalTech HID Forcehpad Upgrade Tool 
 Copyright (c) 2022, FocalTech Systems Co.,Ltd.
 Author: Wayne  <shihwei.huang@focaltech-electronics.com>
*********************************************************************/ 



/******************************************************************************

 Focaltech TouchPad HID Upgrade App
 support  get version, upgrade fw
 
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "global.h"
#include "ftputility.h"
#include "ftpprogram.h"

/***************************************************
 * Definitions
 ***************************************************/

//FTP_EZ_UPG_VERSION
#ifndef FTP_EZ_UPG_VERSION
#define	FTP_EZ_UPG_VERSION 	    "2.1"
#endif //FTP_EZ_UPG_VERSION

// SW Release Date
#ifndef FTP_EZ_UPG_RELEASE_DATE
#define FTP_EZ_UPG_RELEASE_DATE	    "2024-09-27"
#endif //FTP_EZ_UPG_RELEASE_DATE

// File Length
#ifndef FILE_NAME_LENGTH_MAX
#define FILE_NAME_LENGTH_MAX	256
#endif //FILE_NAME_LENGTH_MAX


/*******************************************
 * Global Variables Declaration
 ******************************************/
// Debug
bool g_debug = false;

#ifndef DEBUG_PRINTF
#define DEBUG_PRINTF(fmt, argv...) if(g_debug) printf(fmt, ##argv)
#endif //DEBUG_PRINTF

#ifndef ERROR_PRINTF
#define ERROR_PRINTF(fmt, argv...) fprintf(stderr, fmt, ##argv)
#endif //ERROR_PRINTF

// Firmware File Information
char g_firmware_filename[FILE_NAME_LENGTH_MAX] = {0};
// Firmware Inforamtion
bool bFW_Ver = false;
bool bSW_Ver = false;
// Flag for Firmware Update
bool bUpdate = false;
// Test
bool bTestRun = false;
// Silent Mode (Quiet)
bool g_quiet = false;

// Help Info.
bool g_help = false;

int g_pid =0;
// Parameter Option Settings
const char* const short_options = "u:fvthP:";


const struct option long_options[] =
{
	{ "Upgrade",				1, NULL, 'u'},	
	{ "get-fw-version",			0, NULL, 'f'},
	{ "get-sw-version",			0, NULL, 'v'},
	{ "test",					0, NULL, 't'},	
	{ "help",					0, NULL, 'h'},
	{ "Set-Pid",				1, NULL, 'P'},
};


// Help
void show_help_information(void);

//Device open/close
int open_device(void);
int close_device(void);

// Default Function
int process_parameter(int argc, char **argv);
int resource_init(void);
int resource_free(void);
int main(int argc, char **argv);


/*******************************************
 * Help
 ******************************************/
void show_help_information(void)
{
	printf("--------------------------------\r\n");
	printf("------------API List------------\r\n");
	printf("< Upgrade >\r\n");
	printf("   -u <file_path>\r\n");
	printf("   Ex: ftphid_ezupg_ap –u /lib/firmware/firmware.bin\r\n");
	printf("<Get Fimware Version >\r\n");
	printf("   -f \r\n");
	printf("   Ex: ftphid_ezupg_ap –f\r\n");
	printf("<Get Software Version >\r\n");
	printf("   -v \r\n");
	printf("   Ex: ftphid_ezupg_ap –v\r\n");
	printf("<Help >\r\n");
	printf("   -h \r\n");
	printf("   Ex: ftphid_ezupg_ap –h\r\n");
	printf("--------------------------------\r\n");
	return;
}

/*******************************************
 *  Open Device
 ******************************************/
int open_device(void)
{
    u8 ret = COMM_HID_OK;
    //g_pid = 0x0101;
    // open specific device on i2c bus //pseudo function

    /*** example *********************/
    // Connect to Device
    DEBUG_PRINTF("Get I2C-HID Device Handle (VID=0x%x, PID=0x%x).\r\n", FOCAL_USB_VID, g_pid);
    ret = GetDeviceHandle(FOCAL_USB_VID, g_pid);
    if (ret != COMM_HID_OK)
        ERROR_PRINTF("Device can't connected! errno=0x%x.\n", ret);
    /*********************************/
    return ret;
}


/*******************************************
 *  Close Device
 ******************************************/
int close_device(void)
{
    u8 ret = COMM_HID_OK;

    // close opened i2c device; //pseudo function

    /*** example *********************/
    // Release acquired touch device handler
	CloseDevice();
    /*********************************/

    return ret;
}

/*******************************************
 *  Initialize & Free Resource
 ******************************************/
int resource_init(void)
{
	int ret = PROCESS_ERR_OK;	

	global_init();		
	// Initialize Interface			
	if(bUpdate == true)
	{
		// Open Firmware File
		ret = open_firmware_file(g_firmware_filename, strlen(g_firmware_filename), &g_firmware_fd);		
		if(ret != PROCESS_ERR_OK)
		{
			//ERROR_PRINTF("Fail to open firmware file \"%s\"! errno=0x%x.\r\n", g_firmware_filename, errno);
			return ret;
		}

		// Get Firmware Size
		ret = get_firmware_size(g_firmware_fd, &g_firmware_size);
		if(ret != PROCESS_ERR_OK)
		{
			//ERROR_PRINTF("Fail to open firmware file \"%s\"! errno=0x%x.\r\n", g_firmware_filename, errno);			
			return ret;
		}		

		WriteLog("FW Size = %d",g_firmware_size);
		
		// Make Sure Firmware File Valid
		//DEBUG_PRINTF("Firmware fd=%d, size=%d.\r\n", g_firmware_fd, g_firmware_size);
		if((g_firmware_fd < 0) || (g_firmware_size <= 0))
		{
			WriteLog("Fail to open firmware file \'%s\', size=%d, errno=0x%x.", \
	      		g_firmware_filename, g_firmware_size, g_firmware_fd);
			return PROCESS_ERR_INVLID_FILE;
			
		}
		lseek(g_firmware_fd, 0, SEEK_SET); // Reset file pointer
	}    
    return ret;
}

int resource_free(void)
{
    int err = PROCESS_ERR_OK;  

    /*** example *********************/
	if(bUpdate == true)
	{
		// Close Firmware File
		err = close_firmware_file(g_firmware_fd);
		g_firmware_fd = -1;
	}

	global_free();
    
    /*********************************/

    return err;
}

/***************************************************
* process assignment
***************************************************/
int process_assignment(int argc, char **argv)
{
	int ret = PROCESS_ERR_OK;
	int c = 0;
	int file_path_len = 0;	
	int pid_len = 0;	
	char file_path[FILE_NAME_LENGTH_MAX] = {0};
	char s_pid[FILE_NAME_LENGTH_MAX] = {0};
	while((c = getopt_long (argc, argv, short_options, long_options, NULL)) != -1)
	{
		switch (c)
		{
		    case 'f':				
		    	WriteLog("Get Fw Verison\r\n");
		    	bFW_Ver = true;
			break;            
		    case 'u':
			   	file_path_len = strlen(optarg);
				if ((file_path_len == 0) || (file_path_len > FILE_NAME_LENGTH_MAX))
				{
				    WriteLog("%s: Firmware Path (%s) Invalid!\r\n", __func__, optarg);
				    ret = PROCESS_ERR_INVLID_PARAM;                   
				}						
				memset(file_path, '\0', FILE_NAME_LENGTH_MAX);
				strncpy(file_path, optarg, FILE_NAME_LENGTH_MAX - 1);
				// Check if file path is valid
				//strcpy(file_path, optarg);				
				if(strncmp(file_path, "", FILE_NAME_LENGTH_MAX) == 0)
				{
				    WriteLog("%s: NULL Firmware Path!\r\n", __func__);
				    ret = PROCESS_ERR_INVLID_PARAM;                   
				}
				
				// Set FW Update Flag
				bUpdate = true;
				// Set Global File Path				
				strncpy(g_firmware_filename, file_path, FILE_NAME_LENGTH_MAX);				
				WriteLog("%s: Update FW: %s, File Path: \"%s\".\r\n", __func__, (bUpdate) ? "Yes" : "No", g_firmware_filename);
				WriteLog("Ready to Upgrade FW\r\n");
			break;
			case 'v':
		    	WriteLog("Get sw Verison\r\n");
		    	bSW_Ver = true;
			break;   
			case 't':
				WriteLog("Test Mode\r\n");
				bTestRun = true;				
			break;
			case 'h':				
				g_help = true;				
			break;
			case 'P':				
				pid_len = strlen(optarg);
				if (pid_len == 0) 
				{
				    WriteLog("%s: pid (%s) Invalid!\r\n", __func__, optarg);
				    ret = PROCESS_ERR_INVLID_PARAM;                   
				}				
				strcpy(s_pid, optarg);    
				WriteLog("eztool get pid : %s", s_pid);
				g_pid=strtol(s_pid,NULL,16);
							
			break;
		    default:
				WriteLog("Nothing\r\n");
		    break;
		    
		}

	}
	return ret;

}






/*******************************************
 * Main Function
 ******************************************/
int main(int argc, char **argv)
{
	int ret = 0;		
	u16 fw_ver = 0;	 

	WriteLog("Focal Upgrade Begin");
	ret = process_assignment(argc, argv);
	ret = resource_init();
	
	if(ret)
	{   
	    WriteLog("Get Bin File Fail: %d", ret);
        return ret;
    }
	ret = open_device() ;
    if(ret)
	{   
	    WriteLog("Open Hid Device Fail: %d", ret);
        return ret;
    }	
	
	if(bFW_Ver)
	{
		get_fw_version_data(&fw_ver);
		printf("%x \r\n",fw_ver);
		WriteLog("%x",fw_ver);
	}
	
    if(bSW_Ver)
    {        
    	printf("%s_%s \r\n",FTP_EZ_UPG_VERSION, FTP_EZ_UPG_RELEASE_DATE);
        WriteLog("%s_%s",FTP_EZ_UPG_VERSION, FTP_EZ_UPG_RELEASE_DATE);
    }
	
	if(bUpdate)
	{
		HID_Program_Upgrade();
	}   

	if(bTestRun)
	{
		Run_Test();	
	}
	
    if(g_help == true)
    {
        show_help_information();
    }	
	ret = close_device();

	ret = resource_free();

	WriteLog("end return: %d", ret);
	
	return ret;
}
