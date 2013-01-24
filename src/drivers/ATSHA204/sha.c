//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

    sha.c   SHA204 MDD
Abstract:

   Streams interface driver

Functions:

Notes:

--*/

#include <windows.h>
#include <nkintr.h>

#include <winreg.h>
#include <winioctl.h>
#include <ceddk.h>
#include <devload.h>

#include "sha.h"
#include <sha204_lib_return_codes.h>  // declarations of function return codes
#include <sha204_comm_marshaling.h>   // definitions and declarations for the Command module

#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("SHA"), {
    TEXT("Error"), TEXT("Warn"),  TEXT("Init"),  TEXT("Open"),
    TEXT("Read"),  TEXT("Write"), TEXT("IOCTL"), TEXT("IST"),
    TEXT("Power"), TEXT("9"),     TEXT("10"),    TEXT("11"), 
    TEXT("12"),    TEXT("13"),    TEXT("14"),    TEXT("Trace"),
    },
    0x0003 // ZONE_WRN|ZONE_ERR
};
#endif  // DEBUG


BOOL
SHA_PowerUp(
   PVOID Context
   );

BOOL
SHA_PowerDown(
   PVOID Context
   );

BOOL
DllEntry(
    HINSTANCE   hinstDll,             /*@parm Instance pointer. */
    DWORD   dwReason,                 /*@parm Reason routine is called. */
    LPVOID  lpReserved                /*@parm system parameter. */
    )
{
    if ( dwReason == DLL_PROCESS_ATTACH ) {
        DEBUGREGISTER(hinstDll);
        DEBUGMSG (ZONE_INIT, (TEXT("SHA: Process Attach\r\n")));
    }

    if ( dwReason == DLL_PROCESS_DETACH ) {
        DEBUGMSG (ZONE_INIT, (TEXT("SHA: Process Detach\r\n")));
    }

    return(TRUE);
}
BOOL
SHA_Deinit(
   PSHA_CONTEXT pSHA
   )
{
   DEBUGMSG(ZONE_INIT, (TEXT(">SHA_Deinit\n")));

    if (!pSHA)
        return FALSE;
        
    //HW_Deinit(pSHA);
	DeleteCriticalSection(&pSHA->RegCS);

	LocalFree(pSHA);

	DEBUGMSG(ZONE_INIT, (TEXT("<SHA_Deinit\n")));

	return TRUE;
}


/*++

Called by Device Manager to initialize the streams interface in response to ActivateDevice.
We passed ActivateDevice a pointer to our device context, but must read it out of the registry as "ClientInfo".

Returns context used in XXX_Open, XXX_PowerDown, XXX_PowerUp, and XXX_Deinit

--*/
PSHA_CONTEXT
SHA_Init(
   PVOID Context
   )
{
    LPTSTR ActivePath = (LPTSTR)Context; // HKLM\Drivers\Active\xx
    PSHA_CONTEXT pSHA;
    BOOL bRc = FALSE;

    DEBUGMSG(ZONE_INIT, (TEXT(">SHA_Init(%p)\n"), ActivePath));

    // Allocate for our main data structure and one of it's fields.
    pSHA = (PSHA_CONTEXT)LocalAlloc( LPTR,  sizeof(SHA_CONTEXT) );
    if ( !pSHA )
        return( NULL );

    pSHA->Sig = SHA_SIG;
    InitializeCriticalSection(&pSHA->RegCS);
	pSHA->hProc = (HANDLE)GetCurrentProcessId();

    //if ( HW_Init(pSHA) != ERROR_SUCCESS)
      //  goto ALLOCFAILED;

    pSHA->Dx = D0;
        
    DEBUGMSG(ZONE_INIT, (TEXT("<SHA_Init:0x%x\n"), pSHA ));

    return (pSHA);

//ALLOCFAILED:
    //SHA_Deinit(pSHA);

    return NULL;
}


PSHA_CONTEXT
SHA_Open(
   PSHA_CONTEXT pSHA,       // context returned by SHA_Init.
   DWORD        AccessCode, // @parm access code
   DWORD        ShareMode   // @parm share mode
   )
{
    UNREFERENCED_PARAMETER(ShareMode);
    UNREFERENCED_PARAMETER(AccessCode);

    DEBUGMSG(ZONE_OPEN,(TEXT(">SHA_Open(0x%x, 0x%x, 0x%x)\n"),pSHA, AccessCode, ShareMode));

    pSHA->OpenCount++;

    //HW_Open(pSHA);

    DEBUGMSG(ZONE_OPEN,(TEXT("<SHA_Open:%u\n"), pSHA->OpenCount ));

    return pSHA;
}


