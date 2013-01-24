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
* \file        ChunkAlloc.cpp
* \brief    Chunk allocator source
*            List allocator implementation with no internal fragmentation
*
*//*---------------------------------------------------------------------------
* NOTES:
*/


/*
*******************************************************************************
* Includes
*******************************************************************************
*/
#include <stdio.h>
#include "ChunkAlloc.h"
#include "platform.h"

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
* Chunk class
* This is the class which holds actual chunk data
*//*-------------------------------------------------------------------------*/
class Chunk : public ChunkHandle
{
private:
    unsigned int vbase;                //!< Virtual base
    unsigned int pbase;                //!< Physical base

    unsigned int start;                //!< Start offset
    unsigned int end;                //!< End offset
    unsigned int pad;                //!< Padding required from start

public:
    Chunk();
    Chunk(unsigned int start, unsigned int size, unsigned int pad,
        unsigned int vbase, unsigned int pbase);        //!< Constructor
    ~Chunk();                                           //!< Destructor
    inline void* GetPhyAddr();        //!< Get physical address (ChunkHandle interface)
    inline void* GetVirtAddr();        //!< Get virtual address (ChunkHandle interface)
    inline unsigned int GetSize();    //!< Get size (ChunkHandle interface)
    inline unsigned int GetStart();    //!< Get starting offset
    inline unsigned int GetEnd();    //!< Get end offset
    void Print();                    //!< Print info (ChunkHandle interface)

        
    unsigned int processID;         //!< add by JJG 08.10.08, for check memory leak when driver is closed.
    Chunk*  next;
    Chunk*  prev;
};

//Chunk   HeadChunk;
Chunk*  LastChunk;
//#define pHeadChunk (&HeadChunk)
Chunk*  pHeadChunk;

/*
*******************************************************************************
* Global Variables
*******************************************************************************
*/

/*
*******************************************************************************
* Local Function Declarations
*******************************************************************************
*/

/*
*******************************************************************************
* Function Definitions
*******************************************************************************
*/

//*****************************************************************************
// Chunk
//*****************************************************************************

Chunk::Chunk()
{
    next = NULL;
    prev = NULL;    
}

/*------------------------------------------------------------------------*//*!
* Chunk constructor
*
* \param [in] _start    Starting offset of chunk
* \param [in] _size        Size of chunk
* \param [in] _pad        Padding required from start
* \param [in] _vbase    Virtual address base
* \param [in] _pbase    Physical address base
*
*//*-------------------------------------------------------------------------*/
Chunk::Chunk(unsigned int _start, unsigned int _size, unsigned int _pad,
             unsigned int _vbase, unsigned int _pbase) :
  vbase(_vbase), pbase(_pbase), start(_start), pad(_pad)
{
    end = _start + _size - 1;
    
    processID = (unsigned int)GetOwnerProcess(); //add by JJG 08.10.08, for check memory leak when driver is closed.
    next = NULL;
    prev = NULL;
}

/*------------------------------------------------------------------------*//*!
* Get chunk's physical address
*//*-------------------------------------------------------------------------*/
inline void*
Chunk::GetPhyAddr()
{
    return (void*)(start + pad + pbase);
}

/*------------------------------------------------------------------------*//*!
* Get chunk's virtual address
*//*-------------------------------------------------------------------------*/
inline void*
Chunk::GetVirtAddr()
{
    return (void*)(start + pad + vbase);
}

/*------------------------------------------------------------------------*//*!
* Get chunk's size
*//*-------------------------------------------------------------------------*/
inline unsigned int
Chunk::GetSize()
{
    return end - start + 1;
}

/*------------------------------------------------------------------------*//*!
* Get starting offset
*//*-------------------------------------------------------------------------*/
inline unsigned int
Chunk::GetStart()
{
    return start;
}

/*------------------------------------------------------------------------*//*!
* Get end offset
*//*-------------------------------------------------------------------------*/
inline unsigned int
Chunk::GetEnd()
{
    return end;
}

