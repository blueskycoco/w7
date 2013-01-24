//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
// Copyright (c) Samsung Electronics. Co. LTD.  All rights reserved.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

*/
/*****************************************************************************/
/*                                                                           */
/* PROJECT : PocketStoreII v1.0.0_build001                                   */
/* FILE    : SYSTEM.c                                                        */
/* PURPOSE : This file implements Windows CE Block device driver interface   */
/*          for supporting BIN file system.                                  */
/*                                                                           */
/*****************************************************************************/

#include <windows.h>
#include <bldver.h>
#include <windev.h>
#include <types.h>
#include <excpt.h>
#include <tchar.h>
#include <devload.h>
#include <diskio.h>
#include <storemgr.h>
#include <pm.h>

#include <hsmmcdrv.h>
#include <bsp.h>
#include <ceddk.h>
#include <IROM_SDMMC_loader_cfg.h>

/*****************************************************************************/
/* Debug Definitions                                                         */
/*****************************************************************************/
#define BIBDRV_RTL_PRINT(x)        RETAILMSG(1,x)

#define BIBDRV_INF_MSG_ON 0
#define BIBDRV_ERR_MSG_ON 1
#define BIBDRV_LOG_MSG_ON 0

#if BIBDRV_ERR_MSG_ON
#define BIBDRV_ERR_PRINT(x)        BIBDRV_RTL_PRINT(x)
#else
#define BIBDRV_ERR_PRINT(x)
#endif /* #if BIBDRV_ERR_MSG_ON */

#if BIBDRV_LOG_MSG_ON
#define BIBDRV_LOG_PRINT(x)        BIBDRV_RTL_PRINT(x)
#else
#define BIBDRV_LOG_PRINT(x)
#endif  /* #if BIBDRV_LOG_MSG_ON */

#if BIBDRV_INF_MSG_ON
#define BIBDRV_INF_PRINT(x)        BIBDRV_RTL_PRINT(x)
#else
#define BIBDRV_INF_PRINT(x)
#endif  /* #if BIBDRV_INF_MSG_ON */

#define IOCTL_moviNAND_READ_CMD    CTL_CODE(FILE_DEVICE_HAL, 4070, METHOD_BUFFERED, FILE_ANY_ACCESS)
/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/* Debug Zones.
 */
#ifdef DEBUG

    #define DBG_INIT        0x0001
    #define DBG_OPEN        0x0002
    #define DBG_READ        0x0004
    #define DBG_WRITE       0x0008
    #define DBG_CLOSE       0x0010
    #define DBG_IOCTL       0x0020
    #define DBG_THREAD      0x0040
    #define DBG_EVENTS      0x0080
    #define DBG_CRITSEC     0x0100
    #define DBG_FLOW        0x0200
    #define DBG_IR          0x0400
    #define DBG_NOTHING     0x0800
    #define DBG_ALLOC       0x1000
    #define DBG_FUNCTION    0x2000
    #define DBG_WARNING     0x4000
    #define DBG_ERROR       0x8000

DBGPARAM dpCurSettings = {
    TEXT("Serial"), { TEXT("Init"),
                      TEXT("Open"),
                      TEXT("Read"),
                      TEXT("Write"),
                      TEXT("Close"),
                      TEXT("Ioctl"),
                      TEXT("Error")},
    0
};
#endif


/*****************************************************************************/
/* Imported variable declarations                                            */
/*****************************************************************************/


/*****************************************************************************/
/* Imported function declarations                                            */
/*****************************************************************************/


/*****************************************************************************/
/* Local #define                                                             */
/*****************************************************************************/
#define MAX_READ_WRITE_SECTOR 512


#define USE_DMA 0

/*****************************************************************************/
// Local constant definitions
/*****************************************************************************/

/*****************************************************************************/
// Local typedefs
/*****************************************************************************/

typedef struct _DISK
{
    struct _DISK       *pd_next;
    CRITICAL_SECTION    d_DiskCardCrit; // guard access to global state and card
    HANDLE              hDevice;        // activate Handle
    DISK_INFO           d_DiskInfo;     // for DISK_IOCTL_GET/SETINFO
    DWORD               d_OpenCount;    // open ref count
    LPWSTR              d_ActivePath;   // registry path to active key for this device
} DISK, *PDISK;

/*****************************************************************************/
// Local variables
/*****************************************************************************/

static CRITICAL_SECTION gDiskCrit;
static PDISK gDiskList;             // initialized to 0 in bss
HANDLE g_hMutex;// to get mutex

#if USE_DMA
PHYSICAL_ADDRESS    g_PhyDMABufferAddr;
PVOID g_pVirtDMABufferAddr;
#endif


/*****************************************************************************/
// Local function prototypes
/*****************************************************************************/
static HKEY OpenDriverKey(LPTSTR ActiveKey);
static BOOL GetFolderName(PDISK pDisk, LPWSTR FolderName, DWORD cBytes, DWORD *pcBytes);
static BOOL GetFSDName(PDISK pDisk, LPWSTR FSDName, DWORD cBytes, DWORD *pcBytes);
static BOOL GetDeviceInfo(PDISK pDisk, PSTORAGEDEVICEINFO psdi);

static VOID  CloseDisk           (PDISK pDisk);

static DWORD DoDiskRead          (PDISK pDisk, PVOID pData);
static DWORD GetDiskInfo         (PDISK pDisk, PDISK_INFO pInfo);
static DWORD SetDiskInfo         (PDISK pDisk, PDISK_INFO pInfo);
static PDISK CreateDiskObject    (VOID);
static BOOL  IsValidDisk         (PDISK pDisk);
static BOOL  InitializeNAND      (PDISK pDisk);
static BOOL  InitDisk            (PDISK pDisk, LPTSTR ActiveKey);

extern HANDLE InitMutex(LPCTSTR name);
extern DWORD GetMutex(HANDLE handle);

/*****************************************************************************/
// Extern variables
/*****************************************************************************/
extern UINT32 g_dwSectorCount;
extern UINT32 g_dwIsSDHC;

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      OpenDriverKey                                                        */
/* DESCRIPTION                                                               */
/*      This function opens the driver key specified by the active key       */
/* PARAMETERS                                                                */
/*      ActiveKey       Handle to a currently open key or any of the         */
/*                      following predefined reserved handle values          */
/* RETURN VALUES                                                             */
/*      Return values is HKEY value of "[ActiveKey]\[Key]", The caller is    */
/*      responsible for closing the returned HKEY                            */
/*                                                                           */
/*****************************************************************************/
static HKEY
OpenDriverKey(LPTSTR ActiveKey)
{
    TCHAR   DevKey[256];
    HKEY    hDevKey;
    HKEY    hActive;
    DWORD   ValType;
    DWORD   ValLen;
    DWORD   status;

    //
    // Get the device key from active device registry key
    //
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,   /* Handle to a currently open key       */
                          ActiveKey,            /* Pointer to subkey                    */
                          0,                    /* Option : Reserved - set to 0         */
                          0,                    /* samDesired : Not supported - set to 0 */
                          &hActive);            /* Pointer for receved handele          */

    if (ERROR_SUCCESS != status)
    {
        BIBDRV_RTL_PRINT((TEXT("BIBDRV_PS:OpenDriverKey RegOpenKeyEx(HLM\\%s) returned %d!!!\r\n"),
                        ActiveKey, status));
        return NULL;
    }

    hDevKey = NULL;
    ValLen  = sizeof(DevKey);

    status = RegQueryValueEx(hActive,               /* Handle to a currently open key   */
                             DEVLOAD_DEVKEY_VALNAME,/* Pointer to quary                 */
                             NULL,                  /* Reserved - set to NULL           */
                             &ValType,              /* Pointer to type of data          */
                             (PUCHAR)DevKey,        /* Pointer to data                  */
                             &ValLen);              /* the Length of data               */

    if (ERROR_SUCCESS != status)
    {
        BIBDRV_RTL_PRINT((TEXT("BIBDRV_PS:OpenDriverKey - RegQueryValueEx(%s) returned %d\r\n"),
                        DEVLOAD_DEVKEY_VALNAME, status));

        RegCloseKey(hActive);
        return hDevKey;
    }

    //
    // Get the geometry values from the device key
    //
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,   /* Handle to a currently open key       */
                          DevKey,               /* Pointer to subkey                    */
                          0,                    /* Option : Reserved - set to 0         */
                          0,                    /* samDesired : Not supported - set to 0 */
                          &hDevKey);            /* Pointer for receved handele          */

    if (ERROR_SUCCESS != status)
    {
        hDevKey = NULL;
        BIBDRV_RTL_PRINT((TEXT("BIBDRV_PS:OpenDriverKey RegOpenKeyEx - DevKey(HLM\\%s) returned %d!!!\r\n"),
                        DevKey, status));
    }

    RegCloseKey(hActive);

    return hDevKey;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      GetFolderName                                                        */