BOOL
SHA_Close(
   PSHA_CONTEXT pSHA
   )
{
   DEBUGMSG(ZONE_OPEN,(TEXT("SHA_Close(0x%x)\n"),pSHA));

    if ( pSHA->OpenCount ) {
        
        pSHA->OpenCount--;

        // BUGBUG: power off if no longer open
       // HW_Close(pSHA);
    }

    return TRUE;
}


ULONG
SHA_Write(
   PSHA_CONTEXT pSHA,
   PUCHAR pBuffer,
   ULONG  BufferLength
   )
{
    return 0;
}


ULONG
SHA_Read(
   PSHA_CONTEXT pSHA,
   PUCHAR pBuffer,
   ULONG  BufferLength
   )
{
   return 0;
}

VOID SHA204_Init(PSHA_CONTEXT pSHA)
{
	sha204p_init();
	return;
}
VOID SHA204_Sleep(PSHA_CONTEXT pSHA)
{
	sha204p_sleep();
	return;
}
BOOL SHA204_WakeUp(PSHA_CONTEXT pSHA)
{
	unsigned char response[4],ret_code;
	ret_code=sha204c_wakeup(&response[0]);
	if(ret_code!=SHA204_SUCCESS)
		return FALSE;
	else
		return TRUE;
}
unsigned char SHA204_Execute(PSHA_CONTEXT pSHA,PSHA_DATA data)
{
	
	unsigned char ret_code = sha204m_execute(data->op_code, data->param1, data->param2, data->datalen1, (uint8_t *)(data->data1),
				data->datalen2, (uint8_t *)(data->data2), data->datalen3, (uint8_t *)(data->data3), data->tx_size, data->tx_buffer, data->rx_size, data->rx_buffer);
	
	return ret_code;
}


