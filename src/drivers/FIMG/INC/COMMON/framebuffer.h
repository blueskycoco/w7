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

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <COMMON/pixelFmts.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef void* GLESContext;

typedef struct {
    void*            paddr;
    void*            vaddr;
    void*            vaddrCP; // for Current Process   
    void*            ChunkHandle;
} AddressBase;

typedef void* BufferHandle;
typedef AddressBase  BufferAddress;

//This structure is deprecated and is left here for backward compatibility and internal use. Use GLES2SurfaceData instead.
typedef struct
{
    AddressBase colorAddr;
    AddressBase depthStencilAddr;

    unsigned int width;
    unsigned int height;
    /*
    int       colorFormat;      //must be set to a gl enum
    int       colorType;
    int       depthStencilFormat;   
    */
    PxFmt      nativeColorFormat;
    PxFmt      nativeDepthStencilFormat;
    int       flipped;  //0 is rendering origin is top left, 1 if its bottom left.
} FramebufferData;



#ifdef __cplusplus
}
#endif

#endif //__FRAMEBUFFER_H__
