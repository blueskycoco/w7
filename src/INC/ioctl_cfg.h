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
//------------------------------------------------------------------------------
//
//  File:  ioctl_cfg.h
//
//  Configuration file for the IOCTL component.
//
#ifndef __IOCTL_CFG_H
#define __IOCTL_CFG_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//  RESTRICTION
//
//  This file is a configuration file for the IOCTL component.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_PLATFORM_TYPE/OEM
//
//  Defines the platform type and OEM string.
//
#define IOCTL_PLATFORM_TYPE                 (L"SMDK6410")
#define IOCTL_PLATFORM_OEM                  (L"Samsung SMDK6410")
#define IOCTL_PLATFORM_MANUFACTURER         (L"Samsung Electronics")
#define IOCTL_PLATFORM_NAME                 (L"SMDK6410")

//------------------------------------------------------------------------------
//  Define:  IOCTL_PROCESSOR_VENDOR/NAME/CORE
//
//  Defines the processor information
//

#define IOCTL_PROCESSOR_VENDOR      (L"Samsung Electronics")
#define IOCTL_PROCESSOR_NAME        (L"S3C6410")
#define IOCTL_PROCESSOR_CORE        (L"ARM1176JZF-S")

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_PROCESSOR_INSTRUCTION_SET
//
//  Defines the processor instruction set information
//
#define IOCTL_PROCESSOR_INSTRUCTION_SET    (0)
#define IOCTL_PROCESSOR_CLOCK_SPEED        S3C6410_ACLK

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_HAL_GET_CPUID
//
//  Defines the processor identification code request
//
#define IOCTL_HAL_GET_CPUID                 CTL_CODE(FILE_DEVICE_UNKNOWN, 2100, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_SET_SYSTEM_LEVEL          CTL_CODE(FILE_DEVICE_UNKNOWN, 2101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_PROFILE_DVS               CTL_CODE(FILE_DEVICE_UNKNOWN, 2102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_CLOCK_INFO                CTL_CODE(FILE_DEVICE_UNKNOWN, 2103, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL OALIoCtlHalGetCPUID(
    UINT32 code, void *pInBuffer, UINT32 inSize, void *pOutBuffer,
    UINT32 outSize, UINT32 *lpBytesReturned );

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
