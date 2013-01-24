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
;
; Optimize USB Transfer Performance
; chandolp 
;
; OTG Device Transfer Related Routines
;

    INCLUDE kxarm.h

    TEXTAREA


;----------------------------------------------
; void RxData512(unsigned char *bufPt, volatile ULONG *FifoPt);
; Buffer (r0) must be aligned.
;----------------------------------------------    
    LEAF_ENTRY GetData512

        stmfd        sp!,{r2 - r12}

        mov        r2, #0x1E0
1
        ldr        r3, [r1]
        ldr        r4, [r1]
        ldr        r5, [r1]
        ldr        r6, [r1]
        ldr        r7, [r1]
        ldr        r8, [r1]
        ldr        r9, [r1]
        ldr        r10, [r1]
        ldr        r11, [r1]
        ldr        r12, [r1]

        stmia        r0!, {r3 - r12}
        subs        r2, r2, #40
        bne        %B1

        ldr        r3, [r1]
        ldr        r4, [r1]
        ldr        r5, [r1]
        ldr        r6, [r1]
        ldr        r7, [r1]
        ldr        r8, [r1]
        ldr        r9, [r1]
        ldr        r10, [r1]
        
        stmia        r0!, {r3 - r10}

        ldmfd        sp!, {r2 - r12}

        mov        pc, lr    ; return


    
;----------------------------------------------------------
; void TxData512(unsigned char *bufPt, volatile ULONG *FifoPt);
; Buffer (r0) must be aligned.
;----------------------------------------------------------
    LEAF_ENTRY PutData512

        stmfd        sp!,{r2 - r12}

        mov        r2, #0x1E0
1
        ldmia        r0!, {r3 - r12}

        str        r3, [r1]
        str        r4, [r1]
        str        r5, [r1]
        str        r6, [r1]
        str        r7, [r1]
        str        r8, [r1]
        str        r9, [r1]
        str        r10, [r1]
        str        r11, [r1]
        str        r12, [r1]

        subs        r2, r2, #40
        bne        %B1

        ldmia        r0!, {r3 - r10}
        
        str        r3, [r1]
        str        r4, [r1]
        str        r5, [r1]
        str        r6, [r1]
        str        r7, [r1]
        str        r8, [r1]
        str        r9, [r1]
        str        r10, [r1]
        
        ldmfd        sp!, {r2 - r12}

        mov        pc, lr    ; return

    END

