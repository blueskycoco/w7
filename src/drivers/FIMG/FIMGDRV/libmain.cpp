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

#include <COMMON/fimg_debug.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "register.h"
#include "macros.h"
#include "fgl.h"
#include "platform.h"

#include <windows.h>

#include <bsp.h>
#include "DrvLib.h"

#include "bsp_cfg.h"
#include "fimgdrv.h"

#include <pmplatform.h>


#ifdef PLATFORM_S3C6410

#include "s3c6410_base_regs.h"


#define FIMG_PHY_BASE           S3C6410_BASE_REG_PA_FIMG_3DSE
#define FIMG_PHY_SIZE           0x90000

#else

#ifdef PLATFORM_S5PC100

#include "S5PC100_base_regs.h"
#include "S5PC100_dma_controller.h"

#define DMAC_CODE_BASE    IMAGE_DMA_CODE_UA_START
#define DMAC_CODE_PHY_BASE    IMAGE_DMA_CODE_PA_START
#define DMAC_PDMA_CODE_SIZE_PER_CHANNEL    0xa0 //160Byte
#define DMAC_MDMA_CODE_SIZE_PER_CHANNEL    0x400 //1024Byte

#define FIMG_PHY_BASE       S5PC100_BASE_REG_PA_FIMG_3DSE
#define FIMG_PHY_SIZE       0x90000

#else

#error Unknown hardware platform

#endif // PLATFORM_S5PC100

#endif // PLATFORM_S3C6410




extern int                  g_InitDone;
extern HANDLE               g_hPwrControl;

#define ALLOCATESIZE        0x200000
#define ALLOCATENUM         8

/* Type, Structure & Class Definitions */
struct s3c_3d_mem_alloc
{
    int          size;
    unsigned int vir_addr;
    unsigned int phy_addr;
};


typedef struct ALLOCMEM_POOL {
    void*                   phyAddr;
    void*                   virAddr;
    HANDLE                  handle;
    BOOL                    used;
    void*                   openHandle;
    int                     size;
    struct ALLOCMEM_POOL   *next;
} ALLOCMEM_POOL;

ALLOCMEM_POOL       *pHeadOfMemoryPool = NULL;
ALLOCMEM_POOL       *pDepthBuffer = NULL;

int g_blkSize = 0;
int g_blkNum = 0;
int g_depthBufferSize = 0;

// Pool memory
AddressBase gPoolMem = {
    (void*) 0, NULL
};

// FIMG registers
AddressBase gFimgBase = {
    (void*)FIMG_PHY_BASE, NULL
};

static int fimg_initcount = 0;

#ifdef PLATFORM_S5PC100
static volatile S5PC100_DMAC_REG    *g_pPDMAC0Reg = NULL;
static volatile S5PC100_DMAC_REG    *g_pPDMAC1Reg = NULL;
static volatile S5PC100_DMAC_REG    *g_pMDMACReg = NULL;
static volatile S5PC100_SYSCON_CLK_REG    *g_pSysConReg = NULL;
#endif

BOOL ReadMemoryPoolFromRegistry(int *pBlkNum, int *pBlkSize, int *pDepthBufferSize);
void FimgPowerOn(BOOL isOn);
BOOL AllocateMemoryList();
//void FreeMemoryList();


/***********************************************************************************
 Function Name          : InitFimg
 Inputs                             : None
 Outputs                        : None
 Returns                        : None
 Description                : This function initializes the graphics hardware.
************************************************************************************/
extern "C" void InitFimg ()
{
    unsigned int GPU_version = 0;
    
        fglGetVersion(&GPU_version);
        FIMG_PRINT((FIMG_DBG_MESSAGE, "*** FIMG VERSION : 0x%x ***", GPU_version));
    
        fglSoftReset();

    WRITEREG(FGPF_STENCIL_DEPTH_MASK, 0);
    WRITEREG(FGRA_PIXEL_SAMPOS ,FGL_SAMPLE_CENTER);
}


/***********************************************************************************
 Function Name          : InitDevice_wince
 Inputs                             : primary and secondary surface pointers
 Outputs                        : None
 Returns                        : opengl es 2.0 contexts
 Description                : This is the starting function of OpenGL ES 2.0 which maps the FIMG registers,
                        set the framebuffer structure parameters, allocates a pool of memory and 
                        creates a gl context.
************************************************************************************/
extern "C" BOOL GLES2Initdriver ()
{   

    DWORD dwPhys = 0;
#if (_WIN32_WCE >= 600)
    PHYSICAL_ADDRESS    ioPhysicalBase = {0,0};
    
#ifdef PLATFORM_S5PC100 
    ioPhysicalBase.LowPart = S5PC100_BASE_REG_PA_PDMA0;
    g_pPDMAC0Reg = (S5PC100_DMAC_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S5PC100_DMAC_REG), FALSE);
    
    ioPhysicalBase.LowPart = S5PC100_BASE_REG_PA_PDMA1;
    g_pPDMAC1Reg = (S5PC100_DMAC_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S5PC100_DMAC_REG), FALSE);
    
    ioPhysicalBase.LowPart = S5PC100_BASE_REG_PA_MDMA;
    g_pMDMACReg = (S5PC100_DMAC_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S5PC100_DMAC_REG), FALSE);
    
    ioPhysicalBase.LowPart = S5PC100_BASE_REG_PA_SYSCON_CLK;
    g_pSysConReg = (S5PC100_SYSCON_CLK_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S5PC100_SYSCON_CLK_REG), FALSE);       
#endif    
    
