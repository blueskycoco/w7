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
/**
 * Samsung Project
 * Copyright (c) 2008 Mobile Solution, Samsung Electronics, Inc.
 * All right reserved.
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics Inc. ("Confidential Information"). You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics.
 */

/**
 * @file    GPI_Wince.cpp
 * @brief   Graphic Platform Interface for Windows Mobile 6.1
 * @author  Imbum Oh
 * @version 1.0
 */

/* Includes */

#include <COMMON/fimg_debug.h>

void GPIDebugPrintf(unsigned int ui32DebugLevel,
                                    const char *pszFilePath,
                                    unsigned int ui32Line,
                                    const char *pszFormat,
                                    ...)
{
    BOOL bDebug,bTrace;
    int nLen, i;    
    
    // test for setting debug zone
    bDebug = DEBUGZONE(ui32DebugLevel);
    
    if(bDebug)
    {
        va_list ArgList;
        static char szBuffer[FIMG_MAX_DEBUG_MESSAGE_LEN+1];
        TCHAR szOutMessage[FIMG_MAX_DEBUG_MESSAGE_LEN+1];   
        TCHAR* p1 = szOutMessage;
        char* p2 = szBuffer;            
        
        switch(ui32DebugLevel)
        {
            case ZONEID_FATAL:
            {
                strcpy (szBuffer, "[FIMG:FATAL] ");
                break;
            }
            case ZONEID_ERROR:
            {
                strcpy (szBuffer, "[FIMG:ERR] ");
                break;
            }
            case ZONEID_WARNING:
            {
                strcpy (szBuffer, "[FIMG:WARN] ");
                break;
            }
            case ZONEID_MESSAGE:
            {
                strcpy (szBuffer, "[FIMG:MSG] ");
                break;
            }
            case ZONEID_VERBOSE:
            {
                strcpy (szBuffer, "[FIMG:VERBOSE] ");
                break;
            }
            case ZONEID_CALLTRACE:
            {
                strcpy (szBuffer, "[FIMG:TRACE] ");
                break;
            }
            case ZONEID_ALLOC:
            {
                strcpy (szBuffer, "[FIMG:ALLOC] ");
                break;
            }
            case ZONEID_FLIP:
            {
                strcpy (szBuffer, "[FIMG:FLIP] ");
                break;
            }           
            default:
            {
                strcpy (szBuffer, "[FIMG:Unknown] ");
                break;
            }
        }

        va_start(ArgList, pszFormat);
        vsprintf (&szBuffer[strlen(szBuffer)], pszFormat, ArgList);
        va_end(ArgList);
        
        bTrace = ui32DebugLevel & (ZONEMASK_CALLTRACE|ZONEMASK_VERBOSE|ZONEMASK_FLIP);
        
        if (!bTrace)
        {
            const char* pszFilename = pszFilePath;
            const char* pszCurrentChar = pszFilePath;

            // Strip the path from the filename
            while (*pszCurrentChar != '\0')
            {
                if (*pszCurrentChar == '\\' || *pszCurrentChar == '/')
                {
                    pszFilename = pszCurrentChar + 1;
                }
                ++pszCurrentChar;
            }

            sprintf (&szBuffer[strlen(szBuffer)], " [%d, %s]", ui32Line, pszFilename);
        }
        
        sprintf(&szBuffer[strlen(szBuffer)], "\n");
        
        
        nLen = strlen(szBuffer);
        for (i = 0; i < nLen && *p2; i++)
        {
            *p1++ = (TCHAR) *p2++;
            *p1 = (TCHAR) '\0';
        }

        OutputDebugString(szOutMessage);

    }
}