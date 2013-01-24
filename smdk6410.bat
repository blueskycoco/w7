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
@REM
@REM

@REM To support for SMDK6410X5D
@REM X5D MCP has 2Gb NAND + 512Mb M-DDR + 512Mb OneDRAM
@REM set SMDK6410_X5D=

set PLATFORMCHIP=PLATFORM_S3C6410

@REM To support iROM NANDFlash boot
set BSP_IROMBOOT=1

@REM To support iROM SDMMC boot
set BSP_IROM_SDMMC_CH0_BOOT=
set BSP_IROM_SDMMC_CH1_BOOT=

set WINCEREL=1

@REM Skip touch-screen calibration (use default values)
@REM See: [HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\TOUCH\CalibrationData]
set IMGNOCALIBRATION=1

set BSP_NODISPLAY=
set BSP_NOTOUCH=
set BSP_NOPWRBTN=

set BSP_NONANDFS=

set BSP_NOHSMMC_CH0=1
if /i "%BSP_IROM_SDMMC_CH0_BOOT%"=="1" set BSP_NOHSMMC_CH0=1
set BSP_NOHSMMC_CH1=
if /i "%BSP_IROM_SDMMC_CH1_BOOT%"=="1" set BSP_NOHSMMC_CH1=1
set BSP_HSMMC_CH1_8BIT=

@REM CF_ATAPI and KEYPAD cannot be enabled together due to hardware clash
@REM
set BSP_NOKEYPAD=1
set BSP_NOCFATAPI=

set BSP_NOAUDIO=
set BSP_AUDIO_AC97=1

set BSP_NOD3DM=1
set BSP_NOCAMERA=

set BSP_NOMFC=
set BSP_NOJPEG=
set BSP_NOOES=1
if /i "%SMDK6410_X5D%"=="1" set BSP_NOOES=1
set BSP_NOCMM=

set BSP_NOSERIAL=
set BSP_NOUART0=1
set BSP_NOUART1=
set BSP_NOUART2=1
set BSP_NOUART3=1
set BSP_NOIRDA2=1
set BSP_NOIRDA3=1
set BSP_BLUETOOTH_BUILTIN_UART=1

set BSP_NOI2C=
set BSP_NOSPI=

set BSP_NOUSBHCD=

@REM If you want to exclude USB Function driver in BSP. Set this variable
set BSP_NOUSBFN=
@REM This select default function driver
set BSP_USBFNCLASS=SERIAL
@REM set BSP_USBFNCLASS=MASS_STORAGE

@REM DVFS is not yet implemented.
set BSP_USEDVS=

set BSP_DEBUGPORT=SERIAL_UART0
@REM set BSP_DEBUGPORT=SERIAL_UART1
@REM set BSP_DEBUGPORT=SERIAL_UART2
@REM set BSP_DEBUGPORT=SERIAL_UART3


@REM set BSP_KITL=NONE
@REM set BSP_KITL=SERIAL_UART0
@REM set BSP_KITL=SERIAL_UART1
@REM set BSP_KITL=SERIAL_UART2
@REM set BSP_KITL=SERIAL_UART3
@REM set BSP_KITL=USBSERIAL

@REM For Hive Based Registry

set IMGHIVEREG=

@REM if /i "%BSP_IROM_SDMMC_CH1_BOOT%"=="1" set IMGHIVEREG=

if /i "%IMGHIVEREG%"=="1" set PRJ_ENABLE_FSREGHIVE=1
if /i "%IMGHIVEREG%"=="1" set PRJ_ENABLE_REGFLUSH_THREAD=1

@REM Multipl-XIP using demand paging, BINFS must be turned on
set IMGMULTIXIP=

@REM Does not support
set IMGMULTIBIN=

@REM for using GDI Performance Utility, DispPerf
@REM there are some compatibility issue between CE6.0 R2 and previous version
@REM if you want to use DISPPERF in CE6.0 R2,
@REM build public components(GPE) also.
set DO_DISPPERF=


