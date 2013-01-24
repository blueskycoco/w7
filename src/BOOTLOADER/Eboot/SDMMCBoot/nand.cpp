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

extern "C"
{
#include <windows.h>
#include <bsp.h>
#include "loader.h"
#include <fmd.h>
#include <HSMMCDrv.h>
#include <IROM_SDMMC_loader_cfg.h>

extern volatile UINT32 g_dwIsSDHC;

extern DWORD        g_ImageType;
extern UCHAR        g_TOC[LB_PAGE_SIZE];
extern const PTOC   g_pTOC;
extern DWORD        g_dwTocEntry;
extern PBOOT_CFG    g_pBootCfg;
extern BOOL         g_bBootMediaExist;
extern MultiBINInfo g_BINRegionInfo;
extern BOOL         g_bWaitForConnect;      // Is there a SmartMedia card on this device?
extern char*        BootDeviceString[];
extern DWORD        g_DefaultBootDevice;

}

extern DWORD        g_dwLastWrittenLoc;                     //  Defined in bootpart.lib
extern PSectorInfo  g_pSectorInfoBuf;
extern FlashInfo    g_FlashInfo;
static UCHAR        toc[LB_PAGE_SIZE];

// Define a dummy SetKMode function to satisfy the NAND FMD.
//
DWORD SetKMode (DWORD fMode)
{
    return(1);
}


void BootConfigPrint(void)
{
    EdbgOutputDebugString( "BootCfg { \r\n");
    EdbgOutputDebugString( "  ConfigFlags: 0x%x\r\n", g_pBootCfg->ConfigFlags);
    EdbgOutputDebugString( "  BootDelay: 0x%x\r\n", g_pBootCfg->BootDelay);
    EdbgOutputDebugString( "  ImageIndex: %d \r\n", g_pBootCfg->ImageIndex);
    EdbgOutputDebugString ("  Boot Device: %s\r\n", BootDeviceString[g_pBootCfg->BootDevice]);
    EdbgOutputDebugString( "  IP: %s\r\n", inet_ntoa(g_pBootCfg->EdbgAddr.dwIP));
    EdbgOutputDebugString( "  MAC Address: %B:%B:%B:%B:%B:%B\r\n",
                           g_pBootCfg->EdbgAddr.wMAC[0] & 0x00FF, g_pBootCfg->EdbgAddr.wMAC[0] >> 8,
                           g_pBootCfg->EdbgAddr.wMAC[1] & 0x00FF, g_pBootCfg->EdbgAddr.wMAC[1] >> 8,
                           g_pBootCfg->EdbgAddr.wMAC[2] & 0x00FF, g_pBootCfg->EdbgAddr.wMAC[2] >> 8);
    EdbgOutputDebugString( "  Port: %s\r\n", inet_ntoa(g_pBootCfg->EdbgAddr.wPort));

    EdbgOutputDebugString( "  SubnetMask: %s\r\n", inet_ntoa(g_pBootCfg->SubnetMask));
    EdbgOutputDebugString( "}\r\n");
}


// Set default boot configuration values
static void BootConfigInit(DWORD dwIndex)
{

    EdbgOutputDebugString("+BootConfigInit\r\n");

    g_pBootCfg = &g_pTOC->BootCfg;

    memset(g_pBootCfg, 0, sizeof(BOOT_CFG));

    g_pBootCfg->ImageIndex   = dwIndex;

    g_pBootCfg->BootDevice = g_DefaultBootDevice;

    g_pBootCfg->ConfigFlags  = BOOT_TYPE_MULTISTAGE|BOOT_OPTION_CLEAN;

    g_pBootCfg->BootDelay    = CONFIG_BOOTDELAY_DEFAULT;

    // Setup some defaults so that KITL will work without having to change anything
    g_pBootCfg->ConfigFlags |= CONFIG_FLAGS_KITL | CONFIG_FLAGS_DHCP;
    g_pBootCfg->EdbgAddr.dwIP = inet_addr("169.254.1.101");
    g_pBootCfg->SubnetMask = inet_addr("255.255.255.0");
    g_pBootCfg->EdbgAddr.wMAC[0] = 0x1100;
    g_pBootCfg->EdbgAddr.wMAC[1] = 0x3322;
    g_pBootCfg->EdbgAddr.wMAC[2] = 0x5544;

    EdbgOutputDebugString("-BootConfigInit\r\n");
    return;
}

