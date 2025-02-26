/********************************************************************* 
 FocalTech HID Forcehpad Upgrade Tool 
 Copyright (c) 2022, FocalTech Systems Co.,Ltd.
 Author: Wayne  <shihwei.huang@focaltech-electronics.com>
*********************************************************************/ 



#include "global.h"
#include "ftputility.h"
#include "ftpprogram.h"

/***************************************************
 * Functions
 ***************************************************/ 

/***************************************************
 *  ftp_hid_wr:
 ***************************************************/ 
u8 ftp_hid_wr(u8 *wBuf, u8 wLen, u8*rBuf, u8 rLen)
{	
	u8 ReCode = COMM_HID_OK;
	
	ReCode = WRData(wBuf, wLen, rBuf, rLen);
	return ReCode;
}

 
/*******************************************
 ftp_hid_write: write reg data by hid
******************************************/
u8 ftp_hid_write(u8 *wBuf, u8 wLen)
{	
	u8 ReCode = COMM_HID_OK;
	
	ReCode = SendData(wBuf, wLen);	
	return ReCode;
}

/*******************************************
 ftp_hid_write: read reg data by hid
******************************************/
u8 ftp_hid_read(u8 *rbuf, u8 read_len)
{	

	u8 ReCode = COMM_HID_OK;	
	
	ReCode = ReadData(rbuf, read_len);		
	return ReCode;
}

/*******************************************
 GetChecksum: packekg checksum
******************************************/
u8 GetChecksum(u8 *buf, u8 len)
{
	u8 i;
	u8 checksum = 0;
	for(i = 0; i < len; i++)
		checksum ^= buf[i];
	checksum++;
	return checksum;	
}

/*******************************************
 ftp_hid_io: HID Write/Read IO interface
******************************************/
u8 ftp_hid_io(u8 *wbuf, u8 write_len, u8 *rbuf, unsigned int read_len)
{	
	
	u8 cmdlen = 0;	
	u8 buf[64] = {0};
	u8 gbuf[64] = {0};
	u8 ReCode = COMM_HID_OK;

	if(m_protocol == 0)
	{
		cmdlen = 4 + write_len;	
		buf[0] = 0x06; 
		buf[1] = 0xff;
		buf[2] = 0xff;
		buf[3] = cmdlen;
		memcpy(&buf[4], wbuf, write_len);	
		buf[cmdlen] = GetChecksum(&buf[1], cmdlen - 1);
		ReCode = ftp_hid_wr(buf, cmdlen+1, gbuf, read_len);
		memcpy(rbuf, gbuf, read_len);	
	}
	else
	{
		if(write_len>0)
		{
			cmdlen = 4 + write_len;		
			buf[0] = 0x06; //id
			buf[1] = 0xff;
			buf[2] = 0xff;
			buf[3] = cmdlen;
			memcpy(&buf[4], wbuf, write_len);
			buf[cmdlen] = GetChecksum(&buf[1], cmdlen - 1);
			ReCode = ftp_hid_write(buf, cmdlen + 1);
			if(ReCode)
				return COMM_HID_WRITE_USB_ERROR;
			
		}	
		
		if(read_len>0)
		{		 
			memset(buf, 0, sizeof(buf));
			ReCode = ftp_hid_read(buf, read_len);
			memcpy(rbuf,buf,read_len);		
			
		}
	}
	return ReCode;
}


/*******************************************
 ftp_WriteReg: Register byte write
******************************************/
u8 ftp_WriteReg(u8 RegAddr, u8 RegData)
{
	u8 ReCode = COMM_HID_OK; 
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);
 	
	CmdPacket[0] = CMD_WRITE_REGISTER;
	CmdPacket[1] = RegAddr;
	CmdPacket[2] = RegData;	
	
	if(m_protocol == 0)
	{
		ReCode = ftp_hid_io(CmdPacket, 3, RePacket, 7); //write and read ack 
		
		if(ReCode == COMM_HID_OK)
		{
			if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
			{
				if(RePacket[4] == CMD_WRITE_REGISTER)
				{
					ReCode = COMM_HID_OK;
				}
				else
				{
					ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
				}
			}
			else
			{
				ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
			}
		}
	}
	else
	{	
		ReCode = ftp_hid_io(CmdPacket, 3, RePacket, 0); //Write 	
		usleep(200);
		ReCode = ftp_hid_io(CmdPacket, 0, RePacket, 7); //Read ack 
		
		if(ReCode == COMM_HID_OK)
		{
			if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
			{
				if(RePacket[4] == CMD_WRITE_REGISTER)
				{
					ReCode = COMM_HID_OK;
				}
				else
				{
					ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
				}
			}
			else
			{
				ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
			}
		}
	}
	return ReCode;
}

/*******************************************
 ftp_WriteReg: Register byte read
******************************************/
u8 ftp_ReadReg(u8 RegAddr, u8 *pRegData)
{
	
	u8 ReCode = COMM_HID_OK; 
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	int i=0;
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE); 	


	CmdPacket[0] = CMD_READ_REGISTER;
	CmdPacket[1] = RegAddr;
	
	if(m_protocol == 0)
	{
		ReCode = ftp_hid_io(CmdPacket, 2, RePacket, 8); //Read
		if(ReCode == COMM_HID_OK)
		{
			if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
			{
				if(RePacket[4] == CMD_READ_REGISTER)//����
				{
					*pRegData = RePacket[6];					
					return COMM_HID_OK;
				}
				else
				{					
					ReCode = COMM_HID_PACKET_COMMAND_ERROR;						
				}
			}
			else
			{				
				ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
			}
		}		
		usleep(200);
	}
	else
	{
		ReCode = ftp_hid_io(CmdPacket, 2, RePacket, 0); //Write 
	
		usleep(200);
		for(i=0; i<4; i++)
		{
			ReCode = ftp_hid_io(CmdPacket, 0, RePacket, 8); //Read
			if(ReCode == COMM_HID_OK)
			{
				if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
				{
					if(RePacket[4] == CMD_READ_REGISTER)
					{
						*pRegData = RePacket[6];					
						return COMM_HID_OK;
					}
					else
					{
						ReCode = COMM_HID_PACKET_COMMAND_ERROR;						
					}
				}
				else
				{
					ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
				}
			}
			usleep(200);
		}
	}
	return ReCode;
}

/*******************************************
 Test_Enter_Work
******************************************/
u8 Test_Enter_Work()
{

    u8 retry = 0;
    u8 value = 0;
    u8 ReCode = COMM_HID_OK;
    for(retry=0; retry<5; retry++) 
    {
        if(COMM_HID_OK == ftp_ReadReg(0x00, &value))
        {        
            if((value&0x40) == 0x00)
                return COMM_HID_OK;
            ReCode=ftp_WriteReg(0x00, (value&~0x40));
        }
        else
            ReCode = COMM_HID_READ_USB_ERROR;
        SleepMS(50);    
    }
    return ReCode;
    
}
/*******************************************
 get_fw_version_data: Get Version
******************************************/
u8 get_fw_version_data(u16 *p_fw_version)
{

	u8 ReCode = COMM_HID_OK; 
	u8 Ver[2] = {0};
	Test_Enter_Work();
	/* Check Data Buffer */
	if(p_fw_version == NULL)
	{
		ERROR_PRINTF("%s: NULL Pointer!\r\n", __func__);
		return COMM_HID_INVLID_PARAM;        
	}
	
	ReCode = ftp_ReadReg(0xA6, &Ver[0]);
	ReCode = ftp_ReadReg(0xAD, &Ver[1]);	
	*p_fw_version = (u16)(Ver[0]<<8 | Ver[1]);
	
	return ReCode;	

}


