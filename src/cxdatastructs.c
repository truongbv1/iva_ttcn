/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/
#include "../inc/cxcore.h"


// cvtables.c
const signed char icvDepthToType[] =
{
    -1, -1, CV_8U, CV_8S, CV_16U, CV_16S, -1, -1,
    CV_32F, CV_32S, -1, -1, -1, -1, -1, -1, CV_64F, -1
};
//

#define ICV_FREE_PTR(storage)  \
    ((schar*)(storage)->top + (storage)->block_size - (storage)->free_space)

#define ICV_ALIGNED_SEQ_BLOCK_SIZE  \
    (int)cvAlign(sizeof(CvSeqBlock), CV_STRUCT_ALIGN)

CV_INLINE int
cvAlignLeft( int size, int align )
{
    return size & -align;
}

#define CV_GET_LAST_ELEM( seq, block ) \
    ((block)->data + ((block)->count - 1)*((seq)->elem_size))



/****************************************************************************************\
*            Functions for manipulating memory storage - list of memory blocks           *
\****************************************************************************************/

/* Initialize allocated storage: */
static void
icvInitMemStorage( CvMemStorage* storage, int block_size )
{
    CV_FUNCNAME( "icvInitMemStorage " );

    __BEGIN__;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "" );

    if( block_size <= 0 )
        block_size = CV_STORAGE_BLOCK_SIZE;

    block_size = cvAlign( block_size, CV_STRUCT_ALIGN );
    assert( sizeof(CvMemBlock) % CV_STRUCT_ALIGN == 0 );

    memset( storage, 0, sizeof( *storage ));
    storage->signature = CV_STORAGE_MAGIC_VAL;
    storage->block_size = block_size;

    __END__;
}


/* Create root memory storage: */
CV_IMPL CvMemStorage*
cvCreateMemStorage( int block_size )
{
    CvMemStorage *storage = 0;

    CV_FUNCNAME( "cvCreateMemStorage" );

    __BEGIN__;

    CV_CALL( storage = (CvMemStorage *)cvAlloc( sizeof( CvMemStorage )));
    CV_CALL( icvInitMemStorage( storage, block_size ));

    __END__;

    if( cvGetErrStatus() < 0 )
        cvFree( &storage );

    return storage;
}


/* Create child memory storage: */
CV_IMPL CvMemStorage *
cvCreateChildMemStorage( CvMemStorage * parent )
{
    CvMemStorage *storage = 0;
    CV_FUNCNAME( "cvCreateChildMemStorage" );

    __BEGIN__;

    if( !parent )
        CV_ERROR( CV_StsNullPtr, "" );

    CV_CALL( storage = cvCreateMemStorage(parent->block_size));
    storage->parent = parent;

    __END__;

    if( cvGetErrStatus() < 0 )
        cvFree( &storage );

    return storage;
}


/* Release all blocks of the storage (or return them to parent, if any): */
static void
icvDestroyMemStorage( CvMemStorage* storage )
{
    CV_FUNCNAME( "icvDestroyMemStorage" );

    __BEGIN__;

    int k = 0;

    CvMemBlock *block;
    CvMemBlock *dst_top = 0;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "" );

    if( storage->parent )
        dst_top = storage->parent->top;

    for( block = storage->bottom; block != 0; k++ )
    {
        CvMemBlock *temp = block;

        block = block->next;
        if( storage->parent )
        {
            if( dst_top )
            {
                temp->prev = dst_top;
                temp->next = dst_top->next;
                if( temp->next )
                    temp->next->prev = temp;
                dst_top = dst_top->next = temp;
            }
            else
            {
                dst_top = storage->parent->bottom = storage->parent->top = temp;
                temp->prev = temp->next = 0;
                storage->free_space = storage->block_size - sizeof( *temp );
            }
        }
        else
        {
            cvFree( &temp );
        }
    }

    storage->top = storage->bottom = 0;
    storage->free_space = 0;

    __END__;
}


/* Release memory storage: */
CV_IMPL void
cvReleaseMemStorage( CvMemStorage** storage )
{
    CvMemStorage *st;
    CV_FUNCNAME( "cvReleaseMemStorage" );

    __BEGIN__;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "" );

    st = *storage;
    *storage = 0;

    if( st )
    {
        CV_CALL( icvDestroyMemStorage( st ));
        cvFree( &st );
    }

    __END__;
}


/* Clears memory storage (return blocks to the parent, if any): */
CV_IMPL void
cvClearMemStorage( CvMemStorage * storage )
{
    CV_FUNCNAME( "cvClearMemStorage" );

    __BEGIN__;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "" );

    if( storage->parent )
    {
        icvDestroyMemStorage( storage );
    }
    else
    {
        storage->top = storage->bottom;
        storage->free_space = storage->bottom ? storage->block_size - sizeof(CvMemBlock) : 0;
    }

    __END__;
}


/* Moves stack pointer to next block.
   If no blocks, allocate new one and link it to the storage: */
