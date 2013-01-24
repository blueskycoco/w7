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

/*
***************************************************************************//*!
*
* \file        gldriver.cpp
* \brief File implement the dll functions for OpenGl2.0 Lib.    
*
*//*---------------------------------------------------------------------------
* NOTES:
*/


#include <COMMON/framebuffer.h>
#include <COMMON/osalioctl.h>
#include <COMMON/fimg_debug.h>
#include <pm.h>
#include <pmplatform.h>

#include "fgl.h"
#include "register.h"
#include "platform.h"

#include "glConfig.h"

#include "fimgdrv.h"


#ifdef USE_INTERRUPT
#include <bsp.h>

extern "C" {
    DWORD               g_IntrFIMG      = SYSINTR_NOP;
    HANDLE              g_hPipeEvent    = NULL;
}
#endif

#define             G3D_POWER_HANDLE_VALUE        (0xFFFF0000)
DWORD               g_lockedContext = 0;
int                 g_InitDone  = FALSE;
HANDLE              g_hPwrControl;

#define GLDLL_RETAIL_MSG 0

extern "C" BOOL GLES2Initdriver ();
extern "C" BOOL GLES2DeInitdriver ();
extern "C" BOOL GLES2Opendriver ();
extern "C" BOOL GLES2Closedriver ();
extern "C" void GetSFRAddress(int* sfrAddr);
extern "C" void  FreeSFRAddress(int sfrAddr);
extern "C" void WaitForPipelineStatus(unsigned int pipeline_state);

extern "C" void GetPhysicalAddress(BufferAddress* bufAddr);

extern "C" void DoAllocPhysMem(DWORD hOpenContext, void* bufAddr, int size);
extern "C" void DoFreePhysMem(DWORD hOpenContext, void* bufAddr);
extern "C" void GetMemoryStatus(DWORD hOpenContext, int *outUsedMemory, int *outTotalMemory);
extern "C" void GarbageCollect(DWORD hOpenHandle);

extern "C" void RequestDepthBuffer(DWORD hOpenContext, void* bufAddr);
extern "C" void ReleaseDepthBuffer(DWORD hOpenContext, void* bufAddr);

extern "C" BOOL ReAllocMemoryList(DWORD hOpenContext);
extern "C" void FreeMemoryList(DWORD hOpenContext);

extern "C"  void GetDMASFRAddress(int** sfrAddr);
extern "C"  void FreeDMASFRAddress(int** sfrAddr);
extern "C"  void GetDMACODEAddress(UINT32 offset, int cacheEnable, BufferAddress* bufAddr);
extern "C"  void FreeDMACODEAddress(BufferAddress* bufAddr);


// for managing power
void PowerUpProcess(void);
void PowerDownProcess(void);


DBGPARAM dpCurSettings =            
{                                           
    TEXT("FIMG3D Driver"),
    {                                    
        TEXT("Fatal"),               
        TEXT("Error"),            
        TEXT("Warning"),          
        TEXT("Message"),       
        TEXT("Verbose"),            
        TEXT("Call Trace"),         
        TEXT("Alloc"),          
        TEXT("Flip"),        
        TEXT(""),         
        TEXT(""),              
        TEXT(""),                 
        TEXT(""),               
        TEXT(""),              
        TEXT(""),             
        TEXT(""),          
        TEXT(""),                 
    },                                                  
    DEFAULT_MSG_LEVEL           
};


BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:

        RegisterDbgZones((HMODULE)hInstDll, &dpCurSettings);
        
        DisableThreadLibraryCalls((HMODULE) hInstDll);
        break;

    }

    return (TRUE);
}

static HANDLE g_hSFRMutex = NULL;
void MakeMyMutex (void)
{

    g_hSFRMutex = CreateMutex(
         NULL, // No security attributes
         FALSE, // Initially not owned
         TEXT("FIMG3DSFRMutex")); // Name of mutex object
    if(GetLastError() == ERROR_ALREADY_EXISTS) 
    {
        FIMG_PRINT((FIMG_DBG_VERBOSE, "Mutex already exist."));     
    }
    else
    {
        FIMG_PRINT((FIMG_DBG_VERBOSE, "Mutex is not exist."));
    }      
}