/* DESCRIPTION                                                               */
/*      Function to retrieve the folder name value from the driver key       */
/*      The folder name is used by File System Driver to name this disk volume*/
/* PARAMETERS                                                                */
/*      pDisk       BIBDRV_PS driver own structure pointer                    */
/*      FolderName
/*      cBytes
/*      pcBytes
/* RETURN VALUES                                                             */
/*      If it successes, it returns TRUE. otherwize it returns FALSE         */
/*                                                                           */
/*****************************************************************************/
static BOOL
GetFolderName(PDISK     pDisk,
              LPWSTR    FolderName,
              DWORD     cBytes,
              DWORD    *pcBytes)
{
    HKEY    DriverKey;
    DWORD   ValType;
    DWORD   status;

    DriverKey = OpenDriverKey(pDisk->d_ActivePath);

    if (NULL != DriverKey)
    {
        *pcBytes = cBytes;
        status = RegQueryValueEx(DriverKey,         /* Handle to a currently open key   */
                                 TEXT("Folder"),    /* Pointer to quary                 */
                                 NULL,              /* Reserved - set to NULL           */
                                 &ValType,          /* Pointer to type of data          */
                                 (PUCHAR)FolderName,/* Pointer to data                  */
                                 pcBytes);          /* the Length of data               */

        if (ERROR_SUCCESS != status)
        {
            BIBDRV_RTL_PRINT((TEXT("BIBDRV_PS:GetFolderName - RegQueryValueEx(Folder) returned %d\r\n"),
                            status));

            *pcBytes = 0;
        }
        else
        {
            BIBDRV_RTL_PRINT((TEXT("BIBDRV_PS:GetFolderName - FolderName = %s, length = %d\r\n"),
                            FolderName, *pcBytes));

            *pcBytes += sizeof(WCHAR); // account for terminating 0.
        }

        RegCloseKey(DriverKey);

        if ((ERROR_SUCCESS != status) || (0 == *pcBytes))
        {
            return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      GetFSDName                                                           */
/* DESCRIPTION                                                               */
/*      Function to retrieve the FSD(file system driver) value from the      */
/*      driver key. The FSD is used to load file system driver               */
/* PARAMETERS                                                                */
/*      pDisk       BIBDRV_PS driver own structure pointer                    */
/*      FSDName
/*      cBytes
/*      pcBytes
/* RETURN VALUES                                                             */
/*      If it successes, it returns TRUE. otherwize it returns FALSE         */
/*                                                                           */
/*****************************************************************************/
static BOOL
GetFSDName(PDISK    pDisk,
           LPWSTR   FSDName,
           DWORD    cBytes,
           DWORD   *pcBytes)
{
    HKEY DriverKey;
    DWORD ValType;
    DWORD status;

    DriverKey = OpenDriverKey(pDisk->d_ActivePath);

    if (NULL != DriverKey)
    {
        *pcBytes = cBytes;
        status = RegQueryValueEx(DriverKey,         /* Handle to a currently open key   */
                                 TEXT("FSD"),       /* Pointer to quary                 */
                                 NULL,              /* Reserved - set to NULL           */
                                 &ValType,          /* Pointer to type of data          */
                                 (PUCHAR)FSDName,   /* Pointer to data                  */
                                 pcBytes);          /* the Length of data               */

        if (ERROR_SUCCESS != status)
        {
            BIBDRV_RTL_PRINT((TEXT("BIBDRV_PS:GetFSDName - RegQueryValueEx(FSD) returned %d\r\n"),
                            status));
            *pcBytes = 0;
        }
        else
        {
            BIBDRV_RTL_PRINT((TEXT("BIBDRV_PS:GetFSDName - FSDName = %s, length = %d\r\n"),
                            FSDName, *pcBytes));

            *pcBytes += sizeof(WCHAR); // account for terminating 0.
        }

        RegCloseKey(DriverKey);

        if ((ERROR_SUCCESS != status) || (0 == *pcBytes))
        {
            return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}

static BOOL
GetDeviceInfo(PDISK                 pDisk,
              PSTORAGEDEVICEINFO    psdi)
{
    HKEY DriverKey;
    DWORD ValType;
    DWORD status;
    DWORD dwSize;

    DriverKey = OpenDriverKey(pDisk->d_ActivePath);

    if (DriverKey)
    {
        dwSize = sizeof(psdi->szProfile);
        status = RegQueryValueEx(DriverKey,                 /* Handle to a currently open key   */
                                 TEXT("Profile"),           /* Pointer to quary                 */
                                 NULL,                      /* Reserved - set to NULL           */
                                 &ValType,                  /* Pointer to type of data          */
                                 (LPBYTE)psdi->szProfile,   /* Pointer to data                  */
                                 &dwSize);                  /* the Length of data               */

        if ((status != ERROR_SUCCESS) || (dwSize > sizeof(psdi->szProfile)))
        {
            BIBDRV_RTL_PRINT((TEXT("BIBDRV_PS:GetFolderName - RegQueryValueEx(Profile) returned %d\r\n"),
                            status));
            wcscpy_s( psdi->szProfile, _countof(psdi->szProfile), L"Default");
        }
        else
        {
            BIBDRV_RTL_PRINT((TEXT("BIBDRV_PS:GetProfileName - Profile = %s, length = %d\r\n"),
                            psdi->szProfile, dwSize));
        }
        RegCloseKey(DriverKey);
    }

    psdi->cbSize        = sizeof(STORAGEDEVICEINFO);
    psdi->dwDeviceClass = STORAGE_DEVICE_CLASS_BLOCK;
    psdi->dwDeviceType  = STORAGE_DEVICE_TYPE_ATA;
    psdi->dwDeviceFlags = STORAGE_DEVICE_FLAG_READWRITE;//STORAGE_DEVICE_FLAG_READONLY; //STORAGE_DEVICE_FLAG_READWRITE;

    return TRUE;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      CloseDisk                                                            */
/* DESCRIPTION                                                               */
/*      free all resources associated with the specified disk                */
/* PARAMETERS                                                                */
/*      pDisk       BIBDRV_PS driver own structure pointer                    */
/* RETURN VALUES                                                             */
/*      none                                                                 */
/*                                                                           */
/*****************************************************************************/
static VOID
CloseDisk(PDISK pDisk)
{
    PDISK pd;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++CloseDisk() pDisk=0x%x\r\n"), pDisk));

    //
    // Remove it from the global list of disks
    //
    EnterCriticalSection(&gDiskCrit);

    if (pDisk == gDiskList)
    {
        gDiskList = pDisk->pd_next;
    }
    else
    {
        pd = gDiskList;
        while (pd->pd_next != NULL)
        {
            if (pd->pd_next == pDisk)
            {
                pd->pd_next = pDisk->pd_next;
                break;
            }
            pd = pd->pd_next;
        }
    }

    LeaveCriticalSection(&gDiskCrit);

    //
    // Try to ensure this is the only thread holding the disk crit sec
    //
    Sleep(50);
    EnterCriticalSection    (&(pDisk->d_DiskCardCrit));
    LeaveCriticalSection    (&(pDisk->d_DiskCardCrit));
    DeleteCriticalSection   (&(pDisk->d_DiskCardCrit));

    if (pDisk->d_ActivePath)
    {
        LocalFree(pDisk->d_ActivePath);
    }
    LocalFree(pDisk);

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --CloseDisk() pDisk=0x%x\r\n"), pDisk));
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DoDiskRead                                                           */
/* DESCRIPTION                                                               */
/*      Do read operation from NAND flash memory                             */
/* PARAMETERS                                                                */
/*      pDisk       BIBDRV_PS driver own structure pointer                    */
/*      pData       PSQ_REQ structure pointer,it contains request information*/
/*                  for read operations                                      */
/* RETURN VALUES                                                             */
/*      If it successes, it returns TRUE. otherwize it returns FALSE         */
/*                                                                           */
/*****************************************************************************/
static DWORD
DoDiskRead(PDISK pDisk,
           PVOID pData)
{
    DWORD   status = ERROR_SUCCESS;
    DWORD   num_sg;
    DWORD   bytes_this_sg;
    PSG_REQ pSgr;
    PSG_BUF pSg;
    PUCHAR  pBuf;
    UINT    nSecCount;
    UINT32  nCount;
    static UINT nAccessCnt = 0;
    PUCHAR  tempBuf;
    UINT32  i;
    DWORD     dwStartSector;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++DoDiskRead()\r\n")));

    pSgr = (PSG_REQ) pData;

    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  ========= DoDiskRead Request Info ========= \r\n")));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpSgr->sr_start    = %d(0x%x)\r\n"), pSgr->sr_start, pSgr->sr_start));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpSgr->sr_num_sec  = %d\r\n"),       pSgr->sr_num_sec));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpSgr->sr_num_sg   = %d\r\n"),       pSgr->sr_num_sg));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tlast sector num   = %d\r\n"),       pSgr->sr_start + pSgr->sr_num_sec - 1));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpSgr->sr_status   = 0x%x\r\n"),     pSgr->sr_status));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  =========================================== \r\n")));

    /* Scatter/Gather buffer Bound Check */
    if (pSgr->sr_num_sg > MAX_SG_BUF)
    {
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  Scatter/Gather buffer Bound Check Fail (Too many buffers)\r\n")));
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }



    status          = ERROR_SUCCESS;
    num_sg          = pSgr->sr_num_sg;
    pSg             = &(pSgr->sr_sglist[0]);

    if (num_sg>1)
    {
        bytes_this_sg = 0 ;

        for ( i=0; i<num_sg; i++ )
            bytes_this_sg += (pSgr->sr_sglist[i]).sb_len;

        nSecCount = ((bytes_this_sg - 1) / pDisk->d_DiskInfo.di_bytes_per_sect) + 1;
    }
    else if ( num_sg == 1)
    {
        bytes_this_sg   = pSg->sb_len;
        pBuf            = (LPVOID)pSg->sb_buf;
        nSecCount = ((bytes_this_sg - 1) / pDisk->d_DiskInfo.di_bytes_per_sect) + 1;
    }

    if ((pSgr->sr_start + pSgr->sr_num_sec - 1) > pDisk->d_DiskInfo.di_total_sectors)
    {
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  DoDiskRead::Request Sector OOB Check Fail(sector exceeded)\r\n")));
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  - Disk Totol Sectors        = 0x%xd\r\n"), pDisk->d_DiskInfo.di_total_sectors+MBRSTARTSECTOR));
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  - Requested pSgr->sr_start  = 0x%x\r\n"), pSgr->sr_start));
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  - Requested pSgr->sr_num_sec= 0x%x\r\n"), pSgr->sr_num_sec));
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  - Requested last sector num = 0x%x\r\n"), pSgr->sr_start + pSgr->sr_num_sec - 1));
       status = ERROR_INVALID_PARAMETER;

        goto ddi_exit;
    }
    if ( num_sg >1)
    {
        pBuf = (PUCHAR)malloc(nSecCount*512);
    }

    if ( bytes_this_sg == 0 )
    {
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  WRITE::Buffer length is 0.\r\n")));
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }


    dwStartSector   = (pSgr->sr_start+MBRSTARTSECTOR);

    /*--------------------------*/
    /* Read sectors from disk.  */
    /*--------------------------*/
    do
    {

        BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  ----------------------------------- \r\n")));
        BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tnum_sg        = %d\r\n"),     num_sg));
        BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpSg           = 0x%08x\r\n"), pSg));
        BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tbytes_this_sg = %d\r\n"),     bytes_this_sg));
        BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpSg->sb_buf   = 0x%08x\r\n"), pSg->sb_buf));
        BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpBuf          = 0x%08x\r\n"), pBuf));
        BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  ----------------------------------- \r\n")));

        BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF] DoDiskRead StartSector=%d, nNumOfScts=%d, bytes_this_sg=%d\r\n"),
                        pSgr->sr_start, nSecCount, bytes_this_sg));


        //BIBDRV_RTL_PRINT((TEXT("### DoDiskRead StartSector = 0x%x, nSeccount = 0x%x pBuf = 0x%x\r\n"),
        //                        pSgr->sr_start, nSecCount, (UINT32)pBuf));


        if (nSecCount <= MAX_READ_WRITE_SECTOR)
        {
            status = ERROR_SUCCESS;
            GetMutex(g_hMutex);
#if USE_DMA
            while (SDHC_READ_DMA(dwStartSector, nSecCount, (UINT32)g_PhyDMABufferAddr.LowPart) == FALSE)
#else
            while (SDHC_READ(dwStartSector, nSecCount, (UINT32)pBuf) == FALSE)
#endif
            {
                if ( status == ERROR_SUCCESS )
                {
                    status = ERROR_INVALID_PARAMETER;
                    while(!SDHC_INIT())
                    {
                    }
                }
                else
                {
                    ReleaseMutex(g_hMutex);
                    goto ddi_exit;
                }
            }


            ReleaseMutex(g_hMutex);

#if USE_DMA
            memcpy(pBuf, g_pVirtDMABufferAddr, bytes_this_sg);
#endif

            status = ERROR_SUCCESS;


        }


        else
        {
#if USE_DMA
            tempBuf = pBuf; // to copy data from DMA buffer repeatedly.
#endif
            BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  trans sec count is bigger than MAX\r\n")));
            for ( nCount = 0; nSecCount >= MAX_READ_WRITE_SECTOR ; nCount++, nSecCount -= MAX_READ_WRITE_SECTOR )
            {
                //RETAILMSG(1,(TEXT("nCount = %d nSecCount = %d\n"),nCount,nSecCount));
                status = ERROR_SUCCESS;
                GetMutex(g_hMutex);
#if USE_DMA
                while (SDHC_READ_DMA(dwStartSector+nCount*MAX_READ_WRITE_SECTOR, MAX_READ_WRITE_SECTOR, (UINT32)g_PhyDMABufferAddr.LowPart) == FALSE)
#else
                while (SDHC_READ(dwStartSector+nCount*MAX_READ_WRITE_SECTOR, MAX_READ_WRITE_SECTOR, (UINT32)pBuf+(UINT32)(nCount*MAX_READ_WRITE_SECTOR*512)) == FALSE)
#endif
                {
                    if ( status == ERROR_SUCCESS )
                    {
                        status = ERROR_INVALID_PARAMETER;
                        while(!SDHC_INIT())
                        {
                        }
                    }
                    else
                    {
                        ReleaseMutex(g_hMutex);
                        goto ddi_exit;
                    }
                }
#if USE_DMA
                memcpy(tempBuf, g_pVirtDMABufferAddr, MAX_READ_WRITE_SECTOR*512);
                (UINT32)(tempBuf) = (UINT32)(tempBuf) + (UINT32)(MAX_READ_WRITE_SECTOR*512);
#endif

                ReleaseMutex(g_hMutex);
            }
            if ( nSecCount != 0 )
            {
                status = ERROR_SUCCESS;
                GetMutex(g_hMutex);
#if USE_DMA
                while (SDHC_READ_DMA(dwStartSector+nCount*MAX_READ_WRITE_SECTOR, nSecCount, (UINT32)g_PhyDMABufferAddr.LowPart) == FALSE)
#else
                while (SDHC_READ(dwStartSector+nCount*MAX_READ_WRITE_SECTOR, nSecCount, (UINT32)pBuf+(UINT32)(nCount*MAX_READ_WRITE_SECTOR*512)) == FALSE)
#endif
                {
                    if ( status == ERROR_SUCCESS )
                    {
                        status = ERROR_INVALID_PARAMETER;
                        while(!SDHC_INIT())
                        {
                        }
                    }
                    else
                    {
                        if ( num_sg > 1)
                        {

                        }
                        ReleaseMutex(g_hMutex);
                        goto ddi_exit;
                    }
                }
#if USE_DMA
                memcpy(tempBuf, g_pVirtDMABufferAddr, nSecCount*512);
#endif
                ReleaseMutex(g_hMutex);
            }

            status = ERROR_SUCCESS;
        }

        if ( num_sg > 1)
        {
            tempBuf = pBuf;
            for ( i=0; i<num_sg; i++ )
            {

                memcpy((LPVOID)((pSgr->sr_sglist[i]).sb_buf), tempBuf,
                    (pSgr->sr_sglist[i]).sb_len);
                tempBuf += (pSgr->sr_sglist[i]).sb_len;
            }

            free(pBuf);
        }

    } while (0);