/************************************************
 COMM_FLASH_EnterUpgradeMode: Enter Upgrade mode
*************************************************/
u8 COMM_FLASH_EnterUpgradeMode()
{
	u8 ReCode = COMM_HID_OK;
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);	
	CmdPacket[0] = CMD_ENTER_UPGRADE_MODE;	
	
	ReCode = ftp_hid_io(CmdPacket, 1, RePacket, 6);
	if(ReCode == COMM_HID_OK)
	{		
		//intf("Repacket End[%02x checksum[%d] \r\n", RePacket[RePacket[3]], i);
		if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
		{
			if(RePacket[4] == CMD_ACK)
			{
				ReCode = COMM_HID_OK;
			}
			else
			{
				ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
			}
		}
		else
		{
			ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
		}
	}	
	return ReCode;
}


/**********************************************************
 COMM_FLASH_CheckCurrentState: Get BootLoader Current State
***********************************************************/
u8 COMM_FLASH_CheckCurrentState(u8 *pData)
{
	u8 ReCode = COMM_HID_OK;
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);
	
	CmdPacket[0] = CMD_CHECK_CURRENT_STATE;

	ReCode = ftp_hid_io(CmdPacket,1, RePacket, 7);

	if(ReCode == COMM_HID_OK)
	{			
		if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
		{
			if(RePacket[4] == CMD_CHECK_CURRENT_STATE)
			{
				*pData = RePacket[5];
			}
			else
			{
				ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
			}
		}
		else
		{
			ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
		}		
	}
	return ReCode;

}


/**********************************************************
COMM_FLASH_CheckTPIsReadyForUpgrade: Check ready
***********************************************************/
u8 COMM_FLASH_CheckTPIsReadyForUpgrade()
{
	u8 ReCode = COMM_HID_OK;
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);
	
	CmdPacket[0] = CMD_READY_FOR_UPGRADE;
	ReCode = ftp_hid_io(CmdPacket, 1, RePacket, 7);
	
	if(ReCode == COMM_HID_OK)
	{	
		if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	
		{
			if(RePacket[4] == CMD_READY_FOR_UPGRADE)
			{
				if(2 == RePacket[5])				
					ReCode = COMM_HID_OK;				
				else
					ReCode = COMM_HID_PACKET_COMMAND_ERROR;
			}	
			else			
				ReCode = COMM_HID_PACKET_DATA_ERROR;	
		}
		else			
			ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
	}
	return ReCode;

}

/**********************************************************
COMM_FLASH_Checksum_Upgrade: Get check sum for write done
***********************************************************/
u8 COMM_FLASH_Checksum_Upgrade(unsigned int *pData)
{
	u8 ReCode = COMM_HID_OK;
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);	
	
	CmdPacket[0] = CMD_UPGRADE_CHECKSUM;	

	ReCode = ftp_hid_io(CmdPacket,1, RePacket, 7 + 3);
	
	if(ReCode == COMM_HID_OK)
	{	
		if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
		{
			if(RePacket[4] == CMD_UPGRADE_CHECKSUM)				
				*pData = (RePacket[8] << 24) +  (RePacket[7] << 16) + (RePacket[6] << 8) + RePacket[5];				
			else				
				ReCode = COMM_HID_PACKET_COMMAND_ERROR;					
		}
		else
		{
			ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
		}
	}
	
	return ReCode;

}

/**********************************************************
COMM_FLASH_USB_ReadUpdateID: Get Bootload ID
***********************************************************/
u8 COMM_FLASH_USB_ReadUpdateID(unsigned short* usIcID)
{
	u8 ReCode = COMM_HID_OK;
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	int i=0;
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);

	CmdPacket[0] = CMD_USB_READ_UPGRADE_ID;    

	for(i = 0; i < 10; i++)
	{
		SleepMS(100);
		ReCode = ftp_hid_io(CmdPacket,1, RePacket, 8);
		if(ReCode == COMM_HID_OK)
		{	
			if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
			{
				if(RePacket[4] == CMD_USB_READ_UPGRADE_ID)
				{
					*usIcID= ((RePacket[5])<<8) + RePacket[6];					
					ReCode = COMM_HID_OK;
					break;					
				}
				else
				{
					ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
				}
			}
			else
			{
				ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
			}
		}

	}
	return ReCode;
}

/**********************************************************
COMM_FLASH_USB_EraseFlash: Erase Flash
***********************************************************/
u8 COMM_FLASH_USB_EraseFlash()
{
	u8 ReCode = COMM_HID_OK;
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);

	CmdPacket[0] = CMD_USB_ERASE_FLASH;    

	ReCode = ftp_hid_io(CmdPacket,1, RePacket, 6);
	if(ReCode == COMM_HID_OK)
	{	
		if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
		{
			if(RePacket[4] == CMD_ACK)
			{				
				ReCode = COMM_HID_OK;				
			}
			else
			{
				ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
			}
		}
		else
		{
			ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
		}
	}

	return ReCode;
}

/**********************************************************
COMM_FLASH_USB_EraseFlashArea: Erase Flash
***********************************************************/
u8 COMM_FLASH_USB_EraseFlashArea(u8 erase_id)
{
	u8 ReCode = COMM_HID_OK;
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);

	CmdPacket[0] = CMD_USB_ERASE_FLASH;
	if(erase_id == 0)
	    CmdPacket[1] = 0x0B;
	else if(erase_id == 1)
	    CmdPacket[1] = 0x0C;
    else
	    CmdPacket[1] = 0x0E; 
	ReCode = ftp_hid_io(CmdPacket, 2, RePacket, 6);
	if(ReCode == COMM_HID_OK)
	{	
		if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
		{
			if(RePacket[4] == CMD_ACK)
			{				
				ReCode = COMM_HID_OK;				
			}
			else
			{
				ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
			}
		}
		else
		{
			ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
		}
	}

	return ReCode;
}

/**********************************************************
COMM_FLASH_WriteBinFileLength
***********************************************************/
u8 COMM_FLASH_WriteBinFileLength(unsigned char mode, unsigned int BinSize)
{
	u8 ReCode = COMM_HID_OK;
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);
	
	if(mode == 3)
	{
		CmdPacket[0] = CMD_WRITE_REGISTER;
		CmdPacket[1] = 0x7A;	
		CmdPacket[2] = 0x00;
		CmdPacket[3] = (BinSize >> 16) & 0xFF;
		CmdPacket[4] = (BinSize >> 8) & 0xFF;
		CmdPacket[5] = BinSize & 0xFF;
		ReCode = ftp_hid_io(CmdPacket, 6, RePacket, 7);
		if(ReCode == COMM_HID_OK)
		{	
			if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
			{
				if(RePacket[4] == CMD_WRITE_REGISTER)
				{				
					ReCode = COMM_HID_OK;				
				}
				else
				{
					ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
				}
			}
			else
			{
				ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
			}
		}		
	}
	else
	{
		CmdPacket[0] = 0x4B;	
		CmdPacket[1] = (BinSize >> 16) & 0xFF;
		CmdPacket[2] = (BinSize >> 8) & 0xFF;
		CmdPacket[3] = BinSize & 0xFF;
		ReCode = ftp_hid_io(CmdPacket, 4, RePacket, 7);
		if(ReCode == COMM_HID_OK)
		{	
			if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
			{
				if(RePacket[4] == CMD_ACK)
				{				
					ReCode = COMM_HID_OK;				
				}
				else
				{
					ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
				}
			}
			else
			{
				ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
			}
		}
	}
	
	return ReCode;
}


