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


#ifndef _CXCORE_H_
#define _CXCORE_H_



#include "../inc/cxtypes.h"
#include "../inc/cxerror.h" 

#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>



// _cxcore

extern const signed char icvDepthToType[];

#define icvIplToCvDepth( depth ) \
    icvDepthToType[(((depth) & 255) >> 2) + ((depth) < 0)]



// //#ifdef __cplusplus
// //extern "C" {
// //#endif

/****************************************************************************************\
*          Array allocation, deallocation, initialization and access to elements         *
\****************************************************************************************/

/* <malloc> wrapper.
   If there is no enough memory, the function
   (as well as other OpenCV functions that call cvAlloc)
   raises an error. */
CVAPI(void*)  cvAlloc( size_t size );

/* <free> wrapper.
   Here and further all the memory releasing functions
   (that all call cvFree) take double pointer in order to
   to clear pointer to the data after releasing it.
   Passing pointer to NULL pointer is Ok: nothing happens in this case
*/
CVAPI(void)   cvFree_( void* ptr );
#define cvFree(ptr) (cvFree_(*(ptr)), *(ptr)=0)

/* Allocates and initializes IplImage header */
CVAPI(IplImage*)  cvCreateImageHeader( CvSize size, int depth, int channels );

/* Inializes IplImage header */
CVAPI(IplImage*) cvInitImageHeader( IplImage* image, CvSize size, int depth,
                                   int channels, int origin CV_DEFAULT(0),
                                   int align CV_DEFAULT(4));

/* Creates IPL image (header and data) */
CVAPI(IplImage*)  cvCreateImage( CvSize size, int depth, int channels );

/* Releases (i.e. deallocates) IPL image header */
CVAPI(void)  cvReleaseImageHeader( IplImage** image );

/* Releases IPL image header and data */
CVAPI(void)  cvReleaseImage( IplImage** image );



#define CV_AUTOSTEP  0x7fffffff

/* Initializes CvMat header */
CVAPI(CvMat*) cvInitMatHeader( CvMat* mat, int rows, int cols,
                              int type, void* data CV_DEFAULT(NULL),
                              int step CV_DEFAULT(CV_AUTOSTEP) );


// /* Decrements CvMat data reference counter and deallocates the data if
//    it reaches 0 */
// //CV_INLINE  
void  cvDecRefData( CvArr* arr );
// // {
// //     if( CV_IS_MAT( arr ))
// //     {
// //         CvMat* mat = (CvMat*)arr;
// //         mat->data.ptr = NULL;
// //         if( mat->refcount != NULL && --*mat->refcount == 0 )
// //             cvFree( &mat->refcount );
// //         mat->refcount = NULL;
// //     }
// //     else if( CV_IS_MATND( arr ))
// //     {
// //         CvMatND* mat = (CvMatND*)arr;
// //         mat->data.ptr = NULL;
// //         if( mat->refcount != NULL && --*mat->refcount == 0 )
// //             cvFree( &mat->refcount );
// //         mat->refcount = NULL;
// //     }
// // }



/* Converts CvArr (IplImage or CvMat,...) to CvMat.
   If the last parameter is non-zero, function can
   convert multi(>2)-dimensional array to CvMat as long as
   the last array's dimension is continous. The resultant
   matrix will be have appropriate (a huge) number of rows */
CVAPI(CvMat*) cvGetMat( const CvArr* arr, CvMat* header,
                       int* coi CV_DEFAULT(NULL),
                       int allowND CV_DEFAULT(0));


/* Allocates array data */
CVAPI(void)  cvCreateData( CvArr* arr );

/* Releases array data */
CVAPI(void)  cvReleaseData( CvArr* arr );


/****************************************************************************************\
*                              Dynamic data structures                                   *
\****************************************************************************************/

/* Calculates length of sequence slice (with support of negative indices). */
CVAPI(int) cvSliceLength( CvSlice slice, const CvSeq* seq );


/* Creates new memory storage.
   block_size == 0 means that default,
   somewhat optimal size, is used (currently, it is 64K) */
CVAPI(CvMemStorage*)  cvCreateMemStorage( int block_size CV_DEFAULT(0));


/* Creates a memory storage that will borrow memory blocks from parent storage */
CVAPI(CvMemStorage*)  cvCreateChildMemStorage( CvMemStorage* parent );


/* Releases memory storage. All the children of a parent must be released before
   the parent. A child storage returns all the blocks to parent when it is released */
