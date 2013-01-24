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
//------------------------------------------------------------------------------
//
//  File: ioctl.c
//
//  This file implements the OEM's IO Control (IOCTL) functions and declares
//  global variables used by the IOCTL component.
//
#include <partdrv.h>
#include <bsp.h>
#include <pmplatform.h>
#include <pkfuncs.h>


//Global variable for the Critical section of RTC
CRITICAL_SECTION g_oalRTCcs;


//Global variable to hold the Init variable to check whether critical section is initialised
unsigned long    g_oalRTCcsInit = 0;

//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlPlatformType/OEM
//
//  Platform Type/OEM
//
LPCWSTR g_oalIoCtlPlatformType    = IOCTL_PLATFORM_TYPE;
LPCWSTR g_oalIoCtlPlatformOEM    = IOCTL_PLATFORM_OEM;

//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlProcessorVendor/Name/Core
//
//  Processor information
//
LPCWSTR g_oalIoCtlProcessorVendor    = IOCTL_PROCESSOR_VENDOR;
LPCWSTR g_oalIoCtlProcessorName    = IOCTL_PROCESSOR_NAME;
LPCWSTR g_oalIoCtlProcessorCore        = IOCTL_PROCESSOR_CORE;

//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlInstructionSet
//
//  Processor instruction set identifier
//
UINT32 g_oalIoCtlInstructionSet = IOCTL_PROCESSOR_INSTRUCTION_SET;

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalReboot
//  This function make a Warm Boot of the target device
//
static BOOL OALIoCtlHalReboot(
        UINT32 code, VOID *pInpBuffer, UINT32 inpSize,
        VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("[OAL] ++OALIoCtlHalReboot()\r\n")));

    //-----------------------------
    // Disable DVS and Set to Full Speed
    //-----------------------------
    ChangeDVSLevel(SYS_L0);

    OEMSWReset();

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("[OAL] --OALIoCtlHalReboot()\r\n")));

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetHWEntropy
//
//  Implements the IOCTL_HAL_GET_HWENTROPY handler. This function creates a
//  64-bit value which is unique to the hardware.  This value never changes.
//
static BOOL OALIoCtlHalGetHWEntropy(
        UINT32 dwIoControlCode, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf,
        UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
// TODO by Device Maker : This code is only sample.
// S3C6410 has IDs only for chip type not for each chip.
// If Device Maker wants unique ID for each device
// Device maker must modify this function.
// If Device has Ethernet Device, Use MAC address
// Recommended method is to use NAND's ID or UUIC's ID
// Device maker can make Unique ID with CHIP ID
    UINT8 *UniqueID;
    BOOL rc = FALSE;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetHWEntropy\r\n"));    

    UniqueID = (UINT8 *)OALArgsQuery(OAL_ARGS_QUERY_UUID);

    // Check buffer size
    if (lpBytesReturned != NULL)
    {
        *lpBytesReturned = sizeof(UINT8[16]);
    }

    if (lpOutBuf == NULL || nOutBufSize < sizeof(UINT8[16]))
    {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalGetHWEntropy: Buffer too small\r\n"));
    }
    else
    {
        // Copy pattern to output buffer
        memcpy(lpOutBuf, UniqueID, sizeof(UINT8[16]));
        // We are done
        rc = TRUE;
    }

    // Indicate status
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetHWEntropy(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetHiveCleanFlag
//
//  This function is used by Filesys.exe to query the OEM to determine if the registry hives
//  and user profiles should be deleted and recreated.
//
//  Notes: During a OS start up, Filesys.exe calls HIVECLEANFLAG_SYSTEM twice and followed by
//  HIVECLEANFLAG_USER. We'll clear the shared Args flag after that.
//
static BOOL OALIoCtlHalGetHiveCleanFlag(
        UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf,
        UINT32 nOutBufSize, UINT32 *pOutSize)
{
    BOOL bRet = FALSE;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("++OALIoCtlHalGetHiveCleanFlag()\r\n")));

    if ( (lpInBuf == NULL)
        || (nInBufSize != sizeof(DWORD))
        || (lpOutBuf == NULL)
        || (nOutBufSize != sizeof(BOOL)))
    {
        OALMSG(OAL_ERROR, (TEXT("[OAL:ERR] OALIoCtlHalGetHiveCleanFlag() Invalid Input Parameter\r\n")));
        return FALSE;
    }
    else
    {
        DWORD    *pdwFlags = (DWORD*)lpInBuf;
        BOOL    *pbClean  = (BOOL*)lpOutBuf;
        // This is the global shared Args flag
        BOOL    *bHiveCleanFlag = (BOOL*) OALArgsQuery(BSP_ARGS_QUERY_HIVECLEAN);

        *pbClean = *bHiveCleanFlag;
        bRet = *bHiveCleanFlag;

        if (*pdwFlags == HIVECLEANFLAG_SYSTEM)
        {
            if(bRet)
            {
                OALMSG(TRUE, (TEXT("[OAL] Clear System Hive\r\n")));
            }
            else
            {
                OALMSG(TRUE, (TEXT("[OAL] Not Clear System Hive\r\n")));
            }
        }
        else if (*pdwFlags == HIVECLEANFLAG_USERS)
        {
            if(bRet)
            {
                OALMSG(TRUE, (TEXT("[OAL] Clear User Hive\r\n")));
            }
            else
            {
                OALMSG(TRUE, (TEXT("[OAL] Not Clear User Hive\r\n")));
            }

            // We are done checking HiveCleanFlag by now (system hive is checked before user hive).
            // Now is the time to clear the global shared Args flag if it is set by switch or software.
            *bHiveCleanFlag = FALSE;
        }
        else
        {
            OALMSG(OAL_ERROR, (TEXT("[OAL] Unknown Flag 0x%x\r\n"), *pdwFlags));
        }
    }

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("--OALIoCtlHalGetHiveCleanFlag()\r\n")));

    return(bRet);
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalQueryFormatPartition
//
//  This function is called by Filesys.exe to allow an OEM to specify whether a specific
//  partition is to be formatted on mount. Before Filesys.exe calls this IOCTL, it checks
//  the CheckForFormat registry value in the storage profile for your block driver.
//
static BOOL OALIoCtlHalQueryFormatPartition(
        UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf,
        UINT32 nOutBufSize, UINT32 *pOutSize)
{
    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("++OALIoCtlHalFormatPartition()\r\n")));

    if ( (lpInBuf == NULL)
        ||(nInBufSize != sizeof(STORAGECONTEXT))
        || (lpOutBuf == NULL)
        || (nOutBufSize < sizeof(BOOL)))
    {
        OALMSG(OAL_ERROR, (TEXT("[OAL:ERR] OALIoCtlHalFormatPartition() Invalid Input Parameter\r\n")));
        return FALSE;
    }
    else
    {
        STORAGECONTEXT *pStore = (STORAGECONTEXT *)lpInBuf;
        BOOL  *pbClean = (BOOL *)lpOutBuf;
        // This is the global shared Args flag
        BOOL *bFormatPartFlag = (BOOL *) OALArgsQuery(BSP_ARGS_QUERY_FORMATPART);

        OALMSG(OAL_VERBOSE, (TEXT("[OAL] Store partition info:\r\n")));
        OALMSG(OAL_VERBOSE, (TEXT("[OAL] \tszPartitionName=%s\r\n"), pStore->PartInfo.szPartitionName));
        OALMSG(OAL_VERBOSE, (TEXT("[OAL] \tsnNumSectors=%d\r\n"), pStore->PartInfo.snNumSectors));
        OALMSG(OAL_VERBOSE, (TEXT("[OAL] \tdwAttributes=0x%x\r\n"), pStore->PartInfo.dwAttributes));
        OALMSG(OAL_VERBOSE, (TEXT("[OAL] \tbPartType=0x%x\r\n"), pStore->PartInfo.bPartType));

        // Set return value
        *pbClean = *bFormatPartFlag;

        // Clear the flag so that we don't do it again in next boot unless it is set again.
        *bFormatPartFlag = FALSE;

        if(*pbClean)
        {
            if(pStore->dwFlags & AFS_FLAG_BOOTABLE)
            {
                OALMSG(TRUE, (TEXT("[OAL] Clear Storage (System Registry Hive)\r\n")));
            }
            else
            {
                OALMSG(TRUE, (TEXT("[OAL] Clear Storage\r\n")));
            }
        }
        else
        {
            OALMSG(TRUE, (TEXT("[OAL] Not Clear Storage\r\n")));
        }
    }

    if(pOutSize)
    {
        *pOutSize = sizeof(UINT32);
    }

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("--OALIoCtlHalFormatPartition()\r\n")));

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalQueryDisplaySettings
//
//  This function is called by GDI to query the kernel for information
//  about a preferred resolution for the system to use.
//
static BOOL OALIoCtlHalQueryDisplaySettings(
        UINT32 dwIoControlCode, VOID *lpInBuf, UINT32 nInBufSize,
        VOID *lpOutBuf, UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
    DWORD dwErr = 0;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("++OALIoCtlHalQueryDisplaySettings()\r\n")));

    if (lpBytesReturned)
    {
        *lpBytesReturned = 0;
    }

    if (lpOutBuf == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else if (sizeof(DWORD)*3 > nOutBufSize)
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    }
    else
    {
        // Check the boot arg structure for the default display settings.
        __try
        {
            ((PDWORD)lpOutBuf)[0] = (DWORD)LCD_WIDTH;
            ((PDWORD)lpOutBuf)[1] = (DWORD)LCD_HEIGHT;
            ((PDWORD)lpOutBuf)[2] = (DWORD)LCD_BPP;

            if (lpBytesReturned)
            {
                *lpBytesReturned = sizeof (DWORD)*3;
            }

        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr)
    {
        OALMSG(OAL_ERROR, (TEXT("[OAL:ERR] OALIoCtlHalQueryDisplaySettings() Failed dwErr = 0x%08x\r\n"), dwErr));
        NKSetLastError(dwErr);
    }

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("++OALIoCtlHalQueryDisplaySettings()\r\n")));

    return !dwErr;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalSetSystemLevel
