@REM
@REM (C) Copyright 2006-2008 Marvell International Ltd.
@REM All Rights Reserved
@REM
@REM
@REM Copyright (c) Microsoft Corporation.  All rights reserved.
@REM
@REM
@REM Use of this sample source code is subject to the terms of the Microsoft
@REM license agreement under which you licensed this sample source code. If
@REM you did not accept the terms of the license agreement, you are not
@REM authorized to use this sample source code. For the terms of the license,
@REM please see the license agreement between you and Microsoft or, if applicable,
@REM see the LICENSE.RTF on your install media or the root of your tools installation.
@REM THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
@REM
REM 
REM Copyright 2000-2003 Intel Corporation All Rights Reserved.
REM
REM  filename: clean.bat
REM   - Removes output file directories, which will be recreated during build
REM   - Also removes some individual file types
REM   - Must be run from platform\bvdmain directory


del /s /q build.*
del /s /q *.bif
del /s /q vssver.scc
del /s /q sources.gen
del /s /q *.*~
del /s /q *~

rd /s /q target
rd /s /q lib
rd /s /q cesysgen\files
del /s /q files\uldr.nb0

for  /R %%I  in (.) do (
        if /I obj==%%~nI  rd /s /q   %%~fI

)