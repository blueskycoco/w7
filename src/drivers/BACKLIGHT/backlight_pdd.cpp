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
//  File: Backlight_pdd.c
//
//  Backlight PDD driver source code, S3C6410 
//
#include <windows.h>
#include <ceddk.h>
#include <s3c6410.h>
#include <backlightddsi.h>
#include "backlight_pdd.h"


#define PWM0_1_PRESCALER 0x10
#define PWM1_DIVIDER 0x3
#define PWM_TCNTB1 5000
#define PWM_TCMPB1_UNIT 50
#define LOW_BACKLIGHT 5
///=============================================================================
/// Exported functions
///=============================================================================
///     backlight mdd & pdd interface
///         PddInit()
///         PddDeInit()
///         PddIOControl()
///         PddUpdateBacklight()
///
BOOL PddInit(DWORD);
VOID PddDeInit(DWORD);
BOOL PddUpdateBacklight(DWORD, CEDEVICE_POWER_STATE, BOOL, DWORD, DWORD*);
BOOL PddIOControl(DWORD, DWORD, LPBYTE, DWORD, LPBYTE, DWORD, LPDWORD);

//==============================================================================
// Description: PddInit - PDD should always implement this function. MDD calls
//              it during initialization to fill up the function table with rest
//              of the DDSI functions.
//
// Arguments:   [IN] pszActiveKey - current active backlight driver key.
//              [IN] pMddIfc - MDD interface info
//              [OUT] pPddIfc - PDD interface (the function table to be filled)
//              [IN] hKeyToWatch - HKEY of the key watched by the MDD (e.g.
//                       HKEY_CURRENT_USER). This is configurable in the
//                       backlight registry read by the MDD (REG_REGKEY_VALUE).
//              [IN] szSubKey - subkey of hKeyToWatch if applicable (e.g.
//                       L"ControlPanel\\Backlight"). This is configurable in
//                       the backlight registry read by the MDD
//                      (REG_REGSUBKEY_VALUE).
//
// Ret Value:   The PDD context - pddInfo.
//

DWORD
WINAPI
PddInit(
    const LPCTSTR pszActiveKey,
    const BKL_MDD_INTERFACE_INFO* mddIfc,
    BKL_PDD_INTERFACE_INFO* pddIfc,
    const HKEY hKeyToWatch,
    const LPCTSTR szSubKey
    )
{
    BKL_PDD_INFO* pddInfo = NULL;

    pddInfo = (BKL_PDD_INFO*) LocalAlloc(LPTR, sizeof(BKL_PDD_INFO));
    if (!pddInfo)
    {
        //Failed to allocate Pdd structure
        return NULL;
    }

    pddIfc->version = 1;
    pddIfc->pfnDeinit = PddDeInit;
    pddIfc->pfnIoctl = PddIOControl;
    pddIfc->pfnUpdate = PddUpdateBacklight;

    BacklightInit(pddInfo);

    return (DWORD) pddInfo;

}


//==============================================================================
// Description: PddDeInit - MDD calls it during deinitialization. PDD should 
//              deinit hardware and deallocate memory etc.
//
// Arguments:   [IN] pddContext. pddInfo returned in PddInit.
//
// Ret Value:   None
//

VOID
WINAPI
PddDeInit(
    DWORD pddContext
    )
{
    BKL_PDD_INFO* pddInfo = (BKL_PDD_INFO*) pddContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("[BKL_PDD] DeInit\r\n")));
    BL_Set(pddInfo,FALSE);

    if(pddInfo->v_pGPIORegs != NULL)
    {
        MmUnmapIoSpace((PVOID)(pddInfo->v_pGPIORegs), sizeof(S3C6410_GPIO_REG));
        pddInfo->v_pGPIORegs = NULL;
    }
    if(pddInfo->v_pPWMRegs != NULL)
    {
        MmUnmapIoSpace((PVOID)(pddInfo->v_pPWMRegs), sizeof(S3C6410_PWM_REG));
        pddInfo->v_pPWMRegs = NULL;
    }

    if (pddInfo)
    {
        LocalFree(pddInfo);
    }
}

//==============================================================================
// Description: PddIOControl - IOCTLs passed to PDD.
//
// Arguments:   [IN] pddContext.  pddInfo returned in PddInit.
//
// Ret Value:   TRUE if success else FALSE. SetLastError() if FALSE.
//

