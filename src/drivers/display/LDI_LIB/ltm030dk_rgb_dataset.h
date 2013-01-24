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
#ifndef __LTM030DK_RGB_DATASET_H__
#define __LTM030DK_RGB_DATASET_H__

#if __cplusplus
extern "C"
{
#endif

#define FIRMWARE_CODE   (TRUE)

const unsigned int LTM030DK_RGB_initialize[][3] =
{
#if FIRMWARE_CODE
    {15, 0x01, 1},
    { 0, 0x03, 0},

    { 1, 0x02, 0},
    { 3, 0x00, 0},
    { 7, 0x11, 0},
    {10, 0x0A, 0},
    {11, 0x05, 0},
    {12, 0x07, 0},
    {26, 0x05, 0},
    {27, 0x10, 0},
    {28, 0x71, 0},
    {29, 0xA1, 0},
    {30, 0xAD, 0},
    {31, 0x04, 0},
    {33, 0x07, 0},
    {34, 0x01, 0},
    {35, 0x00, 0},
    {37, 0x00, 0},
    {38, 0xF8, 0},
    {39, 0x0C, 0},
    {40, 0x40, 0},
    {41, 0x08, 0},
    {42, 0x00, 0},
    {43, 0x00, 0},
    {44, 0xe7, 0},

    {66, 0x01, 0},
    {67, 0x53, 0},
    {68, 0x33, 0},
    {69, 0x41, 0},
    {70, 0x01, 0},
    {71, 0x00, 0},
    {72, 0xFC, 0},
    {73, 0x00, 0},
    {74, 0x00, 0},
    {75, 0x00, 0},
    {76, 0x00, 0},
    {77, 0x00, 0},
    {78, 0xFC, 0},
    {79, 0x00, 0},
    {80, 0x00, 0},
    {81, 0x00, 0},
    {82, 0x00, 0},
    {83, 0x00, 0},    
    {84, 0xFC, 0},
    {85, 0x00, 0},
    {86, 0x00, 0},
    {87, 0x00, 0},
    {88, 0x00, 0},
    {89, 0x01, 0},
    {90, 0xFF, 0},
 //   {91, 0x00, 0},
    {92, 0x0D, 0},
    {93, 0x07, 0},
    {94, 0x7E, 0},
    {95, 0x80, 0},
    {96, 0x84, 0},
    {97, 0x8C, 0},
    {98, 0x95, 0},
    {99, 0x9F, 0},
    {100, 0xAB, 0},
    {101, 0xB7, 0},
    {102, 0xC4, 0},
    {103, 0xD0, 0},    
    {104, 0xDC, 0},
    {105, 0xE7, 0},
    {106, 0xF0, 0},
    {107, 0xF8, 0},
    {108, 0xFD, 0},
    {109, 0xFF, 0},    
    
    {110, 0x00, 0},
    {111, 0x01, 0},    
    {112, 0x03, 0},    
    {113, 0x20, 0},    

    {4, 0x01, 0},
#else
    {15, 0x01, 1},
    { 0, 0x03, 0},

    { 1, 0x02, 0},
    { 3, 0x00, 0},
    { 7, 0x11, 0},
    {10, 0x0A, 0},
    {11, 0x05, 0},
    {12, 0x07, 0},
    {26, 0x05, 0},
    {27, 0x10, 0},
    {28, 0x71, 0},
    {29, 0xA1, 0},
    {30, 0xAD, 0},
    {31, 0x04, 0},
    {33, 0x07, 0},
    {34, 0x01, 0},
    {35, 0x00, 0},
    {37, 0x00, 0},
    {38, 0xF8, 0},
    {39, 0x0C, 0},
    {40, 0x40, 0},
    {41, 0x08, 0},
    {42, 0x00, 0},
    {43, 0x00, 0},
    {44, 0xe7, 0},

    {66, 0x00, 0},
    {67, 0x53, 0},
    {68, 0x33, 0},
    {69, 0x41, 0},
    {70, 0x01, 0},
    {71, 0x00, 0},
    {72, 0xFC, 0},
    {73, 0x02, 0},
    {74, 0x08, 0},
    {75, 0x00, 0},
    {76, 0x0F, 0},
    {77, 0x00, 0},
    {78, 0xFC, 0},
    {79, 0x00, 0},
    {80, 0x00, 0},
    {81, 0x00, 0},
    {82, 0x00, 0},
    {83, 0x00, 0},    
    {84, 0xFF, 0},
    {85, 0x02, 0},
    {86, 0xF0, 0},
    {87, 0xFA, 0},
    {88, 0x0D, 0},
    {89, 0x00, 0},
    {90, 0xFF, 0},
    {91, 0x00, 0},
    {92, 0x0D, 0},
    {93, 0x07, 0},
    {94, 0x7E, 0},
    {95, 0x80, 0},
    {96, 0x84, 0},
    {97, 0x8C, 0},
    {98, 0x95, 0},
    {99, 0x9F, 0},
    {100, 0xAB, 0},
    {101, 0xB7, 0},
    {102, 0xC4, 0},
    {103, 0xD0, 0},    
    {104, 0xDC, 0},
    {105, 0xE7, 0},
    {106, 0xF0, 0},
    {107, 0xF8, 0},
    {108, 0xFD, 0},
    {109, 0xFF, 0},    
    
    {110, 0x00, 0},
    {111, 0x01, 0},    
    {112, 0x03, 0},    
    {113, 0x20, 0},    

    {4, 0x01, 0},

#endif

    {0, 0, 0}
};

#if __cplusplus
}
#endif

#endif    // __LTM030DK_RGB_DATASET_H__