#endif  
        
    FIMG_PRINT((FIMG_DBG_MESSAGE, "InitDriver"));
    //-------------------------------------------------------------------------
    // Map FIMG SFRs
    //-------------------------------------------------------------------------
    gFimgBase.vaddr = VirtualAlloc(NULL, FIMG_PHY_SIZE, MEM_RESERVE, PAGE_NOCACHE|PAGE_READWRITE);
    if (!gFimgBase.vaddr)
    {
        FIMG_PRINT((FIMG_DBG_ERROR, "VirtualAlloc fail, GetLastError = 0x%x", GetLastError()));
    }

    if(!VirtualCopy (gFimgBase.vaddr, (LPVOID)((UINT32)FIMG_PHY_BASE>>8), FIMG_PHY_SIZE, PAGE_NOCACHE|PAGE_READWRITE|PAGE_PHYSICAL))
    {
        FIMG_PRINT((FIMG_DBG_ERROR, "VirtualCopy fail, GetLastError = 0x%x", GetLastError()));
        return false;    
    }       
    
    if(!ReadMemoryPoolFromRegistry(&g_blkNum, &g_blkSize, &g_depthBufferSize))
    {
        g_blkSize = ALLOCATESIZE;
        g_blkNum = ALLOCATENUM;
    }  
    
    
    AllocateMemoryList();
    
    return true;
}

extern "C" BOOL GLES2Opendriver ()
{   
    
    EnterCriticalSection(&gles20_open_mutex);
    if(fimg_initcount == 0)
    {
        
        //-------------------------------------------------------------------------
        // Graphics hardware (FIMG) initialization
        //-------------------------------------------------------------------------
    
        FimgPowerOn(TRUE);
        
        InitFimg();
    
        g_InitDone = TRUE;
    }

        
    fimg_initcount++;
    LeaveCriticalSection(&gles20_open_mutex);

    return TRUE;
}


/***********************************************************************************
 Function Name          : CloseDevice
 Inputs                 : None
 Outputs                : None
 Returns                : None
 Description            : This function unmaps the FIMG registers, pool memory.
************************************************************************************/
extern "C" BOOL GLES2DeInitdriver(void)
{   
    // Unmap any memory areas that we may have mapped.
    if (gFimgBase.vaddr)
    {
        VirtualFree( gFimgBase.vaddr, FIMG_PHY_SIZE, MEM_DECOMMIT );
        gFimgBase.vaddr = NULL;
    }
    return true;
}


extern "C" BOOL GLES2Closedriver(void)
{   
    EnterCriticalSection(&gles20_open_mutex);
    fimg_initcount--;

    if(fimg_initcount == 0)
    {   
        
        g_InitDone = FALSE;
        
        FimgPowerOn(FALSE);
    }
    LeaveCriticalSection(&gles20_open_mutex);
    
    return TRUE;
}

extern "C"  void GetPhysicalAddress(BufferAddress* bufAddr)
{
    DWORD dwPhys = 0;       
    //BufferAddress *bufAddr = (BufferAddress *) MapCallerPtr( (LPVOID)unMappedbufAddr, sizeof(BufferAddress));          
    if(TRUE == LockPages((LPVOID)bufAddr->vaddrCP, 1, &dwPhys, LOCKFLAG_QUERY_ONLY))
    {       
        //APR 06Mar07 
        //LockPages was observed to return physical addresses aligned at 4KB.
        //This hack was put here to capture the offset from 4KB aligned physical address if any
        dwPhys |= (DWORD)(((DWORD)bufAddr->vaddrCP) & (0xFFF));
    }
    else
    {
        FIMG_PRINT((FIMG_DBG_ERROR, "LockPage failed.."));
    }   
    bufAddr->paddr = (void*)dwPhys;
    bufAddr->vaddr = bufAddr->vaddrCP;
    
}



#ifdef PLATFORM_S3C6410

void FimgPowerOn(BOOL isOn)
{
    S3C6410_SYSCON_REG *pSysConReg;

        
#if (_WIN32_WCE>=600)
    PHYSICAL_ADDRESS    ioPhysicalBase = {0,0};    
#else
    DWORD dwIPIndex = PWR_IP_3D;
    DWORD dwBytes;
#endif  

    // Alloc and Map System Controller SFR
#if (_WIN32_WCE>=600)
    ioPhysicalBase.LowPart = S3C6410_BASE_REG_PA_SYSCON;    
    pSysConReg = (S3C6410_SYSCON_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C6410_SYSCON_REG), FALSE);
#else
    pSysConReg = (S3C6410_SYSCON_REG *)DrvLib_MapIoSpace(S3C6410_BASE_REG_PA_SYSCON, sizeof(S3C6410_SYSCON_REG), FALSE);
#endif  
    if (pSysConReg == NULL)
    {
        FIMG_PRINT((FIMG_DBG_ERROR, "FimgPowerOn() : pSysConReg DrvLib_MapIoSpace() Failed"));
        return;
    }
        
    if(isOn)
    {
        FIMG_PRINT((FIMG_DBG_MESSAGE, " Power On NORMAL_CFG=0x%x",  pSysConReg->NORMAL_CFG));
#if (_WIN32_WCE>=600)
        pSysConReg->NORMAL_CFG |= (1<<10);      // DOMAIN_G on
#else
        if ( !DeviceIoControl(g_hPwrControl, IOCTL_PWRCON_SET_POWER_ON, &dwIPIndex, sizeof(DWORD), NULL, 0, &dwBytes, NULL) )
        {
            FIMG_PRINT((FIMG_DBG_ERROR, "DD::G3D IOCTL_PWRCON_SET_POWER_ON Failed"));
        }           
#endif
        pSysConReg->HCLK_GATE |= (1<<31);       // Clock On
    }
    else
    {
        FIMG_PRINT((FIMG_DBG_MESSAGE, " Power Off NORMAL_CFG=0x%x",  pSysConReg->NORMAL_CFG));
#if (_WIN32_WCE>=600)
        pSysConReg->NORMAL_CFG &= ~(1<<10);     // DOMAIN_G off
#else
        if ( !DeviceIoControl(g_hPwrControl, IOCTL_PWRCON_SET_POWER_OFF, &dwIPIndex, sizeof(DWORD), NULL, 0, &dwBytes, NULL) )
        {
            FIMG_PRINT((FIMG_DBG_ERROR, "DD::G3D IOCTL_PWRCON_SET_POWER_OFF Failed"));
        }       
#endif
        pSysConReg->HCLK_GATE &= ~(1<<31);       // Clock Off
    }
    
