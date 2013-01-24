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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2005. Samsung Electronics, co. ltd  All rights reserved.

Module Name:

Abstract:

    Platform dependent TOUCH initialization functions

rev:
    2004.4.27    : S3C2440 port 
    2005.05.23    : Magneto porting revision
    2006.06.29    : S3C2443 port
    2007.02.21    : S3C6410 port

Notes:
--*/
#include <windows.h>
#include <types.h>
#include <bsp.h>
#include <creg.hxx>
#include "s3c6410_touch.h"
#include <tchstreamddsi.h>
#include <tchstream.h>
#include <pm.h>
#include <devload.h>


#define CAL_DELTA_RESET             20
#define CAL_HOLD_STEADY_TIME        1500
#define RANGE_MIN                   0
#define RANGE_MAX                   4096


#define RK_HARDWARE_DEVICEMAP_TOUCH     (TEXT("HARDWARE\\DEVICEMAP\\TOUCH"))
#define RV_CALIBRATION_DATA             (TEXT("CalibrationData"))


static TOUCH_DEVICE s_TouchDevice =  {
    FALSE,                                          //bInitialized
    TSP_SAMPLE_RATE_HIGH,                            //nSampleRate
    SYSINTR_NOP,                                    //dwSysIntr
    0,                                              //dwSamplingTimeOut
    FALSE,                                          //bTerminateIST
    0,                                              //hTouchPanelEvent
    D0,                                             //dwPowerState
    0,                                              //nPenIRQ
    DEFAULT_THREAD_PRIORITY                         //dwISTPriority
};


static DWORD                          s_mddContext;
static PFN_TCH_MDD_REPORTSAMPLESET    s_pfnMddReportSampleSet;

static volatile S3C6410_GPIO_REG * g_pGPIOReg = NULL;
static volatile S3C6410_ADC_REG * g_pADCReg = NULL;
static volatile S3C6410_VIC_REG * g_pVIC0Reg = NULL;
static volatile S3C6410_VIC_REG * g_pVIC1Reg = NULL;

static BOOL g_bTSP_DownFlag = FALSE;
static int g_TSP_CurRate = 0;
static unsigned int g_SampleTick_High;
static unsigned int g_SampleTick_Low;
static CRITICAL_SECTION g_csTouchADC;    // Critical Section for ADC Done


static VOID TSP_VirtualFree(VOID);
static BOOL Touch_Pen_Filtering(int *px, int *py);

extern "C" DWORD WINAPI TchPdd_Init(
    LPCTSTR pszActiveKey,
    TCH_MDD_INTERFACE_INFO* pMddIfc,
    TCH_PDD_INTERFACE_INFO* pPddIfc,
    DWORD hMddContext
    );

void WINAPI TchPdd_Deinit(DWORD hPddContext);
void WINAPI TchPdd_PowerUp(DWORD hPddContext);
void WINAPI TchPdd_PowerDown(DWORD hPddContext);
BOOL WINAPI TchPdd_Ioctl(
    DWORD hPddContext,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
    );
VOID  PDDTouchPanelDisable();
BOOL  PDDTouchPanelEnable();
ULONG PDDTouchIST(PVOID   reserved);
void PDDTouchPanelPowerHandler(BOOL boff);



static BOOL
TSP_VirtualAlloc(VOID)
{
    BOOL bRet = TRUE;
    PHYSICAL_ADDRESS    ioPhysicalBase = {0,0};

    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] ++TSP_VirtualAlloc()\r\n")));

    ioPhysicalBase.LowPart = S3C6410_BASE_REG_PA_GPIO;
    g_pGPIOReg = (S3C6410_GPIO_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C6410_GPIO_REG), FALSE);
    if (g_pGPIOReg == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(_T("[TSP:ERR] TSP_VirtualAlloc() : g_pGPIOReg Allocation Fail\r\n")));
        bRet = FALSE;
        goto CleanUp;
    }

    ioPhysicalBase.LowPart = S3C6410_BASE_REG_PA_ADC;
    g_pADCReg = (S3C6410_ADC_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C6410_ADC_REG), FALSE);
    if (g_pADCReg == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(_T("[TSP:ERR] TSP_VirtualAlloc() : g_pADCReg Allocation Fail\r\n")));
        bRet = FALSE;
        goto CleanUp;
    }
    
    ioPhysicalBase.LowPart = S3C6410_BASE_REG_PA_VIC0;
    g_pVIC0Reg = (S3C6410_VIC_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C6410_VIC_REG), FALSE);
    if (g_pVIC0Reg == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(_T("[TSP:ERR] TSP_VirtualAlloc() : g_pVIC0Reg Allocation Fail\r\n")));
        bRet = FALSE;
        goto CleanUp;
    }

    ioPhysicalBase.LowPart = S3C6410_BASE_REG_PA_VIC1;
    g_pVIC1Reg = (S3C6410_VIC_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C6410_VIC_REG), FALSE);
    if (g_pVIC1Reg == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(_T("[TSP:ERR] TSP_VirtualAlloc() : g_pVIC1Reg Allocation Fail\r\n")));
        bRet = FALSE;
        goto CleanUp;
    }