void ID_Print(DWORD i) {
    DWORD j;
    EdbgOutputDebugString("ID[%u] {\r\n", i);
    EdbgOutputDebugString("  dwVersion: 0x%x\r\n",  g_pTOC->id[i].dwVersion);
    EdbgOutputDebugString("  dwSignature: 0x%x\r\n", g_pTOC->id[i].dwSignature);
    EdbgOutputDebugString("  String: '%s'\r\n", g_pTOC->id[i].ucString);
    EdbgOutputDebugString("  dwImageType: 0x%x\r\n", g_pTOC->id[i].dwImageType);
    EdbgOutputDebugString("  dwTtlSectors: 0x%x\r\n", g_pTOC->id[i].dwTtlSectors);
    EdbgOutputDebugString("  dwLoadAddress: 0x%x\r\n", g_pTOC->id[i].dwLoadAddress);
    EdbgOutputDebugString("  dwJumpAddress: 0x%x\r\n", g_pTOC->id[i].dwJumpAddress);
    EdbgOutputDebugString("  dwStoreOffset: 0x%x\r\n", g_pTOC->id[i].dwStoreOffset);
    for (j = 0; j < MAX_SG_SECTORS; j++) {
        if ( !g_pTOC->id[i].sgList[j].dwLength )
            break;
        EdbgOutputDebugString("  sgList[%u].dwSector: 0x%x\r\n", j, g_pTOC->id[i].sgList[j].dwSector);
        EdbgOutputDebugString("  sgList[%u].dwLength: 0x%x\r\n", j, g_pTOC->id[i].sgList[j].dwLength);
    }

    EdbgOutputDebugString("}\r\n");
}

/*
    @func   void | DumpXIPChainSummary | Dump XIPCHAIN_SUMMARY structure
    @rdesc  none
    @comm
    @xref
*/
void DumpXIPChainSummary(PXIPCHAIN_SUMMARY pChainInfo)
{
    EdbgOutputDebugString("[Dump XIP Chain Summary]\r\n");
    EdbgOutputDebugString(" - pvAddr: 0x%x\r\n", pChainInfo->pvAddr);
    EdbgOutputDebugString(" - dwMaxLength : %d\r\n", pChainInfo->dwMaxLength);
    EdbgOutputDebugString(" - usOrder : 0x%x\r\n", pChainInfo->usOrder);
    EdbgOutputDebugString(" - usFlags : 0x%x\r\n", pChainInfo->usFlags);
    EdbgOutputDebugString(" - reserved : 0x%x\r\n", pChainInfo->reserved);
}

