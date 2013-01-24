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
#ifndef __UTILS_H__
#define __UTILS_H__

void Port_Init(void);
void Led_Display(int);

void Uart_Init(void);
void Uart_SendByte(int data);
void Uart_SendString(char *pt);
void Uart_SendDWORD(DWORD d, BOOL cr);
void Uart_SendBYTE(BYTE d, BOOL cr);
char *hex2char(unsigned int val);

#endif    // __UTILS_H__
