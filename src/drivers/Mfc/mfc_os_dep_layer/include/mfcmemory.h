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


#ifndef __SAMSUNG_SYSLSI_APDEV_MFC_MEMORY_H__
#define __SAMSUNG_SYSLSI_APDEV_MFC_MEMORY_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif


void *Phy2Vir_AddrMapping(unsigned int phy_addr, int mem_size, BOOL CacheEnable);
void *Mem_Alloc(unsigned int size);
void Mem_Free(void *addr);
void *Mem_Cpy(void *dst, const void *src, int size);
void *Mem_Cpy(void *dst, const void *src, int size);
int   Mem_Set(void *dst, int value, int size);
int Copy_From_User(void *to, const void *from, unsigned long n);
int Copy_To_User(void *to, const void *from, unsigned long n);
unsigned int get_fb0_addr(void);
unsigned int get_fb1_addr(void);


#ifdef __cplusplus
}
#endif

#endif /* __SAMSUNG_SYSLSI_APDEV_MFC_MEMORY_H__ */
