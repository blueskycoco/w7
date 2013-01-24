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
/*
***************************************************************************//*!
*
* \file        ChunkAlloc.h
* \brief    Chunk allocator header
*            USAGE: Create a new chunk allocator and do New() and Free() on that
*             to get chunk handles. Aligned virtual and physical address could
*              be queried from ChunkHandle
*
*//*---------------------------------------------------------------------------
* NOTES:
*/


#ifndef __CHUNK_ALLOC_H__
#define __CHUNK_ALLOC_H__

/*
*******************************************************************************
* Includes
*******************************************************************************
*/
#include <list>

/*
*******************************************************************************
* Macro definitions and enumerations
*******************************************************************************
*/

/*
*******************************************************************************
* Type, Structure & Class Definitions
*******************************************************************************
*/
/*------------------------------------------------------------------------*//*!
* Handle to memory chunk
*//*-------------------------------------------------------------------------*/
class ChunkHandle
{
public:
    
#if (_WIN32_WCE>=600)     
    unsigned int vaddrCP;           //!< For other process
#endif    
    // Interface functions
    virtual void* GetPhyAddr() = 0;            //!< Get physical address (aligned)
    virtual void* GetVirtAddr() = 0;        //!< Get virtual address (aligned)
    virtual unsigned int GetSize() = 0;        //!< Get size
    virtual void Print() = 0;                //!< Print info

    virtual ~ChunkHandle() {}; //to silence compiler
};

//! Pointer to ChunkHandle
typedef ChunkHandle* PtrChunkH;


/*------------------------------------------------------------------------*//*!
* Chunk list node
*//*-------------------------------------------------------------------------*/
class Node
{
public:
    unsigned int start;                        //!< Starting offset
    unsigned int end;                        //!< Ending offset

    // Interface functions
    Node(unsigned int start, unsigned int size);        //!< Constructor
    inline unsigned int GetSize();                        //!< Get size
    void Print();                                        //!< Print info
};

/*------------------------------------------------------------------------*//*!
* Chunk allocator class
* Usage: Create a ChunkAlloc with virtual base, physical base, memory size
*  and alignment and do New() and Free()
*//*-------------------------------------------------------------------------*/

class ChunkAlloc
{
private:
    unsigned int vbase;                        //!< Virtual base
    unsigned int pbase;                        //!< Physical base
    unsigned int alignment;                    //!< Alignment (in bytes)

    std::list<Node> fl;                        //!< List containing free memory nodes
    std::list<Node> ul;                        //!< List containing used memory nodes

    // Internal functions
    void AddNode(std::list<Node>& l, Node& n);            //!< Add node to list
    void MergeNode(std::list<Node>& l, Node& n);        //!< Add node to list with merging to adjacent nodes

public:
    // Interface functions
    ChunkAlloc(void* vbase, void* pbase, unsigned int size, unsigned int alignment = 1);
    ~ChunkAlloc();    
                                            //!< Constructor (virtual base, physical base, size, alignment in bytes)
    ChunkHandle* New(unsigned int size);    //!< New chunk of atleast size bytes
    void Free(ChunkHandle* ch);                //!< Free chunk
    void Print();                            //!< Print current status of lists (for debugging)
    
    void GarbageCollect();
};

/*
*******************************************************************************
* Global Variable Declarations
*******************************************************************************
*/
extern ChunkAlloc *pCA;
/*
*******************************************************************************
* Function Declarations
*******************************************************************************
*/

#endif /*__CHUNK_ALLOC_H__*/

