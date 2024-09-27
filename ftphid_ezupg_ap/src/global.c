/********************************************************************* 
 FocalTech HID Forcehpad Upgrade Tool 
 Copyright (c) 2022, FocalTech Systems Co.,Ltd.
 Author: Wayne  <shihwei.huang@focaltech-electronics.com>
*********************************************************************/ 



/********************************************************************* 

global.cpp 
global variant

***********************************************************************/

#include "global.h"

//////////////////////////////////////////////////////////////////////
// Definitions
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Global Variable
//////////////////////////////////////////////////////////////////////
bool g_bEnableDebug = false;
bool g_bEnableOutputBufferDebug = false;
bool g_bEnableErrorMsg = false;
// Firmware File
int g_firmware_fd;
int g_firmware_size;

int m_nHidrawFd = -1;  
// Assign initial values to chip data
u16 m_usVID = 0;
u16 m_usPID = 0;
u16 m_usVersion = 0;
// Assign Initial values to buffers
u8* m_inBuf = NULL;
u8* m_outBuf = NULL;
u8 m_inBufSize = 0;
u8 m_outBufSize = 0;
sem_t m_ioMutex;
u8 m_szOutputBuf[64];    // Command Raw Buffer
u8 m_szInputBuf[64]; // Data Raw Buffer


/*******************************************
 *  Var Memory initialize
 ******************************************/
void global_init()
{
	m_nHidrawFd = -1;  
    // Assign initial values to chip data
    m_usVID = 0;
    m_usPID = 0;
    m_usVersion = 0;
    // Assign Initial values to buffers
    m_inBuf = NULL;
    m_inBufSize = 0;
    m_outBuf = NULL;
    m_outBufSize = 0;

    // Initialize mutex
    sem_init(&m_ioMutex, 0 /*scope is in this file*/, 1 /*active in initial*/);
    // Allocate memory to inBuffer
    m_inBufSize = FTP_IICHID_INPUT_BUFFER_SIZE;
    m_inBuf = (unsigned char*)malloc(sizeof(unsigned char) * m_inBufSize);
    memset(m_inBuf, 0, sizeof(unsigned char)*m_inBufSize);

    // Allocate memory to outBuffer
    m_outBufSize = FTP_IICHID_OUTPUT_BUFFER_SIZE;    
    m_outBuf = (unsigned char*)malloc(sizeof(unsigned char) * m_outBufSize);
    memset(m_outBuf, 0, sizeof(unsigned char)*m_outBufSize);
}

/*******************************************
 *  Var Memory free
 ******************************************/
void global_free(void)
{
    // Deinitialize mutex (semaphore)
    sem_destroy(&m_ioMutex);

    // Release input buffer
    if (m_inBuf)
    {
        free(m_inBuf);
        m_inBuf		= NULL;
        m_inBufSize     = 0;
    }

    // Release output buffer
    if (m_outBuf)
    {
        free(m_outBuf);
        m_outBuf        = NULL;
        m_outBufSize    = 0;
    }

    // Clear Chip Data
    m_usVID = 0;
    m_usPID = 0;
    m_usVersion = 0;

    return;
}

/*******************************************
Close Hid raw handle
*******************************************/
void CloseDevice(void)
{
    if (m_nHidrawFd >= 0)
    {
        // Release acquired hidraw device handler
        close(m_nHidrawFd);
        m_nHidrawFd = -1;
    }   
}

