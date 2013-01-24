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

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <pmplatform.h>
#include "JPGDriver.h"


static HANDLE    hMutex;


/*----------------------------------------------------------------------------
*Function: CreateJPGmutex
*Implementation Notes: Create Mutex handle 
-----------------------------------------------------------------------------*/
HANDLE CreateJPGmutex(void)
{
    hMutex = CreateMutex(NULL,  // default security attributes
                         FALSE, // initially not owned
                         NULL); // unnamed mutex
    
    return hMutex;
}

/*----------------------------------------------------------------------------
*Function: LockJPGMutex
*Implementation Notes: lock mutex 
-----------------------------------------------------------------------------*/
DWORD LockJPGMutex(void)
{
    DWORD                Status;

    Status = WaitForSingleObject(hMutex, INFINITE);
    if(Status == WAIT_OBJECT_0) 
    {
        Status = TRUE;
    } 
    else 
    {
        Status = GetLastError();
        RETAILMSG(ZONE_ERROR,(TEXT("JPEG::MUTEX LOCK ERROR : %d\r\n"),Status));
        Status = FALSE;
    }
      
    return Status;
}

/*----------------------------------------------------------------------------
*Function: UnlockJPGMutex
*Implementation Notes: unlock mutex
-----------------------------------------------------------------------------*/
DWORD UnlockJPGMutex(void)
{
    DWORD                Status;
   
    Status = ReleaseMutex(hMutex);
    if(Status != TRUE) 
    {
        Status = GetLastError();
        RETAILMSG(ZONE_ERROR,(TEXT("JPEG::MUTEX UNLOCK ERROR : %d\r\n"),Status));
    }

    return Status;
}

/*----------------------------------------------------------------------------
*Function: DeleteJPGMutex
*Implementation Notes: delete mutex handle 
-----------------------------------------------------------------------------*/
void DeleteJPGMutex(void)
{
    if (hMutex == NULL)
        return;

    CloseHandle(hMutex);
}