/*
    @func   BOOL | WriteOSImageToBootMedia | Stores the image cached in RAM to the Boot Media.
    The image may be comprised of one or more BIN regions.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm
    @xref
*/
BOOL WriteOSImageToBootMedia(DWORD dwImageStart, DWORD dwImageLength, DWORD dwLaunchAddr)
{
    BYTE nCount;
    DWORD dwNumExts;
    PXIPCHAIN_SUMMARY pChainInfo = NULL;
    EXTENSION *pExt = NULL;
    DWORD dwBINFSPartLength = 0;
    HANDLE hPart, hPartEx;
    DWORD dwStoreOffset;
    DWORD dwMaxRegionLength[BL_MAX_BIN_REGIONS] = {0};
    DWORD dwChainStart, dwChainLength;

    //  Initialize the variables
    dwChainStart = dwChainLength = 0;

    OALMSG(OAL_FUNC, (TEXT("+WriteOSImageToBootMedia\r\n")));
    OALMSG(OAL_INFO, (TEXT("+WriteOSImageToBootMedia: g_dwTocEntry =%d, ImageStart: 0x%x, ImageLength: 0x%x, LaunchAddr:0x%x\r\n"),
                            g_dwTocEntry, dwImageStart, dwImageLength, dwLaunchAddr));

    if ( !g_bBootMediaExist )
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: WriteOSImageToBootMedia: device doesn't exist.\r\n")));
        return(FALSE);
    }

    // MBR should be clean to fuse the image
    BP_LowLevelFormat(0,0,0);

    if ( !VALID_TOC(g_pTOC) )
    {
        OALMSG(OAL_WARN, (TEXT("WARN: WriteOSImageToBootMedia: INVALID_TOC\r\n")));
        if ( !TOC_Init(g_dwTocEntry, g_ImageType, dwImageStart, dwImageLength, dwLaunchAddr) )
        {
            OALMSG(OAL_ERROR, (TEXT("ERROR: INVALID_TOC\r\n")));
            return(FALSE);
        }
    }

    // Look in the kernel region's extension area for a multi-BIN extension descriptor.
    // This region, if found, details the number, start, and size of each BIN region.
    //
    for (nCount = 0, dwNumExts = 0 ; (nCount < g_BINRegionInfo.dwNumRegions); nCount++)
    {
        // Does this region contain nk.exe and an extension pointer?
        //
        pExt = (EXTENSION *)GetKernelExtPointer(g_BINRegionInfo.Region[nCount].dwRegionStart,
                                                g_BINRegionInfo.Region[nCount].dwRegionLength );
        EdbgOutputDebugString("INFO: %s: [%d] RegionStart:0x%x, RegionLength:0x%x, pExt:0x%x\r\n",
                                                __FUNCTION__, nCount,
                                                g_BINRegionInfo.Region[nCount].dwRegionStart,
                                                g_BINRegionInfo.Region[nCount].dwRegionLength,
                                                pExt);
        if ( pExt != NULL)
        {
            // If there is an extension pointer region, walk it until the end.
            //
            while (pExt)
            {
                DWORD dwBaseAddr = g_BINRegionInfo.Region[nCount].dwRegionStart;
                pExt = (EXTENSION *)OEMMapMemAddr(dwBaseAddr, (DWORD)pExt);
                EdbgOutputDebugString("INFO: %s: [%d] Found chain extension: '%s' @ 0x%x\r\n", __FUNCTION__, nCount, pExt->name, dwBaseAddr);
                if ((pExt->type == 0) && !strcmp(pExt->name, "chain information"))
                {
                    pChainInfo = (PXIPCHAIN_SUMMARY) OEMMapMemAddr(dwBaseAddr, (DWORD)pExt->pdata);
                    dwNumExts = (pExt->length / sizeof(XIPCHAIN_SUMMARY));
                    EdbgOutputDebugString("INFO: %s: [%d] Found 'chain information' (pChainInfo=0x%x  Extensions=0x%x).\r\n", __FUNCTION__, nCount, (DWORD)pChainInfo, dwNumExts);
                    DumpXIPChainSummary(pChainInfo);
                    dwChainStart = (DWORD)pChainInfo->pvAddr;
                    dwChainLength = pChainInfo->dwMaxLength;
                    goto FoundChainAddress;
                }
                pExt = (EXTENSION *)pExt->pNextExt;
            }
        }
        else {
            //  Search for Chain region. Chain region doesn't have the ROMSIGNATURE set
            EdbgOutputDebugString("Not Found pExtPointer, Check if this is Chain region.\n");

            DWORD   dwRegionStart = g_BINRegionInfo.Region[nCount].dwRegionStart;
            DWORD   dwSig = *(LPDWORD) OEMMapMemAddr(dwRegionStart, dwRegionStart + ROM_SIGNATURE_OFFSET);

            if ( dwSig != ROM_SIGNATURE) {
                //  It is the chain
                dwChainStart = dwRegionStart;
                dwChainLength = g_BINRegionInfo.Region[nCount].dwRegionLength;
                OALMSG(TRUE, (TEXT("Found the Chain region: StartAddress: 0x%X; Length: 0x%X\n"), dwChainStart, dwChainLength));
            }
        }
    }
