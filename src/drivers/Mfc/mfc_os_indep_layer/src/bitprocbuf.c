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


#include "Mfc.h"
#include "MfcMemory.h"
#include "LogMsg.h"
#include "BitProcBuf.h"
#include "MfcConfig.h"
#include "Prism_S.h"



static volatile unsigned char     *vir_pBITPROC_BUF   = NULL;

static unsigned int                phyBITPROC_BUF  = 0;


BOOL MfcBitProcBufMemMapping()
{
    BOOL    ret = FALSE;

    // FIRWARE/WORKING/PARAMETER BUFFER  <-- virtual bitprocessor buffer address mapping
    vir_pBITPROC_BUF = (volatile unsigned char *)Phy2Vir_AddrMapping(S3C6410_BASEADDR_MFC_BITPROC_BUF, MFC_BITPROC_BUF_SIZE, FALSE);
    if (vir_pBITPROC_BUF == NULL)
    {
        RETAILMSG(ZONE_ERROR, (TEXT("MFC::MfcBitProcBufMapping :: For BITPROC_BUF: VirtualAlloc failed!\r\n")));
        return ret;
    }

    // Physical register address mapping
    phyBITPROC_BUF    = S3C6410_BASEADDR_MFC_BITPROC_BUF;


    ret = TRUE;

    return ret;
}

volatile unsigned char *GetBitProcBufVirAddr()
{
    volatile unsigned char    *pBitProcBuf;

    pBitProcBuf    = vir_pBITPROC_BUF;

    return pBitProcBuf;
}

unsigned char *GetParamBufVirAddr()
{
    unsigned char    *pParamBuf;

    pParamBuf = (unsigned char *)(vir_pBITPROC_BUF + MFC_CODE_BUF_SIZE + MFC_WORK_BUF_SIZE);

    return pParamBuf;
}

void MfcFirmwareIntoCodeBuf()
{
    unsigned int  i, j;
    unsigned int  data;

    unsigned int *uAddrFirmwareCode;

    uAddrFirmwareCode = (unsigned int *)vir_pBITPROC_BUF;

    /////////////////////////////////////////////////
    // Putting the Boot & Firmware code into SDRAM //
    /////////////////////////////////////////////////
    // Boot code(1KB) + Codec Firmware (79KB)
    for (i=j=0 ; i<sizeof(bit_code)/sizeof(bit_code[0]); i+=2, j++)
    {
        data = (bit_code[i] << 16) | bit_code[i+1];
    
        *(uAddrFirmwareCode + j) = data;
    }
}