CleanUp:

    if (bRet == FALSE)
    {
        DEBUGMSG(ZONE_ERROR,(_T("[TSP:ERR] TSP_VirtualAlloc() : Failed\r\n")));

        TSP_VirtualFree();
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] --TSP_VirtualAlloc() = %d\r\n"), bRet));

    return bRet;
}

static VOID
TSP_VirtualFree(VOID)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] ++TSP_VirtualFree()\r\n")));

    if (g_pGPIOReg)
    {
        MmUnmapIoSpace((PVOID)g_pGPIOReg, sizeof(S3C6410_SYSCON_REG));
        g_pGPIOReg = NULL;
    }

    if (g_pADCReg)
    {
        MmUnmapIoSpace((PVOID)g_pADCReg, sizeof(S3C6410_ADC_REG));
        g_pADCReg = NULL;
    }

    if (g_pVIC0Reg)
    {
        MmUnmapIoSpace((PVOID)g_pVIC0Reg, sizeof(S3C6410_VIC_REG));
        g_pVIC0Reg = NULL;
    }

    if (g_pVIC1Reg)
    {
        MmUnmapIoSpace((PVOID)g_pVIC1Reg, sizeof(S3C6410_VIC_REG));
        g_pVIC1Reg = NULL;
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] --TSP_VirtualFree()\r\n")));
}

static VOID
TSP_PowerOn(VOID)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] ++TSP_PowerOn()\r\n")));

    g_pADCReg->ADCDLY = ADC_DELAY(TSP_ADC_DELAY);

    g_pADCReg->ADCCON = RESSEL_12BIT | PRESCALER_EN | PRESCALER_VAL(TSP_ADC_PRESCALER) | STDBM_NORMAL;
    
    g_pADCReg->ADCTSC = ADCTSC_WAIT_PENDOWN;
    g_pADCReg->ADCCLRINT = CLEAR_ADC_INT;
    g_pADCReg->ADCCLRWK = CLEAR_ADCWK_INT;

    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] --TSP_PowerOn()\r\n")));
}

static VOID
TSP_PowerOff(VOID)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] ++TSP_PowerOff()\r\n")));

    // To prevent touch locked after wake up,
    // Wait for ADC Done
    // Do not turn off ADC before its A/D conversion finished
    EnterCriticalSection(&g_csTouchADC);
    // ADC Done in PDD_GETXy()..
    LeaveCriticalSection(&g_csTouchADC);

    g_pADCReg->ADCTSC = UD_SEN_DOWN | YM_SEN_EN | YP_SEN_DIS | XM_SEN_DIS | XP_SEN_DIS | PULL_UP_DIS | AUTO_PST_DIS | XY_PST_NOP;

    // ADC Standby Mode, conversion data will be preserved.
    g_pADCReg->ADCCON |= STDBM_STANDBY; 

    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] --TSP_PowerOff()\r\n")));
}

//==============================================================================
// Function Name: PDDCalibrationThread
//
// Description: This function is called from the PDD Init to calibrate the screen.
//              If the calibration data is already present in the registry,
//              this step is skipped, else a call is made into the GWES for calibration.
//
// Arguments:   None.
//
// Ret Value:   Success(1), faliure(0)
//==============================================================================
static HRESULT PDDCalibrationThread()
{
    HKEY hKey;
    DWORD dwType;
    LONG lResult;
    HANDLE hAPIs;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("CalibrationThread+\r\n")));

    // try to open [HKLM\hardware\devicemap\touch] key
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_HARDWARE_DEVICEMAP_TOUCH, 0, KEY_ALL_ACCESS, &hKey))
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("CalibrationThread: calibration: Can't find [HKLM/%s]\r\n"), RK_HARDWARE_DEVICEMAP_TOUCH));
        return E_FAIL;
    }

    // check for calibration data (query the type of data only)
    lResult = RegQueryValueEx(hKey, RV_CALIBRATION_DATA, 0, &dwType, NULL, NULL);
    RegCloseKey(hKey);
    if (lResult == ERROR_SUCCESS)
    {
        // registry contains calibration data, return
        return S_OK;
    }

    hAPIs = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("SYSTEM/GweApiSetReady"));
    if (hAPIs)
    {
        WaitForSingleObject(hAPIs, INFINITE);
        CloseHandle(hAPIs);
    }

    // Perform calibration
    TouchCalibrate();

    // try to open [HKLM\hardware\devicemap\touch] key
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_HARDWARE_DEVICEMAP_TOUCH, 0, KEY_ALL_ACCESS, &hKey))
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("CalibrationThread: calibration: Can't find [HKLM/%s]\r\n"), RK_HARDWARE_DEVICEMAP_TOUCH));
        return E_FAIL;
    }

    // display new calibration data
    lResult = RegQueryValueEx(hKey, RV_CALIBRATION_DATA, 0, &dwType, NULL, NULL);
    if (lResult == ERROR_SUCCESS)
    {
        TCHAR szCalibrationData[100];
        DWORD Size = sizeof(szCalibrationData);

        RegQueryValueEx(hKey, RV_CALIBRATION_DATA, 0, &dwType, (BYTE *) szCalibrationData, (DWORD *) &Size);
    }
    RegCloseKey(hKey);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("CalibrationThread-\r\n")));

    return S_OK;
}