#if (_WIN32_WCE>=600)
    MmUnmapIoSpace((PVOID)pSysConReg, sizeof(S3C6410_SYSCON_REG));  
#else   
    DrvLib_UnmapIoSpace((PVOID)pSysConReg);
#endif  
}

#elif PLATFORM_S5PC100

void FimgPowerOn(BOOL isOn)
{
    S5PC100_SYSCON_PM_REG *pSysConReg;
    S5PC100_SYSCON_CLK_REG *pSysConReg_CLK;
    DWORD dwIPIndex = PWR_IP_G3D;
    DWORD dwBytes;
        
#if (_WIN32_WCE>=600)
    PHYSICAL_ADDRESS    ioPhysicalBase = {0,0};    
#endif  

    // Alloc and Map System Controller SFR
#if (_WIN32_WCE>=600)
    ioPhysicalBase.LowPart = S5PC100_BASE_REG_PA_SYSCON_PM; 
    pSysConReg = (S5PC100_SYSCON_PM_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S5PC100_SYSCON_PM_REG), FALSE);
    
    ioPhysicalBase.LowPart = S5PC100_BASE_REG_PA_SYSCON_CLK;    
    pSysConReg_CLK = (S5PC100_SYSCON_CLK_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S5PC100_SYSCON_CLK_REG), FALSE); 
#else
    pSysConReg = (S5PC100_SYSCON_PM_REG *)DrvLib_MapIoSpace(S5PC100_BASE_REG_PA_SYSCON_PM, sizeof(S5PC100_SYSCON_PM_REG), FALSE);
    pSysConReg_CLK = (S5PC100_SYSCON_CLK_REG *)DrvLib_MapIoSpace(S5PC100_BASE_REG_PA_SYSCON_CLK, sizeof(S5PC100_SYSCON_CLK_REG), FALSE);
#endif  
    if (pSysConReg == NULL)
    {
        FIMG_PRINT((FIMG_DBG_ERROR, "FimgPowerOn() : pSysConReg DrvLib_MapIoSpace() Failed"));
        return;
    }
        
    if(isOn)
    {
        //pSysConReg->NORMAL_CFG |= (1<<10);        // DOMAIN_G on
        if ( !DeviceIoControl(g_hPwrControl, IOCTL_PWRCON_SET_POWER_ON, &dwIPIndex, sizeof(DWORD), NULL, 0, &dwBytes, NULL) )
        {
            FIMG_PRINT((FIMG_DBG_ERROR, "DD::G3D IOCTL_PWRCON_SET_POWER_ON Failed"));
        }           
        pSysConReg_CLK->CLK_GATE_D1_1 |= (1<<8);        // DOMAIN_G on
    }
    else
    {
        //pSysConReg->NORMAL_CFG &= ~(1<<10);       // DOMAIN_G off
        if ( !DeviceIoControl(g_hPwrControl, IOCTL_PWRCON_SET_POWER_OFF, &dwIPIndex, sizeof(DWORD), NULL, 0, &dwBytes, NULL) )
        {
            FIMG_PRINT((FIMG_DBG_ERROR, "DD::G3D IOCTL_PWRCON_SET_POWER_OFF Failed"));
        }       
        pSysConReg_CLK->CLK_GATE_D1_1 &= ~(1<<8);       // DOMAIN_G off
    }
    
#if (_WIN32_WCE>=600)
    MmUnmapIoSpace((PVOID)pSysConReg, sizeof(S5PC100_BASE_REG_PA_SYSCON_PM));   
    MmUnmapIoSpace((PVOID)pSysConReg_CLK, sizeof(S5PC100_BASE_REG_PA_SYSCON_CLK));  
#else   
    DrvLib_UnmapIoSpace((PVOID)pSysConReg);
    DrvLib_UnmapIoSpace((PVOID)pSysConReg_CLK);
#endif  
    
}
#endif

extern "C"  void GetSFRAddress(int* sfrAddr)
{

#if (_WIN32_WCE >= 600)
    
    *sfrAddr = (int)VirtualAllocCopyEx(
                                (HANDLE)GetCurrentProcessId(), 
                                (HANDLE)GetDirectCallerProcessId(), 
                                gFimgBase.vaddr, 
                                FIMG_PHY_SIZE,
                                PAGE_READWRITE | PAGE_NOCACHE);
    
    FIMG_PRINT((FIMG_DBG_MESSAGE, "sfr phy address = 0x%x", gFimgBase.vaddr));
    FIMG_PRINT((FIMG_DBG_MESSAGE, "sfr address = 0x%x", *sfrAddr));

#else
    
    *sfrAddr = (int)gFimgBase.paddr;//(int)MapPtrToProcess(gFimgBase.vaddr, (HANDLE) GetCallerProcess());
    
#endif  
}

