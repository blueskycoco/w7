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
#ifndef __CMM_API_H__
#define __CMM_API_H__

#include <windows.h>

#if __cplusplus
extern "C" {
#endif

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define IOCTL_CODEC_MEM_ALLOC                    CTL_CODE( 0, 0x820, 0, 0 )
#define IOCTL_CODEC_MEM_FREE                    CTL_CODE( 0, 0x821, 0, 0 )
#define IOCTL_CODEC_CACHE_FLUSH                CTL_CODE( 0, 0x822, 0, 0 ) // same as IOCTL_CODEC_CACHE_CLEAN_INVALIDATE
#define IOCTL_CODEC_GET_PHY_ADDR                CTL_CODE( 0, 0x823, 0, 0 )
#define IOCTL_CODEC_CACHE_INVALIDATE            CTL_CODE( 0, 0x824, 0, 0 )
#define IOCTL_CODEC_CACHE_CLEAN                CTL_CODE( 0, 0x825, 0, 0 )
#define IOCTL_CODEC_CACHE_CLEAN_INVALIDATE    CTL_CODE( 0, 0x826, 0, 0 )

#define CODEC_MEM_DRIVER_NAME    L"CMM1:"

typedef struct tagCMM_ALLOC_PRAM_T{
    char                    cacheFlag;
    unsigned int            size;       // memory size
}CMM_ALLOC_PRAM_T;

#if __cplusplus
}
#endif

#endif //__CMM_API_H__