FoundChainAddress:

    // Determine how big the Total BINFS partition needs to be to store all of this.
    //
    if (pChainInfo && dwNumExts == g_BINRegionInfo.dwNumRegions)    // We're downloading all the regions in a multi-region image...
    {
        DWORD i;
        OALMSG(TRUE, (TEXT("Writing multi-regions : Total %d regions\r\n"), dwNumExts));

        for (nCount = 0, dwBINFSPartLength = 0 ; nCount < dwNumExts ; nCount++)
        {
            dwBINFSPartLength += (pChainInfo + nCount)->dwMaxLength;
            OALMSG(OAL_ERROR, (TEXT("[%u] BINFSPartMaxLength: 0x%x, TtlBINFSPartLength: 0x%x \r\n"),
                nCount, (pChainInfo + nCount)->dwMaxLength, dwBINFSPartLength));

            // MultiBINInfo does not store each Regions MAX length, and pChainInfo is not in any particular order.
            // So, walk our MultiBINInfo matching up pChainInfo to find each regions MAX Length
            for (i = 0; i < dwNumExts; i++) {
                if ( g_BINRegionInfo.Region[i].dwRegionStart == (DWORD)((pChainInfo + nCount)->pvAddr) ) {
                    dwMaxRegionLength[i] = (pChainInfo + nCount)->dwMaxLength;
                    OALMSG(TRUE, (TEXT("dwMaxRegionLength[%u]: 0x%x \r\n"), i, dwMaxRegionLength[i]));
                    break;
                }
            }
        }

    }
    else    // A single BIN file or potentially a multi-region update (but the partition's already been created in this latter case).
    {
        dwBINFSPartLength = g_BINRegionInfo.Region[0].dwRegionLength;
        OALMSG(TRUE, (TEXT("Writing single region/multi-region update, dwBINFSPartLength: %u \r\n"), dwBINFSPartLength));
    }

    // Open/Create the BINFS partition where images are stored.  This partition starts immediately after the MBR on the Boot Media and its length is
    // determined by the maximum image size (or sum of all maximum sizes in a multi-region design).
    // Parameters are LOGICAL sectors.
    //
    hPart = BP_OpenPartition(IMAGESTARTSECTOR - MBRSTARTSECTOR,
                             ((dwBINFSPartLength)/SECTOR_SIZE)+1,
                             PART_BINFS,
                             TRUE,
                             PART_OPEN_ALWAYS);

    if (hPart == INVALID_HANDLE_VALUE )
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: WriteOSImageToBootMedia: Failed to open/create partition.\r\n")));
        return(FALSE);
    }

    // Are there multiple BIN files in RAM (we may just be updating one in a multi-BIN solution)?
    //
    for (nCount = 0, dwStoreOffset = 0; nCount < g_BINRegionInfo.dwNumRegions ; nCount++)
    {
        EdbgOutputDebugString("BIN Region Number : %d\r\n", nCount);
        DWORD dwRegionStart  = (DWORD)OEMMapMemAddr(0, g_BINRegionInfo.Region[nCount].dwRegionStart);
        DWORD dwRegionLength = g_BINRegionInfo.Region[nCount].dwRegionLength;

        // Media byte offset where image region is stored.
        dwStoreOffset += nCount ? dwMaxRegionLength[nCount-1] : 0;

        // Set the file pointer (byte indexing) to the correct offset for this particular region.
        //
        if ( !BP_SetDataPointer(hPart, dwStoreOffset) )
        {
            OALMSG(OAL_ERROR, (TEXT("ERROR: StoreImageToBootMedia: Failed to set data pointer in partition (offset=0x%x).\r\n"), dwStoreOffset));
            return(FALSE);
        }

        // Write the region to the BINFS partition.
        if ( !BP_WriteData(hPart, (LPBYTE)dwRegionStart, dwRegionLength) )
        {
            EdbgOutputDebugString("ERROR: StoreImageToBootMedia: Failed to write region to BINFS partition (start=0x%x, length=0x%x).\r\n", dwRegionStart, dwRegionLength);
            return(FALSE);
        }
        if(nCount > 1)  // We will skip Region 0 -> for bootloader
        {

        }
        // update TOC, when we reach to last TOC entry.
        if ((g_pTOC->id[g_dwTocEntry].dwLoadAddress == g_BINRegionInfo.Region[nCount].dwRegionStart) &&
             g_pTOC->id[g_dwTocEntry].dwTtlSectors == FILE_TO_SECTOR_SIZE(dwRegionLength) )
        {
            g_pTOC->id[g_dwTocEntry].dwStoreOffset = dwStoreOffset;
            g_pTOC->id[g_dwTocEntry].dwJumpAddress = 0; // Filled upon return to OEMLaunch

            g_pTOC->id[g_dwTocEntry].dwImageType = g_ImageType;

            g_pTOC->id[g_dwTocEntry].sgList[0].dwSector = FILE_TO_SECTOR_SIZE(g_dwLastWrittenLoc);
            g_pTOC->id[g_dwTocEntry].sgList[0].dwLength = g_pTOC->id[g_dwTocEntry].dwTtlSectors;

            // copy Kernel Region to SDRAM for jump
            memcpy((void*)g_pTOC->id[g_dwTocEntry].dwLoadAddress, (void*)dwRegionStart, dwRegionLength);

            OALMSG(TRUE, (TEXT("Update Last TOC[%d] Entry to TOC cache in memory!\r\n"), g_pTOC->id[g_dwTocEntry]));
        }
     }

    TOC_Print();

    OALMSG(OAL_FUNC, (TEXT("-WriteOSImageToBootMedia\r\n")));

    return(TRUE);
}