BOOL
WINAPI
PddIOControl(
    DWORD pddContext,
    DWORD dwIoControlCode,
    LPBYTE lpInBuf,
    DWORD nInBufSize,
    LPBYTE lpOutBuf,
    DWORD nOutBufSize, 
    LPDWORD lpBytesReturned
    )
{
    BKL_PDD_INFO* pddInfo = (BKL_PDD_INFO*) pddContext;
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

//==============================================================================
// Description: PddUpdateBacklight - Update the current backlight state.
//
// Arguments:   [IN]  pddContext.  pddInfo returned in PddInit.
//              [IN]  powerState - current power state (D0...D4).
//              [IN]  fOnAC - whether the device on external power (TRUE if on
//                       external power else FALSE).
//              [IN]  dwBrightnessToSet - requested brightness to set.
//              [OUT] dwActualBrightness - actual brightness of backlight upon
//                       return.
//
// Ret Value:   TRUE if success else FALSE.
//

BOOL
PddUpdateBacklight(
    DWORD pddContext,
    CEDEVICE_POWER_STATE powerState,
    BOOL fOnAC,
    DWORD dwBrightnessToSet,
    DWORD* dwActualBrightness
    )
{
    BOOL fOk = TRUE;
    BKL_PDD_INFO* pddInfo = (BKL_PDD_INFO*) pddContext;

    //  Enable backlight for D0-D1; otherwise disable
    switch( powerState )
    {
     case D0:
         BL_InitPWM(pddInfo);
         BL_SetBrightness(pddInfo,dwBrightnessToSet);
         BL_Set(pddInfo,TRUE);
        *dwActualBrightness = dwBrightnessToSet;
    break;
     case D1:
         BL_SetBrightness(pddInfo,LOW_BACKLIGHT);
        *dwActualBrightness = LOW_BACKLIGHT;
             break;
     case D2:
     case D3:
     case D4:
             //  Disable backlight
             BL_Set(pddInfo,FALSE);
            *dwActualBrightness = 0;
             break;
     default:
         DEBUGMSG(ZONE_ERROR, (L"+BackLightSetState - Unsupported power state!\r\n"));
         fOk=FALSE;
              break;
    }

    return fOk;
}

//-----------------------------------------------------------------------------
//  Initialize PWM
//  Will be called during initialistion sequence and  while updating backlight for D0 state.
//
extern "C"
void BL_InitPWM(BKL_PDD_INFO* pddInfo)
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("[BKL_PDD]BL_InitPWM Enter\r\n")));
    pddInfo->v_pPWMRegs->TCFG0 = pddInfo->v_pPWMRegs->TCFG0 & (~(0xff<<0)|(PWM0_1_PRESCALER<<0));
    pddInfo->v_pPWMRegs->TCFG1 = pddInfo->v_pPWMRegs->TCFG1&(~(0xf<<4))|(PWM1_DIVIDER<<4);     //Timer1 Devider set 
    pddInfo->v_pPWMRegs->TCON = pddInfo->v_pPWMRegs->TCON&(~(1<<11))|(1<<11);        // enable Timer1 auto reload 
    pddInfo->v_pPWMRegs->TCON = pddInfo->v_pPWMRegs->TCON&(~(1<<9))|(1<<9);     //Timer1 manual update clear (TCNTB1 & TCMPB1)
}


//
// turn on/off the backlight
//
extern "C"
void BL_Set(BKL_PDD_INFO* pddInfo,BOOL bOn)
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("[BKL_PDD]BL_Set(%d)\r\n"),(int)bOn));

    if(bOn) 
    {
        pddInfo->v_pGPIORegs->GPFDAT = pddInfo->v_pGPIORegs->GPFDAT&(~(1<<15))|(1<<15); //Back Light power on
        pddInfo->v_pGPIORegs->GPFCON = pddInfo->v_pGPIORegs->GPFCON&(~(3<<30))|(2<<30); //GPF15=PWM TOUT1
    }
    else 
    {
        pddInfo->v_pGPIORegs->GPFDAT = pddInfo->v_pGPIORegs->GPFDAT&(~(1<<15))|(0<<15); //Back Light power off
        pddInfo->v_pGPIORegs->GPFCON = pddInfo->v_pGPIORegs->GPFCON&(~(3<<30))|(1<<30);  // GPF15 = OUTPUT  
    }
}

void BL_SetBrightness(BKL_PDD_INFO* pddInfo,DWORD dwValue)
{
    UINT32 u32BrightnessSet=0;
    u32BrightnessSet = PWM_TCMPB1_UNIT*dwValue;
    if(u32BrightnessSet<0)
        u32BrightnessSet=0;
    else if(u32BrightnessSet> (PWM_TCNTB1-1) )
        u32BrightnessSet= (PWM_TCNTB1-1);

    pddInfo->v_pPWMRegs->TCNTB1 = PWM_TCNTB1;
    pddInfo->v_pPWMRegs->TCMPB1 =  u32BrightnessSet;
    DEBUGMSG(ZONE_FUNCTION,(TEXT("[BKL_PDD] BacklightRegChanged: BrightNess=%d TCMPB1=%d\r\n"), dwValue, pddInfo->v_pPWMRegs->TCMPB1));

    pddInfo->v_pPWMRegs->TCON = pddInfo->v_pPWMRegs->TCON&(~(1<<8))|(1<<8);    //Timer1 start
    pddInfo->v_pPWMRegs->TCON = pddInfo->v_pPWMRegs->TCON&(~(1<<9))|(0<<9);    //Timer1 manual update set (TCNTB1 & TCMPB1)

}

extern "C"
DWORD BacklightInit(BKL_PDD_INFO* pddInfo)
{

    BOOL bRet = TRUE;
    PHYSICAL_ADDRESS    ioPhysicalBase = {0,0};

    DEBUGMSG(ZONE_FUNCTION, (TEXT("[BKL_PDD] Init\r\n")));

    ioPhysicalBase.LowPart = S3C6410_BASE_REG_PA_GPIO;
    pddInfo->v_pGPIORegs = (volatile S3C6410_GPIO_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C6410_GPIO_REG), FALSE);
    if (pddInfo->v_pGPIORegs == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("[BKL_PDD] v_pGPIORegs MmMapIoSpace() Failed \n\r")));
        return FALSE;
    }

    ioPhysicalBase.LowPart = S3C6410_BASE_REG_PA_PWM;
    pddInfo->v_pPWMRegs = (volatile S3C6410_PWM_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C6410_PWM_REG), FALSE);
    if (pddInfo->v_pPWMRegs == NULL)
    {
      DEBUGMSG(ZONE_ERROR, (TEXT("[BKL_PDD] v_pPWMRegs MmMapIoSpace() Failed \n\r")));
      return FALSE;
    }

    if (pddInfo->v_pGPIORegs)
    {
      pddInfo->v_pGPIORegs->GPFPUD = pddInfo->v_pGPIORegs->GPFPUD&(~(3<<30)); //GPF15 Pull-up/down disabled
    }

    BL_InitPWM(pddInfo);
    BL_Set(pddInfo,TRUE);
//    *pDeviceState = D0;

    return TRUE;
}