//==============================================================================
// Function Name: PDDStartCalibrationThread
//
// Description: This function is creates the calibration thread with
//              PDDCalibrationThread as entry.
//
// Arguments:   None.
//
// Ret Value:   None
//==============================================================================
void PDDStartCalibrationThread()
{
    HANDLE hThread;

    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PDDCalibrationThread, NULL, 0, NULL);
    // We don't need the handle, close it here
    CloseHandle(hThread);
}

//==============================================================================
// Function Name: TchPdd_Init
//
// Description: PDD should always implement this function. MDD calls it during
//              initialization to fill up the function table with rest of the
//              Helper functions.
//
// Arguments:
//              [IN] pszActiveKey - current active touch driver key
//              [IN] pMddIfc - MDD interface info
//              [OUT]pPddIfc - PDD interface (the function table to be filled)
//              [IN] hMddContext - mdd context (send to MDD in callback fn)
//
// Ret Value:   pddContext.
//==============================================================================
extern "C" DWORD WINAPI TchPdd_Init(LPCTSTR pszActiveKey,
    TCH_MDD_INTERFACE_INFO* pMddIfc,
    TCH_PDD_INTERFACE_INFO* pPddIfc,
    DWORD hMddContext
    )
{
    DWORD pddContext = 0;

    UINT32 Irq[3];

    // Initialize once only
    if (s_TouchDevice.bInitialized)
    {
        pddContext = (DWORD) &s_TouchDevice;
        goto cleanup;
    }

    s_TouchDevice.nSampleRate = TSP_SAMPLE_RATE_HIGH;
    s_TouchDevice.dwPowerState = D0;
    s_TouchDevice.dwSamplingTimeOut = INFINITE;
    s_TouchDevice.bTerminateIST = FALSE;
    s_TouchDevice.hTouchPanelEvent = 0;

    // Read parameters from registry
    if (GetDeviceRegistryParams(
            pszActiveKey,
            &s_TouchDevice,
            _countof(s_deviceRegParams),
            s_deviceRegParams) != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: PDDInitializeHardware: Error reading from Registry.\r\n")));
        return FALSE;
    }


    // Remember the callback function pointer
    s_pfnMddReportSampleSet = pMddIfc->pfnMddReportSampleSet;

    // Remember the mdd context
    s_mddContext = hMddContext;

    if (!TSP_VirtualAlloc())
    {
        DEBUGMSG(ZONE_ERROR,(_T("[TSP:ERR] DdsiTouchPanelEnable() : TSP_VirtualAlloc() Failed\r\n")));
        return FALSE;
    }

    // Initialize Critical Section
    InitializeCriticalSection(&g_csTouchADC);

    // Obtain SysIntr values from the OAL for the touch  interrupt.
    Irq[0] = -1;
    Irq[1] = OAL_INTR_FORCE_STATIC;
    Irq[2] = IRQ_PENDN;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &Irq, sizeof(Irq), &s_TouchDevice.dwSysIntr, sizeof(s_TouchDevice.dwSysIntr), NULL))
    {
        DEBUGMSG(ZONE_ERROR,(_T("[TSP:ERR] DdsiTouchPanelEnable() : IOCTL_HAL_REQUEST_SYSINTR Failed\r\n")));
        s_TouchDevice.dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
        TSP_VirtualFree();
        return FALSE;
    }

    //Calibrate the screen, if the calibration data is not already preset in the registry
    PDDStartCalibrationThread();

    s_TouchDevice.bInitialized = TRUE;
    pddContext = (DWORD) &s_TouchDevice;

    TSP_PowerOn();

    // fill up pddifc table
    pPddIfc->version        = 1;
    pPddIfc->pfnDeinit      = TchPdd_Deinit;
    pPddIfc->pfnIoctl       = TchPdd_Ioctl;
    pPddIfc->pfnPowerDown   = TchPdd_PowerDown;
    pPddIfc->pfnPowerUp     = TchPdd_PowerUp;

