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


#ifndef __CMM_DRIVER_H__
#define __CMM_DRIVER_H__

#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include "image_cfg.h"
#include "CMMAPI.h"

#define MAX_INSTANCE_NUM    10
#define CODEC_MEM_START            (IMAGE_CMM_BUFFER_PA_START)
#define CODEC_MEM_SIZE            (IMAGE_CMM_BUFFER_SIZE)


typedef struct tagCODEC_MEM_CTX
{
    UINT8            inst_no;
    HANDLE            callerProcess;
}CODEC_MEM_CTX;

typedef struct tagALLOC_MEM_T{
    struct tagALLOC_MEM_T    *prev;
    struct tagALLOC_MEM_T    *next;
    union{
        DWORD                cached_p_addr;  // physical address
        PHYSICAL_ADDRESS    uncached_p_addr;  // physical address
    };
    UINT8                    *v_addr;  // virtual address in cached area
    UINT8                    *u_addr;  // copyed virtual address for user mode process
    UINT32                    size;       // memory size    
    UINT8                    inst_no;
    char                    cacheFlag;

} ALLOC_MEM_T;
    
typedef struct tagFREE_MEM_T
{
    struct tagFREE_MEM_T        *prev;
    struct tagFREE_MEM_T        *next;
    DWORD                    startAddr;
    DWORD                    size;
}FREE_MEM_T;

    
#if __cplusplus
extern "C" {
#endif

void CleanInvalidateCacheRange(PBYTE StartAddress, PBYTE EndAddress);
void CleanCacheRange(PBYTE StartAddress, PBYTE EndAddress);
void InvalidateCacheRange(PBYTE StartAddress, PBYTE EndAddress);
static void InsertNodeToAllocList(ALLOC_MEM_T *node,  UINT8 inst_no);
static void InsertNodeToFreeList(FREE_MEM_T *node,  UINT8 inst_no);
static void DeleteNodeFromAllocList(ALLOC_MEM_T *node,  UINT8 inst_no);
static void DeleteNodeFromFreeList( FREE_MEM_T *node,  UINT8 inst_no);
static DWORD GetMemArea(UINT32 allocSize, UINT8 inst_no);
static ALLOC_MEM_T * GetCodecVirAddr(UINT8 inst_no, CMM_ALLOC_PRAM_T *in_param);
static void FreeCodecBuff(UINT8 *c_addr);
static void MergeFragmentation(UINT8 inst_no);
static void ReleaseAllocMem(ALLOC_MEM_T *node, CODEC_MEM_CTX *CodecMem);
static UINT8 GetInstanceNo();
static void ReturnInstanceNo(UINT8 inst_no);
static void PrintList();
#if __cplusplus
}
#endif





#endif /*__CMM_DRIVER_H__*/
