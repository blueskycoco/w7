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
//------------------------------------------------------------------------------
//
//  File:  bsp_cfg.h
//
//  This file contains system constant specific for SMDK6410 board.
//
#ifndef __BSP_CFG_H
#define __BSP_CFG_H

#include <soc_cfg.h>

//------------------------------------------------------------------------------
//
//  Define:  BSP_DEVICE_PREFIX
//
//  Prefix used to generate device name for bootload/KITL
//
#define BSP_DEVICE_PREFIX       "SMDK6410"        // Device name prefix

//------------------------------------------------------------------------------
// SMDK6410 Display Dimension
//------------------------------------------------------------------------------
#define LCD_MODULE_LTS222       (0)    // Portrait 2.2" QVGA RGB16
#define LCD_MODULE_LTV350       (1)    // Landscape 3.5" QVGA RGB16
#define LCD_MODULE_LTE480       (2)    // Landscape 4.8" WVGA RGB16
#define LCD_MODULE_EMUL48_D1    (3)    // Landscape 4.8" WVGA RGB16 as D1 (720x480)
#define LCD_MODULE_EMUL48_QV    (4)    // Landscape 4.8" WVGA RGB16 as QVGA (320x240)
#define LCD_MODULE_EMUL48_PQV   (5)    // Landscape 4.8" WVGA RGB16 as PQVGA (240x320)
#define LCD_MODULE_EMUL48_ML    (6)    // Landscape 4.8" WVGA RGB16 as 480x320
#define LCD_MODULE_EMUL48_MP    (7)    // Landscape 4.8" WVGA RGB16 as 320x480
#define LCD_MODULE_LTP700       (8)    // Landscape 7" WVGA RGB24
#define LCD_MODULE_LTM030DK     (9)     // Portrait 3.5" WVGA RGB16
#define SMDK6410_LCD_MODULE    (LCD_MODULE_LTE480)

#if (SMDK6410_LCD_MODULE == LCD_MODULE_LTS222)
#define LCD_WIDTH            240
#define LCD_HEIGHT            320
#define LCD_BPP                16
#elif (SMDK6410_LCD_MODULE == LCD_MODULE_LTV350)
#define LCD_WIDTH            320
#define LCD_HEIGHT            240
#define LCD_BPP                16
#elif (SMDK6410_LCD_MODULE == LCD_MODULE_LTE480)
#define LCD_WIDTH            800
#define LCD_HEIGHT            480
#define LCD_BPP                16
#elif (SMDK6410_LCD_MODULE == LCD_MODULE_EMUL48_D1)
#define LCD_WIDTH            720
#define LCD_HEIGHT            480
#define LCD_BPP                16
#elif (SMDK6410_LCD_MODULE == LCD_MODULE_EMUL48_QV)
#define LCD_WIDTH            320
#define LCD_HEIGHT            240
#define LCD_BPP                16
#elif (SMDK6410_LCD_MODULE == LCD_MODULE_EMUL48_PQV)
#define LCD_WIDTH            240
#define LCD_HEIGHT            320
#define LCD_BPP                16
#elif (SMDK6410_LCD_MODULE == LCD_MODULE_EMUL48_ML)
#define LCD_WIDTH            480
#define LCD_HEIGHT            320
#define LCD_BPP                16
#elif (SMDK6410_LCD_MODULE == LCD_MODULE_EMUL48_MP)
#define LCD_WIDTH            320
#define LCD_HEIGHT            480
#define LCD_BPP                16
#elif (SMDK6410_LCD_MODULE == LCD_MODULE_LTP700)
#define LCD_WIDTH            800
#define LCD_HEIGHT            480
#define LCD_BPP                32    // rgb888 XRGB
#elif (SMDK6410_LCD_MODULE == LCD_MODULE_LTM030DK)
#define LCD_WIDTH            480
#define LCD_HEIGHT           800
#define LCD_BPP              16     // rgb888 XRGB
#define LCD_TYPE            LCD_TYPE_PORTRAIT
#else
#error LCD_MODULE_UNDEFINED_ERROR
#endif

//------------------------------------------------------------------------------
// SMDK6410 Audio Sampling Rate
//------------------------------------------------------------------------------
#define AUDIO_44_1KHz        (44100)
#define AUDIO_48KHz            (48000)
#define AUDIO_SAMPLE_RATE    (AUDIO_44_1KHz)        // Keep sync with EPLL Fout

//------------------------------------------------------------------------------
// SMDK6410 UART Debug Port Baudrate
//------------------------------------------------------------------------------
#define DEBUG_UART0         (0)
#define DEBUG_UART1         (1)
#define DEBUG_UART2         (2)
#define DEBUG_UART3         (3)
#define DEBUG_BAUDRATE      (115200)

//------------------------------------------------------------------------------
// SMDK6410 NAND Flash Timing Parameter
// Need to optimize for each HClock value.
// Default vaule is 7.7.7. this value has maximum margin. 
// so stable but performance cannot be max.
//------------------------------------------------------------------------------
#define NAND_TACLS         (7)
#define NAND_TWRPH0        (7)
#define NAND_TWRPH1        (7)

//------------------------------------------------------------------------------
// SMDK6410 CF Interface Mode
//------------------------------------------------------------------------------
#define CF_INDIRECT_MODE    (0)        // MemPort0 Shared by EBI
#define CF_DIRECT_MODE      (1)        // Independent CF Interface
#define CF_INTERFACE_MODE   (CF_DIRECT_MODE)

//------------------------------------------------------------------------------
// SMDK6410 Keypad Layout
//------------------------------------------------------------------------------
#define LAYOUT0                (0)        // 8*8 Keypad board
#define LAYOUT1                (1)        // On-Board Key
#define LAYOUT2                (2)        // Qwerty Key board
#define MATRIX_LAYOUT        (LAYOUT1)

#endif // __BSP_CFG_H

