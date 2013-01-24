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
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "SsbSipLogMsg.h"


static const LOG_LEVEL log_level = LOG_TRACE;
static const char *modulename = MODULE_NAME;
static const char *level_str[] = {"TRACE", "WARNING", "ERROR"};

void LOG_MSG(LOG_LEVEL level, const char *func_name, const char *msg, ...)
{
    
    char buf[256];
    va_list argptr;

    if (level < log_level)
        return;

    sprintf_s(buf,_countof(buf), "[%s: %s] %s: ", modulename, level_str[level], func_name);

    va_start(argptr, msg);
    vsprintf_s((buf + strlen(buf)),(_countof(buf)-strlen(buf)), msg, argptr);
    printf(buf);
    va_end(argptr);
}
