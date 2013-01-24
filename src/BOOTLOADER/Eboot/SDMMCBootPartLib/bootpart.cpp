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
#include <bootpart.h>
#include "bppriv.h"
#include <fmd.h>
#include "image_cfg.h"
#include <HSMMCDrv.h>
#include <IROM_SDMMC_loader_cfg.h>

// Global Variables.
LPBYTE g_pbMBRSector = NULL;
LPBYTE g_pbBlock     = NULL;
FlashInfo g_FlashInfo;
PSectorInfo g_pSectorInfoBuf;
DWORD g_dwMBRSectorNum = INVALID_ADDR;
PARTSTATE g_partStateTable[NUM_PARTS];
DWORD g_dwLastLogSector;    // Stores the last valid logical sector
DWORD g_dwDataBytesPerBlock;
DWORD g_dwLastWrittenLoc;   // Stores the byte address of the last physical flash address written to

// Defines.
#define MAX_WRITE_SECTOR    (512)
#define SECTOR_PER_SECTOR   (512)

#define _INF_ (TRUE)
#define _ERR_ (TRUE)

// External variables.
#ifdef __cplusplus
extern "C" {
#endif
extern UINT32 g_dwSectorCount;
extern UINT32 g_dwIsSDHC;
#ifdef __cplusplus
    }
#endif

static Addr LBAtoCHS(FlashInfo *pFlashInfo, Addr lba)
{
    if(lba.type == CHS)
        return lba;

    Addr chs;
    DWORD tmp = pFlashInfo->dwNumBlocks * pFlashInfo->wSectorsPerBlock;

    chs.type = CHS;
    chs.chs.cylinder = (WORD)(lba.lba / tmp);
    tmp = lba.lba % tmp;
    chs.chs.head = (WORD)(tmp / pFlashInfo->wSectorsPerBlock);
    chs.chs.sector = (WORD)((tmp % pFlashInfo->wSectorsPerBlock) + 1);

    return chs;
}


static Addr CHStoLBA(FlashInfo *pFlashInfo, Addr chs)
{
    Addr lba;

    if(chs.type == LBA)
        return chs;

    lba.type = LBA;
    lba.lba = ((chs.chs.cylinder * pFlashInfo->dwNumBlocks + chs.chs.head)
        * pFlashInfo->wSectorsPerBlock)+ chs.chs.sector - 1;

    return lba;
}


static DWORD GetMBRSectorNum(void)
{
    return SECTOROFIMAGE;
}


static BOOL WriteMBR(void)
{
    DWORD dwMBRBlockNum = g_dwMBRSectorNum / g_FlashInfo.wSectorsPerBlock;

    RETAILMSG(_INF_, (TEXT("[BP:INF] WriteMBR: MBR block = 0x%x.\r\n"), MBRSTARTSECTOR));

    memset(g_pbBlock, 0xff, g_dwDataBytesPerBlock);
    memcpy(g_pbBlock, g_pbMBRSector, g_FlashInfo.wDataBytesPerSector);

    SDHC_WRITE(MBRSTARTSECTOR, SECTOROFMBR, (UINT32)g_pbBlock);
    return TRUE;

}


static BOOL CreateMBR(void)
{
    // This, plus a valid partition table, is all the CE partition manager needs to recognize
    // the MBR as valid. It does not contain boot code.
    RETAILMSG(_INF_, (TEXT("[BP:INF] CreateMBR: ++\r\n")));
    RETAILMSG(_INF_, (TEXT("[BP:INF] CreateMBR: g_pbMBRSector= 0x%x\r\n"), g_pbMBRSector));
    RETAILMSG(_INF_, (TEXT("[BP:INF] CreateMBR: g_pbBlock= 0x%x\r\n"), g_pbBlock));
    RETAILMSG(_INF_, (TEXT("[BP:INF] CreateMBR: wDataBytesPerSector= %d\r\n"), g_FlashInfo.wDataBytesPerSector));

    memset (g_pbMBRSector, 0xff, g_FlashInfo.wDataBytesPerSector);
    g_pbMBRSector[0] = 0xE9;
    g_pbMBRSector[1] = 0xfd;
    g_pbMBRSector[2] = 0xff;
    g_pbMBRSector[SECTOR_SIZE-2] = 0x55;
    g_pbMBRSector[SECTOR_SIZE-1] = 0xAA;

    // Zero out partition table so that mspart treats entries as empty.
    memset(g_pbMBRSector+PARTTABLE_OFFSET, 0, sizeof(PARTENTRY) * NUM_PARTS);

    RETAILMSG(_INF_, (TEXT("[BP:INF] CreateMBR: --\r\n")));
    return WriteMBR();

}


