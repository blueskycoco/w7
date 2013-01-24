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
//


#include <windows.h>
#include <halether.h>
#include <oal_ethdrv.h>
#include <bsp.h>
#include "loader.h"
#include <usbdbgrndis.h>

#define FROM_BCD(n)    ((((n) >> 4) * 10) + ((n) & 0xf))
#define TO_BCD(n)      ((((n) / 10) << 4) | ((n) % 10))

extern PBOOT_CFG   g_pBootCfg;

// 6410
#define CS8900_Tacs (0x0)   // 0clk
#define CS8900_Tcos (0x4)   // 4clk
#define CS8900_Tacc (0xd)   // 14clk
#define CS8900_Tcoh (0x1)   // 1clk
#define CS8900_Tah  (0x4)   // 4clk
#define CS8900_Tacp (0x6)   // 6clk
#define CS8900_PMC  (0x0)   // normal(1data)

// Function pointers to the support library functions of the currently installed debug ethernet controller.
//
PFN_EDBG_INIT             pfnEDbgInit;
PFN_EDBG_GET_FRAME        pfnEDbgGetFrame;
PFN_EDBG_SEND_FRAME       pfnEDbgSendFrame;


static void InitSROMC_CS8900(void)
{
    volatile S3C6410_SROMCON_REG *s6410SROM = (S3C6410_SROMCON_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_SROMCON, FALSE);

    s6410SROM->SROM_BW = (s6410SROM->SROM_BW & ~(0xF<<4)) |
                            (1<<7)| // nWBE/nBE(for UB/LB) control for Memory Bank1(0=Not using UB/LB, 1=Using UB/LB)
                            (1<<6)| // Wait enable control for Memory Bank1 (0=WAIT disable, 1=WAIT enable)
                            (1<<4); // Data bus width control for Memory Bank1 (0=8-bit, 1=16-bit)

    s6410SROM->SROM_BC1 = ((CS8900_Tacs<<28)+(CS8900_Tcos<<24)+(CS8900_Tacc<<16)+(CS8900_Tcoh<<12)\
                            +(CS8900_Tah<<8)+(CS8900_Tacp<<4)+(CS8900_PMC));
}


/*
    @func   BOOL | InitEthDevice | Initializes the Ethernet device to be used for download.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm
    @xref
*/
BOOL InitEthDevice(PBOOT_CFG pBootCfg)
{
    PBYTE  pBaseIOAddress = NULL;
    UINT32 MemoryBase = 0;
    BOOL bResult = FALSE;

    OALMSG(OAL_FUNC, (TEXT("+InitEthDevice.\r\n")));

    InitSROMC_CS8900();

    // Use the MAC address programmed into flash by the user.
    //
    memcpy(pBSPArgs->kitl.mac, pBootCfg->EdbgAddr.wMAC, 6);

    // Use the CS8900A Ethernet controller for download.
    //
    pfnEDbgInit      = (PFN_EDBG_INIT)CS8900AInit;
    pfnEDbgGetFrame  = (PFN_EDBG_GET_FRAME)CS8900AGetFrame;
    pfnEDbgSendFrame = (PFN_EDBG_SEND_FRAME)CS8900ASendFrame;

    pBaseIOAddress   = (PBYTE)OALPAtoVA(pBSPArgs->kitl.devLoc.LogicalLoc, FALSE);

    MemoryBase       = (UINT32)OALPAtoVA(BSP_BASE_REG_PA_CS8900A_MEMBASE, FALSE);

    // Initialize the Ethernet controller.
    //
    if (!pfnEDbgInit((PBYTE)pBaseIOAddress, MemoryBase, pBSPArgs->kitl.mac))
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: InitEthDevice: Failed to initialize Ethernet controller.\r\n")));
        goto CleanUp;
    }

    // Make sure MAC address has been programmed.
    //
    if (!pBSPArgs->kitl.mac[0] && !pBSPArgs->kitl.mac[1] && !pBSPArgs->kitl.mac[2])
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: InitEthDevice: Invalid MAC address.\r\n")));
        goto CleanUp;
    }

    bResult = TRUE;

CleanUp:

    OALMSG(OAL_FUNC, (TEXT("-InitEthDevice.\r\n")));

    return(bResult);
}


/*
    @func   BOOL | OEMGetRealTime | Returns the current wall-clock time from the RTC.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm
    @xref
*/
static BOOL OEMGetRealTime(LPSYSTEMTIME lpst)
{
    volatile S3C6410_RTC_REG *s6410RTC = (S3C6410_RTC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_RTC, FALSE);

    do
    {
        lpst->wYear         = FROM_BCD(s6410RTC->BCDYEAR) + 2000 ;
        lpst->wMonth        = FROM_BCD(s6410RTC->BCDMON   & 0x1f);
        lpst->wDay          = FROM_BCD(s6410RTC->BCDDATE  & 0x3f);

        lpst->wDayOfWeek    = (s6410RTC->BCDDAY - 1);

        lpst->wHour         = FROM_BCD(s6410RTC->BCDHOUR  & 0x3f);
        lpst->wMinute       = FROM_BCD(s6410RTC->BCDMIN   & 0x7f);
        lpst->wSecond       = FROM_BCD(s6410RTC->BCDSEC   & 0x7f);
        lpst->wMilliseconds = 0;
    }
    while (!(lpst->wSecond));

    return(TRUE);
}


/*
    @func   DWORD | OEMEthGetSecs | Returns a free-running seconds count.
    @rdesc  Number of elapsed seconds since last roll-over.
    @comm
    @xref
*/
DWORD OEMEthGetSecs(void)
{
    SYSTEMTIME sTime;

    OEMGetRealTime(&sTime);
    return((60UL * (60UL * (24UL * (31UL * sTime.wMonth + sTime.wDay) + sTime.wHour) + sTime.wMinute)) + sTime.wSecond);
}


/*
    @func   BOOL | OEMEthGetFrame | Reads data from the Ethernet device.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm
    @xref
*/
BOOL OEMEthGetFrame(PUCHAR pData, PUSHORT pwLength)
{
    switch(g_pBootCfg->BootDevice)
    {
        case BOOT_DEVICE_ETHERNET:
            return(pfnEDbgGetFrame(pData, pwLength));
            break;
            
        case BOOT_DEVICE_USB_RNDIS:
            return(Rndis_RecvFrame(pData, pwLength));
            break;
    }
    return(FALSE);
}


/*
    @func   BOOL | OEMEthSendFrame | Writes data to an Ethernet device.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm
    @xref
*/
BOOL OEMEthSendFrame(PUCHAR pData, DWORD dwLength)
{
    BYTE Retries = 0;

    switch(g_pBootCfg->BootDevice)
    {
        case BOOT_DEVICE_ETHERNET:
            while (Retries++ < 4)
            {
                if (!pfnEDbgSendFrame(pData, dwLength))
                    return(TRUE);

                EdbgOutputDebugString("INFO: OEMEthSendFrame: retrying send (%u)\r\n", Retries);
            }
            break;
            
        case BOOT_DEVICE_USB_RNDIS:
            return(ERROR_SUCCESS == Rndis_SendFrame(pData, dwLength));
            break;
    }

    return(FALSE);
}