CVAPI(void)  cvReleaseMemStorage( CvMemStorage** storage );


/* Clears memory storage. This is the only way(!!!) (besides cvRestoreMemStoragePos)
   to reuse memory allocated for the storage - cvClearSeq,cvClearSet ...
   do not free any memory.
   A child storage returns all the blocks to the parent when it is cleared */
CVAPI(void)  cvClearMemStorage( CvMemStorage* storage );

/* Remember a storage "free memory" position */
CVAPI(void)  cvSaveMemStoragePos( const CvMemStorage* storage, CvMemStoragePos* pos );

/* Restore a storage "free memory" position */
CVAPI(void)  cvRestoreMemStoragePos( CvMemStorage* storage, CvMemStoragePos* pos );

/* Allocates continuous buffer of the specified size in the storage */
CVAPI(void*) cvMemStorageAlloc( CvMemStorage* storage, size_t size );


/* Creates new empty sequence that will reside in the specified storage */
CVAPI(CvSeq*)  cvCreateSeq( int seq_flags, int header_size,
                            int elem_size, CvMemStorage* storage );

/* Changes default size (granularity) of sequence blocks.
   The default size is ~1Kbyte */
CVAPI(void)  cvSetSeqBlockSize( CvSeq* seq, int delta_elems );


/* Initializes sequence writer. The new elements will be added to the end of sequence */
CVAPI(void)  cvStartAppendToSeq( CvSeq* seq, CvSeqWriter* writer );


/* Combination of cvCreateSeq and cvStartAppendToSeq */
CVAPI(void)  cvStartWriteSeq( int seq_flags, int header_size,
                              int elem_size, CvMemStorage* storage,
                              CvSeqWriter* writer );

/* Closes sequence writer, updates sequence header and returns pointer
   to the resultant sequence
   (which may be useful if the sequence was created using cvStartWriteSeq))
*/
CVAPI(CvSeq*)  cvEndWriteSeq( CvSeqWriter* writer );


/* Updates sequence header. May be useful to get access to some of previously
   written elements via cvGetSeqElem or sequence reader */
CVAPI(void)   cvFlushSeqWriter( CvSeqWriter* writer );


/* Initializes sequence reader.
   The sequence can be read in forward or backward direction */
CVAPI(void) cvStartReadSeq( const CvSeq* seq, CvSeqReader* reader,
                           int reverse CV_DEFAULT(0) );




/* Changes sequence reader position. It may seek to an absolute or
   to relative to the current position */
CVAPI(void)   cvSetSeqReaderPos( CvSeqReader* reader, int index,
                                 int is_relative CV_DEFAULT(0));


/* Creates sequence header for array.
   After that all the operations on sequences that do not alter the content
   can be applied to the resultant sequence */
CVAPI(CvSeq*) cvMakeSeqHeaderForArray( int seq_type, int header_size,
                                       int elem_size, void* elements, int total,
                                       CvSeq* seq, CvSeqBlock* block );



/************ Internal sequence functions ************/
CVAPI(void)  cvChangeSeqBlock( void* reader, int direction );
CVAPI(void)  cvCreateSeqBlock( CvSeqWriter* writer );


/* Creates a new set */
CVAPI(CvSet*)  cvCreateSet( int set_flags, int header_size,
                            int elem_size, CvMemStorage* storage );

/* Adds new element to the set and returns pointer to it */
CVAPI(int)  cvSetAdd( CvSet* set_header, CvSetElem* elem CV_DEFAULT(NULL),
                      CvSetElem** inserted_elem CV_DEFAULT(NULL) );


/* Inserts sequence into tree with specified "parent" sequence.
   If parent is equal to frame (e.g. the most external contour),
   then added contour will have null pointer to parent. */
CVAPI(void) cvInsertNodeIntoTree( void* node, void* parent, void* frame );


/****************************************************************************************\
*                                    System functions                                    *
\****************************************************************************************/


/* Get current OpenCV error status */
CVAPI(int) cvGetErrStatus( void );


#define CV_ErrModeLeaf     0   /* Print error and exit program */
#define CV_ErrModeParent   1   /* Print error and continue */
#define CV_ErrModeSilent   2   /* Don't print and continue */

/* Retrives current error processing mode */
CVAPI(int)  cvGetErrMode( void );


/* Sets error status and performs some additonal actions (displaying message box,
   writing message to stderr, terminating application etc.)
   depending on the current error mode */
CVAPI(void) cvError( int status, const char* func_name,
                    const char* err_msg, const char* file_name, int line );