/**********************************************************
COMM_FLASH_SendDataByUSB: Send Write data
***********************************************************/
u8 COMM_FLASH_SendDataByUSB(u8 PacketType, u8 * SendData, u8 DataLength)
{

	if(DataLength > REPORT_SIZE - 8) 
	    return COMM_HID_INVLID_PARAM;
	u8 ReCode = COMM_HID_OK; 
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	int retry=0;
	
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);	
	
	CmdPacket[0] = CMD_SEND_DATA;	
	CmdPacket[1] = PacketType;
	memcpy(CmdPacket + 2, SendData, DataLength);	
	ReCode = ftp_hid_io(CmdPacket, DataLength + 2, RePacket, 0);
	
	if(ReCode == COMM_HID_OK)
	{		
		for(retry=0; retry<4; retry++)
		{
			ReCode = ftp_hid_io(CmdPacket, 0, RePacket, 7);
			if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
			{
				if(RePacket[4] == CMD_ACK)
				{
					return COMM_HID_OK;					
				}
				else
				{
					ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
				}
			}
			else
			{
				ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;
			}
			SleepMS(1);
		}
	}

	return ReCode;

}

/**********************************************************
COMM_FLASH_SendDataAreaByUSB: Send Write area data
***********************************************************/
u8 COMM_FLASH_SendDataAreaByUSB(u32 addr, u8* SendData, u8 DataLength)
{

	if(DataLength > REPORT_SIZE - 8) 
	    return COMM_HID_INVLID_PARAM;
	u8 ReCode = COMM_HID_OK; 
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];	
	
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);	
	
	CmdPacket[0] = CMD_SEND_DATA;
	CmdPacket[1] = (unsigned char)(addr >> 16);
	CmdPacket[2] = (unsigned char)(addr >> 8);
	CmdPacket[3] = (unsigned char)(addr & 0xff);
	memcpy(CmdPacket + 4, SendData, DataLength);	
	ReCode = ftp_hid_io(CmdPacket, DataLength + 4, RePacket, 7);
	
	if(ReCode == COMM_HID_OK)
	{		
		
		if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
		{
			if(RePacket[4] == CMD_ACK)
			{
				return COMM_HID_OK;					
			}
			else
			{
				ReCode = COMM_HID_PACKET_COMMAND_ERROR;	
			}
		}	
		else		
			ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;		
	}

	return ReCode;

}



/**********************************************************
COMM_FLASH_ExitUpgradeMode: exit upgrade mode
***********************************************************/
u8 COMM_FLASH_ExitUpgradeMode()
{
	u8 ReCode = COMM_HID_OK;
	u8 CmdPacket[REPORT_SIZE];
	u8 RePacket[REPORT_SIZE];
	memset(CmdPacket, 0xff, REPORT_SIZE);
	memset(RePacket, 0xff, REPORT_SIZE);


	CmdPacket[0] = CMD_EXIT_UPRADE_MODE;

	ReCode = ftp_hid_io( CmdPacket,1, RePacket, 6);

	if(ReCode == COMM_HID_OK)
	{
		if(RePacket[RePacket[3]] == GetChecksum(RePacket+1, RePacket[3]-1))	//check crc
		{
			if(RePacket[4] == CMD_ACK)
				ReCode = COMM_HID_OK;			
			else			
				ReCode = COMM_HID_PACKET_COMMAND_ERROR;				
		}
		else		
			ReCode = COMM_HID_PACKET_CHECKSUM_ERROR;		
	}
	return ReCode;
}