/*
    @func   BOOL | ReadKernelRegionFromBootMedia |
            BinFS support. Reads the kernel region from Boot Media into RAM.  The kernel region is fixed up
            to run from RAM and this is done just before jumping to the kernel entry point.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm
    @xref
*/
BOOL ReadOSImageFromBootMedia()
{
    HANDLE hPart;
    DWORD   chainaddr, flashaddr;
    DWORD i;

    OALMSG(OAL_FUNC, (TEXT("+ReadOSImageFromBootMedia\r\n")));

    if (!g_bBootMediaExist)
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: ReadOSImageFromBootMedia: device doesn't exist.\r\n")));
        return(FALSE);
    }

    if ( !VALID_TOC(g_pTOC) )
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: ReadOSImageFromBootMedia: INVALID_TOC\r\n")));
        return(FALSE);
    }

    if ( !VALID_IMAGE_DESCRIPTOR(&g_pTOC->id[g_dwTocEntry]) )
    {
        OALMSG(OAL_ERROR, (TEXT("ReadOSImageFromBootMedia: ERROR_INVALID_IMAGE_DESCRIPTOR: 0x%x\r\n"),
            g_pTOC->id[g_dwTocEntry].dwSignature));
        return FALSE;
    }

    if ( !OEMVerifyMemory(g_pTOC->id[g_dwTocEntry].dwLoadAddress, sizeof(DWORD)) ||
         !OEMVerifyMemory(g_pTOC->id[g_dwTocEntry].dwJumpAddress, sizeof(DWORD)) ||
         !g_pTOC->id[g_dwTocEntry].dwTtlSectors )
    {
        OALMSG(OAL_ERROR, (TEXT("ReadOSImageFromBootMedia: ERROR_INVALID_ADDRESS: (address=0x%x, sectors=0x%x, launch address=0x%x)...\r\n"),
            g_pTOC->id[g_dwTocEntry].dwLoadAddress, g_pTOC->id[g_dwTocEntry].dwTtlSectors, g_pTOC->id[g_dwTocEntry].dwJumpAddress));
        return FALSE;
    }

    // Open the BINFS partition (it must exist).
    //
    hPart = BP_OpenPartition( NEXT_FREE_LOC,
                              USE_REMAINING_SPACE,
                              PART_BINFS,
                              TRUE,
                              PART_OPEN_EXISTING);

    if (hPart == INVALID_HANDLE_VALUE )
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: ReadOSImageFromBootMedia: Failed to open existing partition.\r\n")));
        return(FALSE);
    }

    // Set the partition file pointer to the correct offset for the kernel region.
    //
    if ( !BP_SetDataPointer(hPart, g_pTOC->id[g_dwTocEntry].dwStoreOffset) )
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: ReadOSImageFromBootMedia: Failed to set data pointer in partition (offset=0x%x).\r\n"),
            g_pTOC->id[g_dwTocEntry].dwStoreOffset));
        return(FALSE);
    }

    // Read the kernel region from the Boot Media into RAM.
    //
    if ( !BP_ReadData( hPart,
                       (LPBYTE)(g_pTOC->id[g_dwTocEntry].dwLoadAddress),
                       SECTOR_TO_FILE_SIZE(g_pTOC->id[g_dwTocEntry].dwTtlSectors)) )
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: ReadOSImageFromBootMedia: Failed to read kernel region from partition.\r\n")));
        return(FALSE);
    }

    if (!g_pTOC->chainInfo.dwLoadAddress)
    {
        chainaddr = g_pTOC->chainInfo.dwLoadAddress;
        flashaddr = g_pTOC->chainInfo.dwFlashAddress;
        for ( i = 0; i < (g_pTOC->chainInfo.dwLength); i++ )
        {
            OALMSG(TRUE, (TEXT("chainaddr=0x%x, flashaddr=0x%x\r\n"), chainaddr, flashaddr+i));

            SDHC_READ((UINT32)(flashaddr+i), 1, (UINT32)chainaddr);

            chainaddr += 512;
        }
    }
    OALMSG(OAL_FUNC, (TEXT("_ReadOSImageFromBootMedia\r\n")));
    return(TRUE);
}