static BOOL IsValidMBR(void)
{
    RETAILMSG(_INF_, (TEXT("[BP:INF] IsValidMBR: MBR sector = 0x%x\r\n"), MBRSTARTSECTOR));

    SDHC_READ(MBRSTARTSECTOR, SECTOROFMBR, (UINT32)g_pbMBRSector);

    return ((g_pbMBRSector[0] == 0xE9) &&
         (g_pbMBRSector[1] == 0xfd) &&
         (g_pbMBRSector[2] == 0xff) &&
         (g_pbMBRSector[SECTOR_SIZE-2] == 0x55) &&
         (g_pbMBRSector[SECTOR_SIZE-1] == 0xAA));
}


static BOOL IsValidPart (PPARTENTRY pPartEntry)
{
    RETAILMSG(_INF_, (TEXT("[BP:INF] IsValidPart: Part_FileSystem = 0x%x\r\n"), pPartEntry->Part_FileSystem));

    return (pPartEntry->Part_FileSystem != 0xff) && (pPartEntry->Part_FileSystem != 0);
}


/*  AddPartitionTableEntry
 *
 *  Generates the partition entry for the partition table and copies the entry
 *  into the MBR that is stored in memory.
 *
 *
 *  ENTRY
 *      entry - index into partition table
 *      startSector - starting logical sector
 *      totalSectors - total logical sectors
 *      fileSystem - type of partition
 *      bootInd - byte in partition entry that stores various flags such as
 *          active and read-only status.
 *
 *  EXIT
 */
static void AddPartitionTableEntry(DWORD entry, DWORD startSector, DWORD totalSectors, BYTE fileSystem, BYTE bootInd)
{
    PARTENTRY partentry = {0};
    Addr startAddr;
    Addr endAddr;

    ASSERT(entry < 4);

    // no checking with disk info and start/total sectors because we allow
    // bogus partitions for testing purposes

    // initially known partition table entry
    partentry.Part_BootInd = bootInd;
    partentry.Part_FileSystem = fileSystem;
    partentry.Part_StartSector = startSector;
    partentry.Part_TotalSectors = totalSectors;

    // logical block addresses for the first and final sector (start on the second head)
    startAddr.type = LBA;
    startAddr.lba = partentry.Part_StartSector;
    endAddr.type = LBA;
    endAddr.lba = partentry.Part_StartSector + partentry.Part_TotalSectors-1;

    // translate the LBA addresses to CHS addresses
    startAddr = LBAtoCHS(&g_FlashInfo, startAddr);
    endAddr = LBAtoCHS(&g_FlashInfo, endAddr);

    // starting address
    partentry.Part_FirstTrack = (BYTE)(startAddr.chs.cylinder & 0xFF);
    partentry.Part_FirstHead = (BYTE)(startAddr.chs.head & 0xFF);
    // lower 6-bits == sector, upper 2-bits = cylinder upper 2-bits of 10-bit cylinder #
    partentry.Part_FirstSector = (BYTE)((startAddr.chs.sector & 0x3F) | ((startAddr.chs.cylinder & 0x0300) >> 2));

    // ending address:
    partentry.Part_LastTrack = (BYTE)(endAddr.chs.cylinder & 0xFF);
    partentry.Part_LastHead = (BYTE)(endAddr.chs.head & 0xFF);
    // lower 6-bits == sector, upper 2-bits = cylinder upper 2-bits of 10-bit cylinder #
    partentry.Part_LastSector = (BYTE)((endAddr.chs.sector & 0x3F) | ((endAddr.chs.cylinder & 0x0300) >> 2));

    partentry.Part_FirstHead = 0x6;
    partentry.Part_FirstSector = 0x1;
    partentry.Part_FirstTrack = 0x0;
    partentry.Part_LastHead = 0x5;
    partentry.Part_LastSector = 0x21;
    partentry.Part_LastTrack = 0x1;

    memcpy(g_pbMBRSector+PARTTABLE_OFFSET+(sizeof(PARTENTRY)*entry), &partentry, sizeof(PARTENTRY));
    RETAILMSG(_INF_,(TEXT("[BP:INF] PART_BINFS flag is 0x%x\n"),PART_BINFS));
    RETAILMSG(_INF_,(TEXT("[BP:INF] partentry.Part_BootInd 0x%x\n"),partentry.Part_BootInd));
    RETAILMSG(_INF_,(TEXT("[BP:INF] partentry.Part_FileSystem 0x%x\n"),partentry.Part_FileSystem));
    RETAILMSG(_INF_,(TEXT("[BP:INF] partentry.Part_FirstHead 0x%x\n"),partentry.Part_FirstHead));
    RETAILMSG(_INF_,(TEXT("[BP:INF] partentry.Part_FirstSector 0x%x\n"),partentry.Part_FirstSector));
    RETAILMSG(_INF_,(TEXT("[BP:INF] partentry.Part_FirstTrack 0x%x\n"),partentry.Part_FirstTrack));
    RETAILMSG(_INF_,(TEXT("[BP:INF] partentry.Part_LastHead 0x%x\n"),partentry.Part_LastHead));
    RETAILMSG(_INF_,(TEXT("[BP:INF] partentry.Part_LastSector 0x%x\n"),partentry.Part_LastSector));
    RETAILMSG(_INF_,(TEXT("[BP:INF] partentry.Part_StartSector 0x%x\n"),partentry.Part_StartSector));
    RETAILMSG(_INF_,(TEXT("[BP:INF] partentry.Part_TotalSectors 0x%x\n"),partentry.Part_TotalSectors));
}