extern "C" DWORD GLE_Init( LPCTSTR pContext, DWORD dwBusContext)
{
    FIMG_PRINT((FIMG_DBG_CALLTRACE, "GL20 Init"));

    GLES2Initdriver();
#ifdef USE_INTERRUPT
        DWORD Irq = IRQ_3D;
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &Irq, sizeof(UINT32), &g_IntrFIMG, sizeof(UINT32), NULL))
        {
            FIMG_PRINT((FIMG_DBG_ERROR, " Failed to request the FIMG3D sysintr."));         
            g_IntrFIMG = SYSINTR_UNDEFINED;
            return(FALSE);
        }
    
        g_hPipeEvent = CreateEvent(0,FALSE,FALSE,NULL);
        if ( !g_hPipeEvent ) {
            FIMG_PRINT((FIMG_DBG_ERROR, " Error creating event, FIMG3D failed."));
            return(FALSE);
        }
    
        // initialize the interrupt
        if( !InterruptInitialize(g_IntrFIMG, g_hPipeEvent, NULL, 0) )
        {
            FIMG_PRINT((FIMG_DBG_ERROR, " Unable to initialize interrupt: %u", GetLastError()));
            return(FALSE);
        }
    
#endif

    g_hPwrControl = CreateFile( L"PWC0:", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
    if (INVALID_HANDLE_VALUE == g_hPwrControl )
    {
        FIMG_PRINT((FIMG_DBG_ERROR, " G3D - PWC0 Open Device Failed"));
        return FALSE;
    }

    InitializeCriticalSection(&gles20_fimg_mutex);
    InitializeCriticalSection(&gles20_chunkalloc_mutex);
    InitializeCriticalSection(&gles20_open_mutex);
    
    MakeMyMutex();
    
    
    return 0x64102010;
}

extern "C" DWORD GLE_Deinit()
{
    FIMG_PRINT((FIMG_DBG_CALLTRACE, "GL20 Deinit"));


#ifdef USE_INTERRUPT
    
        if (g_hPipeEvent != NULL) {
            CloseHandle(g_hPipeEvent);
            g_hPipeEvent = NULL;
        }
#endif  

    CloseHandle(g_hPwrControl);

    DeleteCriticalSection(&gles20_fimg_mutex);
    DeleteCriticalSection(&gles20_chunkalloc_mutex);
    DeleteCriticalSection(&gles20_open_mutex);    
    
    GLES2DeInitdriver();
    
    return TRUE;
}


int fimg_dd_open_flag = 0;

extern "C" DWORD GLE_Open(DWORD hDeviceContext,  DWORD AccessCode,  DWORD ShareMode )
{
    FIMG_CONTEXT* openHandle;
    static BOOL   is_first_call = TRUE;


    // The first call of this function is not by the G3D application
    // but by the Power Manager in the booting process.
    // Therefore, the G3D instance is not created
    // and the specific value (G3D_POWER_HANDLE_VALUE) is returned.
    static int cnt = 0;
    cnt++;
    

    
    if((AccessCode == 0x0) && (ShareMode == (FILE_SHARE_READ|FILE_SHARE_WRITE)))
    {
        // for pm
        return G3D_POWER_HANDLE_VALUE;
    }
    
    
    fimg_dd_open_flag = 1;
    
    FIMG_PRINT((FIMG_DBG_MESSAGE, "Open Process ID 0x%X", GetOwnerProcess()));

    openHandle = (FIMG_CONTEXT*)malloc(sizeof(FIMG_CONTEXT));
    memset(openHandle, 0, sizeof(FIMG_CONTEXT));
    
    
    GLES2Opendriver();
    
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08X] driver is opened", openHandle));
    
    return (DWORD)openHandle;
}