static void
icvGoNextMemBlock( CvMemStorage * storage )
{
    CV_FUNCNAME( "icvGoNextMemBlock" );

    __BEGIN__;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "" );

    if( !storage->top || !storage->top->next )
    {
        CvMemBlock *block;

        if( !(storage->parent) )
        {
            CV_CALL( block = (CvMemBlock *)cvAlloc( storage->block_size ));
        }
        else
        {
            CvMemStorage *parent = storage->parent;
            CvMemStoragePos parent_pos;

            cvSaveMemStoragePos( parent, &parent_pos );
            CV_CALL( icvGoNextMemBlock( parent ));

            block = parent->top;
            cvRestoreMemStoragePos( parent, &parent_pos );

            if( block == parent->top )  /* the single allocated block */
            {
                assert( parent->bottom == block );
                parent->top = parent->bottom = 0;
                parent->free_space = 0;
            }
            else
            {
                /* cut the block from the parent's list of blocks */
                parent->top->next = block->next;
                if( block->next )
                    block->next->prev = parent->top;
            }
        }

        /* link block */
        block->next = 0;
        block->prev = storage->top;

        if( storage->top )
            storage->top->next = block;
        else
            storage->top = storage->bottom = block;
    }

    if( storage->top->next )
        storage->top = storage->top->next;
    storage->free_space = storage->block_size - sizeof(CvMemBlock);
    assert( storage->free_space % CV_STRUCT_ALIGN == 0 );

    __END__;
}


/* Remember memory storage position: */
CV_IMPL void
cvSaveMemStoragePos( const CvMemStorage * storage, CvMemStoragePos * pos )
{
    CV_FUNCNAME( "cvSaveMemStoragePos" );

    __BEGIN__;

    if( !storage || !pos )
        CV_ERROR( CV_StsNullPtr, "" );

    pos->top = storage->top;
    pos->free_space = storage->free_space;

    __END__;
}


/* Restore memory storage position: */
CV_IMPL void
cvRestoreMemStoragePos( CvMemStorage * storage, CvMemStoragePos * pos )
{
    CV_FUNCNAME( "cvRestoreMemStoragePos" );

    __BEGIN__;

    if( !storage || !pos )
        CV_ERROR( CV_StsNullPtr, "" );
    if( pos->free_space > storage->block_size )
        CV_ERROR( CV_StsBadSize, "" );

    /*
    // this breaks icvGoNextMemBlock, so comment it off for now
    if( storage->parent && (!pos->top || pos->top->next) )
    {
        CvMemBlock* save_bottom;
        if( !pos->top )
            save_bottom = 0;
        else
        {
            save_bottom = storage->bottom;
            storage->bottom = pos->top->next;
            pos->top->next = 0;
            storage->bottom->prev = 0;
        }
        icvDestroyMemStorage( storage );
        storage->bottom = save_bottom;
    }*/

    storage->top = pos->top;
    storage->free_space = pos->free_space;

    if( !storage->top )
    {
        storage->top = storage->bottom;
        storage->free_space = storage->top ? storage->block_size - sizeof(CvMemBlock) : 0;
    }

    __END__;
}


/* Allocate continuous buffer of the specified size in the storage: */
CV_IMPL void*
cvMemStorageAlloc( CvMemStorage* storage, size_t size )
{
    schar *ptr = 0;

    CV_FUNCNAME( "cvMemStorageAlloc" );

    __BEGIN__;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "NULL storage pointer" );

    if( size > INT_MAX )
        CV_ERROR( CV_StsOutOfRange, "Too large memory block is requested" );

    assert( storage->free_space % CV_STRUCT_ALIGN == 0 );

    if( (size_t)storage->free_space < size )
    {
        size_t max_free_space = cvAlignLeft(storage->block_size - sizeof(CvMemBlock), CV_STRUCT_ALIGN);
        if( max_free_space < size )
            CV_ERROR( CV_StsOutOfRange, "requested size is negative or too big" );

        CV_CALL( icvGoNextMemBlock( storage ));
    }

    ptr = ICV_FREE_PTR(storage);
    assert( (size_t)ptr % CV_STRUCT_ALIGN == 0 );
    storage->free_space = cvAlignLeft(storage->free_space - (int)size, CV_STRUCT_ALIGN );

    __END__;

    return ptr;
}


// CV_IMPL CvString
// cvMemStorageAllocString( CvMemStorage* storage, const char* ptr, int len )
// {
//     CvString str;
//     CV_FUNCNAME( "cvMemStorageAllocString" );

//     __BEGIN__;

//     str.len = len >= 0 ? len : (int)strlen(ptr);
//     CV_CALL( str.ptr = (char*)cvMemStorageAlloc( storage, str.len + 1 ));
//     memcpy( str.ptr, ptr, str.len );
//     str.ptr[str.len] = '\0';

//     __END__;

//     return str;
// }


/****************************************************************************************\
*                               Sequence implementation                                  *
\****************************************************************************************/