cleanup:
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Init-\r\n")));
    return pddContext;

}



//==============================================================================
// Function Name: TchPdd_DeInit
//
// Description: MDD calls it during deinitialization. PDD should deinit hardware
//              and deallocate memory etc.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//
// Ret Value:   None
//
//==============================================================================
void WINAPI TchPdd_Deinit(DWORD hPddContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Deinit+\r\n")));

    // Close the IST and release the resources before
    // de-initializing.
    PDDTouchPanelDisable();

    if (s_TouchDevice.bInitialized)
    {
        TSP_PowerOff();
        TSP_VirtualFree();

        // Release interrupt
        if (s_TouchDevice.dwSysIntr != 0)
            {
            KernelIoControl(
                IOCTL_HAL_RELEASE_SYSINTR,
                &s_TouchDevice.dwSysIntr,
                sizeof(s_TouchDevice.dwSysIntr),
                NULL,
                0,
                NULL
                );
            }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Deinit-\r\n")));
}

//==============================================================================
// Function Name: TchPdd_Ioctl
//
// Description: The MDD controls the touch PDD through these IOCTLs.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//              DOWRD dwCode. IOCTL code
//              PBYTE pBufIn. Input Buffer pointer
//              DWORD dwLenIn. Input buffer length
//              PBYTE pBufOut. Output buffer pointer
//              DWORD dwLenOut. Output buffer length
//              PWORD pdwAcutalOut. Actual output buffer length.
//
// Ret Value:   TRUE if success else FALSE. SetLastError() if FALSE.
//==============================================================================
BOOL WINAPI TchPdd_Ioctl(DWORD hPddContext,
      DWORD dwCode,
      PBYTE pBufIn,
      DWORD dwLenIn,
      PBYTE pBufOut,
      DWORD dwLenOut,
      PDWORD pdwActualOut
)
{
    DWORD dwResult = ERROR_INVALID_PARAMETER;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hPddContext);

    // Only allow kernel process to call PDD ioctls.
    if (GetCurrentProcessId() != GetDirectCallerProcessId()) {
        DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Can be called only from device process (caller process id 0x%08x)\r\n", \
            GetDirectCallerProcessId()));
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
    switch (dwCode)
    {
        //  Enable touch panel
        case IOCTL_TOUCH_ENABLE_TOUCHPANEL:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_ENABLE_TOUCHPANEL\r\n"));
            PDDTouchPanelEnable();
            dwResult = ERROR_SUCCESS;
            break;

        //  Disable touch panel
        case IOCTL_TOUCH_DISABLE_TOUCHPANEL:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_DISABLE_TOUCHPANEL\r\n"));
            PDDTouchPanelDisable();
            dwResult = ERROR_SUCCESS;
            break;


        //  Get current sample rate
        case IOCTL_TOUCH_GET_SAMPLE_RATE:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_GET_SAMPLE_RATE\r\n"));


            //  Check parameter validity
            if ((pBufOut != NULL) || (dwLenOut == sizeof(DWORD)))
            {
                __try {
                    if (pdwActualOut)
                        *pdwActualOut = sizeof(DWORD);

                    //  Get the sample rate
                    *((DWORD*)pBufOut) = s_TouchDevice.nSampleRate;

                    dwResult = ERROR_SUCCESS;
                }
                __except(EXCEPTION_EXECUTE_HANDLER ) {
                    dwResult = FALSE;
                    DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_TOUCH_GET_SAMPLE_RATE\r\n"));
                }
            }

            break;

        //  Set the current sample rate
        case IOCTL_TOUCH_SET_SAMPLE_RATE:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_SET_SAMPLE_RATE\r\n"));

                //  Check parameter validity
                if ((pBufIn != NULL) || (dwLenIn == sizeof(DWORD)))
                {
                    __try {
                        //  Set the sample rate
                        s_TouchDevice.nSampleRate = *((DWORD*)pBufIn);
                        dwResult = ERROR_SUCCESS;
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER ) {
                        dwResult = FALSE;
                        DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_TOUCH_SET_SAMPLE_RATE\r\n"));
                    }
                }
            break;

        //  Get touch properties
        case IOCTL_TOUCH_GET_TOUCH_PROPS:

            //  Check parameter validity
            if (pBufOut != NULL || dwLenOut == sizeof(TCH_PROPS))
            {
                __try {
                    if (pdwActualOut)
                        *pdwActualOut = sizeof(TCH_PROPS);

                    //  Fill out the touch driver properties
                    ((TCH_PROPS*)pBufOut)->minSampleRate            = TSP_SAMPLE_RATE_LOW;
                    ((TCH_PROPS*)pBufOut)->maxSampleRate            = TSP_SAMPLE_RATE_HIGH;
                    ((TCH_PROPS*)pBufOut)->minCalCount              = 5;
                    ((TCH_PROPS*)pBufOut)->maxSimultaneousSamples   = 1;
                    ((TCH_PROPS*)pBufOut)->touchType                = TOUCHTYPE_SINGLETOUCH;
                    ((TCH_PROPS*)pBufOut)->calHoldSteadyTime        = CAL_HOLD_STEADY_TIME;
                    ((TCH_PROPS*)pBufOut)->calDeltaReset            = CAL_DELTA_RESET;
                    ((TCH_PROPS*)pBufOut)->xRangeMin                = RANGE_MIN;
                    ((TCH_PROPS*)pBufOut)->xRangeMax                = RANGE_MAX;
                    ((TCH_PROPS*)pBufOut)->yRangeMin                = RANGE_MIN;
                    ((TCH_PROPS*)pBufOut)->yRangeMax                = RANGE_MAX;
                    dwResult = ERROR_SUCCESS;
                }
                __except(EXCEPTION_EXECUTE_HANDLER ) {
                    dwResult = FALSE;
                    DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_TOUCH_GET_TOUCH_PROPS\r\n"));
                }
            }

            break;

        //  Power management IOCTLs
        case IOCTL_POWER_CAPABILITIES:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_CAPABILITIES\r\n"));
            if (pBufOut != NULL && dwLenOut == sizeof(POWER_CAPABILITIES))
            {
                __try {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
                memset(ppc, 0, sizeof(*ppc));
                ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D1) | DX_MASK(D2) | DX_MASK(D3) | DX_MASK(D4);

                if (pdwActualOut)
                    *pdwActualOut = sizeof(POWER_CAPABILITIES);

                dwResult = ERROR_SUCCESS;
                }
                __except(EXCEPTION_EXECUTE_HANDLER ) {
                    dwResult = FALSE;
                    DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_POWER_CAPABILITIES\r\n"));
                }
            }
            break;

        case IOCTL_POWER_GET:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_GET\r\n"));
            if(pBufOut != NULL && dwLenOut == sizeof(CEDEVICE_POWER_STATE))
            {
                __try {
                    *(PCEDEVICE_POWER_STATE) pBufOut = (CEDEVICE_POWER_STATE) s_TouchDevice.dwPowerState;

                    if (pdwActualOut)
                        *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                    dwResult = ERROR_SUCCESS;
                }
                __except(EXCEPTION_EXECUTE_HANDLER ) {
                    dwResult = FALSE;
                    DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_POWER_GET\r\n"));
                }
            }
            break;

        case IOCTL_POWER_SET:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_SET\r\n"));
            if(pBufOut != NULL && dwLenOut == sizeof(CEDEVICE_POWER_STATE))
            {
                __try {

                    CEDEVICE_POWER_STATE dx = *(CEDEVICE_POWER_STATE*)pBufOut;
                    if( VALID_DX(dx) )
                    {
                        DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_SET = to D%u\r\n", dx));
                        s_TouchDevice.dwPowerState = dx;

                        if (pdwActualOut)
                            *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                        //  Enable touchscreen for D0-D2; otherwise disable
                        switch( dx )
                        {
                            case D0:
                            case D1:
                            case D2:
                                //  Enable touchscreen
                                PDDTouchPanelPowerHandler(FALSE);    //Enable Touch ADC
                                break;

                            case D3:
                            case D4:
                                //  Disable touchscreen
                                PDDTouchPanelPowerHandler(TRUE); //Disable Touch ADC
                                break;

                            default:
                                //  Ignore
                                break;
                        }

                        dwResult = ERROR_SUCCESS;
                    }
                }
                __except(EXCEPTION_EXECUTE_HANDLER ) {
                    dwResult = FALSE;
                    DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_POWER_SET\r\n"));
                }                
            }
            break;

        default:
            dwResult = ERROR_NOT_SUPPORTED;
            break;
    }

    if (dwResult != ERROR_SUCCESS)
    {
        SetLastError(dwResult);
        return FALSE;
    }
    return TRUE;
}


