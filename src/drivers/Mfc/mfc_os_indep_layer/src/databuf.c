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
#include "MfcTypes.h"
#include "MfcMemory.h"
#include "LogMsg.h"
#include "DataBuf.h"
#include "MfcConfig.h"

static volatile unsigned char     *vir_pDATA_BUF      = NULL;

static unsigned int                phyDATA_BUF     = 0;


BOOL MfcDataBufMemMapping()
{
    BOOL    ret = FALSE;

    // STREAM BUFFER, FRAME BUFFER  <-- virtual data buffer address mapping
    vir_pDATA_BUF = (volatile unsigned char *)Phy2Vir_AddrMapping(S3C6410_BASEADDR_MFC_DATA_BUF, MFC_DATA_BUF_SIZE, TRUE);
    if (vir_pDATA_BUF == NULL)
    {
        RETAILMSG(ZONE_ERROR, (TEXT("MFC::MfcDataBufMapping :: For DATA_BUF: VirtualAlloc failed!\r\n")));
        return ret;
    }

    // Physical register address mapping
    phyDATA_BUF    = S3C6410_BASEADDR_MFC_DATA_BUF;


    ret = TRUE;

    return ret;
}

volatile unsigned char *GetDataBufVirAddr()
{
    volatile unsigned char    *pDataBuf;

    pDataBuf    = vir_pDATA_BUF;

    return pDataBuf;    
}

volatile unsigned char *GetFramBufVirAddr()
{
    volatile unsigned char    *pFramBuf;

    pFramBuf    = vir_pDATA_BUF + MFC_STRM_BUF_SIZE;

    return pFramBuf;    
}

unsigned int GetDataBufPhyAddr()
{
    unsigned int    phyDataBuf;

    phyDataBuf    = phyDATA_BUF;

    return phyDataBuf;
}

unsigned int GetFramBufPhyAddr()
{
    unsigned int    phyFramBuf;

    phyFramBuf    = phyDATA_BUF + MFC_STRM_BUF_SIZE;

    return phyFramBuf;
}
