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
; Copyright (c) Samsung Electronics. Co. LTD.  All rights reserved.
;
; THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
; ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
; THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
; PARTICULAR PURPOSE.

; Cache Operation
;

    INCLUDE kxarm.h

    TEXTAREA

    EXPORT    CleanInvalidateCacheRange
    EXPORT    CleanCacheRange
    EXPORT    InvalidateCacheRange

;--------------------------------------------------------------------
; void CleanInvalidateCacheRange(PBYTE StartAddress, PBYTE EndAddress);
;--------------------------------------------------------------------
    LEAF_ENTRY CleanInvalidateCacheRange

        stmfd        sp!,{r2 - r12}

        mcrr         p15,0,r1,r0,c14

        ldmfd        sp!, {r2 - r12}

    IF Interworking :LOR: Thumbing
        bx        lr
    ELSE
        mov        pc, lr    ; return
    ENDIF
    
    ENTRY_END
;--------------------------------------------------------------------
; void CleanCacheRange(PBYTE StartAddress, PBYTE EndAddress);
;--------------------------------------------------------------------
    LEAF_ENTRY CleanCacheRange

        stmfd        sp!,{r2 - r12}

        mcrr         p15,0,r1,r0,c12

        ldmfd        sp!, {r2 - r12}

    IF Interworking :LOR: Thumbing
        bx        lr
    ELSE
        mov        pc, lr    ; return
    ENDIF
    
    ENTRY_END
;--------------------------------------------------------------------
; void InvalidateCacheRange(PBYTE StartAddress, PBYTE EndAddress);
;--------------------------------------------------------------------
    LEAF_ENTRY InvalidateCacheRange

        stmfd        sp!,{r2 - r12}

        mcrr         p15,0,r1,r0,c6

        ldmfd        sp!, {r2 - r12}

    IF Interworking :LOR: Thumbing
        bx        lr
    ELSE
        mov        pc, lr    ; return
    ENDIF
    
    ENTRY_END
    
    END

