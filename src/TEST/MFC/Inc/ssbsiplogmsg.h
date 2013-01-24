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


#ifndef __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPLOGMSG_H__
#define __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPLOGMSG_H__


#define MODULE_NAME    "MFC_APP"

typedef enum
{
    LOG_TRACE   = 0,
    LOG_WARNING = 1,
    LOG_ERROR   = 2
} LOG_LEVEL;

void LOG_MSG(LOG_LEVEL level, const char *func_name, const char *msg, ...);


#endif /* __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPLOGMSG_H__ */

