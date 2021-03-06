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

Module Name:    s5x532.h

Abstract:       S5K4BA Camera module setting binary microcode sequence

Functions:


Notes:


--*/

#ifndef _S5X532_H_
#define _S5X532_H_

unsigned char S5K4BA_YCbCr8bit[][2] = 
{
//==========================================================
//    CAMERA INITIAL (Analog & Clock Setting)
//==========================================================
    {0xfc, 0x07},
    {0x66, 0x01},// WDT
    {0xfc, 0x00},
    {0x00, 0xaa},// For EDS Check
    {0x21, 0x03},// peter0223 added
    
    {0xfc, 0x01},
    {0x04, 0x01},// ARM Clock Divider
    
    {0xfc, 0x02},// Analog setting   
    {0x55, 0x1e},// LineADLC on(s551a), off(s550a)
    {0x56, 0x10},// BPR 16code
    {0x30, 0x82},// Analog offset (capture =?h)
    {0x37, 0x25},// Global Gain (default:31)
    
    {0x57, 0x80},// // LineADLC Roffset
    {0x58, 0x80},//89    //90  // LineADLC Goffset
    {0x59, 0x80},//90  // LineADLC offset don't care
    
    {0x44, 0x64},//clamp en[6]=1 on
    {0x4a, 0x30},//clamp level 0011h [7]~[4]
    
    {0x2d, 0x48},// double shutter (default:00)
    {0x4d, 0x08},// Voltage doubler (default:04)
    {0x4e, 0x00},// IO current 8mA set
    {0x4f, 0x8a},// IO current 48mA set
    
    {0x66, 0x41},// 1st comp current 2uA
    {0x43, 0xef},// ec_comp
    {0x62, 0x60},// LD control , CFPN_EN off
    
//==========================================================   
//    Table Set for Sub-Sampling
//==========================================================
    {0xfc, 0x03},
    {0x01, 0x60},
    //{0x2e, 0x00},
    {0x2e, 0x03},//DHL
    {0x05, 0x46},// Output Image Size Set for Capture
    {0x07, 0xb6},
    {0x0e, 0x04},
    {0x12, 0x03},
    
    {0xfc, 0x04},
    {0xc5, 0x26},// Output Image Size Set for Preview
    {0xc7, 0x5e},
    {0xce, 0x04},
    {0xd2, 0x04},
    //{0xee, 0x00},//DHL
    {0xee, 0x01},    
    {0xc0, 0x06},
    {0xc1, 0x60},//frame_H
    {0xc2, 0x02},
    {0xc3, 0x8d},//frame_V
    
    {0xfc, 0x07},
    {0x05, 0x00},
    {0x06, 0x00},
    {0x07, 0x8b},
    {0x08, 0xf5},
    {0x09, 0x00},
    {0x0a, 0xb4},
    {0x0b, 0x00},
    {0x0c, 0xea},
    {0x0d, 0x00},
    {0x0e, 0x40},
    
//==========================================================
//    COMMAND SET 
//==========================================================
    {0xfc, 0x00},
    {0x70, 0x02},
    
    {0xfc, 0x00},
    {0x73, 0x11},//21 Frmae AE Enable, peter0223
    {0x20, 0x02},// Change AWB Mode
    
    {0xfc, 0x00},
    {0x78, 0x6a},// AGC Max
    
    {0xfc, 0x00},
    {0x6c, 0xa0},// AE target
    {0x6d, 0x00},
    
    {0xfc, 0x20},
    {0x16, 0x5a},// AGC frame AE start _for Prevating AE Hunting
    {0x57, 0x18},// Stable_Frame_AE
    
    {0xfc, 0x00},
    {0x83, 0x06},//low condition shutter off // Double shutter off
    
    {0xfc, 0x0b},
    {0x5c, 0x69},//70    //AGC value to start shutter on/off suppress
    {0x5d, 0x65},//60   //AGC value to start double shutter on/off suppress

    {0xfc, 0x20},
    {0x25, 0x00},// CINTR Min
    {0x2a, 0x01},// forbidden
    {0x2b, 0x02},// For Forbidden Area
    {0x2c, 0x0a},
    {0x2d, 0x00},// For Forbidden Area
    {0x2e, 0x00},
    {0x2f, 0x05},// forbidden
    {0x14, 0x78},//70
    {0x01, 0x00},// Stepless_Off
    
    {0xfc, 0x00},
    {0x29, 0x04},// Y level 
    {0x2a, 0x00},
    {0x2b, 0x03},// C level
    {0x2c, 0x80},//60
    
    {0xfc, 0x07},
    {0x37, 0x00},// Flicker 
    
    {0xfc, 0x00},
    {0x72, 0xa0},// Flicker for 32MHz
    {0x74, 0x08},// flicker 60Hz fix
    {0xfc, 0x20},
    {0x02, 0x12},//02 Flicker Dgain Mode
    {0xfc, 0x00},
    {0x62, 0x02},// Hue Control Enable
    
    {0xfc, 0x01},
    //{0x0c, 0x02},// Full YC Enable
    {0x0C, 0x03},//Donghoon

    
//==========================================================
//    COLOR MATRIX
//==========================================================    
    {0xfc, 0x01},    //DL gain 60
    {0x51, 0x08},    //06    //08  07    
    {0x52, 0xe8},    //df    //9B  E7    
    {0x53, 0xfc},    //fd    //FC  FB    
    {0x54, 0x33},    //09    //07  B9    
    {0x55, 0xfe},    //00    //FF  00    
    {0x56, 0xe6},    //17    //5E  5F    
    {0x57, 0xfe},    //fe    //FD  FD    
    {0x58, 0x3d},    //4f    //0E  46    
    {0x59, 0x08},    //06    //07  05    
    {0x5a, 0x21},    //9b    //EE  E6    
    {0x5b, 0xfd},    //ff    //FF  00    
    {0x5c, 0xa3},    //17    //05  D3    
    {0x5d, 0xff},    //ff    //FF  FF    
    {0x5e, 0xbc},    //81    //7A  53    
    {0x5f, 0xfc},    //fd    //FC  FB    
    {0x60, 0x96},    //5b    //23  B1    
    {0x61, 0x07},    //07    //08  08    
    {0x62, 0xaf},    //24    //64  FD     

//==========================================================
//    EDGE ENHANCEMENT                                       
//==========================================================
    {0xfc, 0x05},
    {0x12, 0x3d},
    {0x13, 0x3b},
    {0x14, 0x38},
    {0x15, 0x3b},
    {0x16, 0x3d},
    
    {0x17, 0x3b},
    {0x18, 0x05},
    {0x19, 0x09},
    {0x1a, 0x05},
    {0x1b, 0x3b},
    
    {0x1c, 0x38},
    {0x1d, 0x09},
    {0x1e, 0x1c},
    {0x1f, 0x09},
    {0x20, 0x38},
    
    {0x21, 0x3b},
    {0x22, 0x05},
    {0x23, 0x09},
    {0x24, 0x05},
    {0x25, 0x3b},
    
    {0x26, 0x3d},
    {0x27, 0x3b},
    {0x28, 0x38},
    {0x29, 0x3b},
    {0x2a, 0x3d},
    
    {0xfc, 0x00},
    {0x89, 0x00},// Edge Suppress On
    {0xfc, 0x0b},
    {0x42, 0x50},// Edge AGC MIN
    {0x43, 0x60},// Edge AGC MAX
    {0x45, 0x18},// positive gain AGC MIN
    {0x49, 0x06},// positive gain AGC MAX 
    {0x4d, 0x18},// negative gain AGC MIN
    {0x51, 0x06},// negative gain AGC MAX
    
    {0xfc, 0x05},
    {0x34, 0x28},// APTCLP
    {0x35, 0x03},// APTSC
    {0x36, 0x0b},// ENHANCE
    {0x3f, 0x00},// NON-LIN
    {0x42, 0x10},// EGFALL
    {0x43, 0x00},// HLFALL
    {0x45, 0xa0},// EGREF
    {0x46, 0x7a},// HLREF
    {0x47, 0x40},// LLREF
    {0x48, 0x0c},
    {0x49, 0x31},// CSSEL  EGSEL  CS_DLY

    {0x40, 0x41},// Y delay
    
    // New Wide Luma Edge
    {0xfc, 0x1d},
    {0x86, 0x00},
    {0x87, 0x60},
    {0x88, 0x01},
    {0x89, 0x20},    
    {0x8a, 0x00},
    {0x8b, 0x00},
    {0x8c, 0x00},
    {0x8d, 0x00},
    {0x8e, 0x00},
    {0x8f, 0x20},
    {0x90, 0x00},
    {0x91, 0x00},
    {0x92, 0x00},
    {0x93, 0x0a},    
    {0x94, 0x00},
    {0x95, 0x00},
    {0x96, 0x00},
    {0x97, 0x20},
    {0x98, 0x00},
    {0x99, 0x00},
    {0x9a, 0xff},
    {0x9b, 0xea},
    {0x9c, 0xaa},
    {0x9d, 0xab},    
    {0x9e, 0xff},
    {0x9f, 0xf1},
    {0xa0, 0x55},
    {0xa1, 0x56},
    {0xa2, 0x07},
    
    {0x85, 0x01},
    
//==========================================================
//    GAMMA                                       
//==========================================================    
    {0xfc, 0x1d},
    {0x00, 0x0b},
    {0x01, 0x18},
    {0x02, 0x3d},
    {0x03, 0x9c},
    {0x04, 0x00},
    {0x05, 0x0c},
    {0x06, 0x76},
    {0x07, 0xc2},
    {0x08, 0x00},
    {0x09, 0x56},
    {0x0a, 0x34},
    {0x0b, 0x60},
    {0x0c, 0x85},
    {0x0d, 0xa7},
    {0x0e, 0xaa},
    {0x0f, 0xc6},    
    {0x10, 0xe2},
    {0x11, 0xfc},
    {0x12, 0x13},
    {0x13, 0xab},
    {0x14, 0x29},
    {0x15, 0x3c},
    {0x16, 0x4b},
    {0x17, 0x5a},
    {0x18, 0xff},
    {0x19, 0x69},
    {0x1a, 0x78},
    {0x1b, 0x84},
    {0x1c, 0x91},
    {0x1d, 0xff},
    {0x1e, 0x9c},
    {0x1f, 0xa7},    
    {0x20, 0xb2},
    {0x21, 0xbd},
    {0x22, 0xff},
    {0x23, 0xc7},
    {0x24, 0xd2},
    {0x25, 0xdb},
    {0x26, 0xe4},
    {0x27, 0xff},
    {0x28, 0xec},
    {0x29, 0xf5},
    {0x2a, 0xf0},
    {0x2b, 0x0b},
    {0x2c, 0x18},
    {0x2d, 0x3d},
    {0x2e, 0x9c},
    {0x2f, 0x00},    
    {0x30, 0x0c},
    {0x31, 0x76},
    {0x32, 0xc2},
    {0x33, 0x00},
    {0x34, 0x56},
    {0x35, 0x34},
    {0x36, 0x60},
    {0x37, 0x85},
    {0x38, 0xa7},
    {0x39, 0xaa},
    {0x3a, 0xc6},
    {0x3b, 0xe2},
    {0x3c, 0xfc},
    {0x3d, 0x13},
    {0x3e, 0xab},
    {0x3f, 0x29},    
    {0x40, 0x3c},
    {0x41, 0x4b},
    {0x42, 0x5a},
    {0x43, 0xff},
    {0x44, 0x69},
    {0x45, 0x78},
    {0x46, 0x84},
    {0x47, 0x91},
    {0x48, 0xff},
    {0x49, 0x9c},
    {0x4a, 0xa7},
    {0x4b, 0xb2},
    {0x4c, 0xbd},
    {0x4d, 0xff},
    {0x4e, 0xc7},
    {0x4f, 0xd2},        
    {0x50, 0xdb},
    {0x51, 0xe4},
    {0x52, 0xff},
    {0x53, 0xec},
    {0x54, 0xf5},
    {0x55, 0xf0},
    {0x56, 0x0b},
    {0x57, 0x18},
    {0x58, 0x3d},
    {0x59, 0x9c},
    {0x5a, 0x00},
    {0x5b, 0x0c},
    {0x5c, 0x76},
    {0x5d, 0xc2},
    {0x5e, 0x00},
    {0x5f, 0x56},        
    {0x60, 0x34},
    {0x61, 0x60},
    {0x62, 0x85},
    {0x63, 0xa7},
    {0x64, 0xaa},
    {0x65, 0xc6},
    {0x66, 0xe2},
    {0x67, 0xfc},
    {0x68, 0x13},
    {0x69, 0xab},
    {0x6a, 0x29},
    {0x6b, 0x3c},
    {0x6c, 0x4b},
    {0x6d, 0x5a},
    {0x6e, 0xff},
    {0x6f, 0x69},    
    {0x70, 0x78},
    {0x71, 0x84},
    {0x72, 0x91},
    {0x73, 0xff},
    {0x74, 0x9c},
    {0x75, 0xa7},
    {0x76, 0xb2},
    {0x77, 0xbd},
    {0x78, 0xff},
    {0x79, 0xc7},
    {0x7a, 0xd2},
    {0x7b, 0xdb},
    {0x7c, 0xe4},
    {0x7d, 0xff},
    {0x7e, 0xec},
    {0x7f, 0xf5},
    {0x80, 0xf0},

//==========================================================   
//    HUE CONTROL                                     
//==========================================================
    {0xfc, 0x00},
    {0x48, 0x40},// 2000K
    {0x49, 0x30},
    {0x4a, 0x00},
    {0x4b, 0x00},
    {0x4c, 0x30},
    {0x4d, 0x38},
    {0x4e, 0x00},
    {0x4f, 0x00},

    {0x50, 0x40},// 3000K
    {0x51, 0x30},
    {0x52, 0x00},
    {0x53, 0x00},
    {0x54, 0x30},
    {0x55, 0x38},
    {0x56, 0x00},
    {0x57, 0x00},

    {0x58, 0x3c},//40                  // 5100K
    {0x59, 0x30},//4a                   //40
    {0x5a, 0x00},//0c                   //00
    {0x5b, 0x00},//00
    {0x5c, 0x30},//4a
    {0x5d, 0x38},//40
    {0x5e, 0x00},//f6                   //15 
    {0x5f, 0xfc},//00

//==========================================================   
//    SUPPRESS FUNCTION                                   
//==========================================================
    {0xfc, 0x00},
    {0x7e, 0xf4},

//==========================================================   
//    BPR                                 
//========================================================== 
    {0xfc, 0x0b},
    {0x3d, 0x10},
    
    {0xfc, 0x0b},
    {0x0b, 0x00},
    {0x0c, 0x40},
    {0x0d, 0x5a},
    {0x0e, 0x00},
    {0x0f, 0x20},
    {0x10, 0x00},
    {0x11, 0x10},
    {0x12, 0x00},
    {0x13, 0x7f},
    {0x14, 0x03},
    {0x15, 0xff},
    {0x16, 0x48},
    {0x17, 0x60},
    {0x18, 0x00},
    {0x19, 0x00},
    {0x1a, 0x00},
    {0x1b, 0x20},
    {0x1c, 0x00},
    {0x1d, 0x00},
    {0x1e, 0x00},
    {0x1f, 0x20},

//==========================================================        
//    GR/GB CORRECTION                                      
//========================================================== 
    {0xfc, 0x01},
    {0x45, 0x0c},
    {0xfc, 0x0b},
    {0x21, 0x00},
    {0x22, 0x40},
    {0x23, 0x60},
    {0x24, 0x0d},
    {0x25, 0x20},
    {0x26, 0x0d},
    {0x27, 0x20},

//==========================================================    
//    NR                                                      
//==========================================================  
    {0xfc, 0x01},
    {0x4c, 0x01},
    {0x49, 0x15},
    {0x4b, 0x0a},
    
    {0xfc, 0x0b},
    {0x28, 0x00},
    {0x29, 0x00},
    {0x2a, 0x14},
    {0x2b, 0x00},
    {0x2c, 0x14},
    {0x2d, 0x00},
    {0x2e, 0xD0},
    {0x2f, 0x02},
    {0x30, 0x00},
    {0x31, 0x00},
    {0x32, 0xa0},
    {0x33, 0x00},
    {0x34, 0xe0},
    
//========================================================== 
//    1D-Y/C-SIGMA-LPF                                     
//==========================================================
    {0xfc, 0x01},
    {0x05, 0xC0},
        
    {0xfc, 0x0b},
    {0x35, 0x00},
    {0x36, 0x40},
    {0x37, 0x60},
    {0x38, 0x00},
    {0x39, 0x18},
    {0x3a, 0x00},
    {0x3b, 0x40},
    {0x3c, 0x50},
    {0x3d, 0x60},
    {0x3e, 0x00},
    {0x3f, 0x30},
    {0x40, 0x00},
    {0x41, 0x40},
    {0xd4, 0x40},
    {0xd5, 0x60},
    {0xd6, 0xb0},
    {0xd7, 0xf0},
    {0xd8, 0xb0},
    {0xd9, 0xf0},

//========================================================== 
//    COLOR SUPPRESS                                       
//==========================================================
    {0xfc, 0x0b},
    {0x08, 0x58},
    {0x09, 0x03},
    {0x0a, 0x00},

//========================================================== 
//    SHADING                                              
//==========================================================
    {0xfc, 0x09},
    
    {0x01, 0x06},
    {0x02, 0x40},
    
    {0x03, 0x04},
    {0x04, 0xB0},
        
    {0x05, 0x03},
    {0x06, 0x20},
    {0x07, 0x02},
    {0x08, 0x91},
        
    {0x09, 0x03},
    {0x0A, 0x25},
    {0x0B, 0x02},
    {0x0C, 0x64},
        
    {0x0D, 0x03},
    {0x0E, 0x0F},
    {0x0F, 0x02},
    {0x10, 0x4E},
        
    {0x1D, 0x80},
    {0x1E, 0x00},
    {0x1F, 0x80},
    {0x20, 0x00},
    {0x23, 0x85},
    {0x24, 0x52},
    {0x21, 0x79},
    {0x22, 0xE6},
        
    {0x25, 0x80},
    {0x26, 0x00},
    {0x27, 0x80},
    {0x28, 0x00},
    {0x2B, 0x81},
    {0x2C, 0x48},
    {0x29, 0x81},
    {0x2A, 0x48},
        
    {0x2D, 0x80},
    {0x2E, 0x00},
    {0x2F, 0x80},
    {0x30, 0x00},
    {0x33, 0x7C},
    {0x34, 0x45},
    {0x31, 0x7D},
    {0x32, 0x7D},
    
    {0x35, 0x01},
    {0x36, 0x00},
    {0x37, 0x01},
    {0x38, 0x11},
    {0x39, 0x01},
    {0x3A, 0x4E},
    {0x3B, 0x01},
    {0x3C, 0xAB},
    {0x3D, 0x01},
    {0x3E, 0xDC},
    {0x3F, 0x02},
    {0x40, 0x1A},
    {0x41, 0x02},
    {0x42, 0x6A},
    {0x43, 0x02},
    {0x44, 0xD3},
        
    {0x45, 0x01},
    {0x46, 0x00},
    {0x47, 0x01},
    {0x48, 0x0E},
    {0x49, 0x01},
    {0x4A, 0x40},
    {0x4B, 0x01},
    {0x4C, 0x8A},
    {0x4D, 0x01},
    {0x4E, 0xB5},
    {0x4F, 0x01},
    {0x50, 0xE8},
    {0x51, 0x02},
    {0x52, 0x27},
    {0x53, 0x02},
    {0x54, 0x84},
    
    {0x55, 0x01},
    {0x56, 0x00},
    {0x57, 0x01},
    {0x58, 0x0C},
    {0x59, 0x01},
    {0x5A, 0x37},
    {0x5B, 0x01},
    {0x5C, 0x74},
    {0x5D, 0x01},
    {0x5E, 0x96},
    {0x5F, 0x01},
    {0x60, 0xC9},
    {0x61, 0x02},
    {0x62, 0x04},
    {0x63, 0x02},
    {0x64, 0x4B},
    
    {0x65, 0x00},
    {0x66, 0x9A},
    {0x67, 0x2D},
    {0x68, 0x02},
    {0x69, 0x68},
    {0x6A, 0xB6},
    {0x6B, 0x05},
    {0x6C, 0x6B},
    {0x6D, 0x99},
    {0x6E, 0x07},
    {0x6F, 0x60},
    {0x70, 0xAD},
    {0x71, 0x09},
    {0x72, 0xA2},
    {0x73, 0xD7},
    {0x74, 0x0C},
    {0x75, 0x32},
    {0x76, 0x19},
    {0x77, 0x0F},
    {0x78, 0x0E},
    {0x79, 0x70},
    
    {0x7A, 0x00},
    {0x7B, 0x9C},
    {0x7C, 0x9F},
    {0x7D, 0x02},
    {0x7E, 0x72},
    {0x7F, 0x7A},
    {0x80, 0x05},
    {0x81, 0x81},
    {0x82, 0x94},
    {0x83, 0x07},
    {0x84, 0x7E},
    {0x85, 0x97},
    {0x86, 0x09},
    {0x87, 0xC9},
    {0x88, 0xEA},
    {0x89, 0x0C},
    {0x8A, 0x63},
    {0x8B, 0x8C},
    {0x8C, 0x0F},
    {0x8D, 0x4B},
    {0x8E, 0x7E},
    
    {0x8F, 0x00},
    {0x90, 0x9E},
    {0x91, 0xBD},
    {0x92, 0x02},
    {0x93, 0x7A},
    {0x94, 0xF5},
    {0x95, 0x05},
    {0x96, 0x94},
    {0x97, 0xA8},
    {0x98, 0x07},
    {0x99, 0x98},
    {0x9A, 0x8F},
    {0x9B, 0x09},
    {0x9C, 0xEB},
    {0x9D, 0xD5},
    {0x9E, 0x0C},
    {0x9F, 0x8E},
    {0xA0, 0x7A},
    {0xA1, 0x0F},
    {0xA2, 0x80},
    {0xA3, 0x7D},
    
    {0xA4, 0x6A},
    {0xA5, 0x44},
    {0xA6, 0x23},
    {0xA7, 0x6C},
    {0xA8, 0x15},
    {0xA9, 0x40},
    {0xAA, 0x20},
    {0xAB, 0xB2},
    {0xAC, 0x1C},
    {0xAD, 0x56},
    {0xAE, 0x19},
    {0xAF, 0x01},
    {0xB0, 0x16},
    {0xB1, 0x5F},
    
    {0xB2, 0x68},
    {0xB3, 0x9C},
    {0xB4, 0x22},
    {0xB5, 0xDE},
    {0xB6, 0x14},
    {0xB7, 0xEC},
    {0xB8, 0x20},
    {0xB9, 0x30},
    {0xBA, 0x1B},
    {0xBB, 0xE5},
    {0xBC, 0x18},
    {0xBD, 0x9D},
    {0xBE, 0x16},
    {0xBF, 0x05},
    
    {0xC0, 0x67},
    {0xC1, 0x36},
    {0xC2, 0x22},
    {0xC3, 0x67},
    {0xC4, 0x14},
    {0xC5, 0xA4},
    {0xC6, 0x1F},
    {0xC7, 0xC2},
    {0xC8, 0x1B},
    {0xC9, 0x86},
    {0xCA, 0x18},
    {0xCB, 0x49},
    {0xCC, 0x15},
    {0xCD, 0xBA},
            
    {0x00, 0x02},// shading on

//========================================================== 
//    X-SHADING                                            
//==========================================================
    {0xfc, 0x1B},
    {0x80, 0x01},
    {0x81, 0x00},
    {0x82, 0x4C},
    {0x83, 0x00},
    {0x84, 0x86},
    {0x85, 0x03},
    {0x86, 0x5E},
    {0x87, 0x00},
    {0x88, 0x07},
    {0x89, 0xA4},
    {0x90, 0x00},
    {0x91, 0x88},
    {0x92, 0x00},
    {0x93, 0xC1},
    {0x94, 0x00},
    {0x95, 0xF7},
    {0x96, 0x01},
    {0x97, 0x21},
    {0x98, 0x01},
    {0x99, 0x37},
    {0x9A, 0x01},
    {0x9B, 0x0C},
    {0x9C, 0x00},
    {0x9D, 0xCE},
    {0x9E, 0x00},
    {0x9F, 0x3B},
    {0xA0, 0x00},
    {0xA1, 0x5B},
    {0xA2, 0x00},
    {0xA3, 0x7A},
    {0xA4, 0x00},
    {0xA5, 0x92},
    {0xA6, 0x00},
    {0xA7, 0x91},
    {0xA8, 0x00},
    {0xA9, 0x81},
    {0xAA, 0x00},
    {0xAB, 0x60},
    {0xAC, 0x07},
    {0xAD, 0xCB},
    {0xAE, 0x07},
    {0xAF, 0xC5},
    {0xB0, 0x07},
    {0xB1, 0xBB},
    {0xB2, 0x07},
    {0xB3, 0xAA},
    {0xB4, 0x07},
    {0xB5, 0xA9},
    {0xB6, 0x07},
    {0xB7, 0xB2},
    {0xB8, 0x07},
    {0xB9, 0xBF},
    {0xBA, 0x07},
    {0xBB, 0x5E},
    {0xBC, 0x07},
    {0xBD, 0x3C},
    {0xBE, 0x06},
    {0xBF, 0xF9},
    {0xC0, 0x06},
    {0xC1, 0xBD},
    {0xC2, 0x06},
    {0xC3, 0xB8},
    {0xC4, 0x06},
    {0xC5, 0xE2},
    {0xC6, 0x07},
    {0xC7, 0x1A},
    {0xC8, 0x07},
    {0xC9, 0x15},
    {0xCA, 0x06},
    {0xCB, 0xDE},
    {0xCC, 0x06},
    {0xCD, 0x9C},
    {0xCE, 0x06},
    {0xCF, 0x6F},
    {0xD0, 0x06},
    {0xD1, 0x5E},
    {0xD2, 0x06},
    {0xD3, 0x84},
    {0xD4, 0x06},
    {0xD5, 0xCA},
        
    {0xfc, 0x0b},
    {0xda, 0x00},
    {0xdb, 0x9c},
    {0xdc, 0x00},
    {0xdd, 0xd1},
        
    {0xfc, 0x1b},
    {0x80, 0x01},

//========================================================== 
//    AE WINDOW WEIGHT                                     
//==========================================================
    {0xfc, 0x00},
    {0x03, 0x4b},
    {0xfc, 0x06},
    {0x01, 0x35},
    {0x03, 0xc2},
    {0x05, 0x48},
    {0x07, 0xb8},
    {0x31, 0x2a},
    {0x33, 0x61},
    {0x35, 0x28},
    {0x37, 0x5c},
        
    {0xfc, 0x20},
    {0x60, 0x11},
    {0x61, 0x11},
    {0x62, 0x11},
    {0x63, 0x11},
    {0x64, 0x11},
    {0x65, 0x22},
    {0x66, 0x22},
    {0x67, 0x11},
    {0x68, 0x11},
    {0x69, 0x33},
    {0x6a, 0x33},
    {0x6b, 0x11},
    {0x6c, 0x12},
    {0x6d, 0x55},
    {0x6e, 0x55},
    {0x6f, 0x21},
    {0x70, 0x13},
    {0x71, 0x55},
    {0x72, 0x55},
    {0x73, 0x31},
    {0x74, 0x33},
    {0x75, 0x33},
    {0x76, 0x33},
    {0x77, 0x33},

//==========================================================  
//    SAIT AWB                                              
//==========================================================
    {0xfc, 0x00},
    {0x7b, 0x00},
        
    {0xfc, 0x07},
    {0x3c, 0x10},
    {0x3d, 0x10},
    {0x3e, 0x10},
    {0x3f, 0x10},
        
    {0xfc, 0x01},
    {0xc8, 0xe0},
    {0xfc, 0x00},
    {0x3e, 0x10},
        
    {0xfc, 0x00},
    {0x3e, 0x10},
    {0x3d, 0x04},
    {0x32, 0x02},
    {0x81, 0x10},
    {0xbc, 0xf0},
    {0xfc, 0x22},
    {0x8c, 0x04},
    {0x8d, 0x06},
        
    {0xfc, 0x07},
    {0x97, 0x00},

//=================================                           
// White Point                                                
//=================================
    {0xfc, 0x22},
    {0x01, 0xD8},
    {0x03, 0xA1},
    {0x05, 0xCA},
    {0x07, 0xC8},
    {0x09, 0xB3},
    {0x0b, 0xE2},
    {0x0d, 0xA0},
    {0x0f, 0xF0},
    {0x11, 0x94},
    {0x12, 0x00},
    {0x13, 0xFD},
    {0x15, 0x88},
    {0x16, 0x01},
    {0x17, 0x10},

//=================================                                  
// Basic Setting                            
//=================================
    {0xfc, 0x22},
    {0xA8, 0xFF},
        
    {0xA0, 0x01},
    {0xA1, 0x38},
    {0xA2, 0x0E},
    {0xA3, 0x6D},
    {0xA4, 0x07},
    {0xA5, 0xF5},
    {0xA6, 0x11},
    {0xA7, 0xBE},
    {0xA9, 0x02},
    {0xAA, 0xD2},
    {0xAB, 0x00},
    {0xAC, 0x00},
    {0xAD, 0x02},
    {0xAE, 0x3F},
    {0xAF, 0x19},
    {0xB0, 0x91},
    {0x94, 0x3D},
    {0x95, 0x00},
    {0x96, 0x58},
    {0x97, 0x80},
    {0xD0, 0xA2},
    {0xD1, 0x2E},
    {0xD2, 0x4D},
    {0xD3, 0x28},
    {0xD4, 0x90},
    {0xDB, 0x2E},
    {0xDC, 0x7A},
    {0xDD, 0x28},
    {0xE7, 0x00},
    {0xE8, 0xc7},
    {0xE9, 0x00},
    {0xEA, 0x62},
    {0xEB, 0xD2},
    {0xEC, 0xD9},
    {0xEE, 0xA6},
        
    {0xfc, 0x00},
    {0x8a, 0x02},
    
//=================================
// Pixel Filter Setting            
//=================================
    {0xFC, 0x07},
    {0x95, 0xCF},
        
    {0xfc, 0x01},
    {0xd3, 0x4f},
    {0xd4, 0x00},
    {0xd5, 0x3c},
    {0xd6, 0x80},
    {0xd7, 0x61},
    {0xd8, 0x00},
    {0xd9, 0x49},
    {0xda, 0x00},
    {0xdb, 0x24},
    {0xdc, 0x4b},
    {0xdd, 0x23},
    {0xde, 0xf2},
    {0xdf, 0x20},
    {0xe0, 0x73},
    {0xe1, 0x18},
    {0xe2, 0x69},
    {0xe3, 0x31},
    {0xe4, 0x40},
    {0xe5, 0x34},
    {0xe6, 0x40},
    {0xe7, 0x40},
    {0xe8, 0x32},
    {0xe9, 0x40},
    {0xea, 0x1c},
    {0xeb, 0x00},

//================================= 
// Polygon AWB Region Tune          
//================================= 

    // AWB3 - Polygon Region
    {0xfc, 0x22},
    {0x18, 0x00},
    {0x19, 0x4b},
    {0x1a, 0xfd},
    {0x1b, 0x00},
    {0x1c, 0x41},
    {0x1d, 0xd9},
    {0x1e, 0x00},
    {0x1f, 0x66},
    {0x20, 0xa9},
    {0x21, 0x00},
    {0x22, 0x8b},
    {0x23, 0x82},
    {0x24, 0x00},
    {0x25, 0xa4},
    {0x26, 0x6c},
    {0x27, 0x00},
    {0x28, 0xbd},
    {0x29, 0x5d},
    {0x2a, 0x00},
    {0x2b, 0xdc},
    {0x2c, 0x4d},
    {0x2d, 0x00},
    {0x2e, 0xdc},
    {0x2f, 0x63},
    {0x30, 0x00},
    {0x31, 0xc1},
    {0x32, 0x72},
    {0x33, 0x00},
    {0x34, 0xab},
    {0x35, 0x84},
    {0x36, 0x00},
    {0x37, 0x99},
    {0x38, 0xa0},
    {0x39, 0x00},
    {0x3a, 0x81},
    {0x3b, 0xe9},
    {0x3c, 0x00},
    {0x3d, 0x00},
    {0x3e, 0x00},
    {0x3f, 0x00},
    {0x40, 0x00},
    {0x41, 0x00},

//================================= 
// Moving Equation Weight           
//=================================
    {0xfc, 0x22},
    {0x98, 0x07},

//=================================
// EIT Threshold                   
//=================================
    {0xfc, 0x22},
    {0xb1, 0x00},
    {0xb2, 0x02},
    {0xb3, 0x00},
    {0xb4, 0xC1},
        
    {0xb5, 0x00},
    {0xb6, 0x02},
    {0xb7, 0x00},
    {0xb9, 0xc2},
        
    {0xd7, 0x00},
    {0xd8, 0x35},
    {0xd9, 0x20},
    {0xda, 0x81},

//=================================
// Gain Offset                     
//=================================
    {0xfc, 0x00},
    {0x79, 0xf8},
    {0x7a, 0x08},
        
    {0xfc, 0x07},
    {0x11, 0x01},
        
    {0xfc, 0x22},
    {0x58, 0xf8},
    {0x59, 0x00},
    {0x5A, 0xfc},
    {0x5B, 0x00},
    {0x5C, 0x00},
    {0x5D, 0x00},
    {0x5E, 0x00},
    {0x5F, 0x00},
    {0x60, 0x00},
    {0x61, 0xf8},
    {0x62, 0x00},
    {0x63, 0xf0},
        
    {0xde, 0x00},
    {0xf0, 0x6a},

//=================================
// Green Stablity Enhance          
//=================================
    {0xfc, 0x22},
    {0xb9, 0x00},
    {0xba, 0x00},
    {0xbb, 0x00},
    {0xbc, 0x00},
    {0xe5, 0x01},
    {0xe6, 0xff},
    {0xbd, 0x8c},

//========================================================== 
//    Special Effect                                       
//==========================================================
    {0xfc, 0x07},
    {0x30, 0xc0},
    {0x31, 0x20},
    {0x32, 0x40},
    {0x33, 0xc0},
    {0x34, 0x00},
    {0x35, 0xb0},

//==========================================================
//    ETC                                      
//==========================================================
    {0xfc, 0x01},
    {0x01, 0x01},
    {0x00, 0x90},
    {0xfc, 0x02},
    {0x03, 0x20},
        
    {0xfc, 0x20},
    {0x0f, 0x00},
        
    {0xfc, 0x00},
    {0x02, 0x09},
        
    {0xfc, 0x01},
    //{0x02, 0x00},
    {0x02, 0x02}//Donghoon

};


#endif // _S5X532_H_