/*------------------------------------------------------------------------*//*!
* Print chunk info
*//*-------------------------------------------------------------------------*/
void
Chunk::Print()
{
    //printf("%d\t%d\t%d\t%d\t%p\t%p\n", start, end, pad, GetSize(), GetVirtAddr(), GetPhyAddr());
}


/*------------------------------------------------------------------------*//*!
* chunk destructor
*//*-------------------------------------------------------------------------*/
Chunk::~Chunk()
{
}

//*****************************************************************************
// Node
//*****************************************************************************

/*------------------------------------------------------------------------*//*!
* Node constructor
*//*-------------------------------------------------------------------------*/
Node::Node(unsigned int _start, unsigned int _size) :
  start(_start)
{
    end = start + _size - 1;
}

/*------------------------------------------------------------------------*//*!
* Get size
*//*-------------------------------------------------------------------------*/
inline unsigned int
Node::GetSize()
{
    return (end - start + 1);
}

/*------------------------------------------------------------------------*//*!
* Print info
*//*-------------------------------------------------------------------------*/
void
Node::Print()
{
    //printf("%d\t%d\t%d\n", start, end, GetSize());
}

//*****************************************************************************
// ChunkAlloc
//*****************************************************************************

/*------------------------------------------------------------------------*//*!
* Chunk allocator constructor
*
* \param [in] _vbase        Virtual address base
* \param [in] _pbase        Physical address base
* \param [in] _size            Size of memory pool (total size)
* \param [in] _alignment    Alignment required for chunks

*//*-------------------------------------------------------------------------*/
ChunkAlloc::ChunkAlloc(void* _vbase, void* _pbase, unsigned int _size, unsigned int _alignment):
      vbase((unsigned int)_vbase), pbase((unsigned int)_pbase), alignment(_alignment)
{
    Node c(0, _size);
    AddNode(fl, c);
    
    pHeadChunk = new Chunk();
    memset(pHeadChunk, 0x0, sizeof(Chunk));
    LastChunk = pHeadChunk;
}

ChunkAlloc::~ChunkAlloc()
{
    delete pHeadChunk;
}

/*------------------------------------------------------------------------*//*!
* Add node to list
*//*-------------------------------------------------------------------------*/
void
ChunkAlloc::AddNode(std::list<Node>& l, Node& n)
{
    std::list<Node>::iterator i;

    for(i = l.begin(); i != l.end(); i++) {
        if(i->start >= n.end+1) {
            break;
        }
    }

    l.insert(i, n);
}

/*------------------------------------------------------------------------*//*!
* Add node to list merging with chunks adjacent to one being added
*//*-------------------------------------------------------------------------*/
void
ChunkAlloc::MergeNode(std::list<Node>& l, Node& n)
{
    std::list<Node>::iterator i;

    for(i = l.begin(); i != l.end(); i++) {
        if(i->start == n.end+1) {
            Node tmp = *i;
            l.erase(i);
            tmp.start = n.start;
            MergeNode(l, tmp);
            return;

        } else if(i->end+1 == n.start) {
            Node tmp = *i;
            l.erase(i);
            tmp.end = n.end;
            MergeNode(l, tmp);
            return;

        } else if(i->start > n.end+1) {
            break;
        }
    }

    l.insert(i, n);

    return;
}
/*------------------------------------------------------------------------*//*!
* Get new chunk of atleast size bytes
*//*-------------------------------------------------------------------------*/
ChunkHandle*
ChunkAlloc::New(unsigned int size)
{
    Plat::lock( (&gles20_chunkalloc_mutex),__FUNCTION__);
    
    unsigned int pad = 0;

    for(std::list<Node>::iterator i = fl.begin(); i != fl.end(); i++) {
        if((i->start % alignment) != 0)
            pad = alignment - (i->start % alignment);

        if(i->GetSize() >= size + pad) {
            Chunk* res = new Chunk(i->start, size + pad, pad, vbase, pbase);
            if(i->GetSize() > res->GetSize()) {
                Node n(res->GetStart() + res->GetSize(), i->GetSize() - res->GetSize());
                MergeNode(fl, n);
            }

            //RETAILMSG(1,(TEXT("[FIMG] chunkalloc %d byte\n"), size));
            fl.erase(i);
            
            // for check memory leak. 081008 JJG
            //RETAILMSG(1,(TEXT("[FIMG] LastChunk = 0x%x\n"), LastChunk));
            LastChunk->next = res;
            res->prev = LastChunk;
            //RETAILMSG(1,(TEXT("[FIMG] LastChunk->next = 0x%x\n"), LastChunk->next));
            LastChunk = res;
            
            
            Plat::unlock((CRITICAL_SECTION *) (&gles20_chunkalloc_mutex),__FUNCTION__);
            //RETAILMSG(1,(TEXT("[FIMG] chunkalloc %d byte done\n"), size));
            return (Chunk*)res;
        }
    }
    //Plat::printf("Chunk Allocator unable to allocate %d bytes\n", size);

    Plat::unlock( (&gles20_chunkalloc_mutex),__FUNCTION__);
    
    return NULL;
}