/*  GetPartitionTableIndex
 *
 *  Get the partition index for a particular partition type and active status.
 *  If partition is not found, then the index of the next free partition in the
 *  partition table of the MBR is returned.
 *
 *
 *  ENTRY
 *      dwPartType - type of partition
 *      fActive - TRUE indicates the active partition.  FALSE indicates inactive.
 *
 *  EXIT
 *      pdwIndex - Contains the index of the partition if found.  If not found,
 *          contains the index of the next free partition
 *      returns TRUE if partition found. FALSE if not found.
 */
static BOOL GetPartitionTableIndex(DWORD dwPartType, BOOL fActive, PDWORD pdwIndex)
{
    PPARTENTRY pPartEntry = (PPARTENTRY)(g_pbMBRSector + PARTTABLE_OFFSET);
    DWORD iEntry = 0;

    RETAILMSG(_INF_, (TEXT("[BP:INF] GetPartitionTableIndex: ++\r\n")));
    for (iEntry = 0; iEntry < NUM_PARTS; iEntry++, pPartEntry++)
    {
        RETAILMSG(_INF_, (TEXT("[BP:INF] GetPartitionTableIndex: pPartEntry->Part_FileSystem : 0x%x\r\n"), pPartEntry->Part_FileSystem));

        if ((pPartEntry->Part_FileSystem == dwPartType))
        {
            *pdwIndex = iEntry;
            return TRUE;
        }
        if (!IsValidPart(pPartEntry))
        {
            *pdwIndex = iEntry;
            RETAILMSG(_ERR_, (TEXT("[BP:ERR] GetPartitionTableIndex: IsValidPart error\r\n")));
            return FALSE;
        }
    }

    RETAILMSG(_ERR_, (TEXT("[BP:ERR] GetPartitionTableIndex: -- error\r\n")));
    return FALSE;
}


/* WriteLogicalNumbers
 *
 *  Writes a range of logical sector numbers
 *
 *  ENTRY
 *      dwStartSector - starting logical sector
 *      dwNumSectors - number of logical sectors to mark
 *      fReadOnly - TRUE indicates to mark read-only.  FALSE to mark not read-only
 *
 *  EXIT
 *      TRUE on success
 */
static BOOL WriteLogicalNumbers (DWORD dwStartSector, DWORD dwNumSectors, BOOL fReadOnly)
{
    DWORD dwNumSectorsWritten = 0;

    DWORD dwPhysSector = Log2Phys (dwStartSector);
    DWORD dwBlockNum = dwPhysSector / g_FlashInfo.wSectorsPerBlock;
    DWORD dwOffset = dwPhysSector % g_FlashInfo.wSectorsPerBlock;

    while (dwNumSectorsWritten < dwNumSectors)
    {
        break;
    }

    return TRUE;
}


static DWORD Log2Phys (DWORD dwLogSector)
{
    return dwLogSector;
}


/*  LastLogSector
 *
 *  Returns the last logical sector on the flash
 *
 *  ENTRY
 *
 *  EXIT
 *      Returns the last logical sector on the flash
 */
static DWORD LastLogSector(void)
{
    /*
    if (g_dwLastLogSector)
    {
       return g_dwLastLogSector;
    }*/

    g_dwLastLogSector = SECTOROFIMAGE-8; // to give a gab

    RETAILMSG(_INF_, (TEXT("[BP:INF] LastLogSector: Last log sector is: 0x%x.\r\n"), g_dwLastLogSector));

    return g_dwLastLogSector;
}


/*  FindFreeSectors
 *
 *  Finds the last free logical sector.
 *
 *  ENTRY
 *
 *  EXIT
 *      Last free logical sector.  INVALID_ADDR if none are free.
 */