//==============================================================================
// Function Name: TchPdd_PowerUp
//
// Description: MDD passes xxx_PowerUp stream interface call to PDD.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//
// Ret Value:   None
//==============================================================================
void WINAPI TchPdd_PowerUp(
    DWORD hPddContext
    )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_PowerUp+\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hPddContext);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_PowerUp-\r\n")));
}

//==============================================================================
// Function Name: TchPdd_PowerDown
//
// Description: MDD passes xxx_PowerDown stream interface call to PDD.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//
// Ret Value:   None
//==============================================================================
void WINAPI TchPdd_PowerDown(
    DWORD hPddContext
    )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_PowerDown+\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hPddContext);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_PowerDown-\r\n")));
}



//==============================================================================
//Internal Functions
//==============================================================================

//==============================================================================
// Function Name: PDD_GetXY
//
// Description: Helper function to read 8 samples if it is detailed sampling or else 4 samples and find the average of them.
//
// Arguments:
//                  INT * - Pointer to the x coordinate of the sample.
//                  INT * - Pointer to the y coordinate of the sample.
//
// Ret Value:   TRUE
//==============================================================================

static BOOL
PDD_GetXY(int *px, int *py)
{
    int i,j,k;
    int temp;
    int x[TSP_SAMPLE_NUM], y[TSP_SAMPLE_NUM];
    int dx, dy;
    int TimeOut = 100;  // about 100ms

    EnterCriticalSection(&g_csTouchADC);

    for (i = 0; i < TSP_SAMPLE_NUM; i++)
    {
        g_pADCReg->ADCTSC = ADCTSC_AUTO_ADC;    // Auto Conversion
        g_pADCReg->ADCCON |= ENABLE_START_EN;    // ADC Conversion Start

        while (g_pADCReg->ADCCON & ENABLE_START_EN)
        {    // Wait for Start Bit Cleared
            if(TimeOut-- < 0)
            {
                DEBUGMSG(ZONE_ERROR,(TEXT("ADC cannot start\n")));
                goto ADCfails;
            }        
            Sleep(1);
        }

        TimeOut = 100;  // about 100ms
        while (!(g_pADCReg->ADCCON & ECFLG_END))
        {    // Wait for ADC Conversion Ended
            if(TimeOut-- < 0)
            {
                DEBUGMSG(ZONE_ERROR,(TEXT("ADC Conversion cannot be done\n")));
                goto ADCfails;
            }        
            Sleep(1);
        }

        x[i] = D_XPDATA_MASK(g_pADCReg->ADCDAT0);
        y[i] = D_YPDATA_MASK(g_pADCReg->ADCDAT1);
    }

ADCfails:
    LeaveCriticalSection(&g_csTouchADC);

    for (j = 0; j < TSP_SAMPLE_NUM -1; ++j)
    {
        for (k = j+1; k < TSP_SAMPLE_NUM; ++k)
        {
            if(x[j]>x[k])
            {
                temp = x[j];
                x[j]=x[k];
                x[k]=temp;
            }

            if(y[j]>y[k])
            {
                temp = y[j];
                y[j]=y[k];
                y[k]=temp;
            }
        }
    }

#ifdef    DETAIL_SAMPLING
    // 8 samples Interpolation (weighted 4 samples)
    *px = (x[2] + ((x[3]+x[4])<<1) + (x[3]+x[4]) + x[5]);
    *py = (y[2] + ((y[3]+y[4])<<1) + (y[3]+y[4]) + y[5]);

    if ((*px & 0x7) > 3) *px = (*px>>3) + 1;
    else *px = *px>>3;

    if ((*py & 0x7) > 3) *py = (*py>>3) + 1;
    else *py = *py>>3;

    dx = x[5] - x[2];
    dy = y[5] - y[2];
#else
    // 2 samples average
    *px = (x[1] + x[2] + 1)>>1;
    *py = (y[1] + y[2] + 1)>>1;

    dx = x[2] - x[1];
    dy = y[2] - y[1];
#endif


    if ((dx > TSP_INVALIDLIMIT) || (dy > TSP_INVALIDLIMIT))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static BOOL
Touch_Pen_Filtering(INT *px, INT *py)
{
    BOOL RetVal = TRUE;
    // TRUE  : Valid pen sample
    // FALSE : Invalid pen sample
    INT Filter_Margin;
    static int count = 0;
    static INT x[2], y[2];
    INT TmpX, TmpY;
    INT dx, dy;

    if(*px <0 && *py <0)
    {
        count = 0;
        return FALSE;
    }
    else
    {
        count++;
    }

    if (count > 2)
    {
        // apply filtering rule
        count = 2;

        // average between x,y[0] and *px,y
        TmpX = (x[0] + *px)>>1;
        TmpY = (y[0] + *py)>>1;

        // difference between x,y[1] and TmpX,Y
        dx = (x[1] > TmpX) ? (x[1] - TmpX) : (TmpX - x[1]);
        dy = (y[1] > TmpY) ? (y[1] - TmpY) : (TmpY - y[1]);

        Filter_Margin = (x[1] > x[0]) ? (x[1]-x[0]) : (x[0]-x[1]);
        Filter_Margin += (y[1] > y[0]) ? (y[1]-y[0]) : (y[0]-y[1]);
        Filter_Margin += TSP_FILTER_LIMIT;

        if ((dx > Filter_Margin) || (dy > Filter_Margin)) {
            // Invalid pen sample
            *px = x[1];
            *py = y[1]; // previous valid sample
            RetVal = FALSE;
            count = 0;
        }
        else
        {
            // Valid pen sample
            x[0] = x[1]; y[0] = y[1];
            x[1] = *px; y[1] = *py; // reserve pen samples

            RetVal = TRUE;
        }
    }
    else // (count > 2)
    { // till 2 samples, no filtering rule
        x[0] = x[1]; y[0] = y[1];
        x[1] = *px; y[1] = *py; // reserve pen samples

        RetVal = FALSE;
    }

    return RetVal;
}

//==============================================================================
// Function Name: PDDTouchPanelDisable
//
// Description: This routine closes the IST and releases other resources.
//
// Arguments:  None
//
// Ret Value:   TRUE - Success
//==============================================================================
VOID  PDDTouchPanelDisable()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDTouchPanelDisable+\r\n")));

    //  Disable touch interrupt service thread
    if( s_TouchDevice.hTouchPanelEvent )
    {
        if (s_TouchDevice.dwSysIntr != SYSINTR_NOP)
            InterruptMask(s_TouchDevice.dwSysIntr, TRUE);

        s_TouchDevice.bTerminateIST = TRUE;
        SetEvent( s_TouchDevice.hTouchPanelEvent );
        Sleep(1);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDTouchPanelDisable-\r\n")));
}

//==============================================================================
// Function Name: PDDTouchPanelEnable
//
// Description: This routine creates the touch thread(if it is not already created)
//              initializes the interrupt and unmasks it.
//
// Arguments:  None
//
// Ret Value:   TRUE - Success
//==============================================================================
BOOL  PDDTouchPanelEnable()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDTouchPanelEnable+\r\n")));

    //  Check if already running
    if( s_TouchDevice.hTouchPanelEvent == NULL )
    {
        //  Clear terminate flag
        s_TouchDevice.bTerminateIST = FALSE;

        //  Create touch event
        s_TouchDevice.hTouchPanelEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        if( !s_TouchDevice.hTouchPanelEvent )
        {
            DEBUGMSG(ZONE_ERROR,(TEXT("PDDTouchPanelEnable: Failed to initialize touch event.\r\n")));
            return FALSE;
        }

        //  Map SYSINTR to event
        if (!InterruptInitialize(s_TouchDevice.dwSysIntr, s_TouchDevice.hTouchPanelEvent, NULL, 0))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("PDDTouchPanelEnable: Failed to initialize interrupt.\r\n")));
            return FALSE;
        }

        //  Create IST thread
        HANDLE hthread = CreateThread( NULL, 0, PDDTouchIST, 0, 0, NULL );
        if( hthread == NULL )
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("PDDTouchPanelEnable: Failed to create IST thread\r\n")));
            return FALSE;
        }

        // set IST thread priority
        CeSetThreadPriority (hthread, s_TouchDevice.dwISTPriority);


        if (s_TouchDevice.dwSysIntr != SYSINTR_NOP)
            InterruptMask(s_TouchDevice.dwSysIntr, FALSE);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDTouchPanelEnable-\r\n")));
    return TRUE;
}