extern "C"  void  FreeSFRAddress(int sfrAddr)
{
#if (_WIN32_WCE >= 600)
    VirtualFreeEx((HANDLE)GetDirectCallerProcessId(), (LPVOID)sfrAddr, 0,  MEM_RELEASE);
#else
    //UnMapPtr((LPVOID)sfrAddr);        
#endif  
}

extern "C"  void GetDMASFRAddress(int** sfrAddr)
{
#ifdef PLATFORM_S5PC100
#if (_WIN32_WCE >= 600)

    
    sfrAddr[0] = (int *)VirtualAllocCopyEx(
                                (HANDLE)GetCurrentProcessId(), 
                                (HANDLE)GetDirectCallerProcessId(), 
                                (LPVOID)g_pPDMAC0Reg, 
                                sizeof(S5PC100_DMAC_REG),
                                PAGE_READWRITE | PAGE_NOCACHE);
                                
    sfrAddr[1] = (int *)VirtualAllocCopyEx(
                                (HANDLE)GetCurrentProcessId(), 
                                (HANDLE)GetDirectCallerProcessId(), 
                                (LPVOID)g_pPDMAC1Reg, 
                                sizeof(S5PC100_DMAC_REG),
                                PAGE_READWRITE | PAGE_NOCACHE);
                                
    sfrAddr[2] = (int *)VirtualAllocCopyEx(
                                (HANDLE)GetCurrentProcessId(), 
                                (HANDLE)GetDirectCallerProcessId(), 
                                (LPVOID)g_pMDMACReg, 
                                sizeof(S5PC100_DMAC_REG),
                                PAGE_READWRITE | PAGE_NOCACHE);
                                
    sfrAddr[3] = (int *)VirtualAllocCopyEx(
                                (HANDLE)GetCurrentProcessId(), 
                                (HANDLE)GetDirectCallerProcessId(), 
                                (LPVOID)g_pSysConReg, 
                                sizeof(S5PC100_SYSCON_CLK_REG),
                                PAGE_READWRITE | PAGE_NOCACHE);                                                                                                
    

#else
    
    sfrAddr[0] = (int*)S5PC100_BASE_REG_PA_PDMA0;
    sfrAddr[1] = (int*)S5PC100_BASE_REG_PA_PDMA1;
    sfrAddr[2] = (int*)S5PC100_BASE_REG_PA_MDMA;
    sfrAddr[3] = (int*)S5PC100_BASE_REG_PA_SYSCON_CLK;
#endif
#endif  
}

extern "C"  void  FreeDMASFRAddress(int** sfrAddr)
{
#ifdef PLATFORM_S5PC100
#if (_WIN32_WCE >= 600)
    VirtualFreeEx((HANDLE)GetDirectCallerProcessId(), (LPVOID)sfrAddr[0], 0,  MEM_RELEASE);
    VirtualFreeEx((HANDLE)GetDirectCallerProcessId(), (LPVOID)sfrAddr[1], 0,  MEM_RELEASE);
    VirtualFreeEx((HANDLE)GetDirectCallerProcessId(), (LPVOID)sfrAddr[2], 0,  MEM_RELEASE);
    VirtualFreeEx((HANDLE)GetDirectCallerProcessId(), (LPVOID)sfrAddr[3], 0,  MEM_RELEASE);
#else
    //UnMapPtr((LPVOID)sfrAddr);        
#endif  
#endif
}
void* GetEmptyMemBlock()
{
    ALLOCMEM_POOL *pTemp = pHeadOfMemoryPool;
        
    EnterCriticalSection(&gles20_chunkalloc_mutex);     
    while(pTemp != NULL)
    {
        if(pTemp->used == FALSE) 
        {
            pTemp->used = TRUE;
            break;
        }
        pTemp = pTemp->next;
    }
    
    LeaveCriticalSection(&gles20_chunkalloc_mutex);     
    return pTemp;    
}

extern "C" void DoAllocPhysMem(DWORD hOpenContext, void* bufAddr, int size)
{
    FIMG_CONTEXT* openHandle = (FIMG_CONTEXT*)hOpenContext;
    ALLOCMEM_ITEM *pHead = openHandle->allocated_list;    
    ALLOCMEM_ITEM *pItem, *pPrev = NULL;
    s3c_3d_mem_alloc*   pOutputBuf = (s3c_3d_mem_alloc*)bufAddr;

    ALLOCMEM_POOL *pTemp = NULL;
    int i;
        
        
    for(i=0;i<10;i++)
    {   
        FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] DoAllocPhysMem %dth try", hOpenContext, (i+1)));
        pTemp = (ALLOCMEM_POOL *)GetEmptyMemBlock();        
        if(pTemp != NULL) break;
        Sleep(100);
    }
        
    if(pTemp == NULL)
    {
        FIMG_PRINT((FIMG_DBG_WARNING, "[0x%08x] Buffer Allocator unable to allocate %d bytes", hOpenContext, size));
        pTemp = pHeadOfMemoryPool;
        while(pTemp != NULL)
        {
            FIMG_PRINT((FIMG_DBG_WARNING, "[0x%08x] status 0x%x 0x%x 0x%x by 0x%08x", hOpenContext, pTemp->virAddr, pTemp->phyAddr, pTemp->used, pTemp->openHandle));
            pTemp = pTemp->next;
        }        
    }
    else
    {
        
        pOutputBuf->size = pTemp->size;    
        pOutputBuf->phy_addr = (unsigned int)pTemp->phyAddr;
       
#if (_WIN32_WCE >= 600)      
        void*   pCPAddr;
        pCPAddr = VirtualAllocCopyEx((HANDLE)GetCurrentProcessId(),
                            (HANDLE)GetDirectCallerProcessId(),
                            (LPVOID)pTemp->virAddr,
                            pTemp->size,
                            PAGE_READWRITE | PAGE_NOCACHE );   
        pOutputBuf->vir_addr = (unsigned int)pCPAddr;                                              
#else
        pOutputBuf->vir_addr = (unsigned int)pTemp->virAddr;
#endif

        for(pItem=pHead;pItem != NULL; pItem=pItem->next) 
        {
            pPrev = pItem;
        }
        
            
        pItem = (ALLOCMEM_ITEM*)malloc(sizeof(ALLOCMEM_ITEM));
        pItem->phyAddr = pTemp->phyAddr;
        pItem->virAddr = pTemp->virAddr;
        pItem->virAddrCP = (void*)pOutputBuf->vir_addr;
        pItem->memPool = (void*)pTemp;
        pTemp->used = TRUE;
        pTemp->openHandle = (void*)hOpenContext;
        pItem->next = 0;
        
        if(pPrev != NULL) pPrev->next = pItem;
        else openHandle->allocated_list = pItem;
        
        FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] DoAllocPhysMem 0x%x 0x%x 0x%x %d", hOpenContext, pOutputBuf->phy_addr, pOutputBuf->vir_addr, pTemp->virAddr, pOutputBuf->size));
        FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] After Alloc, Block list for this context", hOpenContext));
        FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] ========================================", hOpenContext));

        for(pItem=openHandle->allocated_list;pItem != NULL; pItem=pItem->next) 
        {
            FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] 0x%x 0x%x 0x%x is Allocated", hOpenContext, pItem->phyAddr, pItem->virAddrCP, pItem->virAddr));
        }    

        FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] ========================================", hOpenContext));  
        
    }        
    
}

