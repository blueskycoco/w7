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
#include "LogMsg.h"


void *Phy2Vir_AddrMapping(unsigned int phy_addr, int mem_size, BOOL CacheEnable)
{
    void *mappedAddr = NULL;
    PHYSICAL_ADDRESS    ioPhysicalBase = {0,0};

    ioPhysicalBase.LowPart = phy_addr;
    mappedAddr = MmMapIoSpace(ioPhysicalBase, mem_size, CacheEnable);
    if (mappedAddr == NULL)
    {
        RETAILMSG(ZONE_ERROR,(TEXT("MFC::[MFC:ERR] %s : Mapping Failed [PA:0x%08x]\r\n"),__FUNCTION__, phy_addr));
    }

    return mappedAddr;
}

void *Mem_Alloc(unsigned int size)
{
    void    *alloc_mem;

    alloc_mem = (void *) malloc(size);
    if (alloc_mem == NULL) {
        RETAILMSG(ZONE_ERROR, (TEXT("MFC::Mem_Alloc ::memory allocation failed!.\r\n")));
        return NULL;
    }

    return alloc_mem;
}

void Mem_Free(void *addr)
{
    free(addr);
}

void *Mem_Cpy(void *dst, const void *src, int size)
{
    return memcpy(dst, src, size);
}

int   Mem_Set(void *dst, int value, int size)
{
    memset(dst, value, size);

    return value;
}

int Copy_From_User(void *to, const void *from, unsigned long n)
{
    return (int)memcpy(to, from, n);
}

int Copy_To_User(void *to, const void *from, unsigned long n)
{
    return (int)memcpy(to, from, n);
}