//
//  For Testing DVS level manually
//  NOTE!!! Remove UpdateDVS() in the OALTimerIntrHandler() function
//
static BOOL OALIoCtlHalSetSystemLevel(
        UINT32 dwIoControlCode, VOID *lpInBuf, UINT32 nInBufSize,
        VOID *lpOutBuf, UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
    DWORD dwErr = 0;
    DWORD dwTargetLevel;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("++OALIoCtlHalSetSystemLevel()\r\n")));

    if (lpBytesReturned)
    {
        *lpBytesReturned = 0;
    }

    if (lpInBuf == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else if (sizeof(DWORD) > nInBufSize)
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    }
    else
    {
        __try
        {
            dwTargetLevel = *((PDWORD)lpInBuf);

            if (dwTargetLevel >= SYS_LEVEL_MAX)
            {
                dwErr = ERROR_INVALID_PARAMETER;
            }
            else
            {
                ChangeDVSLevel((SYSTEM_ACTIVE_LEVEL)dwTargetLevel);
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr)
    {
        OALMSG(OAL_ERROR, (TEXT("[OAL:ERR] OALIoCtlHalSetSystemLevel() Failed dwErr = 0x%08x\r\n"), dwErr));
        NKSetLastError(dwErr);
    }

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("++OALIoCtlHalSetSystemLevel()\r\n")));

    return !dwErr;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalProfileDVS
