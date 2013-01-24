;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this sample source code is subject to the terms of the Microsoft
; license agreement under which you licensed this sample source code. If
; you did not accept the terms of the license agreement, you are not
; authorized to use this sample source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the LICENSE.RTF on your install media or the root of your tools installation.
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
;
; Module Name: usb.s

        OPT 2   ; disable listing

        INCLUDE     kxarm.h

        OPT 1   ; reenable listing
        OPT 128 ; disable listing of macro expansions

        TEXTAREA

        IMPORT  C_IsrHandler


    LEAF_ENTRY ASM_IsrHandler

;       sub sp,sp,#4        ; decrement sp(to store jump address)

        sub lr, lr, #4
        stmfd   sp!, {r0-r12,lr}

        mov r0, lr

        bl  C_IsrHandler

        ldmfd   sp!, {r0-r12,lr}

        movs pc, lr

        ENTRY_END    ; |IsrHandler|

        END

