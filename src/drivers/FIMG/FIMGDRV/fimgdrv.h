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


#ifndef __FIMGDRV_H__
#define __FIMGDRV_H__

typedef struct _ALLOCMEM_ITEM {
    void*                   phyAddr;
    void*                   virAddr;
    void*                   virAddrCP;    // mapped for caller 
    void*           memPool;
    struct _ALLOCMEM_ITEM   *next;
} ALLOCMEM_ITEM;

typedef struct _FIMG_CONTEXT {
    ALLOCMEM_ITEM   *allocated_list;
    DWORD       reserved[3];
} FIMG_CONTEXT;

#endif /*__FIMGDRV_H__*/