/*------------------------------------------------------------------------*//*!
* Free chunk
*//*-------------------------------------------------------------------------*/
void
ChunkAlloc::Free(ChunkHandle* ch)
{
    Chunk *c = (Chunk*) ch;
    
    if(ch == NULL)
        return;
    
    Plat::lock((&gles20_chunkalloc_mutex),__FUNCTION__);
    
    //RETAILMSG(1,(TEXT("[FIMG] chunkalloc free %d byte\n"), c->GetSize()));
    Node n(c->GetStart(), c->GetSize());
    MergeNode(fl, n);
    
    if(c == LastChunk)
    {
        c->prev->next = NULL;
        LastChunk = c->prev;        
    }
    else
    {
        c->prev->next = c->next;
        c->next->prev = c->prev;
    }
    
    delete ch;
    
    Plat::unlock( (&gles20_chunkalloc_mutex),__FUNCTION__);
}

/*------------------------------------------------------------------------*//*!
* Print info
*//*-------------------------------------------------------------------------*/
void
ChunkAlloc::Print()
{
    Plat::lock( (&gles20_chunkalloc_mutex),__FUNCTION__);
    //printf("Free:\nstart\tend\tsize\n");
    for(std::list<Node>::iterator i = fl.begin(); i != fl.end(); i++) {
        i->Print();
    }

    //printf("Used:\nstart\tend\tsize\n");
    for(std::list<Node>::iterator i = ul.begin(); i != ul.end(); i++) {
        i->Print();
    }
    Plat::unlock( (&gles20_chunkalloc_mutex),__FUNCTION__);
}


/*
//!< add by JJG 08.10.08, for check memory leak when driver is closed.
*/
void 
ChunkAlloc::GarbageCollect()
{
    unsigned int processID = (unsigned int)GetOwnerProcess();
    Chunk *temp, *prev=pHeadChunk;
    //RETAILMSG(1,(TEXT("GarbageCollect!!\n")));
    Plat::lock((&gles20_chunkalloc_mutex),__FUNCTION__);
    
    for(temp=pHeadChunk->next; temp != NULL ; temp=temp->next)
    {
        if(temp->processID == processID)
        {
           // RETAILMSG(1,(TEXT("[FIMG] garbage size=%d\n"), temp->GetSize()));
            prev->next = temp->next;
            if(temp->next != NULL) temp->next->prev = prev;
            Node n(temp->GetStart(), temp->GetSize());
            MergeNode(fl, n);
            delete temp;
            temp = prev;
        }
        else 
        {
            prev = temp;
        }
    }
    LastChunk = prev;
  
    //RETAILMSG(1,(TEXT("GarbageCollect end!!\n")));
    Plat::unlock( (&gles20_chunkalloc_mutex),__FUNCTION__);
}