extern "C" DWORD GLE_Close(DWORD hOpenHandle)
{
    FIMG_CONTEXT* handle = (FIMG_CONTEXT*)hOpenHandle;

    // If the OpenHandle is power handle, do nothing.
    // The function returns immediately.
    if (hOpenHandle == G3D_POWER_HANDLE_VALUE)
        return TRUE;

    
    
    fimg_dd_open_flag = 0;
    
    GarbageCollect(hOpenHandle);

    // to avoid locking by closing context
    if(g_lockedContext == hOpenHandle)
    {
        LeaveCriticalSection(&gles20_fimg_mutex);
        g_lockedContext = 0;
    }
    
    free(handle);
    handle = 0;

    GLES2Closedriver();
    
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08X] driver is closed", hOpenHandle));

    return TRUE;
}

extern "C" DWORD GLE_Draw()
{
    FIMG_PRINT((FIMG_DBG_CALLTRACE, "GL20 Draw"));

    return 0;
}


extern "C" void GLE_PowerDown( DWORD hDeviceContext )
{
    FIMG_PRINT((FIMG_DBG_CALLTRACE, "GL20 PowerDown"));
}

extern "C" void GLE_PowerUp(DWORD hDeviceContext )
{
    
}


volatile int is_pwr_down = 0;



//------------------------------------------

