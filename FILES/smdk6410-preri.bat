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
REM This makeimg runs this file before calling romimage.
REM
REM The Multibin_Fixup script modifies the region specifier for each
REM entry in the BIB file as specified in the filelist. This allows us
REM to specify which files in the BIB file go into which region
REM

if "%IMGMULTIXIP%"=="1" copy ce.bib ce_prefixup.bib
if "%IMGMULTIXIP%"=="1" cscript multibin_fixup.js multibin_region_filelist.txt
if "%IMGMULTIXIP%"=="1" del ce.bib
if "%IMGMULTIXIP%"=="1" move multiregion.bib ce.bib
