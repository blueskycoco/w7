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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//
// -----------------------------------------------------------------------------
#ifndef _WM8580_H_
#define _WM8580_H_


typedef enum
{
    WM8580_PLLA1                        = 0x00,
    WM8580_PLLA2                        = 0x01,
    WM8580_PLLA3                        = 0x02,
    WM8580_PLLA4                        = 0x03,
    WM8580_PLLB1                        = 0x04,
    WM8580_PLLB2                        = 0x05,
    WM8580_PLLB3                        = 0x06,
    WM8580_PLLB4                        = 0x07,
    WM8580_CLKSEL                       = 0x08,
    WM8580_PAIF1                        = 0x09,
    WM8580_PAIF2                        = 0x0A,
    WM8580_SAIF1                        = 0x0B,
    WM8580_PAIF3                        = 0x0C,
    WM8580_PAIF4                        = 0x0D,
    WM8580_SAIF2                        = 0x0E,
    WM8580_DAC_CONTROL1                 = 0x0F,
    WM8580_DAC_CONTROL2                 = 0x10,
    WM8580_DAC_CONTROL3                 = 0x11,
    WM8580_DAC_CONTROL4                 = 0x12,
    WM8580_DAC_CONTROL5                 = 0x13,
    WM8580_DIGITAL_ATTENUATION_DACL1    = 0x14,
    WM8580_DIGITAL_ATTENUATION_DACR1    = 0x15,
    WM8580_DIGITAL_ATTENUATION_DACL2    = 0x16,
    WM8580_DIGITAL_ATTENUATION_DACR2    = 0x17,
    WM8580_DIGITAL_ATTENUATION_DACL3    = 0x18,
    WM8580_DIGITAL_ATTENUATION_DACR3    = 0x19,
    WM8580_MASTER_DIGITAL_ATTENUATION   = 0x1C,
    WM8580_ADC_CONTROL1                 = 0x1D,
    WM8580_SPDTXCHAN0                   = 0x1E,
    WM8580_SPDTXCHAN1                   = 0x1F,
    WM8580_SPDTXCHAN2                   = 0x20,
    WM8580_SPDTXCHAN3                   = 0x21,
    WM8580_SPDTXCHAN4                   = 0x22,
    WM8580_SPDTXCHAN5                   = 0x23,
    WM8580_SPDMODE                      = 0x24,
    WM8580_INTMASK                      = 0x25,
    WM8580_GPO1                         = 0x26,
    WM8580_GPO2                         = 0x27,
    WM8580_GPO3                         = 0x28,
    WM8580_GPO4                         = 0x29,
    WM8580_GPO5                         = 0x2A,
    WM8580_INTSTAT                      = 0x2B,
    WM8580_SPDRXCHAN1                   = 0x2C,
    WM8580_SPDRXCHAN2                   = 0x2D,
    WM8580_SPDRXCHAN3                   = 0x2E,
    WM8580_SPDRXCHAN4                   = 0x2F,
    WM8580_SPDRXCHAN5                   = 0x30,
    WM8580_SPDSTAT                      = 0x31,
    WM8580_PWRDN1                       = 0x32,
    WM8580_PWRDN2                       = 0x33,
    WM8580_READBACK                     = 0x34,
    WM8580_RESET                        = 0x35,
}  WM8580_RegAddr;

/* Powerdown Register 1 (register 32h) */
#define WM8580_PWRDN1_PWDN_ALL_ACTIVE      0<<0
#define WM8580_PWRDN1_PWDN_ALL_MUTE        1<<0
#define WM8580_PWRDN1_ADCPD_ENABLE         0<<1
#define WM8580_PWRDN1_ADCPD_DISABLE        1<<1
#define WM8580_PWRDN1_DACPD_0_ENABLE       0<<2
#define WM8580_PWRDN1_DACPD_0_DISABLE      1<<2
#define WM8580_PWRDN1_DACPD_1_ENABLE       0<<3
#define WM8580_PWRDN1_DACPD_1_DISABLE      1<<3
#define WM8580_PWRDN1_DACPD_2_ENABLE       0<<4
#define WM8580_PWRDN1_DACPD_2_DISABLE      1<<4
#define WM8580_PWRDN1_DACPD_ALL_ENABLE     0<<6
#define WM8580_PWRDN1_DACPD_ALL_DISABLE    1<<6

#define WM8580_ALL_POWER_ON                0x000

// WM8580 Codec-chip Initialization Value 
unsigned int WM8580_Codec_Init_Table[][2] =
{
#if 1
    { 0x35, 0x000 },    // R53
    { 0x32, 0x000 },    // R50
    { 0x1A, 0x0FF },    // R26
    { 0x1B, 0x0FF },    // R27
    { 0x1C, 0x1F0 },    // R28
    { 0x08, 0x01C },    // R08
    { 0x0C, 0x182 },    // R12
    { 0x0D, 0x082 },    // R13
#endif
};


#endif // _WM8580_H_