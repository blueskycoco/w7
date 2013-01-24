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

#ifndef _BKL_PDD_H_
#define _BKL_PDD_H_

#if __cplusplus
extern "C"
{
#endif

// This structure is in place to store any local variables that the PDD
// might want to store, which is returned to the MDD
typedef struct {
    volatile S3C6410_GPIO_REG *v_pGPIORegs;
    volatile S3C6410_PWM_REG *v_pPWMRegs;
} BKL_PDD_INFO;

void BL_Set(BKL_PDD_INFO* pddInfo,BOOL bOn);
void BL_InitPWM(BKL_PDD_INFO* pddInfo);
void BL_SetBrightness(BKL_PDD_INFO* pddInfo,DWORD dwValue);
/*
    Initialize hardware etc
    Returned DWORD will be passed to BacklightDeInit and should be used to store context if necessary
*/
DWORD BacklightInit(BKL_PDD_INFO* pddInfo);

#if __cplusplus
       }
#endif

#endif _BKL_PDD_H_