extern "C" void DoFreePhysMem(DWORD hOpenContext, void* bufAddr)
{
    FIMG_CONTEXT* openHandle = (FIMG_CONTEXT*)hOpenContext;
    ALLOCMEM_ITEM* pHead = openHandle->allocated_list;        
    ALLOCMEM_ITEM *pItem, *pPrev = NULL;
    ALLOCMEM_POOL *pTemp;
    s3c_3d_mem_alloc*   pOutputBuf = (s3c_3d_mem_alloc*)bufAddr;    
    
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] DoFreePhysMem 0x%x 0x%x %d", hOpenContext, pOutputBuf->phy_addr, pOutputBuf->vir_addr, pOutputBuf->size));
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] Before Free, Block list for this context", hOpenContext));
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] ========================================", hOpenContext));
    for(pItem=pHead;pItem != NULL; pItem=pItem->next) 
    {
        FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] 0x%x 0x%x 0x%x is Allocated", hOpenContext, pItem->phyAddr, pItem->virAddrCP, pItem->virAddr));
    }    
    
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] ========================================", hOpenContext));
    
    for(pItem=pHead,pPrev=NULL;pItem != NULL; pItem=pItem->next) 
    {
        if((unsigned int)pItem->phyAddr == pOutputBuf->phy_addr) break;
        pPrev = pItem;
    }
    
    if(pItem != NULL)    
    {
        if(pPrev != NULL) pPrev->next = pItem->next;    
        else if(pItem->next != NULL) openHandle->allocated_list = pItem->next;
        else openHandle->allocated_list = NULL;
        
    #if (_WIN32_WCE >= 600)   
        if(pItem->virAddrCP != NULL)
            VirtualFreeEx((HANDLE)GetDirectCallerProcessId(), (LPVOID)pItem->virAddrCP, 0,  MEM_RELEASE);
    #endif        
    
        pTemp = (ALLOCMEM_POOL *)pItem->memPool;
        pTemp->used = FALSE;
        pTemp->openHandle = 0;
        
        
        free(pItem);
    }
        
    
    pItem = 0;
            
}


extern "C" void RequestDepthBuffer(DWORD hOpenContext, void* bufAddr)
{
    FIMG_CONTEXT* openHandle = (FIMG_CONTEXT*)hOpenContext;
    s3c_3d_mem_alloc*   pOutputBuf = (s3c_3d_mem_alloc*)bufAddr;

    if(pDepthBuffer != NULL)
    {            
        pOutputBuf->size = pDepthBuffer->size;    
        pOutputBuf->phy_addr = (unsigned int)pDepthBuffer->phyAddr;

       
#if (_WIN32_WCE >= 600)      
        void*   pCPAddr;
        pCPAddr = VirtualAllocCopyEx((HANDLE)GetCurrentProcessId(),
                            (HANDLE)GetDirectCallerProcessId(),
                            (LPVOID)pDepthBuffer->virAddr,
                            pDepthBuffer->size,
                            PAGE_READWRITE | PAGE_NOCACHE );   
        pOutputBuf->vir_addr = (unsigned int)pCPAddr;                                              
#else
        pOutputBuf->vir_addr = (unsigned int)pDepthBuffer->virAddr;
#endif

        FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] GetDepthBuffer 0x%x 0x%x 0x%x %d", hOpenContext, pOutputBuf->phy_addr, pOutputBuf->vir_addr, pDepthBuffer->virAddr, pOutputBuf->size));
    }
}

extern "C" void ReleaseDepthBuffer(DWORD hOpenContext, void* bufAddr)
{
    FIMG_CONTEXT* openHandle = (FIMG_CONTEXT*)hOpenContext;
    s3c_3d_mem_alloc*   pOutputBuf = (s3c_3d_mem_alloc*)bufAddr;      
    
#if (_WIN32_WCE >= 600)   
    if(pOutputBuf->vir_addr != NULL)
        VirtualFreeEx((HANDLE)GetDirectCallerProcessId(), (LPVOID)pOutputBuf->vir_addr, 0,  MEM_RELEASE);
#endif            
}

