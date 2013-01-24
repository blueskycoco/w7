!if 0
Copyright (c) Microsoft Corporation.  All rights reserved.
!endif
!if 0
Use of this sample source code is subject to the terms of the Microsoft
license agreement under which you licensed this sample source code. If
you did not accept the terms of the license agreement, you are not
authorized to use this sample source code. For the terms of the license,
please see the license agreement between you and Microsoft or, if applicable,
see the LICENSE.RTF on your install media or the root of your tools installation.
THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
!endif
!IF 0


Module Name:

    makefile.inc

!ENDIF

#
# Take the eboot.exe that resulted from the standard build and generate eboot.bin from it.
#
!IF ("$(BSP_IROM_SDMMC_CH0_BOOT)"!="1")&&("$(BSP_IROM_SDMMC_CH1_BOOT)"!="1")
EbootImage:
        romimage eboot.bib

!IF "$(WINCEREL)"=="1"
        copy $(SG_OUTPUT_ROOT)\platform\$(_TGTPLAT)\target\$(_TGTCPU)\$(WINCEDEBUG)\eboot.nb0 $(_FLATRELEASEDIR)
        copy $(SG_OUTPUT_ROOT)\platform\$(_TGTPLAT)\target\$(_TGTCPU)\$(WINCEDEBUG)\eboot.bin $(_FLATRELEASEDIR)
        copy $(SG_OUTPUT_ROOT)\platform\$(_TGTPLAT)\target\$(_TGTCPU)\$(WINCEDEBUG)\eboot.sre $(_FLATRELEASEDIR)
!ENDIF
!ENDIF