/* Retrieves textual description of the error given its code */
CVAPI(const char*) cvErrorStr( int status );



/* Maps IPP error codes to the counterparts from OpenCV */
CVAPI(int) cvErrorFromIppStatus( int ipp_status );

typedef int (CV_CDECL *CvErrorCallback)( int status, const char* func_name,
                    const char* err_msg, const char* file_name, int line, void* userdata );



CVAPI(int) cvStdErrReport( int status, const char* func_name, const char* err_msg,
                          const char* file_name, int line, void* userdata );


typedef void* (CV_CDECL *CvAllocFunc)(size_t size, void* userdata);
typedef int (CV_CDECL *CvFreeFunc)(void* pptr, void* userdata);




typedef IplImage* (CV_STDCALL* Cv_iplCreateImageHeader)
                            (int,int,int,char*,char*,int,int,int,int,int,
                            IplROI*,IplImage*,void*,IplTileInfo*);
typedef void (CV_STDCALL* Cv_iplAllocateImageData)(IplImage*,int,int);
typedef void (CV_STDCALL* Cv_iplDeallocate)(IplImage*,int);
typedef IplROI* (CV_STDCALL* Cv_iplCreateROI)(int,int,int,int,int);
typedef IplImage* (CV_STDCALL* Cv_iplCloneImage)(const IplImage*);




#endif /*_CXCORE_H_ */

/****************************************************************************************\
*                              Contours retrieving                                       *
\****************************************************************************************/

/* Retrieves outer and optionally inner boundaries of white (non-zero) connected
   components in the black (zero) background */
CVAPI(int)  cvFindContours( CvArr* image, CvMemStorage* storage, CvSeq** first_contour,
                            int header_size CV_DEFAULT(sizeof(CvContour)),
                            int mode CV_DEFAULT(CV_RETR_LIST),
                            int method CV_DEFAULT(CV_CHAIN_APPROX_SIMPLE),
                            CvPoint offset CV_DEFAULT(cvPoint(0,0)));


/* Initalizes contour retrieving process.
Calls cvStartFindContours.
Calls cvFindNextContour until null pointer is returned
or some other condition becomes true.
Calls cvEndFindContours at the end. */
CVAPI(CvContourScanner)  cvStartFindContours( CvArr* image, CvMemStorage* storage,
                            int header_size CV_DEFAULT(sizeof(CvContour)),
                            int mode CV_DEFAULT(CV_RETR_LIST),
                            int method CV_DEFAULT(CV_CHAIN_APPROX_SIMPLE),
                            CvPoint offset CV_DEFAULT(cvPoint(0,0)));

/* Retrieves next contour */
CVAPI(CvSeq*)  cvFindNextContour( CvContourScanner scanner );




/* Releases contour scanner and returns pointer to the first outer contour */
CVAPI(CvSeq*)  cvEndFindContours( CvContourScanner* scanner );



/* Initalizes Freeman chain reader.
   The reader is used to iteratively get coordinates of all the chain points.
   If the Freeman codes should be read as is, a simple sequence reader should be used */
CVAPI(void) cvStartReadChainPoints( CvChain* chain, CvChainPtReader* reader );


// them vao
/* curvature: 0 - 1-curvature, 1 - k-cosine curvature. */
CvStatus  icvApproximateChainTC89( CvChain*      chain,
                                   int header_size,
                                   CvMemStorage* storage,
                                   CvSeq**   contour,
                                   int method );

/* Calculates contour boundning rectangle (update=1) or
   just retrieves pre-calculated rectangle (update=0) */
CVAPI(CvRect)  cvBoundingRect( CvArr* points, int update CV_DEFAULT(0) );

/* Calculates area of a contour or contour segment */
CVAPI(double)  cvContourArea( const CvArr* contour,
                              CvSlice slice CV_DEFAULT(CV_WHOLE_SEQ));


/* Initializes sequence header for a matrix (column or row vector) of points -
   a wrapper for cvMakeSeqHeaderForArray (it does not initialize bounding rectangle!!!) */
CVAPI(CvSeq*) cvPointSeqFromMat( int seq_kind, const CvArr* mat,
                                 CvContour* contour_header,
                                 CvSeqBlock* block );

/****************************************************************************************\
*                                  Histogram functions                                   *
\****************************************************************************************/

#define CV_THRESH_BINARY      0  /* value = value > threshold ? max_value : 0       */

/* End of file. */