static DWORD FindFreeSector(void)
{
    DWORD dwFreeSector = 1;
    PPARTENTRY pPartEntry = (PPARTENTRY)(g_pbMBRSector + PARTTABLE_OFFSET);

    for (int i = 0; i < NUM_PARTS; i++)
    {
        if (!IsValidPart (pPartEntry))
        {
            break;
        }

        if ((pPartEntry->Part_StartSector + pPartEntry->Part_TotalSectors) > dwFreeSector)
        {
            dwFreeSector = pPartEntry->Part_StartSector + pPartEntry->Part_TotalSectors;
        }

        RETAILMSG(_INF_, (TEXT("[BP:INF] FindFreeSector: FreeSector is: 0x%x 0x%x 0x%x after processing part 0x%x.\r\n"), dwFreeSector, pPartEntry->Part_StartSector, pPartEntry->Part_TotalSectors, pPartEntry->Part_FileSystem));
        pPartEntry++;
    }

    if (dwFreeSector == 1)
    {
        return INVALID_ADDR;
    }

    if (dwFreeSector % 8)
    {
        dwFreeSector += (8 - dwFreeSector % 8 ); // to align 4KB
    }

    dwFreeSector += 8; // to give a gab between partitions.

    return dwFreeSector;
}


/*  CreatePartition
 *
 *  Creates a new partition.  If it is a boot section partition, then it formats
 *  flash.
 *
 *  ENTRY
 *      dwStartSector - Logical sector to start the partition.  NEXT_FREE_LOC if
 *          none specified.
 *      dwNumSectors - Number of logical sectors of the partition.  USE_REMAINING_SPACE
 *          to indicate to take up the rest of the space on the flash for that partition.
 *      dwPartType - Type of partition to create.
 *      fActive - TRUE indicates to create the active partition.  FALSE for
 *          inactive.
 *      dwPartIndex - Index of the partition entry on the MBR
 *
 *  EXIT
 *      Handle to the partition on success.  INVALID_HANDLE_VALUE on error.
 */
static HANDLE CreatePartition(DWORD dwStartSector, DWORD dwNumSectors, DWORD dwPartType, BOOL fActive, DWORD dwPartIndex)
{
    DWORD dwBootInd = 0;

    if (fActive)
    {
        dwBootInd |= PART_IND_ACTIVE;
    }

    if (dwPartType == PART_BOOTSECTION || dwPartType == PART_BINFS || dwPartType == PART_XIP)
    {
        dwBootInd |= PART_IND_READ_ONLY;
    }

    if (dwStartSector == NEXT_FREE_LOC)
    {
        dwStartSector = FindFreeSector();
        if (dwStartSector == INVALID_ADDR)
        {
            RETAILMSG(_ERR_, (TEXT("[BP:ERR] CreatePartition: can't find free sector.\r\n")));
            return INVALID_HANDLE_VALUE;
        }

        // Start partitions on the next block if they are currently on the wrong block type.
    }

    if (IS_PART_READONLY(dwBootInd))
    {
        // Allow read-only partitions to go to the end of disk, if requested.
        if (dwNumSectors == USE_REMAINING_SPACE)
        {
            DWORD dwLastLogSector = LastLogSector();
            if (dwLastLogSector == INVALID_ADDR)
            {
                RETAILMSG(_ERR_, (TEXT("[BP:ERR] CreatePartition: read only and invalid addr.\r\n")));
                return INVALID_HANDLE_VALUE;
            }
            dwNumSectors = dwLastLogSector - (dwStartSector -1);
        }
    }
    else
    {
        DWORD dwLastLogSector = LastLogSector();
        if (dwLastLogSector == INVALID_ADDR)
        {
            RETAILMSG(_ERR_, (TEXT("[BP:ERR] CreatePartition: not read only and invalid addr.\r\n")));
            return INVALID_HANDLE_VALUE;
        }

        RETAILMSG(1, (TEXT("[BP:INF] ##### Start Sector = 0x%x End sector = 0x%x.\r\n"),dwStartSector,dwLastLogSector));
        DWORD dwNumMaxSectors = dwLastLogSector - (dwStartSector-1);

        // If dwNumSectors was provided, validate it isn't past the max.
        // If dwNumSectors is USE_REMAINING_SPACE, fill disk with max sectors.
        if ((dwNumSectors == USE_REMAINING_SPACE)  || (dwNumMaxSectors <  dwNumSectors)) {
            RETAILMSG(_INF_, (TEXT("[BP:INF] CreatePartition: Num sectors set to 0x%x to allow for compaction blocks.\r\n"), dwNumMaxSectors));
            dwNumSectors = dwNumMaxSectors ;
        }
    }

    RETAILMSG(_INF_, (TEXT("[BP:INF] CreatePartition: Start = 0x%x, Num = 0x%x.\r\n"), dwStartSector, dwNumSectors));
    AddPartitionTableEntry (dwPartIndex, dwStartSector, dwNumSectors, (BYTE)dwPartType, (BYTE)dwBootInd);

    RETAILMSG(_INF_, (TEXT("[BP:INF] Write MBR start.\n")));
    if (!WriteMBR())
    {
        RETAILMSG(_ERR_, (TEXT("[BP:ERR] CreatePartition: WirteMBR() failed.\r\n")));
        return INVALID_HANDLE_VALUE;
    }

    RETAILMSG(_INF_,(TEXT("Write MBR End.\n")));
    g_partStateTable[dwPartIndex].pPartEntry = (PPARTENTRY)(g_pbMBRSector + PARTTABLE_OFFSET + sizeof(PARTENTRY)*dwPartIndex);
    g_partStateTable[dwPartIndex].dwDataPointer = 0;

    return (HANDLE)&g_partStateTable[dwPartIndex];
}


