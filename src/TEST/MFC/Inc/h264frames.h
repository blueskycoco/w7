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
/////
///   H264Frames.h
///
///   Written by Simon Chun (simon.chun@samsung.com)
///   2007/03/13
///

#ifndef __SAMSUNG_SYSLSI_AP_H264_FRAMES_H__
#define __SAMSUNG_SYSLSI_AP_H264_FRAMES_H__


typedef struct
{
    int  width, height;

} H264_CONFIG_DATA;


#define H264_CODING_TYPE_I      7   // I-slice
#define H264_CODING_TYPE_P      5   // P-slice


#ifdef __cplusplus
extern "C" {
#endif

int ExtractConfigStreamH264(FRAMEX_CTX  *pFrameExCtx, void *fp, unsigned char buf[], int buf_size, H264_CONFIG_DATA *conf_data);
int NextFrameH264(FRAMEX_CTX  *pFrameExCtx, void *fp, unsigned char buf[], int buf_size, unsigned int *coding_type);

#ifdef __cplusplus
}
#endif




#endif /* __SAMSUNG_SYSLSI_AP_H264_FRAMES_H__ */
