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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2008. Samsung Electronics, co. ltd  All rights reserved.

Module Name:  

Abstract:

    This file implements the S3C6410 Keyboard function

Notes: 
--*/

#include <windows.h>
#include <keybdpdd.h>

// Add NOP driver for USB HID and RDP support.
BOOL
WINAPI
PS2_NOP_Entry(
    UINT uiPddId,
    PFN_KEYBD_EVENT pfnKeybdEvent,
    PKEYBD_PDD *ppKeybdPdd
    );

BOOL
WINAPI
Matrix_Entry(
    UINT uiPddId,
    PFN_KEYBD_EVENT pfnKeybdEvent,
    PKEYBD_PDD *ppKeybdPdd
    );

PFN_KEYBD_PDD_ENTRY g_rgpfnPddEntries[] = {
    PS2_NOP_Entry,
    Matrix_Entry,
    NULL
};