/* Create empty sequence: */
CV_IMPL CvSeq *
cvCreateSeq( int seq_flags, int header_size, int elem_size, CvMemStorage * storage )
{
    CvSeq *seq = 0;

    CV_FUNCNAME( "cvCreateSeq" );

    __BEGIN__;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "" );
    if( header_size < (int)sizeof( CvSeq ) || elem_size <= 0 )
        CV_ERROR( CV_StsBadSize, "" );

    /* allocate sequence header */
    CV_CALL( seq = (CvSeq*)cvMemStorageAlloc( storage, header_size ));
    memset( seq, 0, header_size );

    seq->header_size = header_size;
    seq->flags = (seq_flags & ~CV_MAGIC_MASK) | CV_SEQ_MAGIC_VAL;
    {
        int elemtype = CV_MAT_TYPE(seq_flags);
        int typesize = CV_ELEM_SIZE(elemtype);

        if( elemtype != CV_SEQ_ELTYPE_GENERIC &&
            typesize != 0 && typesize != elem_size )
            CV_ERROR( CV_StsBadSize,
            "Specified element size doesn't match to the size of the specified element type "
            "(try to use 0 for element type)" );
    }
    seq->elem_size = elem_size;
    seq->storage = storage;

    CV_CALL( cvSetSeqBlockSize( seq, (1 << 10)/elem_size ));

    __END__;

    return seq;
}


/* adjusts <delta_elems> field of sequence. It determines how much the sequence
   grows if there are no free space inside the sequence buffers */
CV_IMPL void
cvSetSeqBlockSize( CvSeq *seq, int delta_elements )
{
    int elem_size;
    int useful_block_size;

    CV_FUNCNAME( "cvSetSeqBlockSize" );

    __BEGIN__;

    if( !seq || !seq->storage )
        CV_ERROR( CV_StsNullPtr, "" );
    if( delta_elements < 0 )
        CV_ERROR( CV_StsOutOfRange, "" );

    useful_block_size = cvAlignLeft(seq->storage->block_size - sizeof(CvMemBlock) -
                                    sizeof(CvSeqBlock), CV_STRUCT_ALIGN);
    elem_size = seq->elem_size;

    if( delta_elements == 0 )
    {
        delta_elements = (1 << 10) / elem_size;
        delta_elements = MAX( delta_elements, 1 );
    }
    if( delta_elements * elem_size > useful_block_size )
    {
        delta_elements = useful_block_size / elem_size;
        if( delta_elements == 0 )
            CV_ERROR( CV_StsOutOfRange, "Storage block size is too small "
                                        "to fit the sequence elements" );
    }

    seq->delta_elems = delta_elements;

    __END__;
}




CV_IMPL int
cvSliceLength( CvSlice slice, const CvSeq* seq )
{
    int total = seq->total;
    int length = slice.end_index - slice.start_index;

    if( length != 0 )
    {
        if( slice.start_index < 0 )
            slice.start_index += total;
        if( slice.end_index <= 0 )
            slice.end_index += total;

        length = slice.end_index - slice.start_index;
    }

    if( length < 0 )
    {
        length += total;
        /*if( length < 0 )
            length += total;*/
    }
    else if( length > total )
        length = total;

    return length;
}





/* Construct a sequence from an array without copying any data.
   NB: The resultant sequence cannot grow beyond its initial size: */
CV_IMPL CvSeq*
cvMakeSeqHeaderForArray( int seq_flags, int header_size, int elem_size,
                         void *array, int total, CvSeq *seq, CvSeqBlock * block )
{
    CvSeq* result = 0;

    CV_FUNCNAME( "cvMakeSeqHeaderForArray" );

    __BEGIN__;

    if( elem_size <= 0 || header_size < (int)sizeof( CvSeq ) || total < 0 )
        CV_ERROR( CV_StsBadSize, "" );

    if( !seq || ((!array || !block) && total > 0) )
        CV_ERROR( CV_StsNullPtr, "" );

    memset( seq, 0, header_size );

    seq->header_size = header_size;
    seq->flags = (seq_flags & ~CV_MAGIC_MASK) | CV_SEQ_MAGIC_VAL;
    {
        int elemtype = CV_MAT_TYPE(seq_flags);
        int typesize = CV_ELEM_SIZE(elemtype);

        if( elemtype != CV_SEQ_ELTYPE_GENERIC &&
            typesize != 0 && typesize != elem_size )
            CV_ERROR( CV_StsBadSize,
            "Element size doesn't match to the size of predefined element type "
            "(try to use 0 for sequence element type)" );
    }
    seq->elem_size = elem_size;
    seq->total = total;
    seq->block_max = seq->ptr = (schar *) array + total * elem_size;

    if( total > 0 )
    {
        seq->first = block;
        block->prev = block->next = block;
        block->start_index = 0;
        block->count = total;
        block->data = (schar *) array;
    }

    result = seq;

    __END__;

    return result;
}


/* The function allocates space for at least one more sequence element.
   If there are free sequence blocks (seq->free_blocks != 0)
   they are reused, otherwise the space is allocated in the storage: */
