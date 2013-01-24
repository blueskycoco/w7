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
#ifndef __KITL_CFG_H
#define __KITL_CFG_H

//------------------------------------------------------------------------------
#include <bsp.h>
#include <oal_kitl.h>
#include <oal_ethdrv.h>
#include <s3c6410_base_regs.h>
#include <usbdbgser.h>
#include <usbdbgrndis.h>


//------------------------------------------------------------------------------
// KITL Devices
//------------------------------------------------------------------------------

OAL_KITL_ETH_DRIVER g_kitlEthCS8900A    = OAL_ETHDRV_CS8900A;
OAL_KITL_ETH_DRIVER g_kitlEthUsbRndis   = OAL_KITLDRV_USBRNDIS;
OAL_KITL_SERIAL_DRIVER g_kitlUsbSerial  = OAL_KITLDRV_USBSERIAL;


OAL_KITL_DEVICE g_kitlDevices[] = {
    { 
        L"6410Ethernet", Internal, BSP_BASE_REG_PA_CS8900A_IOBASE, 0, OAL_KITL_TYPE_ETH, 
        &g_kitlEthCS8900A
    },
    { 
        L"6410USBSerial", Internal, S3C6410_BASE_REG_PA_USBOTG_LINK, 0, OAL_KITL_TYPE_SERIAL, 
        &g_kitlUsbSerial
    },
    {
        L"6410USBRndis", InterfaceTypeUndefined, S3C6410_BASE_REG_PA_USBOTG_LINK, 0, OAL_KITL_TYPE_ETH,
        &g_kitlEthUsbRndis
    },
    {
        L"6410USBDNW", Internal, S3C6410_BASE_REG_PA_USBOTG_LINK, 0, OAL_KITL_TYPE_SERIAL,
        &g_kitlUsbSerial
    },
    {
        NULL, 0, 0, 0, 0, NULL
    }
};  

#endif