/**********************************************************
Program_Upgrade5452
***********************************************************/
u8 Program_Upgrade5452(u8 UpgradeStep)
{
	u8 g_DataBuffer[128* 1024] = {0};
	u8 ProgramCode = PROGRAM_CODE_OK;
	u8 ReCode;
	u8 ucMode = 0;
	u8 data[64];
	u8 ucPacketType = 0;
	u8 retry;
	u8 Step;
	unsigned int DValue = 0;
	unsigned int Checksum = 0, FWChecksum;
	unsigned int DataLen = 0, SentDataLen = 0;
	unsigned int MaxLength = 0;				
	unsigned int i=0;
	unsigned short usIcID;
	bool bUpgrading=false;		

	MaxLength = g_firmware_size;	
   	MaxLength = (MaxLength + 3) & ~3; //= *4 / 4;
   	
	memset(g_DataBuffer, 0xff, MaxLength);	
	ReCode = retrieve_data_from_firmware(g_DataBuffer, g_firmware_size);
	
	//Calculate FW checksum...
	Checksum = 0;
	for(i = 0; i < MaxLength; i += 4)
	{
		DValue = (g_DataBuffer[i + 3] << 24) + (g_DataBuffer[i + 2] << 16) +(g_DataBuffer[i + 1] << 8) + g_DataBuffer[i];
		Checksum ^= DValue;
	}	
	Checksum += 1;	

	Step=UpgradeStep;
	retry=0;
	bUpgrading=true;
	while(bUpgrading)
	{
		switch(Step)
		{
			case(USB_UPGRADE_ENTRY_BOOTLOADER): 			
				//Enter Upgrade Mode... AP->Bootloader
				WriteLog("Enter Upgrade Mode...");				
				COMM_FLASH_EnterUpgradeMode();
				SleepMS(300);
				Step=USB_UPGRADE_ENTRY_UPGRADE;
				retry=0; 
				break;
			case(USB_UPGRADE_ENTRY_UPGRADE):
				//MessageBox(NULL, "Step=1", "Upgrade", MB_OK);
				//BootLoader start to upgrade
				ReCode = COMM_FLASH_EnterUpgradeMode();
				if(ReCode == PROGRAM_CODE_OK)
				{		
					SleepMS(100);
					ReCode = COMM_FLASH_CheckCurrentState(&ucMode);
					if(ucMode == 1) //1: Upgrade Mode; 2: FW Mode
					{
						Step=USB_UPGRADE_ERASE_FLASH;
						retry=0;
					}
					else
					{
						SleepMS(200);	
						WriteLog("Get Current State, Times: %d...", retry++);
						retry++;
						if(retry==3)
						{
							WriteLog("Failed to Get Current State! ");
							ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
							Step=USB_UPGRADE_END;
						}			
					}
				}
				else
				{	
					WriteLog("Enter Upgrade Mode..%dS.", retry+1);
					retry++;
					if(retry==3)
					{
						WriteLog("Failed to enter Upgrade Mode! ");
						ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
						Step=USB_UPGRADE_END;
					}					
				}
				break;
			case(USB_UPGRADE_ERASE_FLASH):				
				//Read chip id & Erase Flash
				ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
				if(ReCode == PROGRAM_CODE_OK)
				{
					ReCode = COMM_FLASH_USB_ReadUpdateID(&usIcID);
					if(ReCode != PROGRAM_CODE_OK)
					{
						WriteLog("Read 5452 id error. ");
						ProgramCode = PROGRAM_CODE_CHIP_ID_ERROR;
						Step=USB_UPGRADE_END;
						continue;
					}
					else						
					{
						if(0x545E != usIcID)
						{
							WriteLog("%04X ID is error. ", usIcID);
							ProgramCode = PROGRAM_CODE_CHIP_ID_ERROR;
							Step=USB_UPGRADE_END;
							continue;
						}
					}		
					COMM_FLASH_USB_EraseFlash();			
					SleepMS(1000);
					WriteLog("Erase Time: 1S ");
					Step=USB_UPGRADE_CHECK_ERASE_READY;
					retry=0;
				}
				else
				{	
					SleepMS(50);
					retry++;
					if(retry==3)
					{
						WriteLog("TP is not ready for upgrade ");
						ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
						Step=USB_UPGRADE_END;
					}
				}
				break;
			case(USB_UPGRADE_CHECK_ERASE_READY):				
				//Check Erase is Ready?
				ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
				if(ReCode == PROGRAM_CODE_OK)
				{
					Step=USB_UPGRADE_SEND_DATA;
					retry=0;	
					DataLen = 0;
					SentDataLen=0;
				}
				else
				{					
					SleepMS(500);
					retry++;
					WriteLog("Erase Time: %d.%dS ", (1+retry/2),(5*(retry%2)));
					if(retry==20)
					{
						WriteLog("Erase Flash Error. "); 
						ProgramCode = PROGRAM_CODE_ERASE_FLASH_ERROR;
						Step=USB_UPGRADE_END;
					}
				}
				break;
			case(USB_UPGRADE_SEND_DATA):				
				//Send Packet Data 
				if(SentDataLen < MaxLength)
				{
					if(retry==0)
					{
						DataLen=0;
						memset(data, 0xff, sizeof(data));									
						if(SentDataLen + MAX_USB_PACKET_SIZE_M1 > MaxLength)
						{
							memcpy(data, g_DataBuffer + SentDataLen, MaxLength - SentDataLen);
							DataLen = MaxLength - SentDataLen;
							ucPacketType = END_PACKET;
						}
						else
						{				
							memcpy(data, g_DataBuffer + SentDataLen, MAX_USB_PACKET_SIZE_M1);
							DataLen = MAX_USB_PACKET_SIZE_M1;
							if(SentDataLen)
								ucPacketType = MID_PACKET;
							else
								ucPacketType = FIRST_PACKET;
						}
						ReCode = COMM_FLASH_SendDataByUSB(ucPacketType, data, DataLen); 					
						retry++;
					}

					if(retry>0)
					{	
						SentDataLen += DataLen;
						if (SentDataLen < MaxLength)
						{
							ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
							if (PROGRAM_CODE_OK == ReCode)
							{
								WriteLog("Updating %d bytes...", SentDataLen);
								retry = 0;
							}
							else
							{
								SleepMS(1);
								if (++retry > 20)
								{
									WriteLog("Upgrade failed! ");
									ProgramCode = PROGRAM_CODE_WRITE_FLASH_ERROR;
									COMM_FLASH_ExitUpgradeMode();		//Reset
									Step = USB_UPGRADE_END;
								}

							}
						}												
					}
				}				
				else
				{
					//Write flash End and check ready (fw calculate checksum)
					SleepMS(200); 
					ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
					if(ReCode == PROGRAM_CODE_OK)	
					{			
						Step=USB_UPGRADE_CHECK_SUM;
						retry=0;
					}
					else
					{
						if(++retry > 5)
						{
							WriteLog("Upgrade failed!");
							ProgramCode = PROGRAM_CODE_WRITE_FLASH_ERROR;
							COMM_FLASH_ExitUpgradeMode();		//Reset
							Step=USB_UPGRADE_END;
						}
						
					}
				}
				break;
			case(USB_UPGRADE_CHECK_SUM):			
				ReCode = COMM_FLASH_Checksum_Upgrade(&FWChecksum);
				if(ReCode == PROGRAM_CODE_OK)
				{				
					if(Checksum == FWChecksum)
					{						
						WriteLog("Checksum Right, PC:0x%x, FW:0x%x!", Checksum, FWChecksum);
						Step=USB_UPGRADE_EXIT;
						retry=0;
						COMM_FLASH_ExitUpgradeMode();		//Reset
					}
					else
					{	
						WriteLog("Checksum error, PC:0x%x, FW:0x%x!", Checksum, FWChecksum);						
						SleepMS(500);
						ProgramCode = PROGRAM_CODE_CHECKSUM_ERROR;
						Step=USB_UPGRADE_END;
					}
				}
				else
				{				
					ProgramCode = PROGRAM_CODE_CHECKSUM_ERROR;
					Step=USB_UPGRADE_END;
				}					
				break;
			case(USB_UPGRADE_EXIT): 
				SleepMS(500);				
				ReCode = ftp_ReadReg(0x9F,&data[0]);
				ReCode = ftp_ReadReg(0xA3,&data[1]);
				WriteLog("Exit Upgrade Mode, Times: %d, id1=%x, id2=%x ", retry,data[1],data[0]);				
				if((data[1] == 0x54) && (data[0] == 0x52))
				{					
					Step=USB_UPGRADE_END;
					retry=0;			
					WriteLog("Upgrade is successful! ");
				}
				else
				{
					retry++;
					if(retry==4)
					{
						ProgramCode = PROGRAM_CODE_RESET_SYSTEM_ERROR;
						Step=USB_UPGRADE_END;
					}
				}				
				break;
			case(USB_UPGRADE_END):
				bUpgrading=false;
				break;
		}
		
	}
	return ProgramCode;
}

