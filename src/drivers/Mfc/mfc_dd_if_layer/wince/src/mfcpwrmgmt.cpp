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



#include <bsp.h>
#include "pmplatform.h"
#include "LogMsg.h"

static BOOL mfc_power_on_off(int on_off)
{
    DWORD    dwIPIndex = PWR_IP_MFC;
    DWORD    dwIOCTL;

    HANDLE   hPwrControl;

    hPwrControl = CreateFile( L"PWC0:",
                              GENERIC_READ|GENERIC_WRITE,
                              FILE_SHARE_READ|FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              0, 0);
    if (INVALID_HANDLE_VALUE == hPwrControl )
    {
        RETAILMSG(ZONE_ERROR, (TEXT("MFC::PWC0 Open Device Failed..\r\n"))); 
        return FALSE;
    }

    if (on_off == 0)
        dwIOCTL = IOCTL_PWRCON_SET_POWER_OFF;
    else
        dwIOCTL = IOCTL_PWRCON_SET_POWER_ON;

    if ( !DeviceIoControl(hPwrControl, dwIOCTL, &dwIPIndex, sizeof(DWORD), NULL, 0, NULL, NULL) )
    {
        RETAILMSG(ZONE_ERROR, (TEXT("MFC::MFC Power(%d) : MFC Power On/Off Failed.\r\n")));         
        CloseHandle(hPwrControl);
        return FALSE;
    }

    CloseHandle(hPwrControl);

    return TRUE;
}


static BOOL mfc_clock_on_off(int on_off)
{
    static volatile S3C6410_SYSCON_REG * pSysConReg = NULL;
    PHYSICAL_ADDRESS    ioPhysicalBase = {0,0};

    // SYSCON_REG register address mapping
    //   Once it is mapped, it keeps the mapping until the OS is rebooted.
    if (pSysConReg == NULL) {
        ioPhysicalBase.LowPart = S3C6410_BASE_REG_PA_SYSCON;
        pSysConReg = (S3C6410_SYSCON_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C6410_SYSCON_REG), FALSE);
        if (pSysConReg == NULL)
        {
            RETAILMSG(ZONE_ERROR, (TEXT("MFC::[[MFC Driver] MFC_Init() : g_pSysConReg MmMapIoSpace() Failed\r\n"))); 
            return FALSE;
        }
    }

    if (on_off == 0) {
        pSysConReg->PCLK_GATE &= ~(1<<0);    // MFC
        pSysConReg->SCLK_GATE &= ~(1<<3);    // MFC
    }
    else {
        pSysConReg->HCLK_GATE |= (1<<0);    // MFC
        pSysConReg->PCLK_GATE |= (1<<0);    // MFC
        pSysConReg->SCLK_GATE |= (1<<3);    // MFC
    }

    return TRUE;
}



BOOL Mfc_Pwr_On()
{
    RETAILMSG(ZONE_FUNCTION, (TEXT("MFC::[MFC POWER] MFC_POWER_ON.\r\n")));

    if (mfc_power_on_off(1) == FALSE)
        return FALSE;

    RETAILMSG(ZONE_FUNCTION, (TEXT("MFC::[MFC POWER] Power is up.\r\n")));

    return TRUE;
}



BOOL Mfc_Pwr_Off()
{
    RETAILMSG(ZONE_FUNCTION, (TEXT("MFC::[MFC POWER] MFC_POWER_OFF.\r\n")));    

    if (mfc_power_on_off(0) == FALSE)
        return FALSE;

    RETAILMSG(ZONE_FUNCTION, (TEXT("MFC::[MFC POWER] Power is down\r\n")));

    return TRUE;
}


BOOL Mfc_Clk_On()
{
    return mfc_clock_on_off(1);
}



BOOL Mfc_Clk_Off()
{
    return mfc_clock_on_off(0);
}

