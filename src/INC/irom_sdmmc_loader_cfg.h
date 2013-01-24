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

#ifndef _SDMMC_LOADER_CFG_H_
#define _SDMMC_LOADER_CFG_H_

#define SHIFT_FOR_SDHC         (1024)
#define STEPLDRBUFFER       (0x50200000)
#define STEPLDRSTARTADDRESS    (0x50000000)
#define SECTOROFSTEPLDR     (16)
//#define STEPLDRSTARTSECTOR  (g_dwSectorCount-SECTOROFSTEPLDR-2) // 2: Signature(1) + Reserved(1)
#define STEPLDRSTARTSECTOR  (g_dwSectorCount-SECTOROFSTEPLDR-2-((g_dwIsSDHC == TRUE) ? SHIFT_FOR_SDHC:0)) // 2: Signature(1) + Reserved(1)

#define SECTOROFTOC         (1)
#define TOCSTARTSECTOR      (STEPLDRSTARTSECTOR-SECTOROFTOC)

#define SDBOOTBUFFER        (0x50200000)
#define SDBOOTSTARTADDRESS    (0x50030000)
#define SDBOOTSIZE          (IMAGE_EBOOT_SIZE)
#define SECTOROFSDBOOT      (SDBOOTSIZE/512)
#define SDBOOTSTARTSECTOR   (TOCSTARTSECTOR-SECTOROFSDBOOT)

#define IMAGEBUFFER         (0x52000000)
#define IMAGESTARTADDRESS   (0x50100000)
#define IMAGESIZE           (IMAGE_NK_SIZE)
#define SECTOROFIMAGE       (IMAGESIZE/512)
#define IMAGESTARTSECTOR    (SDBOOTSTARTSECTOR-SECTOROFIMAGE)

#define SECTOROFMBR         (0x100)
#define MBRSTARTSECTOR      (IMAGESTARTSECTOR-SECTOROFMBR) // to align 4kb

#endif  // _IROM_SDMMC_LOADER_CFG_H_