//
//  For Profiling DVS transition and CPU idle/active rate
//  NOTE!!! Enable #define DVS_LEVEL_PROFILE in DVS.c
//
static BOOL OALIoCtlHalProfileDVS(
        UINT32 dwIoControlCode, VOID *lpInBuf, UINT32 nInBufSize,
        VOID *lpOutBuf, UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
    DWORD dwErr = 0;
    DWORD dwProfileOnOff;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("++OALIoCtlHalProfileDVS()\r\n")));

    if (lpBytesReturned)
    {
        *lpBytesReturned = 0;
    }

    if (lpInBuf == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else if (sizeof(DWORD) > nInBufSize)
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    } 
    else
    {
        __try
        {
            dwProfileOnOff = *((PDWORD)lpInBuf);

            if (dwProfileOnOff == 0)
            {
                ProfileDVSOnOff(FALSE);
            }
            else
            {
                ProfileDVSOnOff(TRUE);
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr)
    {
        OALMSG(OAL_ERROR, (TEXT("[OAL:ERR] OALIoCtlHalProfileDVS() Failed dwErr = 0x%08x\r\n"), dwErr));
        NKSetLastError(dwErr);
    }

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("++OALIoCtlHalProfileDVS()\r\n")));

    return !dwErr;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalClockInfo
