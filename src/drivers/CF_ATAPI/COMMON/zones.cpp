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
#include <windows.h>
#include <debug.h>

DBGPARAM dpCurSettings = {
    TEXT("ATAPI"), {
    TEXT("Init"),TEXT("Deinit"),TEXT("Main"),TEXT("I/O"),
    TEXT("UNUSED"),TEXT("PCI"),TEXT("IOCTL"),TEXT("CDROM"),
    TEXT("DMA"),TEXT("Power"),TEXT("Undefined"),TEXT("Undefined"),
    TEXT("Warning"),TEXT("Error"),TEXT("Helper"), TEXT("CELOG") },
    ZONEMASK_ERROR |
    ZONEMASK_WARNING
};

