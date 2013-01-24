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
* \file        glProf.h
* \brief    Interface for the glprof profiler
*
*//*---------------------------------------------------------------------------
* NOTES:
*
*/
#ifndef __GLPROF_H__
#define __GLPROF_H__

enum { GLPROF_GL = 1, GLPROF_GPU = 2};
enum GLProfStart {GLPROF_NOW, GLPROF_ON_BUFFER_SWAP};

struct glprof_glstats {
    float fps;              //frames per second
    float primsPerSec;      //primitives per second
    float drawCallsPerSec;  //draw calls per second - not really interesting 
                            //  for most demos
    float timeinterval;     //time interval over which these stats have been
                            // gathered in secs. User can use this to get 
                            // total number of frames / prims etc.
                            // eg. total frames = fps*timeinterval;
};

bool glprofInit(unsigned int profFlags);
bool glprofDeinit();
bool glprofReset();
void glprofStart(GLProfStart start = GLPROF_ON_BUFFER_SWAP);
void glprofStop();

//glprofStop must be called before the following functions
void glprofPrintReport();
glprof_glstats glprofGetGlStats();

//internally used functions
void glprofBufferSwap();
void glprofPrimitives(unsigned int glprimType, unsigned int n);

#endif //__GLPROF_H__
