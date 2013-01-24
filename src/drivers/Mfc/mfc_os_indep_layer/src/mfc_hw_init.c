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
#include "MfcSfr.h"
#include "BitProcBuf.h"
#include "DataBuf.h"
#include "MfcTypes.h"
#include "LogMsg.h"
#include "MfcConfig.h"
#include "FramBufMgr.h"


BOOL MFC_MemorySetup(void)
{
    //MfcMemMapping();
    BOOL ret_sfr, ret_bit, ret_dat;
    unsigned char *pDataBuf;


    // MFC SFR(Special Function Registers), Bitprocessor buffer, Data buffer의 
    // physical address 를 virtual address로 mapping 한다 
    ret_sfr    = MfcSfrMemMapping();
    if (ret_sfr == FALSE) {
        RETAILMSG(ZONE_ERROR, (TEXT("MFC::MfcMemorySetup :: MFC SFR Memory mapping fail!.\r\n")));
        return FALSE;
    }

    ret_bit = MfcBitProcBufMemMapping();
    if (ret_bit == FALSE) {
        RETAILMSG(ZONE_ERROR, (TEXT("MFC::MfcMemorySetup :: Bitprocessor buffer Memory mapping fail!.\r\n")));
        return FALSE;
    }

    ret_dat    = MfcDataBufMemMapping();
    if (ret_dat == FALSE) {
        RETAILMSG(ZONE_ERROR, (TEXT("MFC::MfcMemorySetup :: Data buffer Memory mapping fail!.\r\n")));
        return FALSE;
    }


    // FramBufMgr Module Initialization
    pDataBuf = (unsigned char *)GetDataBufVirAddr();
    FramBufMgrInit(pDataBuf + MFC_STRM_BUF_SIZE, MFC_FRAM_BUF_SIZE);


    return TRUE;
}


BOOL MFC_HW_Init(void)
{
    /////////////////////////
    //                     //
    // 1. Reset the MFC IP //
    //                     //
    /////////////////////////
    MfcReset();


    ////////////////////////////////////////
    //                                    //
    // 2. Download Firmware code into MFC //
    //                                    //
    ////////////////////////////////////////
    MfcFirmwareIntoCodeBuf();
    MfcFirmwareIntoCodeDownReg();
    RETAILMSG(ZONE_FUNCTION, (TEXT("MFC::MFC_HW_Init ::Download  FirmwareIntoBitProcessor  OK.\r\n")));


    ////////////////////////////
    //                        //
    // 3. Start Bit Processor //
    //                        //
    ////////////////////////////
    MfcStartBitProcessor();


    ////////////////////////////////////////////////////////////////////
    //                                                                //
    // 4. Set the Base Address Registers for the following 3 buffers  //
    //        (CODE_BUF, WORKING_BUF, PARAMETER_BUF)                  //
    //                                                                //
    ////////////////////////////////////////////////////////////////////
    MfcConfigSFR_BITPROC_BUF();


    //////////////////////////////////
    //                              //
    // 5. Set the Control Registers //
    //       - STRM_BUF_CTRL        //
    //       - FRME_BUF_CTRL        //
    //       - DEC_FUNC_CTRL        //
    //       - WORK_BUF_CTRL        //
    //                              //
    //////////////////////////////////
    MfcConfigSFR_CTRL_OPTS();



    GetFirmwareVersion();


    return TRUE;
}