ddi_exit:

    if ( (num_sg > 1) && (status != ERROR_SUCCESS) )
    {
        free(pBuf);
    }

    pSgr->sr_status = status;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --DoDiskRead()\r\n")));

    return status;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DoDiskWrite                                                           */
/* DESCRIPTION                                                               */
/*      Do Write operation To moviNAND flash memory                             */
/* PARAMETERS                                                                */
/*      pDisk       BIBDRV driver own structure pointer                    */
/*      pData       PSQ_REQ structure pointer,it contains request information*/
/*                  for read operations                                      */
/* RETURN VALUES                                                             */
/*      If it successes, it returns TRUE. otherwize it returns FALSE         */
/*                                                                           */
/*****************************************************************************/
static DWORD
DoDiskWrite(PDISK pDisk,
           PVOID pData)
{
    DWORD   status = ERROR_SUCCESS;
    DWORD   num_sg,i;
    DWORD   bytes_this_sg;
    PSG_REQ pSgr;
    PSG_BUF pSg;
    PUCHAR  pBuf;
    UINT    nSecCount;
    UINT32  nCount;
    static UINT nAccessCnt = 0;
    PUCHAR  tempBuf;
    DWORD    dwStartSector;


    pSgr = (PSG_REQ) pData;

    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  ========= DoDiskWrite Request Info ========= \r\n")));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpSgr->sr_start    = %d(0x%x)\r\n"), pSgr->sr_start, pSgr->sr_start));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpSgr->sr_num_sec  = %d\r\n"),       pSgr->sr_num_sec));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpSgr->sr_num_sg   = %d\r\n"),       pSgr->sr_num_sg));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tlast sector num   = %d\r\n"),       pSgr->sr_start + pSgr->sr_num_sec - 1));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  \tpSgr->sr_status   = 0x%x\r\n"),     pSgr->sr_status));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  =========================================== \r\n")));

    /* Scatter/Gather buffer Bound Check */
    if (pSgr->sr_num_sg > MAX_SG_BUF)
    {
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  Scatter/Gather buffer Bound Check Fail (Too many buffers)\r\n")));
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }

    status          = ERROR_SUCCESS;
    num_sg          = pSgr->sr_num_sg;
    pSg             = &(pSgr->sr_sglist[0]);

    BIBDRV_INF_PRINT((TEXT("[SDMMC:INF]  ----------------------------------- \r\n")));
    BIBDRV_INF_PRINT((TEXT("[SDMMC:INF]  \tnum_sg        = %d\r\n"),     num_sg));
    BIBDRV_INF_PRINT((TEXT("[SDMMC:INF]  \tpSg           = 0x%08x\r\n"), pSg));
    BIBDRV_INF_PRINT((TEXT("[SDMMC:INF]  \tbytes_this_sg = %d\r\n"),     bytes_this_sg));
    BIBDRV_INF_PRINT((TEXT("[SDMMC:INF]  \tpSg->sb_buf   = 0x%08x\r\n"), pSg->sb_buf));
    BIBDRV_INF_PRINT((TEXT("[SDMMC:INF]  \tpBuf          = 0x%08x\r\n"), pBuf));
    BIBDRV_INF_PRINT((TEXT("[SDMMC:INF]  ----------------------------------- \r\n")));

    if (num_sg>1)
    {
        bytes_this_sg = 0 ;

        for ( i=0; i<num_sg; i++ )
            bytes_this_sg += (pSgr->sr_sglist[i]).sb_len;

        nSecCount = ((bytes_this_sg - 1) / pDisk->d_DiskInfo.di_bytes_per_sect) + 1;
    }
    else if ( num_sg == 1)
    {
        bytes_this_sg   = pSg->sb_len;
        pBuf            = (LPVOID)pSg->sb_buf;
        nSecCount = ((bytes_this_sg - 1) / pDisk->d_DiskInfo.di_bytes_per_sect) + 1;
    }


    /*----------------------------------------------*/
    // Make sure request doesn't exceed the disk    */
    /*----------------------------------------------*/
    if ((pSgr->sr_start + nSecCount - 1) > pDisk->d_DiskInfo.di_total_sectors)
    {
        BIBDRV_ERR_PRINT((TEXT("[SDMMC:ERR]  Request Sector OOB Check Fail(sector exceeded)\r\n")));
        BIBDRV_ERR_PRINT((TEXT("[SDMMC:ERR]  - Disk Totol Sectors        = %d\r\n"), pDisk->d_DiskInfo.di_total_sectors));
        BIBDRV_ERR_PRINT((TEXT("[SDMMC:ERR]  - Requested pSgr->sr_start  = %d\r\n"), pSgr->sr_start));
        BIBDRV_ERR_PRINT((TEXT("[SDMMC:ERR]  - Requested pSgr->sr_num_sec= %d\r\n"), pSgr->sr_num_sec));
        BIBDRV_ERR_PRINT((TEXT("[SDMMC:ERR]  - Requested last sector num = %d\r\n"), pSgr->sr_start + pSgr->sr_num_sec - 1));
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }

    if ( num_sg >1)
    {
        pBuf = (PUCHAR)malloc(nSecCount*512);
        tempBuf = pBuf;

        for ( i=0; i<num_sg; i++ )
        {
            memcpy(tempBuf, (LPVOID)((pSgr->sr_sglist[i]).sb_buf),
                (pSgr->sr_sglist[i]).sb_len);
            tempBuf += (pSgr->sr_sglist[i]).sb_len;
        }
    }

    if ( bytes_this_sg == 0 )
    {
        BIBDRV_ERR_PRINT((TEXT("[SDMMC:ERR]  WRITE::Buffer length is 0.\r\n")));
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }

    dwStartSector = pSgr->sr_start + MBRSTARTSECTOR;

    /*--------------------------*/
    /* Read sectors from disk.  */
    /*--------------------------*/
    do
    {

        if (nSecCount <= MAX_READ_WRITE_SECTOR)
        {
            status = ERROR_SUCCESS;
#if USE_DMA
            memcpy(g_pVirtDMABufferAddr, pBuf, bytes_this_sg);
#endif

            GetMutex(g_hMutex);
#if USE_DMA
            while (SDHC_WRITE_DMA(dwStartSector, nSecCount, (UINT32)g_PhyDMABufferAddr.LowPart) == FALSE)
#else
            while (SDHC_WRITE(dwStartSector, nSecCount, (UINT32)pBuf) == FALSE)
#endif
            {
                if ( status == ERROR_SUCCESS )
                {
                    status = ERROR_INVALID_PARAMETER;
                    while(!SDHC_INIT())
                    {
                    }
                }
                else
                {
                    ReleaseMutex(g_hMutex);
                    goto ddi_exit;
                }
            }


            ReleaseMutex(g_hMutex);

            status = ERROR_SUCCESS;
        }


        else
        {
            for ( nCount = 0; nSecCount >= MAX_READ_WRITE_SECTOR ; nCount++, nSecCount -= MAX_READ_WRITE_SECTOR )
            {
                status = ERROR_SUCCESS;

#if USE_DMA
                memcpy(g_pVirtDMABufferAddr, pBuf+(nCount*MAX_READ_WRITE_SECTOR*512), MAX_READ_WRITE_SECTOR*512);
#endif
                GetMutex(g_hMutex);
#if USE_DMA
                while (SDHC_WRITE_DMA(dwStartSector+nCount*MAX_READ_WRITE_SECTOR, MAX_READ_WRITE_SECTOR, (UINT32)g_PhyDMABufferAddr.LowPart) == FALSE)
#else
                while (SDHC_WRITE(dwStartSector+nCount*MAX_READ_WRITE_SECTOR, MAX_READ_WRITE_SECTOR, (UINT32)pBuf+(UINT32)(nCount*MAX_READ_WRITE_SECTOR*512)) == FALSE)
#endif
                {
                    if ( status == ERROR_SUCCESS )
                    {
                        status = ERROR_INVALID_PARAMETER;
                        while(!SDHC_INIT())
                        {
                        }
                    }
                    else
                    {
                        ReleaseMutex(g_hMutex);
                        goto ddi_exit;
                    }
                }
                ReleaseMutex(g_hMutex);

            }

            if ( nSecCount != 0 )
            {
                status = ERROR_SUCCESS;
#if USE_DMA
                memcpy(g_pVirtDMABufferAddr, pBuf+(nCount*MAX_READ_WRITE_SECTOR*512), nSecCount*512);
#endif
                GetMutex(g_hMutex);
#if USE_DMA
                while (SDHC_WRITE(dwStartSector+nCount*MAX_READ_WRITE_SECTOR, nSecCount, (UINT32)g_PhyDMABufferAddr.LowPart) == FALSE)
#else
                while (SDHC_WRITE(dwStartSector+nCount*MAX_READ_WRITE_SECTOR, nSecCount, (UINT32)pBuf+(UINT32)(nCount*MAX_READ_WRITE_SECTOR*512)) == FALSE)
#endif
                {
                    if ( status == ERROR_SUCCESS )
                    {
                        status = ERROR_INVALID_PARAMETER;
                        while(!SDHC_INIT())
                        {
                        }
                    }
                    else
                    {
                        ReleaseMutex(g_hMutex);
                        goto ddi_exit;
                    }
                }
                ReleaseMutex(g_hMutex);

            }

            status = ERROR_SUCCESS;
        }


        if (num_sg>1)
        {
            free(pBuf);
        }


    }  while (0);


