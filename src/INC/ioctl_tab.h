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
//
//------------------------------------------------------------------------------
//
//  File:  ioctl_tab.h
//
//  Configuration file for the OAL IOCTL component.
//
//  This file is included by the platform's ioctl.c file and defines the
//  global IOCTL table, g_oalIoCtlTable[]. Therefore, this file may ONLY
//  define OAL_IOCTL_HANDLER entries.
//
// IOCTL CODE,                          Flags   Handler Function
//------------------------------------------------------------------------------

// Use common OAL IOCTL table
#include <oal_ioctl_tab.h>

// Add platform specific OAL IOCTLs
{ IOCTL_HAL_GET_HWENTROPY,                  0,    OALIoCtlHalGetHWEntropy           },
{ IOCTL_HAL_GET_HIVE_CLEAN_FLAG,            0,    OALIoCtlHalGetHiveCleanFlag       },
{ IOCTL_HAL_QUERY_FORMAT_PARTITION,         0,    OALIoCtlHalQueryFormatPartition   },
{ IOCTL_HAL_QUERY_DISPLAYSETTINGS,          0,    OALIoCtlHalQueryDisplaySettings   },
{ IOCTL_HAL_GET_CPUID,                      0,    OALIoCtlHalGetCPUID               },
{ IOCTL_HAL_SET_SYSTEM_LEVEL,               0,    OALIoCtlHalSetSystemLevel         },
{ IOCTL_HAL_PROFILE_DVS,                    0,    OALIoCtlHalProfileDVS             },
{ IOCTL_HAL_CLOCK_INFO,                     0,    OALIoCtlHalClockInfo              },
{ IOCTL_HAL_GET_POWER_DISPOSITION,          0,    OALIoCtlHalGetPowerDisposition    },

// Required Termination
{ 0,                                        0,    NULL    }

//------------------------------------------------------------------------------