static void
icvGrowSeq( CvSeq *seq, int in_front_of )
{
    CV_FUNCNAME( "icvGrowSeq" );

    __BEGIN__;

    CvSeqBlock *block;

    if( !seq )
        CV_ERROR( CV_StsNullPtr, "" );
    block = seq->free_blocks;

    if( !block )
    {
        int elem_size = seq->elem_size;
        int delta_elems = seq->delta_elems;
        CvMemStorage *storage = seq->storage;

        if( seq->total >= delta_elems*4 )
            cvSetSeqBlockSize( seq, delta_elems*2 );

        if( !storage )
            CV_ERROR( CV_StsNullPtr, "The sequence has NULL storage pointer" );

        /* If there is a free space just after last allocated block
           and it is big enough then enlarge the last block.
           This can happen only if the new block is added to the end of sequence: */
        if( (unsigned)(ICV_FREE_PTR(storage) - seq->block_max) < CV_STRUCT_ALIGN &&
            storage->free_space >= seq->elem_size && !in_front_of )
        {
            int delta = storage->free_space / elem_size;

            delta = MIN( delta, delta_elems ) * elem_size;
            seq->block_max += delta;
            storage->free_space = cvAlignLeft((int)(((schar*)storage->top + storage->block_size) -
                                              seq->block_max), CV_STRUCT_ALIGN );
            EXIT;
        }
        else
        {
            int delta = elem_size * delta_elems + ICV_ALIGNED_SEQ_BLOCK_SIZE;

            /* Try to allocate <delta_elements> elements: */
            if( storage->free_space < delta )
            {
                int small_block_size = MAX(1, delta_elems/3)*elem_size +
                                       ICV_ALIGNED_SEQ_BLOCK_SIZE;
                /* try to allocate smaller part */
                if( storage->free_space >= small_block_size + CV_STRUCT_ALIGN )
                {
                    delta = (storage->free_space - ICV_ALIGNED_SEQ_BLOCK_SIZE)/seq->elem_size;
                    delta = delta*seq->elem_size + ICV_ALIGNED_SEQ_BLOCK_SIZE;
                }
                else
                {
                    CV_CALL( icvGoNextMemBlock( storage ));
                    assert( storage->free_space >= delta );
                }
            }

            CV_CALL( block = (CvSeqBlock*)cvMemStorageAlloc( storage, delta ));
            block->data = (schar*)cvAlignPtr( block + 1, CV_STRUCT_ALIGN );
            block->count = delta - ICV_ALIGNED_SEQ_BLOCK_SIZE;
            block->prev = block->next = 0;
        }
    }
    else
    {
        seq->free_blocks = block->next;
    }

    if( !(seq->first) )
    {
        seq->first = block;
        block->prev = block->next = block;
    }
    else
    {
        block->prev = seq->first->prev;
        block->next = seq->first;
        block->prev->next = block->next->prev = block;
    }

    /* For free blocks the <count> field means
     * total number of bytes in the block.
     *
     * For used blocks it means current number
     * of sequence elements in the block:
     */
    assert( block->count % seq->elem_size == 0 && block->count > 0 );

    if( !in_front_of )
    {
        seq->ptr = block->data;
        seq->block_max = block->data + block->count;
        block->start_index = block == block->prev ? 0 :
            block->prev->start_index + block->prev->count;
    }
    else
    {
        int delta = block->count / seq->elem_size;
        block->data += block->count;

        if( block != block->prev )
        {
            assert( seq->first->start_index == 0 );
            seq->first = block;
        }
        else
        {
            seq->block_max = seq->ptr = block->data;
        }

        block->start_index = 0;

        for( ;; )
        {
            block->start_index += delta;
            block = block->next;
            if( block == seq->first )
                break;
        }
    }

    block->count = 0;

    __END__;
}


/****************************************************************************************\
*                             Sequence Writer implementation                             *
\****************************************************************************************/

/* Initialize sequence writer: */
CV_IMPL void
cvStartAppendToSeq( CvSeq *seq, CvSeqWriter * writer )
{
    CV_FUNCNAME( "cvStartAppendToSeq" );

    __BEGIN__;

    if( !seq || !writer )
        CV_ERROR( CV_StsNullPtr, "" );

    memset( writer, 0, sizeof( *writer ));
    writer->header_size = sizeof( CvSeqWriter );

    writer->seq = seq;
    writer->block = seq->first ? seq->first->prev : 0;
    writer->ptr = seq->ptr;
    writer->block_max = seq->block_max;

    __END__;
}


/* Initialize sequence writer: */
CV_IMPL void
cvStartWriteSeq( int seq_flags, int header_size,
                 int elem_size, CvMemStorage * storage, CvSeqWriter * writer )
{
    CvSeq *seq = 0;

    CV_FUNCNAME( "cvStartWriteSeq" );

    __BEGIN__;

    if( !storage || !writer )
        CV_ERROR( CV_StsNullPtr, "" );

    CV_CALL( seq = cvCreateSeq( seq_flags, header_size, elem_size, storage ));
    cvStartAppendToSeq( seq, writer );

    __END__;
}


