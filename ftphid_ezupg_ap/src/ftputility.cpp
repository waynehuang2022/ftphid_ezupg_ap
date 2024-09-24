/********************************************************************* 
 FocalTech HID Forcehpad Upgrade Tool 
 Copyright (c) 2022, FocalTech Systems Co.,Ltd.
 Author: Wayne  <shihwei.huang@focaltech-electronics.com>
*********************************************************************/ 



/********************************************************************* 

ftputility.cpp 
open file, read file data  

***********************************************************************/

#include "global.h"
#include "ftputility.h"

/***********************************************************
open_firmware_file: Get file handle with specific filename
************************************************************/
int open_firmware_file(char *filename, size_t filename_len, int *fd)
{
	int err = PROCESS_ERR_OK;
	int temp_fd = 0;

	// Make Sure Filename Valid
	if(filename == NULL)
	{
		ERROR_PRINTF("%s: NULL Filename String Pointer!\r\n", __func__);
		return PROCESS_ERR_INVLID_PARAM;
	}	

	// Make Sure Filename Length Valid
	if(filename_len == 0)
	{
		ERROR_PRINTF("%s: Filename String Length is Zero!\r\n", __func__);
		return PROCESS_ERR_INVLID_PARAM;
	}

	DEBUG_PRINTF("Open file \"%s\".\r\n", filename);
	temp_fd = open(filename, O_RDONLY);
	if(temp_fd < 0)
	{
		ERROR_PRINTF("%s: Failed to open firmware file \'%s\', errno=0x%x.\r\n", __func__, filename, temp_fd);
		return PROCESS_ERR_INVLID_FILE;		
	}

	DEBUG_PRINTF("File \"%s\" opened, fd=%d.\r\n", filename, temp_fd);
	*fd = temp_fd;
	
	return err;
}

/***************************************************
close_firmware_file: Release file handle
***************************************************/
int close_firmware_file(int fd)
{
	int err = PROCESS_ERR_OK;
   
	if(fd >= 0)
	{
		err = close(fd);
		if(err < 0)
		{
			ERROR_PRINTF("Failed to close firmware file(fd=%d), errno=0x%x.\r\n", fd, err);
	 		err = PROCESS_ERR_INVLID_FILE;
	 	}
	}
      
	return err;
}

/***************************************************
get_firmware_size: Get firmware size
***************************************************/
int get_firmware_size(int fd, int *firmware_size)
{
	int err = PROCESS_ERR_OK;	
	struct stat file_stat;
   
	err = fstat(fd, &file_stat);
	if(err < 0)
	{
		ERROR_PRINTF("Fail to get firmware file size, errno=0x%x.\r\n", err);
		err = PROCESS_ERR_INVLID_FILE;
	}
	else
	{
		DEBUG_PRINTF("File size=%zd.\r\n", file_stat.st_size);
		*firmware_size = file_stat.st_size;		
	}
   
	return err;
}

/***************************************************
retrieve_data_from_firmware: retrueve data
***************************************************/
int retrieve_data_from_firmware(unsigned char *data, int data_size)
{
	int err = PROCESS_ERR_OK;

	// Make Sure Page Data Buffer Valid
	if(data == NULL)
	{
		ERROR_PRINTF("%s: NULL Page Data Buffer!\r\n", __func__);
		return PROCESS_ERR_INVLID_PARAM;
	}	

	// Make Sure Page Data Buffer Size Valid
	if(data_size == 0)
	{
		ERROR_PRINTF("%s: Data Buffer Size is Zero!\r\n", __func__);
		return PROCESS_ERR_INVLID_PARAM;
	}

	// Read Data from File   
	err = read(g_firmware_fd, data, data_size);
	if(err != data_size)
	{
		ERROR_PRINTF("%s: Fail to get %d bytes from fd %d! (result=%d)\r\n", __func__, data_size, g_firmware_fd, err);
		err = PROCESS_ERR_GET_DATA_FAIL;
	}

	return err;
}