/**************************************************************************
FindHidrawDevice()
Find hidraw device name with specific VID and PID, such as /dev/hidraw0
**************************************************************************/
u8 FindHidrawDevice(int nVID, int nPID, char *pszDevicePath)
{

    u8 ReCode = COMM_HID_OK;
    int nError = 0;
    int nFd = 0;
    bool bFound = false;
    DIR *pDirectory = NULL;
    struct dirent *pDirEntry = NULL;
    const char *pszPath = "/dev";
	char szName[50];
    char szFile[64] = {0};	
    struct hidraw_devinfo info;

    // Check if filename ptr is valid
    if (pszDevicePath == NULL)
    {
        //ERR("%s: NULL Device Path Buffer!", __func__);        
        return COMM_HID_INVLID_PARAM;       
    }

    // Open Directory
    pDirectory = opendir(pszPath);
    if (pDirectory == NULL)
    {
        //ERR("%s: Fail to Open Directory %s.\r\n", __func__, pszPath);
        return COMM_HID_INVLID_DEVICE;
    }

    // Traverse Directory Elements
    while ((pDirEntry = readdir(pDirectory)) != NULL)
    {
        // Only reserve hidraw devices
        if (strncmp(pDirEntry->d_name, "hidraw", 6))
            continue;
        memset(szName, '\0', sizeof(szName));
		memset(szFile, '\0', sizeof(szFile));		
		strncpy(szName, pDirEntry->d_name, sizeof(szName));
        snprintf(szFile, sizeof(szFile), "%s/%s", pszPath, szName);
        /* Open the Device with non-blocking reads. In real life,
          don't use a hard coded path; use libudev instead. */
        nError = open(szFile, O_RDWR | O_NONBLOCK);        
        if (nError < 0)
        {
            continue;
        }
        nFd = nError;

        /* Get Raw Info */
        nError = ioctl(nFd, HIDIOCGRAWINFO, &info);
        if (nError >= 0)
        {
           
            if ((info.vendor == nVID) && (info.product == nPID))
            {
		//		m_usVID = (unsigned short) nVID;
		//		m_usPID = (unsigned short) nPID;
                memcpy(pszDevicePath, szFile, sizeof(szFile));
                bFound = true;
            }
        }

        // Close Device
        close(nFd);
        
        // Stop the loop if found
        if (bFound == true)
            break;
    }

    if (!bFound)
        ReCode = COMM_HID_FIND_NO_DEVICE;

    // Close Directory
    closedir(pDirectory);
    return ReCode;
}

/**************************************************
GetDeviceHandle(): 
Get device handle with specific VID and PID
***************************************************/
u8 GetDeviceHandle(int nVID, int nPID)
{

    u8 ReCode = COMM_HID_OK;
    u32 handle = 0;
    char szHidrawDevPath[64] = {0};

         
    ReCode = FindHidrawDevice(nVID, nPID, szHidrawDevPath);
	if(ReCode == COMM_HID_FIND_NO_DEVICE)
	{
		return ReCode;		
	}

    // Acquire hidraw device handler for I/O
    handle = open(szHidrawDevPath, O_RDWR | O_NONBLOCK);
    if (handle < 0)
    {
        WriteLog("%s: Fail to Open Device %s! errno=%d.", __func__, szHidrawDevPath, handle);
        return COMM_HID_FIND_NO_DEVICE;
    }

    // Success
    m_nHidrawFd = handle;

    return ReCode;
}

/**************************************************
SendData(): Send data to device 
***************************************************/
u8 SendData(unsigned char* sBuf, int sLen)
{

    u8 ReCode = COMM_HID_OK;
	u16 i = 0;
	u32 nResult = 0;
	
    if ((unsigned)sLen > m_outBufSize)
    {
        ReCode = COMM_HID_INVALID_BUFFER_SIZE;        
//        ERR("%s: Send data length > defined size (sLen=%d, defined size=%d), ret=%d.\r\n", __func__, sLen, m_outBufSize, ReCode );        
        return ReCode;
    }
    // Mutex locks the critical section
    sem_wait(&m_ioMutex);
    // Copy data to local buffer and write buffer data to usb
    memset(m_outBuf, 0xff, m_outBufSize);    
    memcpy(m_outBuf, sBuf, sLen);

    // Send Buffer Data to hidraw device    
	nResult = ioctl(m_nHidrawFd, HIDIOCSFEATURE(FTP_IICHID_OUTPUT_BUFFER_SIZE), m_outBuf);

	if (nResult < 0)
	{
	    ReCode = COMM_HID_WRITE_USB_ERROR;
		printf("error read HIDIOCSOUTPUT");    
    }   
        
    // Mutex unlocks the critical section
    sem_post(&m_ioMutex);

    return ReCode;
}