/**********************************************************
Program_Upgrade5822
***********************************************************/
u8 Program_Upgrade5822(u8 UpgradeStep)
{
	u8 g_DataBuffer[128* 1024] = {0};
	u8 ProgramCode = PROGRAM_CODE_OK;
	u8 ReCode;
	u8 ucMode = 0;
	u8 data[64];
	u8 ucPacketType = 0;
	u8 retry;
	u8 Step;
	unsigned int DValue = 0;
	unsigned int Checksum = 0, FWChecksum;
	unsigned int DataLen = 0, SentDataLen = 0;
	unsigned int MaxLength = 0;				
	unsigned int i=0;
	unsigned short usIcID;
	bool bUpgrading=false;
	
	if(g_firmware_size<54*1024) 	
		MaxLength = 54 *1024;		
	else		
		MaxLength = g_firmware_size;
		
	memset(g_DataBuffer, 0xff, MaxLength);	
	ReCode = retrieve_data_from_firmware(g_DataBuffer, g_firmware_size);
	
	//Calculate FW checksum...
	Checksum = 0;
	for(i = 0; i < MaxLength; i += 4)
	{
		DValue = (g_DataBuffer[i + 3] << 24) + (g_DataBuffer[i + 2] << 16) +(g_DataBuffer[i + 1] << 8) + g_DataBuffer[i];
		Checksum ^= DValue;
	}	
	Checksum += 1;	

	Step=UpgradeStep;
	retry=0;
	bUpgrading=true;
	while(bUpgrading)
	{
		switch(Step)
		{
			case(USB_UPGRADE_ENTRY_BOOTLOADER): 			
				//Enter Upgrade Mode... AP->Bootloader
				WriteLog("Enter Upgrade Mode...");				
				COMM_FLASH_EnterUpgradeMode();
				SleepMS(300);
				Step=USB_UPGRADE_ENTRY_UPGRADE;
				retry=0; 
				break;
			case(USB_UPGRADE_ENTRY_UPGRADE):
				//MessageBox(NULL, "Step=1", "Upgrade", MB_OK);
				//BootLoader start to upgrade
				ReCode = COMM_FLASH_EnterUpgradeMode();
				if(ReCode == PROGRAM_CODE_OK)
				{		
					SleepMS(100);
					ReCode = COMM_FLASH_CheckCurrentState(&ucMode);
					if(ucMode == 1) //1: Upgrade Mode; 2: FW Mode
					{
						Step=USB_UPGRADE_ERASE_FLASH;
						retry=0;
					}
					else
					{
						SleepMS(200);	
						WriteLog("Get Current State, Times: %d...", retry++);
						retry++;
						if(retry==3)
						{
							WriteLog("Failed to Get Current State! ");
							ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
							Step=USB_UPGRADE_END;
						}			
					}
				}
				else
				{	
					WriteLog("Enter Upgrade Mode..%dS.", retry+1);
					retry++;
					if(retry==3)
					{
						WriteLog("Failed to enter Upgrade Mode! ");
						ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
						Step=USB_UPGRADE_END;
					}					
				}
				break;
			case(USB_UPGRADE_ERASE_FLASH):				
				//Read chip id & Erase Flash
				ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
				if(ReCode == PROGRAM_CODE_OK)
				{
					ReCode = COMM_FLASH_USB_ReadUpdateID(&usIcID);
					if(ReCode != PROGRAM_CODE_OK)
					{
						WriteLog("Read 5452 id error. ");
						ProgramCode = PROGRAM_CODE_CHIP_ID_ERROR;
						Step=USB_UPGRADE_END;
						continue;
					}
					else						
					{
						if(0x582E != usIcID)
						{
							WriteLog("%04X ID is error. ", usIcID);
							ProgramCode = PROGRAM_CODE_CHIP_ID_ERROR;
							Step=USB_UPGRADE_END;
							continue;
						}
					}		
					COMM_FLASH_USB_EraseFlash();			
					SleepMS(1000);
					WriteLog("Erase Time: 1S ");
					Step=USB_UPGRADE_CHECK_ERASE_READY;
					retry=0;
				}
				else
				{	
					SleepMS(50);
					retry++;
					if(retry==3)
					{
						WriteLog("TP is not ready for upgrade ");
						ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
						Step=USB_UPGRADE_END;
					}
				}
				break;
			case(USB_UPGRADE_CHECK_ERASE_READY):				
				//Check Erase is Ready?
				ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
				if(ReCode == PROGRAM_CODE_OK)
				{
					Step=USB_UPGRADE_SEND_DATA;
					retry=0;	
					DataLen = 0;
					SentDataLen=0;
				}
				else
				{					
					SleepMS(500);
					retry++;
					WriteLog("Erase Time: %d.%dS ", (1+retry/2),(5*(retry%2)));
					if(retry==20)
					{
						WriteLog("Erase Flash Error. "); 
						ProgramCode = PROGRAM_CODE_ERASE_FLASH_ERROR;
						Step=USB_UPGRADE_END;
					}
				}
				break;
			case(USB_UPGRADE_SEND_DATA):				
				//Send Packet Data 
				if(SentDataLen < MaxLength)
				{
					if(retry==0)
					{
						DataLen=0;
						memset(data, 0xff, sizeof(data));									
						if(SentDataLen + MAX_USB_PACKET_SIZE_M1 > MaxLength)
						{
							memcpy(data, g_DataBuffer + SentDataLen, MaxLength - SentDataLen);
							DataLen = MaxLength - SentDataLen;
							ucPacketType = END_PACKET;
						}
						else
						{				
							memcpy(data, g_DataBuffer + SentDataLen, MAX_USB_PACKET_SIZE_M1);
							DataLen = MAX_USB_PACKET_SIZE_M1;
							if(SentDataLen)
								ucPacketType = MID_PACKET;
							else
								ucPacketType = FIRST_PACKET;
						}
						ReCode = COMM_FLASH_SendDataByUSB(ucPacketType, data, DataLen); 					
						retry++;
					}

					if(retry>0)
					{	
						SentDataLen += DataLen;
						if (SentDataLen < MaxLength)
						{
							ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
							if (PROGRAM_CODE_OK == ReCode)
							{
								WriteLog("Updating %d bytes...", SentDataLen);
								retry = 0;
							}
							else
							{
								SleepMS(1);
								if (++retry > 20)
								{
									WriteLog("Upgrade failed! ");
									ProgramCode = PROGRAM_CODE_WRITE_FLASH_ERROR;
									COMM_FLASH_ExitUpgradeMode();		//Reset
									Step = USB_UPGRADE_END;
								}

							}
						}												
					}
				}				
				else
				{
					//Write flash End and check ready (fw calculate checksum)
					SleepMS(200); 
					ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
					if(ReCode == PROGRAM_CODE_OK)	
					{			
						Step=USB_UPGRADE_CHECK_SUM;
						retry=0;
					}
					else
					{
						if(++retry > 5)
						{
							WriteLog("Upgrade failed!");
							ProgramCode = PROGRAM_CODE_WRITE_FLASH_ERROR;
							COMM_FLASH_ExitUpgradeMode();		//Reset
							Step=USB_UPGRADE_END;
						}
						
					}
				}
				break;
			case(USB_UPGRADE_CHECK_SUM):			
				ReCode = COMM_FLASH_Checksum_Upgrade(&FWChecksum);
				if(ReCode == PROGRAM_CODE_OK)
				{				
					if(Checksum == FWChecksum)
					{						
						WriteLog("Checksum Right, PC:0x%x, FW:0x%x!", Checksum, FWChecksum);
						Step=USB_UPGRADE_EXIT;
						retry=0;
						COMM_FLASH_ExitUpgradeMode();		//Reset
					}
					else
					{	
						WriteLog("Checksum error, PC:0x%x, FW:0x%x!", Checksum, FWChecksum);						
						SleepMS(500);
						ProgramCode = PROGRAM_CODE_CHECKSUM_ERROR;
						Step=USB_UPGRADE_END;
					}
				}
				else
				{				
					ProgramCode = PROGRAM_CODE_CHECKSUM_ERROR;
					Step=USB_UPGRADE_END;
				}					
				break;
			case(USB_UPGRADE_EXIT): 
				SleepMS(500);				
				ReCode = ftp_ReadReg(0x9F,&data[0]);
				ReCode = ftp_ReadReg(0xA3,&data[1]);
				WriteLog("Exit Upgrade Mode, Times: %d, id1=%x, id2=%x ", retry,data[1],data[0]);				
				if((data[1] == 0x58) && (data[0] == 0x22))
				{					
					Step=USB_UPGRADE_END;
					retry=0;			
					WriteLog("Upgrade is successful! ");
				}
				else
				{
					retry++;
					if(retry==4)
					{
						ProgramCode = PROGRAM_CODE_RESET_SYSTEM_ERROR;
						Step=USB_UPGRADE_END;
					}
				}				
				break;
			case(USB_UPGRADE_END):
				bUpgrading=false;
				break;
		}
		
	}
	return ProgramCode;
}

