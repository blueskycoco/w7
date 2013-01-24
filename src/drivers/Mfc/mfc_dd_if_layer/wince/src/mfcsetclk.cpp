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


#include <windows.h>
#include <s3c6410.h>
#include "LogMsg.h"

static BOOL mfc_set_MFC_CLKDIV0(int mfc_ratio)
{
    volatile S3C6410_SYSCON_REG * pSysConReg = NULL;
    PHYSICAL_ADDRESS    ioPhysicalBase = {0,0};

    ioPhysicalBase.LowPart = S3C6410_BASE_REG_PA_SYSCON;
    pSysConReg = (S3C6410_SYSCON_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C6410_SYSCON_REG), FALSE);
    if (pSysConReg == NULL)
    {
        RETAILMSG(ZONE_ERROR, (TEXT("MFC::[MFC Driver] MFC_Init() : g_pSysConReg MmMapIoSpace() Failed\r\n")));        
        return FALSE;
    }

    pSysConReg->CLK_DIV0 = (pSysConReg->CLK_DIV0 & ~(0xF << 28)) | (mfc_ratio << 28);

    MmUnmapIoSpace((PVOID)pSysConReg, sizeof(S3C6410_SYSCON_REG));
    pSysConReg = NULL;


    return TRUE;
}

BOOL Mfc_Set_ClkDiv(int divider)
{
    if ((divider < 1) || (divider > 16)) {
        RETAILMSG(ZONE_FUNCTION, (TEXT("MFC::[[MFC Driver] Mfc_Set_ClkDiv() : MFC clock divider must be 1 ~ 16.\r\n")));                
        return FALSE;
    }


    mfc_set_MFC_CLKDIV0(divider - 1);

    return TRUE;
}