/**************************************************
ReadData(): Read data for device
***************************************************/
u8 ReadData(unsigned char* rBuf, int rLen)
{    

	u8 ReCode = COMM_HID_OK;
	u16 i = 0;
	u32 nResult = 0;
	
    nResult = 0;

    // Mutex locks the critical section
    sem_wait(&m_ioMutex);   

	m_inBuf[0] = 0x06; /* Report Number */	
	nResult = ioctl(m_nHidrawFd, HIDIOCGFEATURE(FTP_IICHID_INPUT_BUFFER_SIZE), m_inBuf);
	
	if (nResult < 0) {
		printf("error HIDIOCGFEATURE");
		ReCode = COMM_HID_READ_USB_ERROR;
	}
	else
	{
	    memcpy(rBuf, m_inBuf, rLen);
	}

    // Mutex unlocks the critical section
    sem_post(&m_ioMutex);

    return ReCode;
}

/**************************************************
GetDevVidPid(): Get current device VID,PID
***************************************************/
u8 GetDevVidPid(unsigned int* p_nVid, unsigned int* p_nPid)
{

	u8 ReCode = COMM_HID_OK;

	// Make Sure Input Pointers Valid
	if((p_nVid == NULL) || (p_nPid == NULL))
	{
		//ERR("%s: Input Parameters Invalid! (p_nVid=%p, p_nPid=%p)\r\n", __func__, p_nVid, p_nPid);
		return COMM_HID_INVLID_PARAM;
	}

	// Make Sure Device Found
	if((m_usVID == 0) && (m_usPID == 0))
	{
		//ERR("%s: I2C-HID device has never been found!\r\n", __func__);
		return COMM_HID_INVLID_DEVICE;
	}

	// Set PID & VID
	*p_nVid = m_usVID;
	*p_nPid = m_usPID;

	return ReCode;
}


/**************************************************
WriteLog():
save log for recording process of upgrade 
***************************************************/
void WriteLog(const char *pszFormat, ...)
{

#if (_SAVE_LOG_ == 0)
	return;
#endif
    time_t m_tRawTime;
    struct timeval m_tvCurTime;
    char m_szDateBuffer[80];
    char m_szDateTimeBuffer[88];
    FILE *fd;
    char szLogBuffer[LOG_BUF_SIZE] = {0};
    va_list pArgs;
  
    // Make Sure Log String Valid
    if (pszFormat == NULL){
      printf("Log String Valid\r\n");
        return;
    }

    // Make Sure Log File Exist
    fd = fopen(DEFAULT_LOG_FILENAME, "a+");
    if (fd == NULL)
	{
		printf("%s: Fail to open \"%s\"! (errno=%d)\r\n", __func__, DEFAULT_LOG_FILENAME, errno);
               return;
	}
    // Clear Buffer
    memset(m_szDateBuffer, 0, sizeof(m_szDateBuffer));
    memset(m_szDateTimeBuffer, 0, sizeof(m_szDateTimeBuffer));
    // Get Current Time
    time(&m_tRawTime);
    strftime(m_szDateBuffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&m_tRawTime));
    gettimeofday(&m_tvCurTime, NULL);
    sprintf(m_szDateTimeBuffer, "%s:%03d:%03d", m_szDateBuffer, (int)m_tvCurTime.tv_usec / 1000, (int)m_tvCurTime.tv_usec % 1000);
    // Load String to LogBuffer with Variable Argument List
    va_start(pArgs, pszFormat);
    vsnprintf(szLogBuffer, sizeof(szLogBuffer), pszFormat, pArgs);
    va_end(pArgs);
    //Write data to file
    fprintf(fd, "%s [DEBUG] %s\n", m_szDateTimeBuffer, szLogBuffer);
    fclose(fd);
    return;
}