BOOL WriteRawImageToBootMedia(DWORD dwImageStart, DWORD dwImageLength, DWORD dwLaunchAddr)
{
    LPBYTE pbBuffer;

    OALMSG(OAL_FUNC, (TEXT("+WriteRawImageToBootMedia\r\n")));

    if ( !g_bBootMediaExist )
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: WriteRawImageToBootMedia: device doesn't exist.\r\n")));
        return(FALSE);
    }

    pbBuffer = OEMMapMemAddr(dwImageStart, dwImageStart);

     if (g_ImageType == IMAGE_TYPE_STEPLDR)
    {
        RETAILMSG(1,(TEXT("Fusing Steploader from memory 0x%x\n"),pbBuffer));
        SDHC_WRITE(STEPLDRSTARTSECTOR, SECTOROFSTEPLDR, (UINT32)pbBuffer);
    }
    else if (g_ImageType == IMAGE_TYPE_LOADER)
    {
        RETAILMSG(1,(TEXT("Fusing MMCBoot from memory 0x%x\n"),pbBuffer));
        SDHC_WRITE(SDBOOTSTARTSECTOR, SECTOROFSDBOOT, (UINT32)pbBuffer);
    }
    else
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: Can not found Image Type\r\n")));
        return FALSE;
    }

    OALMSG(OAL_FUNC, (TEXT("_WriteRawImageToBootMedia\r\n")));

    return TRUE;
}
