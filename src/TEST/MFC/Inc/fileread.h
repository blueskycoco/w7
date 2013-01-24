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
///   FileRead.h
///
///   Written by Simon Chun (simon.chun@samsung.com)
///   2007/09/13
///

#ifndef __SAMSUNG_SYSLSI_AP_FILE_READ_H__
#define __SAMSUNG_SYSLSI_AP_FILE_READ_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

void *SSB_FILE_OPEN(LPCTSTR filename);
void  SSB_FILE_REWIND(void *p);
void  SSB_FILE_CLOSE(void *p);
int   SSB_FILE_READ(void *p, unsigned char *buf, unsigned int num_read, unsigned int *num_written);

int   SSB_FILE_GET_POS(void *p);
int   SSB_FILE_SET_POS(void *p, int offset, int origin);


#ifdef __cplusplus
}
#endif


#define DATA_FILE       (0)
#define DATA_MEM        (1)
#define DATAREAD_TYPE   (DATA_MEM)


// Possible return value of SSB_FILE_READ function
#define SSB_FILE_EOF    (0x8FFFFFFF)

// Definitions for the 'origin' argument in SSB_FILE_SET_POS function
#define SSB_FILE_POS_BEGIN      (0)
#define SSB_FILE_POS_CURRENT    (1)
#define SSB_FILE_POS_END        (2)


// ������ FILE�� �ƴ� memory buffer�� stream �Է��� ���.. (FRAMEX_IN_TYPE_MEM Ÿ��!)
typedef struct
{
    unsigned char *p_start;
    unsigned char *p_end;

    unsigned char *p_cur;
} MMAP_STRM_PTR;



#endif /* __SAMSUNG_SYSLSI_AP_FILE_READ_H__ */