/**********************************************************
Program_Upgrade5456
***********************************************************/
u8 Program_Upgrade5456(u8 UpgradeStep)
{
	u8 g_DataBuffer[128* 1024] = {0};
	u8 bChecksum[4]={0};	
	u8 ProgramCode = PROGRAM_CODE_OK;
	u8 ReCode;
	u8 ucMode = 0;
	u8 data[64];
	u8 ucPacketType = 0;
	u8 retry;
	u8 Step;
	unsigned int DValue = 0;
	unsigned int Checksum = 0, FWChecksum;
	unsigned int DataLen = 0, SentDataLen = 0;
	unsigned int MaxLength = 0;				
	unsigned int i=0;
	unsigned short usIcID;
	bool bUpgrading=false;		


	MaxLength = g_firmware_size;    
	MaxLength =  (MaxLength+255)/256 * 256;
	if(MaxLength > 64*1024)
	   	MaxLength = 64*1024;    	    

	memset(g_DataBuffer, 0xff, MaxLength);	
	ReCode = retrieve_data_from_firmware(g_DataBuffer, g_firmware_size);
	
	//Calculate FW checksum...
	Checksum = 0;
	for(i = 0; i < MaxLength; i += 4)
	{
		DValue = (g_DataBuffer[i + 3] << 24) + (g_DataBuffer[i + 2] << 16) +(g_DataBuffer[i + 1] << 8) + g_DataBuffer[i];
		Checksum ^= DValue;
	}	

	bChecksum[0] = ((unsigned char*)&Checksum)[0] + 1;
	bChecksum[1] = ((unsigned char*)&Checksum)[1];
	bChecksum[2] = ((unsigned char*)&Checksum)[2];
	bChecksum[3] = ((unsigned char*)&Checksum)[3];
	Checksum = ((unsigned int*)&bChecksum)[0];	

	
	Step=UpgradeStep;
	retry=0;
	bUpgrading=true;
	while(bUpgrading)
	{
		switch(Step)
		{
			case(USB_UPGRADE_ENTRY_BOOTLOADER): 			
				//Enter Upgrade Mode... AP->Bootloader
				WriteLog("Enter Upgrade Mode...");				
				COMM_FLASH_EnterUpgradeMode();
				SleepMS(300);
				Step=USB_UPGRADE_ENTRY_UPGRADE;
				retry=0; 
				break;
			case(USB_UPGRADE_ENTRY_UPGRADE):
				//MessageBox(NULL, "Step=1", "Upgrade", MB_OK);
				//BootLoader start to upgrade
				ReCode = COMM_FLASH_EnterUpgradeMode();
				if(ReCode == PROGRAM_CODE_OK)
				{		
					SleepMS(100);
					ReCode = COMM_FLASH_CheckCurrentState(&ucMode);
					if(ucMode == 1) //1: Upgrade Mode; 2: FW Mode
					{
						Step=USB_UPGRADE_ERASE_FLASH;
						retry=0;
					}
					else
					{
						SleepMS(200);	
						WriteLog("Get Current State, Times: %d...", retry++);
						retry++;
						if(retry==3)
						{
							WriteLog("Failed to Get Current State! ");
							ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
							Step=USB_UPGRADE_END;
						}			
					}
				}
				else
				{	
					WriteLog("Enter Upgrade Mode..%dS.", retry+1);
					retry++;
					if(retry==3)
					{
						WriteLog("Failed to enter Upgrade Mode! ");
						ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
						Step=USB_UPGRADE_END;
					}					
				}
				break;
			case(USB_UPGRADE_ERASE_FLASH):				
				//Read chip id & Erase Flash
				ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
				if(ReCode == PROGRAM_CODE_OK)
				{
					ReCode = COMM_FLASH_USB_ReadUpdateID(&usIcID);
					if(ReCode != PROGRAM_CODE_OK)
					{
						WriteLog("Read 5452 id error. ");
						ProgramCode = PROGRAM_CODE_CHIP_ID_ERROR;
						Step=USB_UPGRADE_END;
						continue;
					}
					else						
					{
						if(0x542C != usIcID)
						{
							WriteLog("%04X ID is error. ", usIcID);
							ProgramCode = PROGRAM_CODE_CHIP_ID_ERROR;
							Step=USB_UPGRADE_END;
							continue;
						}
					}		
					COMM_FLASH_USB_EraseFlash();			
					SleepMS(1000);
					WriteLog("Erase Time: 1S ");
					Step=USB_UPGRADE_CHECK_ERASE_READY;
					retry=0;
				}
				else
				{	
					SleepMS(50);
					retry++;
					if(retry==3)
					{
						WriteLog("TP is not ready for upgrade ");
						ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
						Step=USB_UPGRADE_END;
					}
				}
				break;
			case(USB_UPGRADE_CHECK_ERASE_READY):				
				//Check Erase is Ready?
				ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
				if(ReCode == PROGRAM_CODE_OK)
				{
					Step=USB_UPGRADE_SEND_DATA;
					retry=0;	
					DataLen = 0;
					SentDataLen=0;
				}
				else
				{					
					SleepMS(500);
					retry++;
					WriteLog("Erase Time: %d.%dS ", (1+retry/2),(5*(retry%2)));
					if(retry==20)
					{
						WriteLog("Erase Flash Error. "); 
						ProgramCode = PROGRAM_CODE_ERASE_FLASH_ERROR;
						Step=USB_UPGRADE_END;
					}
				}
				break;
			case(USB_UPGRADE_SEND_DATA):				
				//Send Packet Data 
				if(SentDataLen < MaxLength)
				{
					if(retry==0)
					{
						DataLen=0;
						memset(data, 0xff, sizeof(data));									
						if(SentDataLen + MAX_USB_PACKET_SIZE_M1 > MaxLength)
						{
							memcpy(data, g_DataBuffer + SentDataLen, MaxLength - SentDataLen);
							DataLen = MaxLength - SentDataLen;
							ucPacketType = END_PACKET;
						}
						else
						{				
							memcpy(data, g_DataBuffer + SentDataLen, MAX_USB_PACKET_SIZE_M1);
							DataLen = MAX_USB_PACKET_SIZE_M1;
							if(SentDataLen)
								ucPacketType = MID_PACKET;
							else
								ucPacketType = FIRST_PACKET;
						}
						ReCode = COMM_FLASH_SendDataByUSB(ucPacketType, data, DataLen); 					
						retry++;
					}

					if(retry>0)
					{	
						SentDataLen += DataLen;
						if (SentDataLen < MaxLength)
						{
							ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
							if (PROGRAM_CODE_OK == ReCode)
							{
								WriteLog("Updating %d bytes...", SentDataLen);
								retry = 0;
							}
							else
							{
								SleepMS(1);
								if (++retry > 20)
								{
									WriteLog("Upgrade failed! ");
									ProgramCode = PROGRAM_CODE_WRITE_FLASH_ERROR;
									COMM_FLASH_ExitUpgradeMode();		//Reset
									Step = USB_UPGRADE_END;
								}

							}
						}												
					}
				}				
				else
				{
					//Write flash End and check ready (fw calculate checksum)
					SleepMS(200); 
					ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
					if(ReCode == PROGRAM_CODE_OK)	
					{			
						Step=USB_UPGRADE_CHECK_SUM;
						retry=0;
					}
					else
					{
						if(++retry > 5)
						{
							WriteLog("Upgrade failed!");
							ProgramCode = PROGRAM_CODE_WRITE_FLASH_ERROR;
							COMM_FLASH_ExitUpgradeMode();		//Reset
							Step=USB_UPGRADE_END;
						}
						
					}
				}
				break;
			case(USB_UPGRADE_CHECK_SUM):			
				ReCode = COMM_FLASH_Checksum_Upgrade(&FWChecksum);
				if(ReCode == PROGRAM_CODE_OK)
				{				
					if(Checksum == FWChecksum)
					{						
						WriteLog("Checksum Right, PC:0x%x, FW:0x%x!", Checksum, FWChecksum);
						Step=USB_UPGRADE_EXIT;
						retry=0;
						COMM_FLASH_ExitUpgradeMode();		//Reset
					}
					else
					{	
						WriteLog("Checksum error, PC:0x%x, FW:0x%x!", Checksum, FWChecksum);						
						SleepMS(500);
						ProgramCode = PROGRAM_CODE_CHECKSUM_ERROR;
						Step=USB_UPGRADE_END;
					}
				}
				else
				{				
					ProgramCode = PROGRAM_CODE_CHECKSUM_ERROR;
					Step=USB_UPGRADE_END;
				}					
				break;
			case(USB_UPGRADE_EXIT): 
				SleepMS(500);				
				ReCode = ftp_ReadReg(0x9F,&data[0]);
				ReCode = ftp_ReadReg(0xA3,&data[1]);
				WriteLog("Exit Upgrade Mode, Times: %d, id1=%x, id2=%x ", retry,data[1],data[0]);				
				if((data[1] == 0x54) && (data[0] == 0x56))
				{					
					Step=USB_UPGRADE_END;
					retry=0;			
					WriteLog("Upgrade is successful! ");
				}
				else
				{
					retry++;
					if(retry==4)
					{
						ProgramCode = PROGRAM_CODE_RESET_SYSTEM_ERROR;
						Step=USB_UPGRADE_END;
					}
				}				
				break;
			case(USB_UPGRADE_END):
				bUpgrading=false;
				break;
		}
		
	}
	return ProgramCode;
}


