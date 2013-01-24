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

#include "fgl.h"
#include "register.h"

#ifdef USE_INTERRUPT
#include <bsp.h>
extern DWORD                g_IntrFIMG;
extern HANDLE               g_hPipeEvent;
#endif

void _gl_asm_nop(void);

void WaitForPipelineStatus(unsigned int pipelineFlags)
{
    unsigned int pipeline_state;    
    WRITEREG(FGGB_PIPEMASK, pipelineFlags);
    do
    {
        WRITEREG(FGGB_INTMASK, 1);  
        WaitForSingleObject(g_hPipeEvent,INFINITE);
        WRITEREG(FGGB_INTMASK, 0);      
        WRITEREG(FGGB_INTPENDING, 1);
        InterruptDone(g_IntrFIMG);      
        READREGP(FGGB_PIPEINTSTATE, pipeline_state);        
    }
    while(pipeline_state & pipelineFlags);   
}

/***************************************************************************** 
 * FUNCTIONS: fglSoftReset
 * SYNOPSIS: This function resets FIMG-3DSE, but the SFR values are not affected
 * PARAMETERS:
 * RETURNS: FGL_ERR_NO_ERROR - always.
 * ERRNO:
 *****************************************************************************/

FGL_Error
fglSoftReset(void)
{
    unsigned int i;
    
    WRITEREG(FGGB_RST, FGL_TRUE);
    
    /* delay */
    for(i = 0; i < 50; i++)
    {
#ifndef __GNUC__
    #ifndef WIN32
        __asm ("nop");
    #else
        _gl_asm_nop();
    #endif
#else
        __asm__ __volatile__ ("nop");
#endif
    }

    WRITEREG(FGGB_RST, FGL_FALSE);

    return FGL_ERR_NO_ERROR;
}

/***************************************************************************** 
 * FUNCTIONS: fglGetVersion
 * SYNOPSIS: This function gets FIMG-3DSE version.
 * PARAMETERS: [OUT] ver : FIMG3DSE version code
 * RETURNS: FGL_ERR_NO_ERROR - always.
 * ERRNO:
 *****************************************************************************/

FGL_Error
fglGetVersion(unsigned int* ver)
{

    READREGP(FGGB_VERSION, *ver);

    return FGL_ERR_NO_ERROR;
}