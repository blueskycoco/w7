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
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//

#ifndef    _S3C6410_ADC_TOUCH_MACRO_H_
#define    _S3C6410_ADC_TOUCH_MACRO_H_

#include <nkintr.h>


#if __cplusplus
extern "C"
{
#endif


#ifdef DEBUG

#define ZONE_FUNCTION   DEBUGZONE(2)

#endif



// ADCCON
#define RESSEL_10BIT        (0<<16)
#define RESSEL_12BIT        (1<<16)

#define ECFLG_PROCESS       (0<<15)
#define ECFLG_END           (1<<15)
#define PRESCALER_DIS       (0<<14)
#define PRESCALER_EN        (1<<14)
#define PRESCALER_VAL(n)    (((n)&0xff)<<6)
#define PRESCALER_MASK      (0xff<<6)
#define SEL_MUX_AIN0        (0<<3)
#define SEL_MUX_AIN1        (1<<3)
#define SEL_MUX_AIN2        (2<<3)
#define SEL_MUX_AIN3        (3<<3)
#define SEL_MUX_YM          (4<<3)
#define SEL_MUX_YP          (5<<3)
#define SEL_MUX_XM          (6<<3)
#define SEL_MUX_XP          (7<<3)
#define SEL_MUX_MASK        (7<<3)
#define STDBM_NORMAL        (0<<2)
#define STDBM_STANDBY       (1<<2)
#define READ_START_DIS      (0<<1)
#define READ_START_EN       (1<<1)
#define ENABLE_START_DIS    (0<<0)
#define ENABLE_START_EN     (1<<0)

// ADCTSC
#define UD_SEN_DOWN         (0<<8)
#define UD_SEN_UP           (1<<8)
#define YM_SEN_DIS          (0<<7)
#define YM_SEN_EN           (1<<7)
#define YP_SEN_EN           (0<<6)
#define YP_SEN_DIS          (1<<6)
#define XM_SEN_DIS          (0<<5)
#define XM_SEN_EN           (1<<5)
#define XP_SEN_EN           (0<<4)
#define XP_SEN_DIS          (1<<4)
#define PULL_UP_EN          (0<<3)
#define PULL_UP_DIS         (1<<3)
#define AUTO_PST_DIS        (0<<2)
#define AUTO_PST_EN         (1<<2)
#define XY_PST_NOP          (0<<0)
#define XY_PST_XPOS         (1<<0)
#define XY_PST_YPOS         (2<<0)
#define XY_PST_WAITINT      (3<<0)
#define XY_PST_MASK         (3<<0)

// ADCDLY
#define FILCLKSRC_EXTCLK    (0<<16)
#define FILCLKSRC_RTCCLK    (1<<16)
#define ADC_DELAY(n)        ((n+1)&0xffff)
#define ADC_DELAY_MASK      (0xffff)

// ADCDAT0
#define D_UPDOWN_DOWN       (0<<15)
#define D_UPDOWN_UP         (1<<15)
#define D_AUTO_PST_DIS      (0<<14)
#define D_AUTO_PST_EN       (1<<14)
#define D_XY_PST_NOP        (0<<12)
#define D_XY_PST_XPOS       (1<<12)
#define D_XY_PST_YPOS       (2<<12)
#define D_XY_PST_WAITINT    (3<<12)
#define D_XY_PST_MASK       (3<<12)

#define D_XPDATA_MASK(n)    ((n)&0xfff)
#define D_YPDATA_MASK(n)    ((n)&0xfff)

// ADCUPDN
#define TSC_DN_INT          (1<<1)
#define TSC_UP_INT          (1<<0)

// ADCCLRINT
#define CLEAR_ADC_INT       (0xff)    // can be any value...

// ADCCLRWK
#define CLEAR_ADCWK_INT     (0xff)    // can be any value...

#define ADCTSC_WAIT_PENDOWN     (UD_SEN_DOWN    |YM_SEN_EN|YP_SEN_DIS|XM_SEN_DIS|XP_SEN_DIS|PULL_UP_EN|AUTO_PST_DIS|XY_PST_WAITINT)
#define ADCTSC_WAIT_PENUP       (UD_SEN_UP        |YM_SEN_EN|YP_SEN_DIS|XM_SEN_DIS|XP_SEN_DIS|PULL_UP_EN|AUTO_PST_DIS|XY_PST_WAITINT)
#define ADCTSC_AUTO_ADC         (UD_SEN_DOWN    |YM_SEN_EN|YP_SEN_DIS|XM_SEN_DIS|XP_SEN_DIS|PULL_UP_DIS|AUTO_PST_EN|XY_PST_NOP)

#define DETAIL_SAMPLING
#define NEW_FILTER_SCHEME

#define TSP_ADC_DELAY           (10000)
#define TSP_ADC_PRESCALER       (24)

#define TSP_SAMPLE_RATE_LOW     (50)    // 50 Samples per Sec
#define TSP_SAMPLE_RATE_HIGH    (100)   // 100 Samples per Sec

#define TSP_TIMER_DIVIDER       (DIVIDE_1)     // use DIVIDE_1 .. DIVIDE_16
#define TSP_TIMER_CNT_LOW       (S3C6410_PCLK/SYS_TIMER_PRESCALER/(1<<TSP_TIMER_DIVIDER)/TSP_SAMPLE_RATE_LOW-1)
#define TSP_TIMER_CNT_HIGH      (S3C6410_PCLK/SYS_TIMER_PRESCALER/(1<<TSP_TIMER_DIVIDER)/TSP_SAMPLE_RATE_HIGH-1)

#define TSP_INVALIDLIMIT        (40)


#define TSP_FILTER_LIMIT        (3)

#ifdef DETAIL_SAMPLING
#define TSP_SAMPLE_NUM          (8)
#else
#define TSP_SAMPLE_NUM          (4)
#endif

#define DEFAULT_THREAD_PRIORITY             109

#define PARAM_DWORD             1
#define offset(s, f)        FIELD_OFFSET(s, f)
#define fieldsize(s, f)     sizeof(((s*)0)->f)


typedef struct {
    LPTSTR name;
    DWORD  type;
    BOOL   required;
    DWORD  offset;
    DWORD  size;
    PVOID  pDefault;
} DEVICE_REGISTRY_PARAM;


typedef struct
{
    BOOL        bInitialized;

    DWORD       nSampleRate;

    DWORD       dwSysIntr;
    DWORD       dwSamplingTimeOut;
    BOOL        bTerminateIST;
    HANDLE      hTouchPanelEvent;
    DWORD       dwPowerState;

    LONG        nPenIRQ;
    DWORD       dwISTPriority;
}TOUCH_DEVICE;


//------------------------------------------------------------------------------
//  Device registry parameters
static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"SampleRate", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, nSampleRate),
        fieldsize(TOUCH_DEVICE, nSampleRate), (VOID*)TSP_SAMPLE_RATE_HIGH
    },
    {
        L"Priority256", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, dwISTPriority),
        fieldsize(TOUCH_DEVICE, dwISTPriority), (VOID*)DEFAULT_THREAD_PRIORITY
    },
    {
        L"SysIntr", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, dwSysIntr),
        fieldsize(TOUCH_DEVICE, dwSysIntr), (VOID*)SYSINTR_NOP
    }
};

DWORD
GetDeviceRegistryParams(
    LPCWSTR szContext,
    VOID *pBase,
    DWORD count, 
    const DEVICE_REGISTRY_PARAM params[]
    );


#if __cplusplus
}
#endif

#endif    // _S3C6410_ADC_TOUCH_MACRO_H_
