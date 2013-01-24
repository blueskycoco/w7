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


#ifndef __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPVC1DECODE_H__
#define __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPVC1DECODE_H__



typedef struct
{
    int width;
    int height;
} SSBSIP_VC1_STREAM_INFO;



typedef unsigned int    VC1_DEC_CONF;

#define VC1_DEC_GETCONF_STREAMINFO            0x00001001
#define VC1_DEC_GETCONF_PHYADDR_FRAM_BUF    0x00001002

#define VC1_DEC_SETCONF_POST_ROTATE            0x00002001


#ifdef __cplusplus
extern "C" {
#endif


void *SsbSipVC1DecodeInit();
int   SsbSipVC1DecodeExe(void *openHandle, long lengthBufFill);
int   SsbSipVC1DecodeDeInit(void *openHandle);

int   SsbSipVC1DecodeSetConfig(void *openHandle, VC1_DEC_CONF conf_type, void *value);
int   SsbSipVC1DecodeGetConfig(void *openHandle, VC1_DEC_CONF conf_type, void *value);


void *SsbSipVC1DecodeGetInBuf(void *openHandle, long size);
void *SsbSipVC1DecodeGetOutBuf(void *openHandle, long *size);



#ifdef __cplusplus
}
#endif


// Error codes
#define SSBSIP_VC1_DEC_RET_OK                        (0)
#define SSBSIP_VC1_DEC_RET_ERR_INVALID_HANDLE        (-1)
#define SSBSIP_VC1_DEC_RET_ERR_INVALID_PARAM        (-2)

#define SSBSIP_VC1_DEC_RET_ERR_CONFIG_FAIL            (-100)
#define SSBSIP_VC1_DEC_RET_ERR_DECODE_FAIL            (-101)
#define SSBSIP_VC1_DEC_RET_ERR_GETCONF_FAIL            (-102)

#endif /* __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPVC1DECODE_H__ */
