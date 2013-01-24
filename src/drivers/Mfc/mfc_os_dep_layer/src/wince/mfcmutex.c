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

*/


#include <windows.h>

#include "MfcMutex.h"


#define  MUTEX_TIMEOUT                    5000

static CRITICAL_SECTION cs;

BOOL MFC_Mutex_Create(void)
{
    InitializeCriticalSection(&cs);

    return TRUE;
}

void MFC_Mutex_Delete(void)
{
    DeleteCriticalSection(&cs);
}

BOOL MFC_Mutex_Lock(void)
{
    EnterCriticalSection(&cs);

    return TRUE;
}

BOOL MFC_Mutex_Release(void)
{
    LeaveCriticalSection(&cs);

    return TRUE;
}