//
//  For Get Current Clock Information from register
//
//  lpInBuf : NULL
//  lpInBufSize : Don't care
//  lpOutBuf : struct ClockInfo{}
//  lpOutBuf : sizeof(struct ClockInfo{})
static BOOL OALIoCtlHalClockInfo(
        UINT32 dwIoControlCode, VOID *lpInBuf, UINT32 nInBufSize,
        VOID *lpOutBuf, UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
    DWORD dwErr = 0;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("++OALIoCtlHalClockInfo()\r\n")));

    if (lpBytesReturned)
    {
        *lpBytesReturned = 0;
    }

    if (lpOutBuf == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else if (sizeof(ClockInfo) > nOutBufSize)
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    }
    else
    {
        __try
        {
            FillClockInfo(lpOutBuf);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr)
    {
        OALMSG(OAL_ERROR, (TEXT("[OAL:ERR] OALIoCtlHalClockInfo() Failed dwErr = 0x%08x\r\n"), dwErr));
        NKSetLastError(dwErr);
    }

    OALMSG(OAL_IOCTL&&OAL_FUNC, (TEXT("++OALIoCtlHalClockInfo()\r\n")));

    return !dwErr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRegistry
//
static BOOL OALIoCtlHalInitRegistry( 
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize) 
{
    KITLIoctl(IOCTL_HAL_INITREGISTRY, NULL, 0, NULL, 0, NULL);
    return TRUE;
}

//------------------------------------------------------------------------------
// Function: OALIoCtlHalGetPowerDisposition
//
// IOCTL_HAL_GET_POWER_DISPOSITION IOCTL Handler
//

BOOL OALIoCtlHalGetPowerDisposition(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    if ((!pOutBuffer) || (outSize<sizeof(DWORD)))
    {
        OALMSG(TRUE, (L"***Error: Incorrect use of IOCTL_HAL_GET_POWER_DISPOSITION.\r\n"));
        return FALSE;
    }

    /* see pkfuncs.h for meaning of this value */
    *((DWORD *)pOutBuffer) = (DWORD)POWER_DISPOSITION_SUSPENDRESUME_MANUAL;
    if (pOutSize)
        *pOutSize = sizeof(DWORD);
    
    return(TRUE);
}


//------------------------------------------------------------------------------
//
//  Global: g_oalIoCtlTable[]
//
//  IOCTL handler table. This table includes the IOCTL code/handler pairs
//  defined in the IOCTL configuration file. This global array is exported
//  via oal_ioctl.h and is used by the OAL IOCTL component.
//
const OAL_IOCTL_HANDLER g_oalIoCtlTable[] =
{
#include "ioctl_tab.h"
};

//------------------------------------------------------------------------------