//==============================================================================
// Function Name: PDDTouchPanelGetPoint
//
// Description: This function is used to read the touch coordinates from the h/w.
//
// Arguments:
//                  TOUCH_PANEL_SAMPLE_FLAGS * - Pointer to the sample flags. This flag is filled with the
//                                                                      values TouchSampleDownFlag or TouchSampleValidFlag;
//                  INT * - Pointer to the x coordinate of the sample.
//                  INT * - Pointer to the y coordinate of the sample.
//
// Ret Value:   None
//==============================================================================

void PDDTouchPanelGetPoint(TOUCH_PANEL_SAMPLE_FLAGS *pTipState,INT *pUncalX,INT *pUncalY)
{
    static int PrevX=0;
    static int PrevY=0;
    int TmpX = 0;
    int TmpY = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDTouchPanelGetPoint\r\n")));

    if (g_pVIC1Reg->VICRAWINTR & (1<<(PHYIRQ_PENDN-VIC1_BIT_OFFSET)))        // s_TouchDevice.dwSysIntr Interrupt Case
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("[TSP] s_TouchDevice.dwSysIntr(PHYIRQ_PENDN) Case\r\n")));

        if ((g_pADCReg->ADCDAT0 & D_UPDOWN_UP)
            || (g_pADCReg->ADCDAT1 & D_UPDOWN_UP))
        {
            DEBUGMSG(ZONE_FUNCTION, (_T("[TSP] Pen Up\r\n")));

            g_bTSP_DownFlag = FALSE;

            g_pADCReg->ADCTSC = ADCTSC_WAIT_PENDOWN;
        }
        else
        {
            DEBUGMSG(ZONE_FUNCTION, (_T("[TSP] Pen Down\r\n")));

            g_bTSP_DownFlag = TRUE;

            g_pADCReg->ADCTSC = ADCTSC_WAIT_PENUP;

        }

        *pUncalX = PrevX;
        *pUncalY = PrevY;

        g_pADCReg->ADCCLRWK = CLEAR_ADCWK_INT;

    }


    // Set up interrupt/timer for next sample
    if (g_bTSP_DownFlag)
    {
        if (PDD_GetXY(&TmpX, &TmpY) == TRUE)
        {
            if(Touch_Pen_Filtering(&TmpX, &TmpY))
            {
                *pTipState = TouchSampleValidFlag | TouchSampleDownFlag;
                *pTipState &= ~TouchSampleIgnore;
            }
            else        // Invalid touch pen
            {
                *pTipState = TouchSampleIgnore;
            }

            *pUncalX = PrevX = TmpX;
            *pUncalY = PrevY = TmpY;

            g_pADCReg->ADCTSC = ADCTSC_WAIT_PENUP;
        }
        else
        {
            *pTipState = TouchSampleIgnore;
        }

        // Pen down so set MDD timeout for polling mode.
        s_TouchDevice.dwSamplingTimeOut = 1000 / s_TouchDevice.nSampleRate;
    }
    else
    {
        // Reset the delayed sample counter
        *pUncalX  = 0;
        *pUncalY = 0;

        // Pen up so set MDD timeout for interrupt mode.
        s_TouchDevice.dwSamplingTimeOut = INFINITE;
        *pTipState = TouchSampleValidFlag;
        InterruptDone(s_TouchDevice.dwSysIntr);        // Not handled in MDD
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] --DdsiTouchPanelGetPoint()\r\n")));
}