extern "C" void GetMemoryStatus(DWORD hOpenContext, int *outUsedMemory, int *outTotalMemory)
{
    ALLOCMEM_POOL *pTemp = pHeadOfMemoryPool;
    int usedMemory = 0, totalMemory = 0;            
    while(pTemp != NULL)
    {
        if(pTemp->used == TRUE) 
        {
            usedMemory += pTemp->size;
        }
        totalMemory += pTemp->size;
        
        pTemp = pTemp->next;
    }
     
    *outUsedMemory = usedMemory;
    *outTotalMemory = totalMemory;
}


extern "C"  void GetDMACODEAddress(UINT32 offset, int cacheEnable, BufferAddress* bufAddr)
{
#ifdef PLATFORM_S5PC100
    HANDLE hMapping;
    PVOID vaddr;
    DWORD physaddr = DMAC_CODE_PHY_BASE + offset*DMAC_MDMA_CODE_SIZE_PER_CHANNEL;
    
    hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, DMAC_MDMA_CODE_SIZE_PER_CHANNEL, NULL);
    if(hMapping == NULL)
    {
        FIMG_PRINT((FIMG_DBG_ERROR, "CreateFileMapping error"));
        return;
    }
    
    vaddr = (void *)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);    
    if (vaddr < 0)
    {
        FIMG_PRINT((FIMG_DBG_ERROR, "vaddr not mapped"));
        return;
    }
    
    FIMG_PRINT((FIMG_DBG_MESSAGE, "  vaddr = 0x%x, physaddr = 0x%x", vaddr, physaddr));
    if(!VirtualCopy (vaddr, (LPVOID)((UINT32)physaddr>>8), DMAC_MDMA_CODE_SIZE_PER_CHANNEL, PAGE_NOCACHE|PAGE_READWRITE|PAGE_PHYSICAL))
    {
        FIMG_PRINT((FIMG_DBG_ERROR, "VirtualCopy error"));
        return;    
    }       

#if (_WIN32_WCE >= 600)    //if(!VirtualCopy (pVaShared, virtaddr, szBlock, PAGE_NOCACHE|PAGE_READWRITE))
    bufAddr->vaddrCP = (PVOID)VirtualAllocCopyEx((HANDLE)GetCurrentProcessId(),
                        (HANDLE)GetDirectCallerProcessId(),
                        vaddr,
                        DMAC_MDMA_CODE_SIZE_PER_CHANNEL,
                        PAGE_READWRITE | PAGE_NOCACHE);
#else
    bufAddr->vaddrCP = vaddr;
#endif    
                        
    bufAddr->paddr = (PVOID)physaddr;
    bufAddr->vaddr = (DWORD*)malloc(sizeof(DWORD)*2);//(PVOID)hMapping;
    ((DWORD*)(bufAddr->vaddr))[0] = (DWORD)hMapping;
    ((DWORD*)(bufAddr->vaddr))[1] = (DWORD)vaddr;
#endif                       
}

extern "C"  void  FreeDMACODEAddress(BufferAddress* bufAddr)
{
#ifdef PLATFORM_S5PC100
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[FreeDMACODEAddress]  vaddr = 0x%x Handle = 0x%x", bufAddr->vaddrCP, bufAddr->vaddr));
    if(bufAddr->vaddrCP != NULL)
    {
#if (_WIN32_WCE >= 600)              
        VirtualFreeEx((HANDLE)GetDirectCallerProcessId(), (LPVOID)bufAddr->vaddrCP, 0,  MEM_RELEASE);
#endif        
    }
    
    if (bufAddr->vaddr)
    {
        if(((DWORD*)(bufAddr->vaddr))[1])
        {
            UnmapViewOfFile((PVOID)((DWORD*)(bufAddr->vaddr))[1]);
        }
        
        if(((DWORD*)(bufAddr->vaddr))[0])
    {
            CloseHandle((HANDLE)((DWORD*)(bufAddr->vaddr))[0]);
        }
        
        free((DWORD*)bufAddr->vaddr);
        bufAddr->vaddr = NULL;
    }
#endif
}



extern "C" void GarbageCollect(DWORD hOpenContext)
{
    FIMG_CONTEXT* openHandle = (FIMG_CONTEXT*)hOpenContext;
    ALLOCMEM_ITEM* pHead = openHandle->allocated_list;        
    ALLOCMEM_ITEM *pItem, *pNext;
    ALLOCMEM_POOL *pTemp;
    
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] Garbage Collect", hOpenContext));
        
    for(pItem=pHead;pItem != NULL; ) 
    {
        pNext = pItem->next;
        pTemp = (ALLOCMEM_POOL *)pItem->memPool;
        pTemp->used = FALSE;
        pTemp->openHandle = 0;
#if (_WIN32_WCE >= 600)      
        if(pItem->virAddrCP != NULL)
            VirtualFreeEx((HANDLE)GetDirectCallerProcessId(), (LPVOID)pItem->virAddrCP, 0,  MEM_RELEASE);
#endif

        FIMG_PRINT((FIMG_DBG_WARNING, "[0x%08x] Garbage Collect 0x%x 0x%x", hOpenContext, pTemp->phyAddr, pTemp->virAddr));

        free(pItem);
        pItem = 0;            
        
        pItem = pNext;
    }
}



