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
#include "JPGMem.h"
#include "JPGMisc.h"
#include "JPGDriver.h"



/*----------------------------------------------------------------------------
*Function: Phy2VirAddr

*Parameters:         dwContext        :
*Return Value:        True/False
*Implementation Notes: memory mapping from physical addr to virtual addr
-----------------------------------------------------------------------------*/
void *Phy2VirAddr(UINT32 phy_addr, int mem_size)
{
    void *mappedAddr = NULL;
    PHYSICAL_ADDRESS    ioPhysicalBase = {0,0};

    ioPhysicalBase.LowPart = phy_addr;
    mappedAddr = MmMapIoSpace(ioPhysicalBase, mem_size, FALSE);
    if (mappedAddr == NULL)
    {
        RETAILMSG(ZONE_ERROR,(TEXT("[MFC:ERR] %s : Mapping Failed [PA:0x%08x]\n"),__FUNCTION__, phy_addr));
    }

    return mappedAddr;
}

void FreeVirAddr(void * vir_addr, int mem_size)
{
    MmUnmapIoSpace(vir_addr, mem_size);
}


/*----------------------------------------------------------------------------
*Function: JPGMemMapping

*Parameters:         dwContext        :
*Return Value:        True/False
*Implementation Notes: JPG register mapping from physical addr to virtual addr
-----------------------------------------------------------------------------*/
BOOL JPGMemMapping(S3C6410_JPG_CTX *base)
{
    // JPG HOST Register
    base->v_pJPG_REG = (volatile S3C6410_JPG_HOSTIF_REG *)Phy2VirAddr(JPG_REG_BASE_ADDR, sizeof(S3C6410_JPG_HOSTIF_REG));
    if (base->v_pJPG_REG == NULL)
    {
        ERRORMSG(1,(TEXT("DD::v_pJPG_REG: VirtualAlloc failed!\r\n")));
        return FALSE;
    }
    
    return TRUE;
}

void JPGMemFree(S3C6410_JPG_CTX *base)
{
    MmUnmapIoSpace(base->v_pJPG_REG, sizeof(S3C6410_JPG_HOSTIF_REG));
    base->v_pJPG_REG = NULL;
}

/*----------------------------------------------------------------------------
*Function: JPGBuffMapping

*Parameters:         dwContext        :
*Return Value:        True/False
*Implementation Notes: JPG Buffer mapping from physical addr to virtual addr 
-----------------------------------------------------------------------------*/
BOOL JPGBuffMapping(S3C6410_JPG_CTX *base)
{
    base->v_pJPGData_Buff = (volatile UINT8 *)Phy2VirAddr(JPG_DATA_BASE_ADDR, JPG_TOTAL_BUF_SIZE);
    if (base->v_pJPGData_Buff == NULL)
    {
        ERRORMSG(1,(TEXT("DD::v_pJPGData_Buff: VirtualAlloc failed!\r\n")));
        return FALSE;
    }

    return TRUE;
}

void JPGBuffFree(S3C6410_JPG_CTX *base)
{
    MmUnmapIoSpace(base->v_pJPGData_Buff, JPG_TOTAL_BUF_SIZE);
    base->v_pJPGData_Buff = NULL;
}