/* Update sequence header: */
CV_IMPL void
cvFlushSeqWriter( CvSeqWriter * writer )
{
    CvSeq *seq = 0;

    CV_FUNCNAME( "cvFlushSeqWriter" );

    __BEGIN__;

    if( !writer )
        CV_ERROR( CV_StsNullPtr, "" );

    seq = writer->seq;
    seq->ptr = writer->ptr;

    if( writer->block )
    {
        int total = 0;
        CvSeqBlock *first_block = writer->seq->first;
        CvSeqBlock *block = first_block;

        writer->block->count = (int)((writer->ptr - writer->block->data) / seq->elem_size);
        assert( writer->block->count > 0 );

        do
        {
            total += block->count;
            block = block->next;
        }
        while( block != first_block );

        writer->seq->total = total;
    }

    __END__;
}


/* Calls icvFlushSeqWriter and finishes writing process: */
CV_IMPL CvSeq *
cvEndWriteSeq( CvSeqWriter * writer )
{
    CvSeq *seq = 0;

    CV_FUNCNAME( "cvEndWriteSeq" );

    __BEGIN__;

    if( !writer )
        CV_ERROR( CV_StsNullPtr, "" );

    CV_CALL( cvFlushSeqWriter( writer ));
    seq = writer->seq;

    /* Truncate the last block: */
    if( writer->block && writer->seq->storage )
    {
        CvMemStorage *storage = seq->storage;
        schar *storage_block_max = (schar *) storage->top + storage->block_size;

        assert( writer->block->count > 0 );

        if( (unsigned)((storage_block_max - storage->free_space)
            - seq->block_max) < CV_STRUCT_ALIGN )
        {
            storage->free_space = cvAlignLeft((int)(storage_block_max - seq->ptr), CV_STRUCT_ALIGN);
            seq->block_max = seq->ptr;
        }
    }

    writer->ptr = 0;

    __END__;

    return seq;
}


/* Create new sequence block: */
CV_IMPL void
cvCreateSeqBlock( CvSeqWriter * writer )
{
    CV_FUNCNAME( "cvCreateSeqBlock" );

    __BEGIN__;

    CvSeq *seq;

    if( !writer || !writer->seq )
        CV_ERROR( CV_StsNullPtr, "" );

    seq = writer->seq;

    cvFlushSeqWriter( writer );

    CV_CALL( icvGrowSeq( seq, 0 ));

    writer->block = seq->first->prev;
    writer->ptr = seq->ptr;
    writer->block_max = seq->block_max;

    __END__;
}


/****************************************************************************************\
*                               Sequence Reader implementation                           *
\****************************************************************************************/

/* Initialize sequence reader: */
CV_IMPL void
cvStartReadSeq( const CvSeq *seq, CvSeqReader * reader, int reverse )
{
    CvSeqBlock *first_block;
    CvSeqBlock *last_block;

    CV_FUNCNAME( "cvStartReadSeq" );

    if( reader )
    {
        reader->seq = 0;
        reader->block = 0;
        reader->ptr = reader->block_max = reader->block_min = 0;
    }

    __BEGIN__;

    if( !seq || !reader )
        CV_ERROR( CV_StsNullPtr, "" );

    reader->header_size = sizeof( CvSeqReader );
    reader->seq = (CvSeq*)seq;

    first_block = seq->first;

    if( first_block )
    {
        last_block = first_block->prev;
        reader->ptr = first_block->data;
        reader->prev_elem = CV_GET_LAST_ELEM( seq, last_block );
        reader->delta_index = seq->first->start_index;

        if( reverse )
        {
            schar *temp = reader->ptr;

            reader->ptr = reader->prev_elem;
            reader->prev_elem = temp;

            reader->block = last_block;
        }
        else
        {
            reader->block = first_block;
        }

        reader->block_min = reader->block->data;
        reader->block_max = reader->block_min + reader->block->count * seq->elem_size;
    }
    else
    {
        reader->delta_index = 0;
        reader->block = 0;

        reader->ptr = reader->prev_elem = reader->block_min = reader->block_max = 0;
    }

    __END__;
}


/* Change the current reading block
 * to the previous or to the next:
 */
CV_IMPL void
cvChangeSeqBlock( void* _reader, int direction )
{
    CV_FUNCNAME( "cvChangeSeqBlock" );

    __BEGIN__;

    CvSeqReader* reader = (CvSeqReader*)_reader;

    if( !reader )
        CV_ERROR( CV_StsNullPtr, "" );

    if( direction > 0 )
    {
        reader->block = reader->block->next;
        reader->ptr = reader->block->data;
    }
    else
    {
        reader->block = reader->block->prev;
        reader->ptr = CV_GET_LAST_ELEM( reader->seq, reader->block );
    }
    reader->block_min = reader->block->data;
    reader->block_max = reader->block_min + reader->block->count * reader->seq->elem_size;

    __END__;
}




/* Set reader position to given position,
 * either absolute or relative to the
 *  current one:
 */