BOOL AllocateMemoryList()
{
    void *virAddr, *phyAddr, *pSharedAddr;
    int i;
    ALLOCMEM_POOL *pTemp, *pPrev;
    HANDLE hMapping = NULL;
    BOOL retVal = TRUE; 
                
    MEMORYSTATUS mem_status;

    
    
    FIMG_PRINT((FIMG_DBG_MESSAGE, "Alloc Block Num = %d  Alloc Block Size = 0x%x", g_blkNum, g_blkSize));
    
    
    GlobalMemoryStatus(&mem_status);
    FIMG_PRINT((FIMG_DBG_MESSAGE, "alloc physmem = %d / %d", mem_status.dwAvailPhys, mem_status.dwTotalPhys ));
    FIMG_PRINT((FIMG_DBG_MESSAGE, "alloc virtmem = %d/%d", mem_status.dwAvailVirtual, mem_status.dwTotalVirtual ));
  
    if(g_depthBufferSize > 0)
    {
        FIMG_PRINT((FIMG_DBG_MESSAGE, "Only for Depthbuffer is allocated."));
        
        virAddr = AllocPhysMem(g_depthBufferSize, PAGE_READWRITE|PAGE_NOCACHE, 0, 0, (PULONG)&phyAddr);
        if(virAddr == NULL)
        {
            FIMG_PRINT((FIMG_DBG_ERROR, " depth buffer allocated is fail."));
            retVal = FALSE;
        }   
        else
        {
        
#if (_WIN32_WCE >= 600)   
            pSharedAddr = virAddr;
            {
#else
            hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, g_depthBufferSize, NULL);   
            pSharedAddr = (void *)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);     
        
            if(!VirtualCopy (pSharedAddr, (LPVOID)((UINT32)phyAddr>>8), g_depthBufferSize, PAGE_NOCACHE|PAGE_READWRITE|PAGE_PHYSICAL))
            {
                FIMG_PRINT((FIMG_DBG_ERROR, "depthbuffer alloccopy is fail"));
            }
            else
            {
                VirtualFree (virAddr, 0, MEM_RELEASE);    // this will release the VM    
#endif        
                pDepthBuffer = (ALLOCMEM_POOL*) malloc(sizeof(ALLOCMEM_POOL));
                pDepthBuffer->phyAddr = phyAddr;
                pDepthBuffer->virAddr = pSharedAddr;
                pDepthBuffer->handle = hMapping;
                pDepthBuffer->used = FALSE;
                pDepthBuffer->next = NULL;
                pDepthBuffer->openHandle = 0;
                pDepthBuffer->size = g_depthBufferSize;
                FIMG_PRINT((FIMG_DBG_MESSAGE, "depthbuffer is allocated on 0x%x 0x%x, size is %dBytes", pSharedAddr, phyAddr, g_depthBufferSize));
            }
        }   
    }
  
#if 0
  ///////////////////// Allocated from Camera.
  phyAddr = (void*)CAM_POOL_BASE;
  virAddr = (void*)VirtualAlloc(NULL, CAM_POOL_SIZE, MEM_RESERVE, PAGE_NOACCESS);
    VirtualCopy (virAddr, (LPVOID)((UINT32)phyAddr>>8), CAM_POOL_SIZE, PAGE_NOCACHE|PAGE_READWRITE|PAGE_PHYSICAL);

  while(((DWORD)phyAddr+ALLOCATESIZE) <= (CAM_POOL_BASE+CAM_POOL_SIZE))
  {
        pTemp = (ALLOCMEM_POOL*) malloc(sizeof(ALLOCMEM_POOL));
        pTemp->phyAddr = phyAddr;
        pTemp->virAddr = virAddr;
        pTemp->handle = NULL;
        pTemp->used = FALSE;
        pTemp->next = NULL;
        pTemp->openHandle = 0;
        pTemp->size = ALLOCATESIZE;
        
        if((DWORD)phyAddr == CAM_POOL_BASE)
        {
            pHeadOfMemoryPool = pTemp;
        }
        else
        {
            pPrev->next = pTemp;
        }
        pPrev = pTemp;
        
    phyAddr = (void*)((DWORD)phyAddr + ALLOCATESIZE);
    virAddr = (void*)((DWORD)virAddr + ALLOCATESIZE);       
  }
#endif  
  
  //////////////////// Allocated From AllocPhysMem.
    for(i=0;i<g_blkNum;i++)
    {
        virAddr = AllocPhysMem(g_blkSize, PAGE_READWRITE|PAGE_NOCACHE, 0, 0, (PULONG)&phyAddr);
        if(virAddr == NULL)
        {
            
            FIMG_PRINT((FIMG_DBG_ERROR, "%d th allocated is fail",i+1));
            retVal = FALSE;
            break;
        }   
        
#if (_WIN32_WCE >= 600)   
        pSharedAddr = virAddr;
#else
        hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, g_blkSize, NULL);   
        
        pSharedAddr = (void *)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);     
        
        if(!VirtualCopy (pSharedAddr, (LPVOID)((UINT32)phyAddr>>8), g_blkSize, PAGE_NOCACHE|PAGE_READWRITE|PAGE_PHYSICAL))
        {
            
            FIMG_PRINT((FIMG_DBG_ERROR, "%d th alloccopy is fail",i+1));
            
            break;
        }
        
        VirtualFree (virAddr, 0, MEM_RELEASE);    // this will release the VM    