ddi_exit:

    if ( (status != ERROR_SUCCESS) && (num_sg > 1) )
        free(pBuf);

    pSgr->sr_status = status;

    BIBDRV_INF_PRINT((TEXT("[BIBDRV:OUT] --DoDiskWrtie()\r\n")));

    return status;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      GetDiskInfo                                                          */
/* DESCRIPTION                                                               */
/*      Get disk information from pDisk structure                            */
/* PARAMETERS                                                                */
/*      pDisk       BIBDRV_PS driver own structure pointer                    */
/*      pInfo       DISK Information structure pointer                       */
/* RETURN VALUES                                                             */
/*      it always returns ERROR_SUCCESS                                      */
/*                                                                           */
/*****************************************************************************/
static DWORD
GetDiskInfo(PDISK       pDisk,
            PDISK_INFO  pInfo)
{
    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++GetDiskInfo()\r\n")));

    memcpy(pInfo, &(pDisk->d_DiskInfo), sizeof(DISK_INFO));
    pInfo->di_flags &= ~DISK_INFO_FLAG_UNFORMATTED;

    BIBDRV_INF_PRINT((TEXT("[NFALT:INF] \tpInfo->di_total_sectors    =%d\r\n"), pInfo->di_total_sectors));
    BIBDRV_INF_PRINT((TEXT("[NFALT:INF] \tpInfo->di_bytes_per_sect   =%d\r\n"), pInfo->di_bytes_per_sect));
    BIBDRV_INF_PRINT((TEXT("[NFALT:INF] \tpInfo->di_cylinders        =%d\r\n"), pInfo->di_cylinders));
    BIBDRV_INF_PRINT((TEXT("[NFALT:INF] \tpInfo->di_heads            =%d\r\n"), pInfo->di_heads));
    BIBDRV_INF_PRINT((TEXT("[NFALT:INF] \tpInfo->di_sectors          =%d\r\n"), pInfo->di_sectors));
    BIBDRV_INF_PRINT((TEXT("[NFALT:INF] \tpInfo->di_flags            =%X\r\n"), pInfo->di_flags));

    /*
    The device supports demand paging.
    Read and write requests are synchronous and do not involve
    memory manager calls, loader operations, or thread switches.
    pInfo->di_flags |= DISK_INFO_FLAG_PAGEABLE;
    */

    /*
    The device does not support CHS addressing;
    values for di_cylinders, di_heads, and di_sectors may be simulations,
    estimations, or not provided.

    pInfo->di_flags |= DISK_INFO_FLAG_CHS_UNCERTAIN;
    */

    /*
    The device requires a low-level format with the IOCTL_DISK_FORMAT_MEDIA.
    The FAT file system currently ignores this flag.
    */

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --GetDiskInfo()\r\n")));

    return ERROR_SUCCESS;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      SetDiskInfo                                                          */
/* DESCRIPTION                                                               */
/*      Set disk information to pDisk structure                              */
/* PARAMETERS                                                                */
/*      pDisk       BIBDRV_PS driver own structure pointer                    */
/*      pInfo       DISK Information structure pointer                       */
/* RETURN VALUES                                                             */
/*      it always returns ERROR_SUCCESS                                      */
/*                                                                           */
/*****************************************************************************/
static DWORD
SetDiskInfo(PDISK       pDisk,
            PDISK_INFO  pInfo)
{
    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++SetDiskInfo()\r\n")));

    pDisk->d_DiskInfo = *pInfo;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --SetDiskInfo()\r\n")));

    return ERROR_SUCCESS;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      CreateDiskObject                                                     */
/* DESCRIPTION                                                               */
/*      Create a DISK structure, init some fields and link it.               */
/* PARAMETERS                                                                */
/*      none                                                                 */
/* RETURN VALUES                                                             */
/*      new DISK structure pointer                                           */
/*                                                                           */
/*****************************************************************************/
static PDISK
CreateDiskObject(VOID)
{
    PDISK pDisk;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++CreateDiskObject()\r\n")));

    pDisk = LocalAlloc(LPTR, sizeof(DISK));

    if (pDisk != NULL)
    {
        pDisk->hDevice      = NULL;
        pDisk->d_OpenCount  = 0;
        pDisk->d_ActivePath = NULL;

        /* Initialize CiriticalSection for Disk Object(NAND flash) */
        InitializeCriticalSection(&(pDisk->d_DiskCardCrit));

        EnterCriticalSection(&gDiskCrit);
        pDisk->pd_next      = gDiskList;
        gDiskList           = pDisk;
        LeaveCriticalSection(&gDiskCrit);
    }

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --CreateDiskObject()\r\n")));

    return pDisk;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      IsValidDisk                                                          */
/* DESCRIPTION                                                               */
/*      This function checks Disk validation                                 */
/* PARAMETERS                                                                */
/*      pDisk                                                                */
/*          Pointer to disk handle                                           */
/* RETURN VALUES                                                             */
/*      Return TRUE if pDisk is valid, FALSE if not.                         */
/*                                                                           */
/*****************************************************************************/
static BOOL
IsValidDisk(PDISK pDisk)
{
#if 0
    PDISK   pd;
    BOOL    bRet = FALSE;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++IsValidDisk()\r\n")));

    EnterCriticalSection(&gDiskCrit);

    pd = gDiskList;
    while (pd)
    {
        if (pd == pDisk)
        {
            bRet = TRUE;
            break;
        }
        pd = pd->pd_next;
    }

    LeaveCriticalSection(&gDiskCrit);

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --IsValidDisk()\r\n")));

    return bRet;
#endif
    return TRUE;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      BIBDRV_Entry                                                         */
/* DESCRIPTION                                                               */
/*      This function is BIBDRV_PS.dll Entry Point                            */
/* PARAMETERS                                                                */
/*      DllInstance                                                          */
/*      Reason                                                               */
/*      Reserved                                                             */
/* RETURN VALUES                                                             */
/*      it always returns TRUE                                               */
/*                                                                           */
/*****************************************************************************/
BOOL WINAPI
BIBDRV_Entry(HINSTANCE    DllInstance,
             INT          Reason,
             LPVOID       Reserved)
{
    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++BIBDRVEntry()\r\n")));

    switch(Reason) {
        case DLL_PROCESS_ATTACH:
            BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  DLL_PROCESS_ATTACH\r\n")));
            DEBUGREGISTER(DllInstance);
            break;

        case DLL_PROCESS_DETACH:
            BIBDRV_ERR_PRINT((TEXT("[BIBDRV:INF]  DLL_PROCESS_DETACH\r\n")));
            DeleteCriticalSection(&gDiskCrit);
            break;
    }

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --BIBDRVEntry()\r\n")));
    return TRUE;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      GetROPartitionInfo                                                              */
/* DESCRIPTION                                                               */
/*      This function prints MBR contents                                    */
/* PARAMETERS                                                                */
/*      pDisk                                                                */
/*          Disk handle                                                      */
/* RETURN VALUES                                                             */
/*      none                                                                 */
/* NOTE                                                                      */
/*                                                                           */
/*****************************************************************************/
#if defined(_BIBDRV_MBR_DEBUG_)
#include <Bootpart.h>

#define NUM_PARTS                   4
#define SIZE_END_SIG              2
#define PARTTABLE_OFFSET        (512 - SIZE_END_SIG - (sizeof(PARTENTRY) * NUM_PARTS))
void
GetROPartitionInfo(PDISK pDisk)
{
    UCHAR       aBuf[528];
    UCHAR      *pBuf;
    PARTENTRY  *pPartEntry;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] ++GetROPartitionInfo()\r\n")));

    pBuf       = &aBuf[0];
    pPartEntry = (PARTENTRY *) (pBuf + PARTTABLE_OFFSET);

    do {
        SDHC_READ(MBRSTARTSECTOR,1,(UINT32)pBuf);

        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  MBR MAGIC CODE (First): 0x%x,0x%x,0x%x\r\n"), aBuf[0], aBuf[1], aBuf[2]));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  MBR MAGIC CODE (Last) : 0x%x,0x%x\r\n"),      aBuf[512 - 2], aBuf[512 - 1]));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_BootInd      = 0x%X\r\n"), pPartEntry[0].Part_BootInd));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstHead    = 0x%X\r\n"), pPartEntry[0].Part_FirstHead));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstSector  = 0x%X\r\n"), pPartEntry[0].Part_FirstSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstTrack   = 0x%X\r\n"), pPartEntry[0].Part_FirstTrack));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FileSystem   = 0x%X\r\n"), pPartEntry[0].Part_FileSystem));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastHead     = 0x%X\r\n"), pPartEntry[0].Part_LastHead));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastSector   = 0x%X\r\n"), pPartEntry[0].Part_LastSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastTrack    = 0x%X\r\n"), pPartEntry[0].Part_LastTrack));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_StartSector  = 0x%X\r\n"),   pPartEntry[0].Part_StartSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_TotalSectors = 0x%X\r\n"),   pPartEntry[0].Part_TotalSectors));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_BootInd      = 0x%X\r\n"), pPartEntry[1].Part_BootInd));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstHead    = 0x%X\r\n"), pPartEntry[1].Part_FirstHead));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstSector  = 0x%X\r\n"), pPartEntry[1].Part_FirstSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstTrack   = 0x%X\r\n"), pPartEntry[1].Part_FirstTrack));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FileSystem   = 0x%X\r\n"), pPartEntry[1].Part_FileSystem));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastHead     = 0x%X\r\n"), pPartEntry[1].Part_LastHead));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastSector   = 0x%X\r\n"), pPartEntry[1].Part_LastSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastTrack    = 0x%X\r\n"), pPartEntry[1].Part_LastTrack));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_StartSector  = 0x%X\r\n"),   pPartEntry[1].Part_StartSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_TotalSectors = 0x%X\r\n"),   pPartEntry[1].Part_TotalSectors));

        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_BootInd      = 0x%X\r\n"), pPartEntry[2].Part_BootInd));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstHead    = 0x%X\r\n"), pPartEntry[2].Part_FirstHead));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstSector  = 0x%X\r\n"), pPartEntry[2].Part_FirstSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstTrack   = 0x%X\r\n"), pPartEntry[2].Part_FirstTrack));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FileSystem   = 0x%X\r\n"), pPartEntry[2].Part_FileSystem));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastHead     = 0x%X\r\n"), pPartEntry[2].Part_LastHead));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastSector   = 0x%X\r\n"), pPartEntry[2].Part_LastSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastTrack    = 0x%X\r\n"), pPartEntry[2].Part_LastTrack));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_StartSector  = 0x%X\r\n"),   pPartEntry[2].Part_StartSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_TotalSectors = 0x%X\r\n"),   pPartEntry[2].Part_TotalSectors));

        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_BootInd      = 0x%X\r\n"), pPartEntry[3].Part_BootInd));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstHead    = 0x%X\r\n"), pPartEntry[3].Part_FirstHead));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstSector  = 0x%X\r\n"), pPartEntry[3].Part_FirstSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FirstTrack   = 0x%X\r\n"), pPartEntry[3].Part_FirstTrack));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_FileSystem   = 0x%X\r\n"), pPartEntry[3].Part_FileSystem));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastHead     = 0x%X\r\n"), pPartEntry[3].Part_LastHead));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastSector   = 0x%X\r\n"), pPartEntry[3].Part_LastSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_LastTrack    = 0x%X\r\n"), pPartEntry[3].Part_LastTrack));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_StartSector  = 0x%X\r\n"),   pPartEntry[3].Part_StartSector));
        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  Part_TotalSectors = 0x%X\r\n"),   pPartEntry[3].Part_TotalSectors));

        BIBDRV_RTL_PRINT((TEXT("[BIBDRV:INF]  MBR Start Sector = 0x%x\r\n"),  MBRSTARTSECTOR));
    } while(0);

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --GetROPartitionInfo()\r\n")));
}