/**********************************************************
Program_Upgrade8112
***********************************************************/
u8 Program_Upgrade8112(u8 UpgradeStep)
{
	u8 g_DataBuffer[148 * 1024] = {0};
	u8 g_UpgradeBuffer[120 * 1024] = {0};
	u8 ProgramCode = PROGRAM_CODE_OK;
	u8 ReCode;
	u8 ucMode = 0;
	u8 data[64];
	u8 ucPacketType = 0;
	u8 retry;
	u8 Step;
	u8 upgradeloop = 0;
	unsigned int BufferIndex = 0;	
	unsigned int DValue = 0;
	unsigned int Checksum = 0, FWChecksum;
	unsigned int DataLen = 0, SentDataLen = 0;
	unsigned int MaxLength = 0;
	unsigned int MaxPacketSize = 0;
	unsigned int i=0;
	unsigned short usIcID;
	bool bUpgrading = false;

	ReCode = retrieve_data_from_firmware(g_DataBuffer, g_firmware_size);
	Step=UpgradeStep;
	
	for(upgradeloop = 0; upgradeloop < 4; upgradeloop++)
	{

		if(ProgramCode != PROGRAM_CODE_OK)
			return ProgramCode;
		
		if(g_DataBuffer[BufferIndex++] == upgradeloop)
		{
			MaxLength = (g_DataBuffer[BufferIndex] << 24) | (g_DataBuffer[BufferIndex + 1] << 16) | (g_DataBuffer[BufferIndex + 2] << 8) | g_DataBuffer[BufferIndex + 3];
			BufferIndex += 4;
			if(MaxLength == 0)
				continue;				
		}
		else
		{
			return PROGRAM_CODE_CHECK_DATA_ERROR;
		}
		
		memset(g_UpgradeBuffer, 0xff, sizeof(g_UpgradeBuffer));
		memcpy(g_UpgradeBuffer, g_DataBuffer + BufferIndex, MaxLength);
		BufferIndex += MaxLength;
		
		MaxLength = (MaxLength + 3) & ~3; // =  *4 / 4;

		Checksum = 0;

		//Calculate FW checksum...
		for(i=0; i<MaxLength ; i+=4)
		{
			DValue = (g_UpgradeBuffer[i + 3] << 24) + (g_UpgradeBuffer[i + 2] << 16) +(g_UpgradeBuffer[i + 1] << 8) + g_UpgradeBuffer[i];
			Checksum ^= DValue;
		}		
		Checksum += 1;	

		MaxPacketSize = MAX_USB_PACKET_SIZE_M2;
		
		if(upgradeloop > 0)
		{
			if(upgradeloop == 3)
			{
				MaxPacketSize = MAX_USB_PACKET_SIZE_M1;
				COMM_FLASH_ExitUpgradeMode();
				SleepMS(500);
				ftp_WriteReg(0xB7, 0x01);
				SleepMS(50);
				ftp_WriteReg(0x00, 0x00);
				SleepMS(100);	
				Step = USB_UPGRADE_ENTRY_BOOTLOADER;
			}
			else
			{
				Step = USB_UPGRADE_ENTRY_UPGRADE;
			}
		}		
		retry=0;
		bUpgrading=true;
		while(bUpgrading)
		{
			switch(Step)
			{
				case(USB_UPGRADE_ENTRY_BOOTLOADER): 			
					//Enter Upgrade Mode... AP->Bootloader
					WriteLog("Enter Upgrade Mode...");				
					COMM_FLASH_EnterUpgradeMode();
					SleepMS(300);
					Step=USB_UPGRADE_ENTRY_UPGRADE;
					retry=0; 
					break;
				case(USB_UPGRADE_ENTRY_UPGRADE):
					//MessageBox(NULL, "Step=1", "Upgrade", MB_OK);
					//BootLoader start to upgrade
					ReCode = COMM_FLASH_EnterUpgradeMode();
					if(ReCode == PROGRAM_CODE_OK)
					{		
						SleepMS(100);
						ReCode = COMM_FLASH_CheckCurrentState(&ucMode);
						if(ucMode == 1) //1: Upgrade Mode; 2: FW Mode
						{
							Step=USB_UPGRADE_ERASE_FLASH;
							retry=0;
						}
						else
						{
							SleepMS(200);	
							WriteLog("Get Current State, Times: %d...", retry++);
							retry++;
							if(retry==3)
							{
								WriteLog("Failed to Get Current State! ");
								ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
								Step=USB_UPGRADE_END;
							}			
						}
					}
					else
					{	
						WriteLog("Enter Upgrade Mode..%dS.", retry+1);
						retry++;
						if(retry==3)
						{
							WriteLog("Failed to enter Upgrade Mode! ");
							ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
							Step=USB_UPGRADE_END;
						}					
					}
					break;
				case(USB_UPGRADE_ERASE_FLASH):				
					//Read chip id & Erase Flash
					ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
					if(ReCode == PROGRAM_CODE_OK)
					{
						ReCode = COMM_FLASH_USB_ReadUpdateID(&usIcID);
						if(ReCode != PROGRAM_CODE_OK)
						{
							WriteLog("Read 5452 id error. ");
							ProgramCode = PROGRAM_CODE_CHIP_ID_ERROR;
							Step=USB_UPGRADE_END;
							continue;
						}
						else						
						{
							
							if((0x81B2 != usIcID && upgradeloop < 3 ) && (0x36A8 != usIcID && upgradeloop == 3))
							{
								WriteLog("%04X ID is error. ", usIcID);
								ProgramCode = PROGRAM_CODE_CHIP_ID_ERROR;
								Step=USB_UPGRADE_END;
								continue;
							}
						}	

						COMM_FLASH_WriteBinFileLength(upgradeloop, MaxLength);
						
						if(upgradeloop == 3)
							COMM_FLASH_USB_EraseFlash();
						else							
							COMM_FLASH_USB_EraseFlashArea(upgradeloop);
															
						SleepMS(1000);
						WriteLog("Erase Time: 1S ");
						Step=USB_UPGRADE_CHECK_ERASE_READY;
						retry=0;
					}
					else
					{	
						SleepMS(50);
						retry++;
						if(retry==3)
						{
							WriteLog("TP is not ready for upgrade ");
							ProgramCode = PROGRAM_CODE_ENTER_UPGRADE_MODE_ERROR;
							Step=USB_UPGRADE_END;
						}
					}
					break;
				case(USB_UPGRADE_CHECK_ERASE_READY):				
					//Check Erase is Ready?
					ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
					if(ReCode == PROGRAM_CODE_OK)
					{
						Step=USB_UPGRADE_SEND_DATA;
						retry=0;	
						DataLen = 0;
						SentDataLen=0;
					}
					else
					{					
						SleepMS(500);
						retry++;
						WriteLog("Erase Time: %d.%dS ", (1+retry/2),(5*(retry%2)));
						if(retry==20)
						{
							WriteLog("Erase Flash Error. "); 
							ProgramCode = PROGRAM_CODE_ERASE_FLASH_ERROR;
							Step=USB_UPGRADE_END;
						}
					}
					break;
				case(USB_UPGRADE_SEND_DATA):				
					//Send Packet Data 
					if(SentDataLen < MaxLength)
					{
						if(retry==0)
						{
							DataLen=0;
							memset(data, 0xff, sizeof(data));									
							if(SentDataLen + MaxPacketSize > MaxLength)
							{
								memcpy(data, g_UpgradeBuffer + SentDataLen, MaxLength - SentDataLen);
								DataLen = MaxLength - SentDataLen;
								ucPacketType = END_PACKET;
							}
							else
							{				
								memcpy(data, g_UpgradeBuffer + SentDataLen, MaxPacketSize);
								DataLen = MaxPacketSize;
								if(SentDataLen)
									ucPacketType = MID_PACKET;
								else
									ucPacketType = FIRST_PACKET;
							}
							if(upgradeloop == 3)								
								ReCode = COMM_FLASH_SendDataByUSB(ucPacketType, data, DataLen); 					
							else
								ReCode = COMM_FLASH_SendDataAreaByUSB(SentDataLen, data, DataLen);
							retry++;
						}

						if(retry>0)
						{	
							SentDataLen += DataLen;
							if (SentDataLen < MaxLength)
							{
								ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
								if (PROGRAM_CODE_OK == ReCode)
								{
									WriteLog("Updating %d bytes...", SentDataLen);
									retry = 0;
								}
								else
								{
									SleepMS(1);
									if (++retry > 10)
									{
										WriteLog("Upgrade failed! ");
										ProgramCode = PROGRAM_CODE_WRITE_FLASH_ERROR;
										COMM_FLASH_ExitUpgradeMode();		//Reset
										Step = USB_UPGRADE_END;
									}

								}
							}												
						}
					}				
					else
					{
						//Write flash End and check ready (fw calculate checksum)
						SleepMS(200); 
						ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
						if(ReCode == PROGRAM_CODE_OK)	
						{			
							Step=USB_UPGRADE_CHECK_SUM;
							retry=0;
						}
						else
						{
							if(++retry > 5)
							{
								WriteLog("Upgrade failed!");
								ProgramCode = PROGRAM_CODE_WRITE_FLASH_ERROR;
								COMM_FLASH_ExitUpgradeMode();		//Reset
								Step=USB_UPGRADE_END;
							}
							
						}
					}
					break;
				case(USB_UPGRADE_CHECK_SUM):			
					ReCode = COMM_FLASH_Checksum_Upgrade(&FWChecksum);
					if(ReCode == PROGRAM_CODE_OK)
					{
						if(Checksum == FWChecksum)
						{						
							WriteLog("Checksum Right, PC:0x%x, FW:0x%x!", Checksum, FWChecksum);
							Step=USB_UPGRADE_END;
							retry=0;							
							//COMM_FLASH_ExitUpgradeMode();		//Reset
						}
						else
						{	
							WriteLog("Checksum error, PC:0x%x, FW:0x%x!", Checksum, FWChecksum);						
							SleepMS(100);
							ProgramCode = PROGRAM_CODE_CHECKSUM_ERROR;
							Step=USB_UPGRADE_END;
						}
					}
					else
					{				
						ProgramCode = PROGRAM_CODE_CHECKSUM_ERROR;
						Step=USB_UPGRADE_END;
					}					
					break;				
				case(USB_UPGRADE_END):
					bUpgrading=false;
					break;
			}
			
		}
	}

	if(ProgramCode == PROGRAM_CODE_OK)
	{
		COMM_FLASH_ExitUpgradeMode();
		SleepMS(500);
		ReCode = ftp_ReadReg(0x9F,&data[0]);
		ReCode = ftp_ReadReg(0xA3,&data[1]);
		WriteLog("Exit Upgrade Mode: id1=%x, id2=%x ", data[1], data[0]);
		if(((data[1]==0x81) && (data[0]==0x12)) || ((data[1]==0x50) && (data[0]==0x07)))
		{		
			WriteLog("Upgrade is successful! ");
			ftp_ReadReg(0xA6,&data[0]);
			ftp_ReadReg(0xAD,&data[1]);
			WriteLog("%d, %d", data[0], data[1]);
		}
		else
		{			
			ProgramCode = PROGRAM_CODE_RESET_SYSTEM_ERROR;			
		}
	}
	
	return ProgramCode;

}