#endif        
        pTemp = (ALLOCMEM_POOL*) malloc(sizeof(ALLOCMEM_POOL));
        pTemp->phyAddr = phyAddr;
        pTemp->virAddr = pSharedAddr;
        pTemp->handle = hMapping;
        pTemp->used = FALSE;
        pTemp->next = NULL;
        pTemp->openHandle = 0;
        pTemp->size = g_blkSize;
        
        if(i == 0)
        {
            pHeadOfMemoryPool = pTemp;
        }
        else
        {
            pPrev->next = pTemp;
        }
        
        pPrev = pTemp;
    }
    
    GlobalMemoryStatus(&mem_status);
    FIMG_PRINT((FIMG_DBG_MESSAGE, "alloc physmem = %d / %d", mem_status.dwAvailPhys, mem_status.dwTotalPhys ));
    FIMG_PRINT((FIMG_DBG_MESSAGE, "alloc virtmem = %d / %d", mem_status.dwAvailVirtual, mem_status.dwTotalVirtual));
    
    pTemp = pHeadOfMemoryPool;
    while(pTemp != NULL)
    {
        FIMG_PRINT((FIMG_DBG_VERBOSE, "Allocated Pool 0x%x 0x%x 0x%x", pTemp->virAddr, pTemp->phyAddr, pTemp->used));
        pTemp = pTemp->next;
    }
    
    
    return retVal;
 
}

extern "C" void FreeMemoryList(DWORD hOpenContext)
{
#if (_WIN32_WCE < 600)        
    HANDLE hMapping;
    ALLOCMEM_POOL *pTemp, *pNext;
#endif  
    
    MEMORYSTATUS mem_status;
  
    GlobalMemoryStatus(&mem_status);
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[FIMGDRV] Before Release physmem = %d / %d", mem_status.dwAvailPhys, mem_status.dwTotalPhys ));
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[FIMGDRV] Before Release virtmem = %d / %d", mem_status.dwAvailVirtual, mem_status.dwTotalVirtual ));
      
  
    
    if(pDepthBuffer != NULL)
    {
#if (_WIN32_WCE < 600)    
        hMapping = pDepthBuffer->handle;
    
        if (pDepthBuffer->virAddr != NULL)
        {
            UnmapViewOfFile((LPVOID)pDepthBuffer->virAddr);
        }

        if (hMapping != NULL)
        {
             CloseHandle(hMapping);
        }   
     
        free(pDepthBuffer);         
#endif        
        pDepthBuffer = NULL;
    }
    
    if(pHeadOfMemoryPool != NULL)
    {
#if (_WIN32_WCE < 600)  
        pTemp = pHeadOfMemoryPool;
        while(pTemp != NULL)
        {
            pNext = pTemp->next;
          
            hMapping = pTemp->handle;
    
            if (pTemp->virAddr != NULL)
            {
                UnmapViewOfFile((LPVOID)pTemp->virAddr);
            }

            if (hMapping != NULL)
            {
                 CloseHandle(hMapping);
            }   
     
            free(pTemp);
            pTemp = pNext;
        }         
#endif         
        pHeadOfMemoryPool = NULL;   
    }
    
    GlobalMemoryStatus(&mem_status);
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[FIMGDRV] Before Release physmem = %d / %d", mem_status.dwAvailPhys, mem_status.dwTotalPhys ));
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[FIMGDRV] Before Release virtmem = %d / %d", mem_status.dwAvailVirtual, mem_status.dwTotalVirtual ));

}

extern "C" BOOL ReAllocMemoryList(DWORD hOpenContext)
{
    FIMG_PRINT((FIMG_DBG_MESSAGE, "[0x%08x] Re Allocate Memroy List", hOpenContext));
    
    if(pHeadOfMemoryPool != NULL || pDepthBuffer != NULL)
    {
        FIMG_PRINT((FIMG_DBG_ERROR, "[0x%08x] ReAllocateList Error! Already allocated", hOpenContext));
        return FALSE;
    }
    
    if(!ReadMemoryPoolFromRegistry(&g_blkNum, &g_blkSize, &g_depthBufferSize))
    {
        g_blkSize = ALLOCATESIZE;
        g_blkNum = ALLOCATENUM;
    }
    
    if(!AllocateMemoryList())
    {
        FIMG_PRINT((FIMG_DBG_ERROR, "[0x%08x] ReAllocateList Error!", hOpenContext));
        FreeMemoryList(hOpenContext);
        return FALSE;
    }
    return TRUE;
}

BOOL ReadMemoryPoolFromRegistry(int *pBlkNum, int *pBlkSize, int *pDepthBufferSize)
{
    HKEY  hKey = 0;
    DWORD dwType  = 0;
    DWORD dwSize  = sizeof ( DWORD );
    DWORD dwValue = -1;

    if( ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"Drivers\\BuiltIn\\FIMG", 0, 0, &hKey ))
    {
        //mio
        return FALSE;
    }

    *pDepthBufferSize = 0;
    if( ERROR_SUCCESS == RegQueryValueEx( hKey, L"DepthBufSize", 0, &dwType, (BYTE *)&dwValue, &dwSize ) )
    {
        if(   ( REG_DWORD == dwType )
           && ( sizeof( DWORD ) == dwSize ))
        {
            *pDepthBufferSize = dwValue;
        }
    }

    if( ERROR_SUCCESS == RegQueryValueEx( hKey, L"AllocblkNum", 0, &dwType, (BYTE *)&dwValue, &dwSize ) )
    {
        if(   ( REG_DWORD == dwType )
           && ( sizeof( DWORD ) == dwSize ))
        {
            *pBlkNum = dwValue;
        }
        else return FALSE;
    }
    else return FALSE;
  
    
    if( ERROR_SUCCESS == RegQueryValueEx( hKey, L"AllocblkSize", 0, &dwType, (BYTE *)&dwValue, &dwSize ) )
    {
        if(   ( REG_DWORD == dwType )
           && ( sizeof( DWORD ) == dwSize ))
        {
            *pBlkSize = dwValue;
        }
        else return FALSE;
    }
    else return FALSE;
    
    return TRUE;
}
