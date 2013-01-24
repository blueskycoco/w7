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
#ifndef _S3C6410_HSMMC_H_
#define _S3C6410_HSMMC_H_

#include "../s3c6410_hsmmc_lib/SDHC.h"

typedef class CSDHControllerCh1 : public CSDHCBase {
    public:
        // Constructor
        CSDHControllerCh1() : CSDHCBase() {}

        // Destructor
        virtual ~CSDHControllerCh1() {}

        // Perform basic initialization including initializing the hardware
        // so that the capabilities register can be read.
        virtual BOOL Init(LPCTSTR pszActiveKey);

        virtual VOID PowerUp();

        virtual LPSDHC_DESTRUCTION_PROC GetDestructionProc() {
            return &DestroyHSMMCHCCh1Object;
        }

        static VOID DestroyHSMMCHCCh1Object(PCSDHCBase pSDHC);

    protected:
        BOOL InitClkPwr();
        BOOL InitGPIO();
        BOOL InitHSMMC();

        BOOL InitCh();
} *PCSDHControllerCh1;

#endif // _S3C6410_HSMMC_H_