//==============================================================================
// Function Name: PDDTouchIST
//
// Description: This is the IST which waits on the touch event or Time out value.
//              Normally the IST waits on the touch event infinitely, but once the
//              pen down condition is recognized the time out interval is changed
//              to dwSamplingTimeOut.
//
// Arguments:
//                  PVOID - Reseved not currently used.
//
// Ret Value:   None
//==============================================================================
ULONG PDDTouchIST(PVOID   reserved)
{
    TOUCH_PANEL_SAMPLE_FLAGS    SampleFlags = 0;
    CETOUCHINPUT input;
    INT32                       RawX, RawY;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(reserved);

    DEBUGMSG(ZONE_FUNCTION, (L"PDDTouchIST: IST thread started\r\n"));

    //  Loop until told to stop
    while(!s_TouchDevice.bTerminateIST)
    {
        //  Wait for touch IRQ, timeout or terminate flag
       WaitForSingleObject(s_TouchDevice.hTouchPanelEvent, s_TouchDevice.dwSamplingTimeOut);

        //  Check for terminate flag
        if (s_TouchDevice.bTerminateIST)
            break;

        PDDTouchPanelGetPoint( &SampleFlags, &RawX, &RawY);

        if ( SampleFlags & TouchSampleIgnore )
        {
            // do nothing, not a valid sample
            continue;
        }

         //convert x-y coordination to one sample set before sending it to touch MDD.
         if(ConvertTouchPanelToTouchInput(SampleFlags, RawX, RawY, &input))
         {
               //send this 1 sample to mdd
               s_pfnMddReportSampleSet(s_mddContext, 1, &input);
         }

        DEBUGMSG(ZONE_FUNCTION, ( TEXT( "Sample:   (%d, %d)\r\n" ), RawX, RawY ) );

    }

    // IST thread terminating
    InterruptDone(s_TouchDevice.dwSysIntr);
    InterruptDisable(s_TouchDevice.dwSysIntr);

    CloseHandle(s_TouchDevice.hTouchPanelEvent);
    s_TouchDevice.hTouchPanelEvent = NULL;

    DEBUGMSG(ZONE_FUNCTION, (L"PDDTouchIST: IST thread ending\r\n"));

    return ERROR_SUCCESS;
}



//==============================================================================
// Function: PDDTouchPanelPowerHandler
//
// This function indicates to the driver that the system is entering
// or leaving the suspend state.
//
// Parameters:
//      bOff
//          [in] TRUE indicates that the system is turning off. FALSE
//          indicates that the system is turning on.
//
// Returns:
//      None.
//==============================================================================
void PDDTouchPanelPowerHandler(BOOL boff)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] ++DdsiTouchPanelPowerHandler(%d)\r\n"), boff));

    if (boff)
    {
        TSP_PowerOff();
    }
    else
    {
        TSP_PowerOn();
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("[TSP] --DdsiTouchPanelPowerHandler()\r\n")));
}