CV_IMPL void
cvSetSeqReaderPos( CvSeqReader* reader, int index, int is_relative )
{
    CV_FUNCNAME( "cvSetSeqReaderPos" );

    __BEGIN__;

    CvSeqBlock *block;
    int elem_size, count, total;

    if( !reader || !reader->seq )
        CV_ERROR( CV_StsNullPtr, "" );

    total = reader->seq->total;
    elem_size = reader->seq->elem_size;

    if( !is_relative )
    {
        if( index < 0 )
        {
            if( index < -total )
                CV_ERROR( CV_StsOutOfRange, "" );
            index += total;
        }
        else if( index >= total )
        {
            index -= total;
            if( index >= total )
                CV_ERROR( CV_StsOutOfRange, "" );
        }

        block = reader->seq->first;
        if( index >= (count = block->count) )
        {
            if( index + index <= total )
            {
                do
                {
                    block = block->next;
                    index -= count;
                }
                while( index >= (count = block->count) );
            }
            else
            {
                do
                {
                    block = block->prev;
                    total -= block->count;
                }
                while( index < total );
                index -= total;
            }
        }
        reader->ptr = block->data + index * elem_size;
        if( reader->block != block )
        {
            reader->block = block;
            reader->block_min = block->data;
            reader->block_max = block->data + block->count * elem_size;
        }
    }
    else
    {
        schar* ptr = reader->ptr;
        index *= elem_size;
        block = reader->block;

        if( index > 0 )
        {
            while( ptr + index >= reader->block_max )
            {
                int delta = (int)(reader->block_max - ptr);
                index -= delta;
                reader->block = block = block->next;
                reader->block_min = ptr = block->data;
                reader->block_max = block->data + block->count*elem_size;
            }
            reader->ptr = ptr + index;
        }
        else
        {
            while( ptr + index < reader->block_min )
            {
                int delta = (int)(ptr - reader->block_min);
                index += delta;
                reader->block = block = block->prev;
                reader->block_min = block->data;
                reader->block_max = ptr = block->data + block->count*elem_size;
            }
            reader->ptr = ptr + index;
        }
    }

    __END__;
}



/****************************************************************************************\
*                                      Set implementation                                *
\****************************************************************************************/

/* Creates empty set: */
CV_IMPL CvSet*
cvCreateSet( int set_flags, int header_size, int elem_size, CvMemStorage * storage )
{
    CvSet *set = 0;

    CV_FUNCNAME( "cvCreateSet" );

    __BEGIN__;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "" );
    if( header_size < (int)sizeof( CvSet ) ||
        elem_size < (int)sizeof(void*)*2 ||
        (elem_size & (sizeof(void*)-1)) != 0 )
        CV_ERROR( CV_StsBadSize, "" );

    set = (CvSet*) cvCreateSeq( set_flags, header_size, elem_size, storage );
    set->flags = (set->flags & ~CV_MAGIC_MASK) | CV_SET_MAGIC_VAL;

    __END__;

    return set;
}


/* Add new element to the set: */
CV_IMPL int
cvSetAdd( CvSet* set, CvSetElem* element, CvSetElem** inserted_element )
{
    int id = -1;

    CV_FUNCNAME( "cvSetAdd" );

    __BEGIN__;

    CvSetElem *free_elem;

    if( !set )
        CV_ERROR( CV_StsNullPtr, "" );

    if( !(set->free_elems) )
    {
        int count = set->total;
        int elem_size = set->elem_size;
        schar *ptr;
        CV_CALL( icvGrowSeq( (CvSeq *) set, 0 ));

        set->free_elems = (CvSetElem*) (ptr = set->ptr);
        for( ; ptr + elem_size <= set->block_max; ptr += elem_size, count++ )
        {
            ((CvSetElem*)ptr)->flags = count | CV_SET_ELEM_FREE_FLAG;
            ((CvSetElem*)ptr)->next_free = (CvSetElem*)(ptr + elem_size);
        }
        assert( count <= CV_SET_ELEM_IDX_MASK+1 );
        ((CvSetElem*)(ptr - elem_size))->next_free = 0;
        set->first->prev->count += count - set->total;
        set->total = count;
        set->ptr = set->block_max;
    }

    free_elem = set->free_elems;
    set->free_elems = free_elem->next_free;

    id = free_elem->flags & CV_SET_ELEM_IDX_MASK;
    if( element )
        CV_MEMCPY_INT( free_elem, element, (size_t)set->elem_size/sizeof(int) );

    free_elem->flags = id;
    set->active_count++;

    if( inserted_element )
        *inserted_element = free_elem;

    __END__;

    return id;
}



/****************************************************************************************\
*                                 Working with sequence tree                             *
\****************************************************************************************/



typedef struct CvTreeNode
{
    int       flags;         /* micsellaneous flags */
    int       header_size;   /* size of sequence header */
    struct    CvTreeNode* h_prev; /* previous sequence */
    struct    CvTreeNode* h_next; /* next sequence */
    struct    CvTreeNode* v_prev; /* 2nd previous sequence */
    struct    CvTreeNode* v_next; /* 2nd next sequence */
}
CvTreeNode;