extern "C" BOOL GLE_IOControl(DWORD hOpenContext,
                                DWORD dwCode,
                                PBYTE pBufIn,
                                DWORD dwLenIn,
                                PBYTE pBufOut,
                                DWORD dwLenOut,
                                PDWORD pdwActualOut)
{
    BOOL    RetVal = TRUE;
//  DWORD   wait_ret;   //unreferenced
    
#if (_WIN32_WCE<600)    
    DWORD dwSavedPermissions = SetProcPermissions( (DWORD)-1 ) ;
#endif    

    switch (dwCode)
    {
    case IOCTL_POWER_CAPABILITIES:
        {
            PPOWER_CAPABILITIES ppc;
    
            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(POWER_CAPABILITIES)) )
            {
                RetVal = FALSE;
                SetLastError (ERROR_INVALID_PARAMETER);             
                break;
            }
    
            ppc = (PPOWER_CAPABILITIES)pBufOut;
    
            memset(ppc, 0, sizeof(POWER_CAPABILITIES));
    
            // support D0, D4
            ppc->DeviceDx = 0x11;
    
            // no wake
            // no inrush
    
            // Report our nominal power consumption in uAmps rather than mWatts.
            ppc->Flags = POWER_CAP_PREFIX_MICRO | POWER_CAP_UNIT_AMPS;
    
            //ppc->Power[D0] = 56000;
    
            *pdwActualOut = sizeof(POWER_CAPABILITIES);
        }
        break;  
    case IOCTL_POWER_QUERY:
        break;
    case IOCTL_POWER_SET:
        {
            CEDEVICE_POWER_STATE NewDx;
    
            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) )
            {
                RetVal = FALSE;
                SetLastError (ERROR_INVALID_PARAMETER);             
                break;
            }
    
            NewDx = *(PCEDEVICE_POWER_STATE)pBufOut;
    
            if ( VALID_DX(NewDx) )
            {
    
                switch ( NewDx )
                {
                case D0:
                    PowerUpProcess();
                    break;
                case D4:

                    PowerDownProcess();
                    break;                  
                default:
                    break;                  

                }
    
                // return our state
                //*(PCEDEVICE_POWER_STATE)pBufOut = g_Dx;
    
    
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
            }
            else
            {
                RetVal = FALSE;
                SetLastError (ERROR_INVALID_PARAMETER);     
            }
            break;
        }               
    case IOCTL_getSFRAddress:
        {
            int* outAddr = (int*)pBufOut;
            GetSFRAddress(outAddr);
            break;
        }    
    case IOCTL_freeSFRAddress:
        {
            int* inAddr = (int*)pBufIn;
            FreeSFRAddress(*inAddr);
            break;
        }        
    case IOCTL_getDMASFRAddress:
        {
            int** outAddr = (int**)pBufOut;
            GetDMASFRAddress(outAddr);
            break;
        }    
    case IOCTL_freeDMASFRAddress:
        {
            int** inAddr = (int**)pBufIn;
            FreeDMASFRAddress(inAddr);
            break;
        }               
    case IOCTL_waitForPipelineStatus:
        {
            unsigned int* pipelineFlags = (unsigned int*)pBufIn;
            WaitForPipelineStatus(*pipelineFlags);
            break;
        }                         
    case IOCTL_GetPhysicalAddress:
        {
            BufferAddress* bufferAddr = (BufferAddress*)pBufIn;
            GetPhysicalAddress(bufferAddr);
        } 
        break; 
    case IOCTL_AllocPhysMem:
        {
            int size = *(int*)pBufIn;
            DoAllocPhysMem(hOpenContext, (void*)pBufOut, size);
        }
        break;
    case IOCTL_FreePhysMem:
        {
            DoFreePhysMem(hOpenContext, (void*)pBufIn);
        }
        break;             
    case IOCTL_RequestDepthBuffer:
        {
            RequestDepthBuffer(hOpenContext, (void*)pBufOut);
        }
        break;
    case IOCTL_ReleaseDepthBuffer:
        {
            ReleaseDepthBuffer(hOpenContext, (void*)pBufIn);
        }        
        break;
    case IOCTL_ReAllocAllMemoryBlock:
        {
            if(!ReAllocMemoryList(hOpenContext))
            {
                RetVal = FALSE;
            }
        }
        break;  
    case IOCTL_ReleaseAllMemoryBlock:
        {
            FreeMemoryList(hOpenContext);
        }
        break;          
    case IOCTL_GetPhysMemStatus:
        {
            GetPhysMemStatusArg *outArgs = (GetPhysMemStatusArg*)pBufOut;
            int usedMemory, totalMemory;
            GetMemoryStatus(hOpenContext, &usedMemory, &totalMemory);
            outArgs->usedMemory = usedMemory;
            outArgs->totalMemory = totalMemory;
            
        }
        break;      
    case IOCTL_LockDrawCall:
        {
            //g_DrawCall = 1;
            EnterCriticalSection(&gles20_fimg_mutex);
            g_lockedContext = (DWORD)hOpenContext;
        }
        break;
    case IOCTL_UnlockDrawCall:
        {
            //g_DrawCall = 0;
            LeaveCriticalSection(&gles20_fimg_mutex);
            g_lockedContext = 0;
        }
        break;        
    case IOCTL_getDMACODEAddress:
        {
            GetDMACODEArg* inArg = (GetDMACODEArg*)pBufIn;
            BufferAddress* bufferAddr = (BufferAddress*)pBufOut;
            GetDMACODEAddress(inArg->offset, inArg->cacheEnable, bufferAddr);
        }    
        break;
    case IOCTL_freeDMACODEAddress:
        {
            BufferAddress* bufferAddr = (BufferAddress*)pBufIn;
            FreeDMACODEAddress(bufferAddr);
        }
        break;
    }
    
#if (_WIN32_WCE<600)        
    SetProcPermissions( dwSavedPermissions ) ;  
#endif  
    
    return RetVal;
}


// for managing power
void PowerUpProcess(void)
{
    FIMG_PRINT((FIMG_DBG_CALLTRACE, "PowerUpProcess"));
    if(g_InitDone == FALSE) return; 

    fglSoftReset();
    is_pwr_down = 0;
    //g_DrawCall = 0;
    //LeaveCriticalSection(&gles20_fimg_mutex);
    
    ReleaseMutex(g_hSFRMutex);
    FIMG_PRINT((FIMG_DBG_CALLTRACE, "PowerUpProcess Down"));
}

void PowerDownProcess(void)
{
    FIMG_PRINT((FIMG_DBG_CALLTRACE, "PowerDownProcess"));
    if(g_InitDone == FALSE) return; 
    // this power down process is available now. But if there are many vertices, it may not work.
    //EnterCriticalSection(&gles20_fimg_mutex);
    
    WaitForSingleObject(g_hSFRMutex, INFINITE);
    

    is_pwr_down = 1;
    FIMG_PRINT((FIMG_DBG_CALLTRACE, "PowerDownProcess Down"));
    
}