/**********************************************************
HID_Program_Upgrade: Begin to upgrade
***********************************************************/
u8 HID_Program_Upgrade()
{

	u8 ProgramCode = PROGRAM_CODE_OK;
	u8 ReCode;
	u8 ucMode = 0;
	u16 usIcID = 0;
	u8 data[2] = {0};
	u8 ucStep = 0;
	//1. Read chip id
	ReCode = COMM_FLASH_CheckCurrentState(&ucMode);
	if(ReCode == COMM_HID_OK && ucMode == 1) // bootloader
	{
		ReCode = COMM_FLASH_CheckTPIsReadyForUpgrade();
		if(ReCode == PROGRAM_CODE_OK)
		{
			ReCode = COMM_FLASH_USB_ReadUpdateID(&usIcID);
			ucStep = USB_UPGRADE_ERASE_FLASH;
		}
	}
	else
	{
		ReCode = ftp_ReadReg(0x9F,&data[0]);
		ReCode = ftp_ReadReg(0xA3,&data[1]);
		usIcID = u16(data[1]<<8 | data[0]);
		ucStep = USB_UPGRADE_ENTRY_BOOTLOADER;
	}

	if(ReCode == COMM_HID_OK)
	{
		if(usIcID == 0x5452 || usIcID == 0x545E) //FT3438
		{
			Program_Upgrade5452(ucStep);
		}
		else if(usIcID == 0x5822 || usIcID == 0x582E) //FT3637
		{
			Program_Upgrade5822(ucStep);
		}
		else if(usIcID == 0x5456 || usIcID == 0x5422) //FT3437U
		{
			Program_Upgrade5456(ucStep);
		}
		else if(usIcID == 0x8112 || usIcID == 0x81B2) //FT8112
		{
			Program_Upgrade8112(ucStep);
		}
		
	}
	else
		ProgramCode = PROGRAM_CODE_COMM_ERROR;
	
	
	return ProgramCode;
}


/*******************************************
 get_boot_fw_version_data: Get Boot Version
******************************************/
u8 get_boot_fw_version_data(u16 *p_fw_version)
{

	u8 ReCode = COMM_HID_OK; 
	u8 Ver[2] = {0};	
	
	
	/* Check Data Buffer */
	if(p_fw_version == NULL)
	{
		ERROR_PRINTF("%s: NULL Pointer!\r\n", __func__);
		return COMM_HID_INVLID_PARAM;
	}

	COMM_FLASH_EnterUpgradeMode();
	SleepMS(200);
	ReCode = COMM_FLASH_EnterUpgradeMode();
	
	if(ReCode == COMM_HID_OK)
	{
		ReCode = ftp_ReadReg(0xA6, &Ver[0]);
		ReCode = ftp_ReadReg(0xAD, &Ver[1]);	
		*p_fw_version = (u16)(Ver[0]<<8 | Ver[1]);
	}	
	COMM_FLASH_ExitUpgradeMode();

	return ReCode;	

}



/*******************************************
 check protocol
******************************************/
void auto_check_protocol()
{
	u8 value = 0xff;
	u8 Recode = COMM_HID_OK;

	m_protocol = 1;
	Recode = ftp_ReadReg(0x00, &value);
	if(Recode == COMM_HID_OK && value == 0)
		return;	
  	m_protocol = 0;
}

/**********************************************************
Run_Test: Test For debug
***********************************************************/
void Run_Test(void)
{
	int i;

	for(i=0; i <10; i++)
	{
		printf("enter upgrade mode\r\n");
		COMM_FLASH_EnterUpgradeMode();
		SleepMS(1000);
		if(COMM_FLASH_EnterUpgradeMode()==0)
		{
			SleepMS(1000);
			COMM_FLASH_ExitUpgradeMode();
			printf("exit upgrade mode: %i\r\n", i);
			break;
			
		}
	}	
		
}