#endif  //(_BIBDRV_MBR_DEBUG_)


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      InitDisk                                                             */
/* DESCRIPTION                                                               */
/*      This function initializes Disk handle                                */
/* PARAMETERS                                                                */
/*      pDisk                                                                */
/*          Disk handle                                                      */
/*      ActiveKey                                                            */
/*          Pointer to active key                                            */
/* RETURN VALUES                                                             */
/*      Return TRUE if Operation success, FALSE if not.                      */
/* NOTE                                                                      */
/*      call from DSK_Init                                                   */
/*                                                                           */
/*****************************************************************************/
static BOOL
InitDisk(PDISK  pDisk,
         LPTSTR ActiveKey)
{
    BOOL            bRet = FALSE;
#if USE_DMA
    DMA_ADAPTER_OBJECT Adapter;
#endif

    g_hMutex = InitMutex(TEXT("HSMMC"));

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++InitDisk() Entered\r\n")));

    GetMutex(g_hMutex);
    while (!SDHC_INIT());
    ReleaseMutex(g_hMutex);

    pDisk->d_DiskInfo.di_total_sectors  = SECTOROFIMAGE+SECTOROFMBR; // to align
    pDisk->d_DiskInfo.di_bytes_per_sect = 512;
    pDisk->d_DiskInfo.di_cylinders      = 1;
    pDisk->d_DiskInfo.di_heads          = 1;
    pDisk->d_DiskInfo.di_sectors        = pDisk->d_DiskInfo.di_total_sectors;
    pDisk->d_DiskInfo.di_flags          = DISK_INFO_FLAG_PAGEABLE | DISK_INFO_FLAG_CHS_UNCERTAIN;

    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF] total_sectors    = 0x%x\r\n"), pDisk->d_DiskInfo.di_total_sectors));

    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF] total_sectors    = %d\r\n"), pDisk->d_DiskInfo.di_total_sectors));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF] bytes_per_sect   = %d\r\n"), pDisk->d_DiskInfo.di_bytes_per_sect));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF] di_cylinders     = %d\r\n"), pDisk->d_DiskInfo.di_cylinders));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF] di_heads         = %d\r\n"), pDisk->d_DiskInfo.di_heads));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF] di_sectors       = %d\r\n"), pDisk->d_DiskInfo.di_sectors));
    BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF] storage size     = %d bytes\r\n"),
                    pDisk->d_DiskInfo.di_total_sectors * pDisk->d_DiskInfo.di_bytes_per_sect));