/*  BP_ReadData
 *
 *  Reads data from the partition starting at the data pointer.  Call fails
 *  if length of the buffer is too long (i.e. trying to read past the end
 *  of the partition)
 *
 *  ENTRY
 *      hPartition - handle to the partition
 *      pbBuffer - pointer to buffer of data to read
 *      dwLength - number of bytes to read
 *
 *  EXIT
 *      TRUE on success
 */
BOOL BP_ReadData(HANDLE hPartition, LPBYTE pbBuffer, DWORD dwLength)
{
    if (hPartition == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    static LPBYTE pbSector = g_pbBlock;
    PPARTSTATE pPartState = (PPARTSTATE) hPartition;
    DWORD dwNextPtrValue = pPartState->dwDataPointer + dwLength;
    DWORD dwStartSector, dwSectors, i, dwSecTemp;

    if (!pbBuffer || !pbSector || dwLength == 0)
    {
        return(FALSE);
    }

    RETAILMSG(_INF_, (TEXT("[BP:INF] ReadData: Start = 0x%x, Length = 0x%x.\r\n"), pPartState->dwDataPointer, dwLength));

    // Check to make sure buffer size is within limits of partition
    if (((dwNextPtrValue - 1) / g_FlashInfo.wDataBytesPerSector) >= pPartState->pPartEntry->Part_TotalSectors) {
        RETAILMSG(_ERR_, (TEXT("[BP:ERR] ReadData: trying to read past end of partition.\r\n")));
        return FALSE;
    }

    if ( pPartState->dwDataPointer % g_FlashInfo.wDataBytesPerSector != 0 )
    {
        dwStartSector = pPartState->dwDataPointer / g_FlashInfo.wDataBytesPerSector + 1;
    }
    else
    {
        dwStartSector = pPartState->dwDataPointer / g_FlashInfo.wDataBytesPerSector;
    }

    if ( dwLength % g_FlashInfo.wDataBytesPerSector != 0 )
    {
        dwSectors = dwLength / g_FlashInfo.wDataBytesPerSector + 1;
    }
    else
    {
        dwSectors = dwLength / g_FlashInfo.wDataBytesPerSector;
    }

    RETAILMSG(_INF_,(TEXT("[BP:INF] Load Image to 0x%x from sector 0x%x  to sector 0x%x\n"),
                    pbBuffer,dwStartSector+IMAGESTARTSECTOR, dwSectors));
#if 0
    if (!SDHC_READ(dwStartSector+IMAGESTARTSECTOR, dwSectors, (UINT32)pbBuffer))
    {
        RETAILMSG(_ERR_,(TEXT("[BP:ERR] Failed to Load Image from SDMMC\n")));
        return (FALSE);
    }

#else
    if (dwSectors < 512)
    {
        SDHC_READ(dwStartSector+IMAGESTARTSECTOR, dwSectors, (UINT32)pbBuffer);
    }
    else
    {
        for ( i = 0 ,  dwSecTemp = dwSectors; dwSecTemp >= MAX_WRITE_SECTOR ; i++ , dwSecTemp -= MAX_WRITE_SECTOR)
        {
            SDHC_READ(dwStartSector+IMAGESTARTSECTOR+(i*MAX_WRITE_SECTOR), MAX_WRITE_SECTOR, (UINT32)pbBuffer+(UINT32)(i*SECTOR_PER_SECTOR*MAX_WRITE_SECTOR));
        }

        if ( dwSecTemp != 0 )
        {
            SDHC_READ(dwStartSector+IMAGESTARTSECTOR+(i*MAX_WRITE_SECTOR), dwSecTemp, (UINT32)pbBuffer+(UINT32)(i*SECTOR_PER_SECTOR*MAX_WRITE_SECTOR));
        }
    }
#endif

    pPartState->dwDataPointer = dwNextPtrValue;
    return (TRUE);
}


/*  BP_WriteData
 *
 *  Writes data to the partition starting at the data pointer.  Call fails
 *  if length of the buffer is too long (i.e. trying to write past the end
 *  of the partition)
 *
 *  ENTRY
 *      hPartition - handle to the partition
 *      pbBuffer - pointer to buffer of data to write
 *      dwLength - length in bytes of the buffer
 *
 *  EXIT
 *      TRUE on success
 */
BOOL BP_WriteData(HANDLE hPartition, LPBYTE pbBuffer, DWORD dwLength)
{
    if (hPartition == INVALID_HANDLE_VALUE)
        return FALSE;

    PPARTSTATE pPartState = (PPARTSTATE) hPartition;
    DWORD dwNextPtrValue = pPartState->dwDataPointer + dwLength;
    DWORD dwStartSector;
    DWORD dwSectors;
    DWORD i,dwSecTemp;

    RETAILMSG(_INF_, (TEXT("[BP:INF] WriteData: Start = 0x%x, Length = 0x%x.\r\n"), pPartState->dwDataPointer, dwLength));

    if (!pbBuffer || !g_pbBlock || dwLength == 0)
    {
        RETAILMSG(_ERR_,(TEXT("[BP:ERR] BP_WriteData Fails.  pbBuffer = 0x%x, g_pbBlock = 0x%x, dwLength = 0x%x\r\n"), pbBuffer, g_pbBlock, dwLength));
        return(FALSE);
    }

    // Check to make sure buffer size is within limits of partition
    RETAILMSG(_INF_, (TEXT("[BP:INF] WriteData: writing sector number = 0x%x.\r\n"),((dwNextPtrValue - 1) / g_FlashInfo.wDataBytesPerSector)));
    RETAILMSG(_INF_, (TEXT("[BP:INF] WriteData: partition total sector number = 0x%x.\r\n"), pPartState->pPartEntry->Part_TotalSectors));
    if (((dwNextPtrValue - 1) / g_FlashInfo.wDataBytesPerSector) >= pPartState->pPartEntry->Part_TotalSectors)
    {
        RETAILMSG(_ERR_, (TEXT("[BP:ERR] WriteData: trying to write past end of partition.\r\n")));
        return FALSE;
    }

    if ( pPartState->dwDataPointer % g_FlashInfo.wDataBytesPerSector != 0 )
    {
        dwStartSector = pPartState->dwDataPointer / g_FlashInfo.wDataBytesPerSector + 1;
        RETAILMSG(_INF_, (TEXT("[BP:INF] WriteData: dwDataPointer is not divided by 512 #1 dwStartSector 0x%x\r\n"),dwStartSector));
    }
    else
    {
        dwStartSector = pPartState->dwDataPointer / g_FlashInfo.wDataBytesPerSector;
        RETAILMSG(_INF_, (TEXT("[BP:INF] WriteData: dwDataPointer is divided by 512 #1 dwStartSector 0x%x\r\n"),dwStartSector));
    }

    if ( dwLength % g_FlashInfo.wDataBytesPerSector != 0 )
    {
        dwSectors = dwLength / g_FlashInfo.wDataBytesPerSector + 1;
        RETAILMSG(_INF_, (TEXT("[BP:INF] WriteData: dwLength is not divided by 512 #1 dwSectors 0x%x\r\n"),dwSectors));
    }
    else
    {
        dwSectors = dwLength / g_FlashInfo.wDataBytesPerSector;
        dwNextPtrValue = pPartState->dwDataPointer + dwSectors * 512;
        RETAILMSG(_INF_, (TEXT("[BP:INF] WriteData: dwLength is divided by 512 #1 dwSectors 0x%x\r\n"),dwSectors));
    }

    g_dwLastWrittenLoc = (dwStartSector+IMAGESTARTSECTOR) * 512;

    RETAILMSG(_INF_, (TEXT("[BP:INF] WriteData: dwStartsector 0x%x, sectors 0x%x, pbBuffer 0x%x\n"),dwStartSector,dwSectors,pbBuffer));

    if (dwSectors < 512)
    {
        SDHC_WRITE(dwStartSector+IMAGESTARTSECTOR, dwSectors, (UINT32)pbBuffer);
    }
    else
    {
        for ( i = 0 ,  dwSecTemp = dwSectors; dwSecTemp >= MAX_WRITE_SECTOR ; i++ , dwSecTemp -= MAX_WRITE_SECTOR)
        {
            SDHC_WRITE(dwStartSector+IMAGESTARTSECTOR+(i*MAX_WRITE_SECTOR), MAX_WRITE_SECTOR, (UINT32)pbBuffer+(UINT32)(i*SECTOR_PER_SECTOR*MAX_WRITE_SECTOR));
        }

        if ( dwSecTemp != 0 )
        {
            SDHC_WRITE(dwStartSector+IMAGESTARTSECTOR+(i*MAX_WRITE_SECTOR), dwSecTemp, (UINT32)pbBuffer+(UINT32)(i*SECTOR_PER_SECTOR*MAX_WRITE_SECTOR));
        }
    }

    pPartState->dwDataPointer = dwNextPtrValue;
    return(TRUE);
}


/*  BP_OpenPartition
 *
 *  Opens/creates a partition depending on the creation flags.  If it is opening
 *  and the partition has already been opened, then it returns a handle to the
 *  opened partition.  Otherwise, it loads the state information of that partition
 *  into memory and returns a handle.
 *
 *  ENTRY
 *      dwStartSector - Logical sector to start the partition.  NEXT_FREE_LOC if none
 *          specified.  Ignored if opening existing partition.
 *      dwNumSectors - Number of logical sectors of the partition.  USE_REMAINING_SPACE
 *          to indicate to take up the rest of the space on the flash for that partition (should
 *          only be used when creating extended partitions).  This parameter is ignored
 *          if opening existing partition.
 *      dwPartType - Type of partition to create/open.
 *      fActive - TRUE indicates to create/open the active partition.  FALSE for
 *          inactive.
 *      dwCreationFlags - PART_CREATE_NEW to create only.  Fail if it already
 *          exists.  PART_OPEN_EXISTING to open only.  Fail if it doesn't exist.
 *          PART_OPEN_ALWAYS creates if it does not exist and opens if it
 *          does exist.
 *
 *  EXIT
 *      Handle to the partition on success.  INVALID_HANDLE_VALUE on error.
 */
HANDLE BP_OpenPartition(DWORD dwStartSector, DWORD dwNumSectors, DWORD dwPartType, BOOL fActive, DWORD dwCreationFlags)
{
    DWORD dwPartIndex;
    BOOL fExists;

    ASSERT (g_pbMBRSector);

    if (!IsValidMBR())
    {
        DWORD dwFlags = 0;

        if (dwCreationFlags == PART_OPEN_EXISTING) {
            RETAILMSG(_ERR_, (TEXT("[BP:ERR] OpenPartition: Invalid MBR.  Cannot open existing partition 0x%x.\r\n"), dwPartType));
            return INVALID_HANDLE_VALUE;
        }

        RETAILMSG(_ERR_, (TEXT("[BP:ERR] OpenPartition: Invalid MBR.  Create a new MBR for moviBoot.\r\n")));

        BP_LowLevelFormat (0, 0, 0); // It does not format. just for creating MBR.
        dwPartIndex = 0;
        fExists = FALSE;
    }
    else {
        fExists = GetPartitionTableIndex(dwPartType, fActive, &dwPartIndex);
    }

    RETAILMSG(_INF_, (TEXT("[BP:INF] OpenPartition: Partition Exists=0x%x for part 0x%x.\r\n"), fExists, dwPartType));
    if (fExists) {
        // Partition was found.
        if (dwCreationFlags == PART_CREATE_NEW)
        {
            return INVALID_HANDLE_VALUE;
        }

        if (g_partStateTable[dwPartIndex].pPartEntry == NULL) {
            // Open partition.  If this is the boot section partition, then file pointer starts after MBR
            g_partStateTable[dwPartIndex].pPartEntry = (PPARTENTRY)(g_pbMBRSector + PARTTABLE_OFFSET + sizeof(PARTENTRY)*dwPartIndex);
            g_partStateTable[dwPartIndex].dwDataPointer = 0;
        }
        return (HANDLE)&g_partStateTable[dwPartIndex];
    }
    else {
        // If there are already 4 partitions, or creation flag specified OPEN_EXISTING, fail.
        if ((dwPartIndex == NUM_PARTS) || (dwCreationFlags == PART_OPEN_EXISTING))
        {
            return INVALID_HANDLE_VALUE;
        }

        // Create new partition
        RETAILMSG(_INF_,(TEXT("[BP:INF] Can not find appropriate partition\n")));
        RETAILMSG(_INF_,(TEXT("[BP:INF] Create partition from 0x%x to 0x%x\n"),dwStartSector,dwNumSectors));
        return CreatePartition (dwStartSector, dwNumSectors, dwPartType, fActive, dwPartIndex);
    }

    return INVALID_HANDLE_VALUE;
}


/*  BP_SetDataPointer
 *
 *  Sets the data pointer of a particular partition.  Data pointer stores the logical
 *  byte address where the next read or write will occur.
 *
 *  ENTRY
 *      hPartition - handle to the partition
 *      dwAddress - Address to set data pointer to
 *
 *  EXIT
 *      TRUE on success
 */
BOOL BP_SetDataPointer(HANDLE hPartition, DWORD dwAddress)
{
    if (hPartition == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    RETAILMSG(_INF_,(TEXT("[BP:INF] BP_SetDataPointer at 0x%x\r\n"), dwAddress));

    PPARTSTATE pPartState = (PPARTSTATE) hPartition;

    if (dwAddress >= pPartState->pPartEntry->Part_TotalSectors * SECTOR_SIZE)
    {
        return FALSE;
    }

    pPartState->dwDataPointer = dwAddress;
    return TRUE;
}


/*  BP_GetPartitionInfo
 *
 *  Get the partition entry for an open partition.
 *
 *  ENTRY
 *      hPartition - handle to the partition
 *
 *  EXIT
 *      The partition entry
 */
PPARTENTRY BP_GetPartitionInfo(HANDLE hPartition)
{
    if (!hPartition)
    {
        return NULL;
    }

    return ((PPARTSTATE)hPartition)->pPartEntry;
}


/*  BP_Init
 *
 *  Sets up locations for various objects in memory provided by caller
 *
 *  ENTRY
 *      pMemory - pointer to memory for storing objects
 *      dwSize - size of the memory
 *      lpActiveReg - used by FMD_Init. NULL if not needed.
 *      pRegIn - used by FMD_Init. NULL if not needed.
 *      pRegOut - used by FMD_Init. NULL if not needed.
 *
 *  EXIT
 *      TRUE on success
 */
BOOL BP_Init(LPBYTE pMemory, DWORD dwSize, LPCTSTR lpActiveReg, PPCI_REG_INFO pRegIn, PPCI_REG_INFO pRegOut)
{
    DWORD dwBufferSize;

    RETAILMSG(_INF_,(TEXT("[BP:INF] BP_Init++\r\n")));

    if (!pMemory) {
        RETAILMSG(_ERR_,(TEXT("[BP:ERR] BP_Init Fails No memory fails!!!\r\n")));
        return FALSE;
    }
    else {
        RETAILMSG(_INF_,(TEXT("[BP:INF] BP_Init: pMemory=0x%x\r\n"), pMemory));
    }

    // Check to make sure size is enough for one sector, one block, and sectorinfo buffer for one block
    g_FlashInfo.flashType = NAND;
    g_FlashInfo.dwNumBlocks = SECTOROFIMAGE/256;        // to sync bootpart
    g_FlashInfo.wSectorsPerBlock = 256;
    g_FlashInfo.wDataBytesPerSector = 512;
    g_FlashInfo.dwBytesPerBlock = (g_FlashInfo.wSectorsPerBlock)*(g_FlashInfo.wDataBytesPerSector);

    g_dwDataBytesPerBlock = g_FlashInfo.wDataBytesPerSector * g_FlashInfo.wSectorsPerBlock;
    dwBufferSize = g_FlashInfo.wDataBytesPerSector + g_dwDataBytesPerBlock +
        (g_FlashInfo.wSectorsPerBlock * sizeof(SectorInfo));
    if (dwSize < dwBufferSize)
    {
        RETAILMSG(_ERR_,(TEXT("[BP:ERR] BP_Init Fails buffer size = %x < required = %x!!!\r\n"), dwSize, dwBufferSize));
        return FALSE;
    }

    for (int i = 0; i < NUM_PARTS; i++)
    {
        g_partStateTable[i].pPartEntry= NULL;
        g_partStateTable[i].dwDataPointer = 0;
    }

    g_pbMBRSector = pMemory;  //size = g_FlashInfo.wDataBytesPerSector;
    g_pbBlock = pMemory + g_FlashInfo.wDataBytesPerSector;  //size = g_dwDataBytesPerBlock;
    g_pSectorInfoBuf = (PSectorInfo)(g_pbBlock + g_dwDataBytesPerBlock);  //size = g_FlashInfo.wSectorsPerBlock * sizeof(SectorInfo);
    g_dwLastLogSector = 0;

    RETAILMSG(_INF_,(TEXT("[BP:INF] BP_Init: g_pbMBRSector=0x%x\r\n"), g_pbMBRSector));
    RETAILMSG(_INF_,(TEXT("[BP:INF] BP_Init: g_pbBlock=0x%x\r\n"), g_pbBlock));
    RETAILMSG(_INF_,(TEXT("[BP:INF] BP_Init: g_pSectorInfoBuf=0x%x\r\n"), g_pSectorInfoBuf));

    RETAILMSG(_INF_,(TEXT("[BP:INF] BP_Init--\r\n")));

    return TRUE;
}


/*  BP_LowLevelFormat
 *
 *  Called when preparing flash for a multiple-BIN download.
 *  Erases, verifies, and writes logical sector numbers in the range to be written.
 *
 *  ENTRY
 *      dwStartBlock - starting physical block for format
 *      dwNumBlocks - number of physical blocks to format
 *      dwFlags - Flags used in formatting.
 *
 *  EXIT
 *      TRUE returned on success and FALSE on failure.
 */
BOOL BP_LowLevelFormat(DWORD dwStartBlock, DWORD dwNumBlocks, DWORD dwFlags)
{
    RETAILMSG(_INF_,(TEXT("[BP:INF] BP_LowLevelFormat: SD/MMC card does not need to be erased before fusing IMAGE\r\n")));
    // MBR goes in the first sector of the starting block.  This will be logical sector 0.
    g_dwMBRSectorNum = MBRSTARTSECTOR;

    // Create an MBR.
    CreateMBR();

    RETAILMSG (_INF_, (TEXT("[BP:INF] MBR is written successfully.\r\n")));
    return(TRUE);
}