// Insert contour into tree given certain parent sequence.
// If parent is equal to frame (the most external contour),
// then added contour will have null pointer to parent:
CV_IMPL void
cvInsertNodeIntoTree( void* _node, void* _parent, void* _frame )
{
    CV_FUNCNAME( "cvInsertNodeIntoTree" );

    __BEGIN__;

    CvTreeNode* node = (CvTreeNode*)_node;
    CvTreeNode* parent = (CvTreeNode*)_parent;

    if( !node || !parent )
        CV_ERROR( CV_StsNullPtr, "" );

    node->v_prev = _parent != _frame ? parent : 0;
    node->h_next = parent->v_next;

    assert( parent->v_next != node );

    if( parent->v_next )
        parent->v_next->h_prev = node;
    parent->v_next = node;

    __END__;
}



/****************************************************************************************\
*                                  Chain Approximation                                   *
\****************************************************************************************/

typedef struct _CvPtInfo
{
    CvPoint pt;
    int k;                      /* support region */
    int s;                      /* curvature value */
    struct _CvPtInfo *next;
}
_CvPtInfo;


/* curvature: 0 - 1-curvature, 1 - k-cosine curvature. */
CvStatus
icvApproximateChainTC89( CvChain*               chain,
                         int                    header_size,
                         CvMemStorage*          storage, 
                         CvSeq**                contour, 
                         int    method )
{
    static const int abs_diff[] = { 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1 };

    char            local_buffer[1 << 16];
    char*           buffer = local_buffer;
    int             buffer_size;

    _CvPtInfo       temp;
    _CvPtInfo       *array, *first = 0, *current = 0, *prev_current = 0;
    int             i, j, i1, i2, s, len;
    int             count;

    CvChainPtReader reader;
    CvSeqWriter     writer;
    CvPoint         pt = chain->origin;
   
    assert( chain && contour && buffer );

    buffer_size = (chain->total + 8) * sizeof( _CvPtInfo );

    *contour = 0;

    if( !CV_IS_SEQ_CHAIN_CONTOUR( chain ))
        return CV_BADFLAG_ERR;

    if( header_size < (int)sizeof(CvContour) )
        return CV_BADSIZE_ERR;
    
    cvStartWriteSeq( (chain->flags & ~CV_SEQ_ELTYPE_MASK) | CV_SEQ_ELTYPE_POINT,
                     header_size, sizeof( CvPoint ), storage, &writer );
    
    if( chain->total == 0 )
    {        
        CV_WRITE_SEQ_ELEM( pt, writer );
        goto exit_function;
    }

    cvStartReadChainPoints( chain, &reader );

    if( method > CV_CHAIN_APPROX_SIMPLE && buffer_size > (int)sizeof(local_buffer))
    {
        buffer = (char *) cvAlloc( buffer_size );
        if( !buffer )
            return CV_OUTOFMEM_ERR;
    }

    array = (_CvPtInfo *) buffer;
    count = chain->total;

    temp.next = 0;
    current = &temp;

    /* Pass 0.
       Restores all the digital curve points from the chain code.
       Removes the points (from the resultant polygon)
       that have zero 1-curvature */
    for( i = 0; i < count; i++ )
    {
        int prev_code = *reader.prev_elem;

        reader.prev_elem = reader.ptr;
        CV_READ_CHAIN_POINT( pt, reader );

        /* calc 1-curvature */
        s = abs_diff[reader.code - prev_code + 7];

        if( method <= CV_CHAIN_APPROX_SIMPLE )
        {
            if( method == CV_CHAIN_APPROX_NONE || s != 0 )
            {
                CV_WRITE_SEQ_ELEM( pt, writer );
            }
        }
        else
        {
            if( s != 0 )
                current = current->next = array + i;
            array[i].s = s;
            array[i].pt = pt;
        }
    }

    //assert( pt.x == chain->origin.x && pt.y == chain->origin.y );

    if( method <= CV_CHAIN_APPROX_SIMPLE )
        goto exit_function;

    current->next = 0;

    len = i;
    current = temp.next;

    assert( current );

    /* Pass 1.
       Determines support region for all the remained points */
    do
    {
        CvPoint pt0;
        int k, l = 0, d_num = 0;

        i = (int)(current - array);
        pt0 = array[i].pt;

        /* determine support region */
        for( k = 1;; k++ )
        {
            int lk, dk_num;
            int dx, dy;
            Cv32suf d;

            assert( k <= len );

            /* calc indices */
            i1 = i - k;
            i1 += i1 < 0 ? len : 0;
            i2 = i + k;
            i2 -= i2 >= len ? len : 0;

            dx = array[i2].pt.x - array[i1].pt.x;
            dy = array[i2].pt.y - array[i1].pt.y;

            /* distance between p_(i - k) and p_(i + k) */
            lk = dx * dx + dy * dy;

            /* distance between p_i and the line (p_(i-k), p_(i+k)) */
            dk_num = (pt0.x - array[i1].pt.x) * dy - (pt0.y - array[i1].pt.y) * dx;
            d.f = (float) (((double) d_num) * lk - ((double) dk_num) * l);

            if( k > 1 && (l >= lk || ((d_num > 0 && d.i <= 0) || (d_num < 0 && d.i >= 0))))
                break;

            d_num = dk_num;
            l = lk;
        }

        current->k = --k;

        /* determine cosine curvature if it should be used */
        if( method == CV_CHAIN_APPROX_TC89_KCOS )
        {
            /* calc k-cosine curvature */
            for( j = k, s = 0; j > 0; j-- )
            {
                double temp_num;
                int dx1, dy1, dx2, dy2;
                Cv32suf sk;

                i1 = i - j;
                i1 += i1 < 0 ? len : 0;
                i2 = i + j;
                i2 -= i2 >= len ? len : 0;

                dx1 = array[i1].pt.x - pt0.x;
                dy1 = array[i1].pt.y - pt0.y;
                dx2 = array[i2].pt.x - pt0.x;
                dy2 = array[i2].pt.y - pt0.y;

                if( (dx1 | dy1) == 0 || (dx2 | dy2) == 0 )
                    break;

                temp_num = dx1 * dx2 + dy1 * dy2;
                temp_num =
                    (float) (temp_num /
                             sqrt( ((double)dx1 * dx1 + (double)dy1 * dy1) *
                                   ((double)dx2 * dx2 + (double)dy2 * dy2) ));
                sk.f = (float) (temp_num + 1.1);

                assert( 0 <= sk.f && sk.f <= 2.2 );
                if( j < k && sk.i <= s )
                    break;

                s = sk.i;
            }
            current->s = s;
        }
        current = current->next;
    }
    while( current != 0 );

    prev_current = &temp;
    current = temp.next;

    /* Pass 2.
       Performs non-maxima supression */
    do
    {
        int k2 = current->k >> 1;

        s = current->s;
        i = (int)(current - array);

        for( j = 1; j <= k2; j++ )
        {
            i2 = i - j;
            i2 += i2 < 0 ? len : 0;

            if( array[i2].s > s )
                break;

            i2 = i + j;
            i2 -= i2 >= len ? len : 0;

            if( array[i2].s > s )
                break;
        }

        if( j <= k2 )           /* exclude point */
        {
            prev_current->next = current->next;
            current->s = 0;     /* "clear" point */
        }
        else
            prev_current = current;
        current = current->next;
    }
    while( current != 0 );

    /* Pass 3.
       Removes non-dominant points with 1-length support region */
    current = temp.next;
    assert( current );
    prev_current = &temp;

    do
    {
        if( current->k == 1 )
        {
            s = current->s;
            i = (int)(current - array);

            i1 = i - 1;
            i1 += i1 < 0 ? len : 0;

            i2 = i + 1;
            i2 -= i2 >= len ? len : 0;

            if( s <= array[i1].s || s <= array[i2].s )
            {
                prev_current->next = current->next;
                current->s = 0;
            }
            else
                prev_current = current;
        }
        else
            prev_current = current;
        current = current->next;
    }
    while( current != 0 );

    if( method == CV_CHAIN_APPROX_TC89_KCOS )
        goto copy_vect;

    /* Pass 4.
       Cleans remained couples of points */
    assert( temp.next );

    if( array[0].s != 0 && array[len - 1].s != 0 )      /* specific case */
    {
        for( i1 = 1; i1 < len && array[i1].s != 0; i1++ )
        {
            array[i1 - 1].s = 0;
        }
        if( i1 == len )
            goto copy_vect;     /* all points survived */
        i1--;

        for( i2 = len - 2; i2 > 0 && array[i2].s != 0; i2-- )
        {
            array[i2].next = 0;
            array[i2 + 1].s = 0;
        }
        i2++;

        if( i1 == 0 && i2 == len - 1 )  /* only two points */
        {
            i1 = (int)(array[0].next - array);
            array[len] = array[0];      /* move to the end */
            array[len].next = 0;
            array[len - 1].next = array + len;
        }
        temp.next = array + i1;
    }

    current = temp.next;
    first = prev_current = &temp;
    count = 1;

    /* do last pass */
    do
    {
        if( current->next == 0 || current->next - current != 1 )
        {
            if( count >= 2 )
            {
                if( count == 2 )
                {
                    int s1 = prev_current->s;
                    int s2 = current->s;

                    if( s1 > s2 || (s1 == s2 && prev_current->k <= current->k) )
                        /* remove second */
                        prev_current->next = current->next;
                    else
                        /* remove first */
                        first->next = current;
                }
                else
                    first->next->next = current;
            }
            first = current;
            count = 1;
        }
        else
            count++;
        prev_current = current;
        current = current->next;
    }
    while( current != 0 );

  copy_vect:

    /* gather points */
    current = temp.next;
    assert( current );

    do
    {
        CV_WRITE_SEQ_ELEM( current->pt, writer );
        current = current->next;
    }
    while( current != 0 );

exit_function:

    *contour = cvEndWriteSeq( &writer );

    assert( writer.seq->total > 0 );

    if( buffer != local_buffer )
        cvFree( &buffer );
    return CV_OK;
}





/* End of file. */
