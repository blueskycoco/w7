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
#include <windows.h>
#include <pehdr.h>
#include <romldr.h>
#include <bsp_cfg.h>
#include "image_cfg.h"
#include "utils.h"
#include <IROM_SDMMC_loader_cfg.h>

#ifdef USE_CHANNEL0
#define CHANNEL_NO                  (0)
#else
#define CHANNEL_NO                  (1)
#endif
#define NAND_SB_PAGE_SIZE_BYTES     (0x200)        // 1 Page = 0x200 (512 Bytes)
#define NAND_SB_BLOCK_SIZE_BYTES    (0x4000)        // 1 Block = 16 KB
#define NAND_SB_PAGES_PER_BLOCK     (NAND_SB_BLOCK_SIZE_BYTES / NAND_SB_PAGE_SIZE_BYTES)    // 32-pages

#define NAND_LB_PAGE_SIZE_BYTES     (0x800)        // 1 Page = 0x800 (2048 Bytes)
#define NAND_LB_BLOCK_SIZE_BYTES    (0x20000)    // 1 Block = 128 KB
#define NAND_LB_PAGES_PER_BLOCK     (NAND_LB_BLOCK_SIZE_BYTES / NAND_LB_PAGE_SIZE_BYTES)    // 64-pages

#define NAND_BYTES_PER_PAGE         ((g_bLargeBlock==TRUE)?NAND_LB_PAGE_SIZE_BYTES:NAND_SB_PAGE_SIZE_BYTES)
#define NAND_PAGES_PER_BLOCK        ((g_bLargeBlock==TRUE)?NAND_LB_PAGES_PER_BLOCK:NAND_SB_PAGES_PER_BLOCK)

    // NOTE: we assume that this Steppingstone loader occupies *part* the first (good) NAND flash block.  More
    // specifically, the loader takes up 4096 bytes (or 8 NAND pages) of the first block.  We'll start our image
    // copy on the very next page.

#define LOAD_ADDRESS_PHYSICAL       (IMAGE_EBOOT_PA_START)
#define LOAD_IMAGE_BYTE_COUNT       (IMAGE_EBOOT_SIZE)
#define LOAD_IMAGE_PAGE_COUNT       (LOAD_IMAGE_BYTE_COUNT / NAND_BYTES_PER_PAGE)
#define LOAD_IMAGE_PAGE_OFFSET      (NAND_PAGES_PER_BLOCK * 2)    // StpeLoader uses 2 Block

// The function and variable that is provided by IROM, Refer to S3C6410 IROM Application Note.
#define SDMMC_TOTAL_SECTOR      *((volatile unsigned int*)(0x0C004000-0x4)) // globalBlockSizeHide: Total block count of the SDMMC device in D-TCM0
#define START_SECTOR            (SDMMC_TOTAL_SECTOR-SECTOROFSDBOOT-SECTOROFTOC-SECTOROFSTEPLDR-2)   // 2: Signature(1) + Reserved(1)
#define DEVICE_COPY_FUNCTION    (0x0C004008)     // CopyMMCtoMem function pointer in D-TCM1
typedef BOOL (*CopyMMCtoMem)   (int, unsigned int, unsigned short, unsigned int*, BOOL);


// Globals variables.
ROMHDR * volatile const pTOC = (ROMHDR *)-1;

typedef void (*PFN_IMAGE_LAUNCH)();

static BOOLEAN SetupCopySection(ROMHDR *const pTOC)
{
    // This code doesn't make use of global variables so there are no copy sections.
    // To reduce code size, this is a stub function...
    return(TRUE);
}

void main(void)
{
    volatile unsigned int *pBuf;

    // Set up copy section (initialized globals).
    //
    // NOTE: after this call, globals become valid.
    //
    SetupCopySection(pTOC);

    // Enable the ICache.
    //System_EnableICache();        // I-Cache was already enabled in startup.s

    // Set up all GPIO ports for LED.
    Port_Init();
    Led_Display(0x6);

    // Copy image from NAND flash to RAM.
    pBuf = (volatile unsigned int *)LOAD_ADDRESS_PHYSICAL;

    // Calling the function copy SD/MMC(MoviNAND/iNand) Card Data to memory.
    //
    // This Function copies movinand(MMC) Card Data to memory.
    // Always use EPLL source clock.
    // This function works at EPLL.
    // @param channel : HSMMC Controller channel number.
    // @param StartBlkAddress : Source card(movinand(MMC)) Address.(It must block address.)
    // @param blockSize : Number of blocks to copy.
    // @param memoryPtr : Buffer to copy from.
    // @param with_init : reinitialize or not
    // @return bool(unsigend char) - Success or failure.
    //
    ((CopyMMCtoMem)(*((UINT32 *)(DEVICE_COPY_FUNCTION))))(CHANNEL_NO,    // Channel
                                                          START_SECTOR, // StartBlkAddress
                                                          SECTOROFSDBOOT,   // BlockSize
                                                          (unsigned int *)pBuf,    // MemoryPtr
                                                          FALSE);   // with init

    ((PFN_IMAGE_LAUNCH)(LOAD_ADDRESS_PHYSICAL))();
}