#if USE_DMA
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
    Adapter.InterfaceType = Internal;

    // Allocate DMA Buffer
    g_pVirtDMABufferAddr = HalAllocateCommonBuffer(&Adapter, MAX_READ_WRITE_SECTOR * 512, &g_PhyDMABufferAddr, FALSE);
    if (g_pVirtDMABufferAddr == NULL)
    {
        BIBDRV_ERR_PRINT((_T("[BIBDRV:ERR] MapDMABuffers() : DMA Buffer Allocation Failed\n\r")));
        bRet = FALSE;
        return bRet;
    }
#endif

    bRet = TRUE;

#if defined(_BIBDRV_MBR_DEBUG_)
    GetMutex(g_hMutex);
    GetROPartitionInfo(pDisk);
    ReleaseMutex(g_hMutex);
#endif

    return bRet;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DSK_Init                                                             */
/* DESCRIPTION                                                               */
/*      Create Disk Object, Initialize Disk Object                           */
/* PARAMETERS                                                                */
/*      dwContext   BIBDRV_PS driver own structure pointer                    */
/* RETURN VALUES                                                             */
/*      Returns context data for this Init instance                          */
/*                                                                           */
/*****************************************************************************/
DWORD
DSK_Init(DWORD dwContext)
{
    PDISK   pDisk;
    LPWSTR  ActivePath  = (LPWSTR)dwContext;
    DWORD dwActivePathSize;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++DSK_Init() Entered\r\n")));

    if (gDiskList == NULL)
    {
        InitializeCriticalSection(&gDiskCrit);
    }

    pDisk = CreateDiskObject();

    if (pDisk == NULL)
    {
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  LocalAlloc(PDISK) failed %d\r\n"), GetLastError()));
        return 0;
    }

    if (ActivePath)
    {
        dwActivePathSize=wcslen(ActivePath) * sizeof(WCHAR) + sizeof(WCHAR);
        if (pDisk->d_ActivePath = LocalAlloc(LPTR,dwActivePathSize ))
        {
            /* Copy Active Path to Dist structure */
            wcscpy_s( pDisk->d_ActivePath, dwActivePathSize, ActivePath);
        }

        BIBDRV_INF_PRINT((TEXT("[BIBDRV:INF]  ActiveKey (copy) = %s (@ 0x%08X)\r\n"),
                        pDisk->d_ActivePath, pDisk->d_ActivePath));
    }

    EnterCriticalSection(&gDiskCrit);

    if (InitDisk(pDisk, ActivePath) == FALSE)
    {
        /* If pDisk already memory allocated, it should be deallocated */
        if (pDisk)
        {
            if (pDisk->d_ActivePath)
            {
                /* Counter function of LocalAlloc */
                LocalFree(pDisk->d_ActivePath);
                pDisk->d_ActivePath = NULL;
            }
            CloseDisk(pDisk);
        }
        return 0;
    }

    LeaveCriticalSection(&gDiskCrit);
 
    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --DSK_Init() returning pDisk=0x%x\r\n"), pDisk));

    return (DWORD)pDisk;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DSK_Close                                                            */
/* DESCRIPTION                                                               */
/*      This function Close Disk                                             */
/* PARAMETERS                                                                */
/*      Handle                                                               */
/*          Disk handle                                                      */
/* RETURN VALUES                                                             */
/*      Return TRUE if Operation success, FALSE if not.                      */
/* NOTE                                                                      */
/*      call from DSK_Deinit                                                 */
/*                                                                           */
/*****************************************************************************/
BOOL
DSK_Close(DWORD Handle)
{
    PDISK pDisk = (PDISK)Handle;
    BOOL bClose = FALSE;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++DSK_Close()\r\n")));

    do {
        if (!IsValidDisk(pDisk))
        {
            break;
        }

        EnterCriticalSection(&(pDisk->d_DiskCardCrit));

        pDisk->d_OpenCount--;

        if (pDisk->d_OpenCount <= 0)
        {
            pDisk->d_OpenCount = 0;
        }

        LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

        bClose = TRUE;

    } while(0);

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --DSK_Close\r\n")));

    return bClose;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DSK_Deinit                                                           */
/* DESCRIPTION                                                               */
/*      This function deinitializes Disk                                     */
/* PARAMETERS                                                                */
/*      dwContext                                                            */
/*          Disk handle                                                      */
/* RETURN VALUES                                                             */
/*      Return TRUE if Operation success, FALSE if not.                      */
/* NOTE                                                                      */
/*      Device deinit - devices are expected to close down.                  */
/*      The device manager does not check the return code.                   */
/*                                                                           */
/*****************************************************************************/
BOOL
DSK_Deinit(DWORD dwContext) // future: pointer to the per disk structure
{
    PDISK pDisk = (PDISK)dwContext;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++DSK_Deinit()\r\n")));

    DSK_Close(dwContext);
    CloseDisk((PDISK)dwContext);

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --DSK_Deinit()\r\n")));

    return TRUE;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DSK_Open                                                             */
/* DESCRIPTION                                                               */
/*      This function opens Disk                                             */
/* PARAMETERS                                                                */
/*      dwData                                                               */
/*          Disk handle                                                      */
/*      dwAccess                                                             */
/*          Not used                                                         */
/*      dwShareMode                                                          */
/*          Not used                                                         */
/* RETURN VALUES                                                             */
/*      Return address of pDisk(disk handle)                                 */
/* NOTE                                                                      */
/*                                                                           */
/*****************************************************************************/
DWORD
DSK_Open(DWORD dwData,
         DWORD dwAccess,
         DWORD dwShareMode)
{
    PDISK pDisk = (PDISK)dwData;
    DWORD dwRet = 0;

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++DSK_Open(0x%x)\r\n"),dwData));

    do {
        if (IsValidDisk(pDisk) == FALSE)
        {
            BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  DSK_Open - Passed invalid disk handle\r\n")));
            break;
        }

        EnterCriticalSection(&(pDisk->d_DiskCardCrit));
        {
            pDisk->d_OpenCount++;
        }
        LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

        dwRet = (DWORD)pDisk;

    } while(0);

    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --DSK_Open(0x%x) returning %d\r\n"),dwData, dwRet));

    return dwRet;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DSK_IOControl                                                        */
/* DESCRIPTION                                                               */
/*      This function is Disk IO Control                                     */
/* PARAMETERS                                                                */
/*      Handle                                                               */
/*          Disk handle                                                      */
/*      dwIoControlCode                                                      */
/*          IO Control Code                                                  */
/*      pInBuf                                                               */
/*          Pointer to input buffer                                          */
/*      nInBufSize                                                           */
/*          Size of input buffer                                             */
/*      pOutBuf                                                              */
/*          Pointer to output buffer                                         */
/*      nOutBufSize                                                          */
/*          Size of output buffer                                            */
/*      pBytesReturned                                                       */
/*          Pointer to byte returned                                         */
/* RETURN VALUES                                                             */
/*      Return TRUE if Operation success, FALSE if not.                      */
/* NOTE                                                                      */
/*      I/O Control function - responds to info, read and write control codes*/
/*      The read and write take a scatter/gather list in pInBuf              */
/*                                                                           */
/*****************************************************************************/
BOOL
DSK_IOControl(DWORD  Handle,
              DWORD  dwIoControlCode,
              PBYTE  pInBuf,
              DWORD  nInBufSize,
              PBYTE  pOutBuf,
              DWORD  nOutBufSize,
              PDWORD pBytesReturned)
{
    PSG_REQ pSG;
    PDISK   pDisk = (PDISK)Handle;
    BOOL    bRes  = TRUE;
    BOOL    bPrintFlg = TRUE;

    EnterCriticalSection(&gDiskCrit);
    BIBDRV_LOG_PRINT((TEXT("[BIBDRV: IN] ++DSK_IOControl() (IO code=%x) 0x%x\r\n"), dwIoControlCode, Handle));

    /* Disk Handle(pDisk) validation check */
    if (IsValidDisk(pDisk) == FALSE)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  Invalid Disk\r\n")));
        bRes = FALSE;

        goto IOControlError;
    }

    /* dwIoControlCode Print */
    switch (dwIoControlCode)
    {
    case DISK_IOCTL_READ:
        BIBDRV_LOG_PRINT((TEXT("[BIBDRV:LOG]  dwIoControlCode = DISK_IOCTL_READ\r\n")));
        break;
    case IOCTL_DISK_READ:
        BIBDRV_LOG_PRINT((TEXT("[BIBDRV:LOG]  dwIoControlCode = IOCTL_DISK_READ\r\n")));
        break;
    case DISK_IOCTL_WRITE:
        BIBDRV_LOG_PRINT((TEXT("[BIBDRV:LOG]  dwIoControlCode = DISK_IOCTL_WRITE\r\n")));
        break;
    case IOCTL_DISK_WRITE:
        BIBDRV_LOG_PRINT((TEXT("[BIBDRV:LOG]  dwIoControlCode = IOCTL_DISK_WRITE\r\n")));
        break;
    case DISK_IOCTL_GETINFO:
        BIBDRV_LOG_PRINT((TEXT("[BIBDRV:LOG]  dwIoControlCode = DISK_IOCTL_GETINFO\r\n")));
        break;
    case DISK_IOCTL_SETINFO:
        BIBDRV_LOG_PRINT((TEXT("[BIBDRV:LOG]  dwIoControlCode = DISK_IOCTL_SETINFO\r\n")));
        break;
    case DISK_IOCTL_INITIALIZED:
        BIBDRV_LOG_PRINT((TEXT("[BIBDRV:LOG]  dwIoControlCode = DISK_IOCTL_INITIALIZED\r\n")));
        break;
    case DISK_IOCTL_GETNAME:
        BIBDRV_LOG_PRINT((TEXT("[BIBDRV:LOG]  dwIoControlCode = DISK_IOCTL_GETNAME\r\n")));
        break;
    case IOCTL_DISK_GETINFO:
        BIBDRV_LOG_PRINT((TEXT("[BIBDRV:LOG]  dwIoControlCode = IOCTL_DISK_GETINFO\r\n")));
        break;
    case IOCTL_DISK_DEVICE_INFO:
        BIBDRV_LOG_PRINT((TEXT("[BIBDRV:LOG]  dwIoControlCode = IOCTL_DISK_DEVICE_INFO\r\n")));
        break;
    case DISK_IOCTL_FORMAT_MEDIA:
    case IOCTL_DISK_FORMAT_MEDIA:
        SetLastError(ERROR_SUCCESS);
        goto IOControlError;
        break;
    }

    /*------------------*/
    /* Check parameters */
    /*------------------*/
    switch (dwIoControlCode)
    {
    case DISK_IOCTL_WRITE:
    case IOCTL_DISK_WRITE:
     case DISK_IOCTL_READ:
    case IOCTL_DISK_READ:
    case DISK_IOCTL_GETINFO:
    case DISK_IOCTL_SETINFO:
    case DISK_IOCTL_INITIALIZED:
        if (nInBufSize == 0)
        {
            BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  DISK_IOCTL InBufSize is NULL\r\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            bRes = FALSE;
            goto IOControlError;
        }
        if (pInBuf == NULL)
        {
            BIBDRV_ERR_PRINT((TEXT("[BIBDRV:ERR]  DISK_IOCTL InBuf is NULL\r\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            bRes = FALSE;
            goto IOControlError;
        }
        break;
    case DISK_IOCTL_GETNAME:
    case IOCTL_DISK_GETINFO:
        if (pOutBuf == NULL)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            bRes = FALSE;
            goto IOControlError;
        }
        break;
    case IOCTL_DISK_DEVICE_INFO:
        if(!pInBuf || nInBufSize != sizeof(STORAGEDEVICEINFO))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            bRes = FALSE;
            goto IOControlError;
        }
        break;
    case IOCTL_DISK_GET_STORAGEID:
        ((PSTORAGE_IDENTIFICATION)pOutBuf)->dwSize = 0;//sizeof(STORAGE_IDENTIFICATION);
        ((PSTORAGE_IDENTIFICATION)pOutBuf)->dwFlags = SERIALNUM_INVALID;
        ((PSTORAGE_IDENTIFICATION)pOutBuf)->dwFlags |= MANUFACTUREID_INVALID;
        ((PSTORAGE_IDENTIFICATION)pOutBuf)->dwManufactureIDOffset=0;
        ((PSTORAGE_IDENTIFICATION)pOutBuf)->dwSerialNumOffset=0;
        bRes = TRUE;
        goto IOControlError;
        break;
    case IOCTL_DISK_FLUSH_CACHE:
        bRes = TRUE;
        goto IOControlError;
    case IOCTL_DISK_DELETE_SECTORS:
        bRes = TRUE;
        goto IOControlError;

    case IOCTL_DISK_FORMAT_MEDIA:
        bRes = TRUE;
        goto IOControlError;
        break;

    default:
        BIBDRV_ERR_PRINT((TEXT("[BIBDRV:MSG]  DSK_IOControl unkonwn code(%04x)\r\n"), dwIoControlCode));
        SetLastError(ERROR_INVALID_PARAMETER);
        bRes = FALSE;
        goto IOControlError;
    }

    //
    // Execute dwIoControlCode
    //
    switch (dwIoControlCode)
    {
    case DISK_IOCTL_READ:
    case IOCTL_DISK_READ:
        pSG = (PSG_REQ)pInBuf;

        DoDiskRead(pDisk, (PVOID) pSG);
        if (ERROR_SUCCESS != pSG->sr_status)
        {
            SetLastError(pSG->sr_status);
            bRes = FALSE;
        }
        else
        {
            bRes = TRUE;
            if (pBytesReturned)
                *(pBytesReturned) = pDisk->d_DiskInfo.di_bytes_per_sect;
        }


        break;

    case DISK_IOCTL_WRITE:
    case IOCTL_DISK_WRITE:

        pSG = (PSG_REQ)pInBuf;

        DoDiskWrite(pDisk, (PVOID) pSG);
        if (ERROR_SUCCESS != pSG->sr_status)
        {
            SetLastError(pSG->sr_status);
            RETAILMSG(1,(TEXT("Write Error with error Code %d\n"),pSG->sr_status));
            bRes = FALSE;
        }
        else
        {
            bRes = TRUE;
            if (pBytesReturned)
                *(pBytesReturned) = pDisk->d_DiskInfo.di_bytes_per_sect;
        }
        break;

    case DISK_IOCTL_GETINFO:
        GetDiskInfo(pDisk, (PDISK_INFO) pInBuf);
        if (pBytesReturned)
            *(pBytesReturned) = sizeof(DISK_INFO);
        bRes = TRUE;
        break;

    case IOCTL_DISK_GETINFO:
        GetDiskInfo(pDisk, (PDISK_INFO) pOutBuf);
        if (pBytesReturned)
            *(pBytesReturned) = sizeof(DISK_INFO);
        bRes = TRUE;
        break;

    case DISK_IOCTL_SETINFO:
        RETAILMSG(1, (TEXT("-DISK_IOCTL_SETINFO\r\n")));
        SetDiskInfo(pDisk, (PDISK_INFO)pInBuf);
        if (pBytesReturned)
            *(pBytesReturned) = sizeof(DISK_INFO);
        bRes = TRUE;
        break;

    case DISK_IOCTL_GETNAME:
        RETAILMSG(1, (TEXT("-DISK_IOCTL_GETNAME\r\n")));
        bRes = GetFolderName(pDisk, (LPWSTR)pOutBuf, nOutBufSize, pBytesReturned);
        break;
    case IOCTL_DISK_DEVICE_INFO: // new ioctl for disk info
        RETAILMSG(1, (TEXT("-IOCTL_DISK_DEVICE_INFO\r\n")));
        bRes = GetDeviceInfo(pDisk, (PSTORAGEDEVICEINFO)pInBuf);
        if (pBytesReturned)
            *(pBytesReturned) = sizeof(STORAGEDEVICEINFO);
        break;
    case DISK_IOCTL_INITIALIZED:
        bRes = TRUE;
        break;

    default:
        RETAILMSG(1, (TEXT("-DSK_IOControl (default:0x%x) \r\n"), dwIoControlCode));
        SetLastError(ERROR_INVALID_PARAMETER);
        bRes = FALSE;
        break;
    }

IOControlError:
    LeaveCriticalSection(&gDiskCrit);
    BIBDRV_LOG_PRINT((TEXT("[BIBDRV:OUT] --DSK_IOControl()\r\n")));

    return bRes;
}


DWORD
DSK_Read(DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes)
{
    BIBDRV_LOG_PRINT((TEXT("DSK_Read\r\n")));
    return 0;
}

DWORD
DSK_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    BIBDRV_LOG_PRINT((TEXT("DSK_Write\r\n")));
    return 0;
}

DWORD
DSK_Seek(DWORD Handle, long lDistance, DWORD dwMoveMethod)
{
    BIBDRV_LOG_PRINT((TEXT("DSK_Seek\r\n")));
    return 0;
}

void
DSK_PowerUp(void)
{
}

void
DSK_PowerDown(void)
{
}
