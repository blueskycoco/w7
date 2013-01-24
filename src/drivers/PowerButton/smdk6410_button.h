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


#ifndef _SMDK6410_BUTTON_H_
#define _SMDK6410_BUTTON_H_

BOOL Button_initialize_register_address(void *pGPIOReg);

void Button_pwrbtn_port_initialize(void);
BOOL Button_pwrbtn_set_interrupt_method(EINT_SIGNAL_METHOD eMethod);
BOOL Button_pwrbtn_set_filter_method(EINT_FILTER_METHOD eMethod, unsigned int uiFilterWidth);
void Button_pwrbtn_enable_interrupt(void);
void Button_pwrbtn_disable_interrupt(void);
void Button_pwrbtn_clear_interrupt_pending(void);
BOOL Button_pwrbtn_is_pushed(void);

void Button_rstbtn_port_initialize(void);
BOOL Button_rstbtn_set_interrupt_method(EINT_SIGNAL_METHOD eMethod);
BOOL Button_rstbtn_set_filter_method(EINT_FILTER_METHOD eMethod, unsigned int uiFilterWidth);
void Button_rstbtn_enable_interrupt(void);
void Button_rstbtn_disable_interrupt(void);
void Button_rstbtn_clear_interrupt_pending(void);
BOOL Button_rstbtn_is_pushed(void);

#endif    // _SMDK6410_BUTTON_H_