BOOL
SHA_IOControl(
    PSHA_CONTEXT pSHA,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
   )
{
    DWORD dwErr = ERROR_SUCCESS;
    BOOL  bRc = TRUE;
    //PUCHAR puc;
	PSHA_DATA	puc;
	unsigned char ret_code;
	PPOWER_CAPABILITIES ppc;

    DEBUGMSG(ZONE_IOCTL,(TEXT(">SHA_IOControl(0x%x, 0x%x, %d, 0x%x)\n"),
        dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut ));

    if ( !pSHA || !pSHA->OpenCount ) {
        DEBUGMSG (ZONE_IOCTL|ZONE_ERR, (TEXT("SHA_IOControl: ERROR_INVALID_HANDLE\r\n")));
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    if (pdwActualOut)
        *pdwActualOut = 0;

    EnterCriticalSection(&pSHA->RegCS);

    switch (dwCode) {
        //
        // IOCTL_SHA_xxx
        //
        case IOCTL_SHA204_INIT:
			SHA204_Init(pSHA);
            break;
		case IOCTL_SHA204_SLEEP:
			SHA204_Sleep(pSHA);
			break;
		case IOCTL_SHA204_WAKEUP:
			SHA204_WakeUp(pSHA);
			break;

        case IOCTL_SHA204_EXECUTE:
            if ( (dwLenIn < sizeof(SHA_DATA)) || !pBufIn || !((PSHA_DATA)pBufIn)->tx_buffer || !((PSHA_DATA)pBufIn)->rx_buffer) {
                dwErr = ERROR_INVALID_PARAMETER;
                bRc = FALSE;
                break;
            }
			puc=(PSHA_DATA)pBufIn;
            if(((PSHA_DATA)pBufIn)->data1)
				puc->data1 = (PUCHAR)MapPtrToProcess(((PSHA_DATA)pBufIn)->data1, pSHA->hProc );
			else
				puc->data1 = NULL;
			
            if(((PSHA_DATA)pBufIn)->data2)
				puc->data2 = (PUCHAR)MapPtrToProcess(((PSHA_DATA)pBufIn)->data2, pSHA->hProc );
			else
				puc->data2 = NULL;
			
            if(((PSHA_DATA)pBufIn)->data3)
				puc->data3 = (PUCHAR)MapPtrToProcess(((PSHA_DATA)pBufIn)->data3, pSHA->hProc );
			else
				puc->data3 = NULL;
        
			puc->tx_buffer= (PUCHAR)MapPtrToProcess(((PSHA_DATA)pBufIn)->tx_buffer, pSHA->hProc );
			puc->rx_buffer = (PUCHAR)MapPtrToProcess(((PSHA_DATA)pBufIn)->rx_buffer, pSHA->hProc );
			ret_code = SHA204_Execute(pSHA,puc);
			if(puc->data1!=NULL)
            	UnMapPtr(puc->data1);
			if(puc->data1!=NULL)
				UnMapPtr(puc->data2);
			if(puc->data1!=NULL)
				UnMapPtr(puc->data3);
			UnMapPtr(puc->tx_buffer);
			UnMapPtr(puc->rx_buffer);

            if ( SHA204_SUCCESS == ret_code ) {
                if (pdwActualOut)
                    *pdwActualOut = ((PSHA_DATA)pBufIn)->rx_size;
            } else
                bRc = FALSE;
            
            break;

        case IOCTL_SHA204_GET_FASTCALL:
            if ( (dwLenOut < sizeof(SHA_FASTCALL)) || !pBufOut ) {
                bRc = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

            // Check caller process & fail if they are not in device.exe!
            if (GetCallerProcess() != pSHA->hProc ) {
                DEBUGMSG (ZONE_ERR, (TEXT("ERROR_ACCESS_DENIED: Caller(0x%X) != Current(0x%X)\r\n"), 
                    GetCallerProcess(), pSHA->hProc ));
                bRc = FALSE;
                dwErr = ERROR_ACCESS_DENIED;
                break;
            }
            
            ((PSHA_FASTCALL)pBufOut)->Context  = pSHA;
            ((PSHA_FASTCALL)pBufOut)->SHA204_Init  = SHA204_Init;
            ((PSHA_FASTCALL)pBufOut)->SHA204_Sleep = SHA204_Sleep;
			((PSHA_FASTCALL)pBufOut)->SHA204_WakeUp  = SHA204_WakeUp;
			((PSHA_FASTCALL)pBufOut)->SHA204_Execute = SHA204_Execute;


            if (pdwActualOut)
                *pdwActualOut = sizeof(SHA_FASTCALL);

            break;

        //
        // Power Management
        //
        case IOCTL_POWER_CAPABILITIES:
            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(POWER_CAPABILITIES)) ) {
                bRc = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            ppc=(PPOWER_CAPABILITIES)pBufOut;
            //bRc = HW_PowerCapabilities(pSHA, (PPOWER_CAPABILITIES)pBufOut);
			memset(ppc, 0, sizeof(*ppc));
			ppc->DeviceDx = 0x1; 
			ppc->Flags = POWER_CAP_PREFIX_MICRO | POWER_CAP_UNIT_AMPS;
    		ppc->Power[D0] = 600; 
            if ( bRc ) {
                *pdwActualOut = sizeof(POWER_CAPABILITIES);
            }
            break;

        case IOCTL_POWER_SET: 
            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) {
                bRc = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
			pSHA->Dx=*(PCEDEVICE_POWER_STATE)pBufOut;
            //bRc = HW_PowerSet(pSHA, (PCEDEVICE_POWER_STATE)pBufOut);
            if ( bRc ) {
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
            }
            break;

        case IOCTL_POWER_GET: 
            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) {
                bRc = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
			*(PCEDEVICE_POWER_STATE)pBufOut=pSHA->Dx;
            //bRc = HW_PowerGet(pSHA, (PCEDEVICE_POWER_STATE)pBufOut);
            if ( bRc ) {
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
            }
            break;
        default:
            bRc  = FALSE;
            dwErr = ERROR_INVALID_FUNCTION;
            DEBUGMSG (ZONE_ERR, (TEXT("SHA_IOControl Unknown Ioctl: 0x%X\r\n"), dwCode));
            break;            
    }

    LeaveCriticalSection(&pSHA->RegCS);

    if ( !bRc ) {
        DEBUGMSG (ZONE_ERR, (TEXT("SHA_IOControl ERROR: %u\r\n"), dwErr));
        SetLastError(dwErr);
    }

    DEBUGMSG(ZONE_IOCTL,(TEXT("<SHA_IOControl:%d\n"), bRc));

    return bRc;
}


ULONG
SHA_Seek(
   PVOID Context,
   LONG  Position,
   DWORD Type
   )
{
    return (ULONG)-1;
}


BOOL
SHA_PowerUp(
   PVOID Context
   )
{
    return TRUE;//HW_PowerUp(Context);
}


BOOL
SHA_PowerDown(
   PVOID Context
   )
{
    return TRUE;//HW_PowerDown(Context);
}


// EOF
