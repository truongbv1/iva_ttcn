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

#ifndef _CXCORE_TYPES_H_
#define _CXCORE_TYPES_H_

#if !defined _CRT_SECURE_NO_DEPRECATE && _MSC_VER > 1300
#define _CRT_SECURE_NO_DEPRECATE /* to avoid multiple Visual Studio 2005 warnings */
#endif

#ifndef SKIP_INCLUDES
  #include <assert.h>
  #include <stdlib.h>
  #include <string.h>
  #include <float.h>

  #if defined __ICL
    #define CV_ICC   __ICL
  #elif defined __ICC
    #define CV_ICC   __ICC
  #elif defined __ECL
    #define CV_ICC   __ECL
  #elif defined __ECC
    #define CV_ICC   __ECC
  #endif

  #if defined WIN32 && (!defined WIN64 || defined EM64T) && \
      (_MSC_VER >= 1400 || defined CV_ICC) \
      || (defined __SSE2__ && defined __GNUC__ && __GNUC__ >= 4)
    #include <emmintrin.h>
    #define CV_SSE2 1
  #else
    #define CV_SSE2 0
  #endif

  #if defined __BORLANDC__
    #include <fastmath.h>
  #elif defined WIN64 && !defined EM64T && defined CV_ICC
    #include <mathimf.h>
  #else
    #include <math.h>
  #endif

  #ifdef HAVE_IPL
      #ifndef __IPL_H__
          #if defined WIN32 || defined WIN64
              #include <ipl.h>
          #else
              #include <ipl/ipl.h>
          #endif
      #endif
  #elif defined __IPL_H__
      #define HAVE_IPL
  #endif
#endif // SKIP_INCLUDES

#if defined WIN32 || defined WIN64
    #define CV_CDECL __cdecl
    #define CV_STDCALL __stdcall
#else
    #define CV_CDECL
    #define CV_STDCALL
#endif

#ifndef CV_EXTERN_C
    #ifdef __cplusplus
        #define CV_EXTERN_C extern "C"
        #define CV_DEFAULT(val) = val
    #else
        #define CV_EXTERN_C
        #define CV_DEFAULT(val)
    #endif
#endif

#ifndef CV_EXTERN_C_FUNCPTR
    #ifdef __cplusplus
        #define CV_EXTERN_C_FUNCPTR(x) extern "C" { typedef x; }
    #else
        #define CV_EXTERN_C_FUNCPTR(x) typedef x
    #endif
#endif

#ifndef CV_INLINE
#if defined __cplusplus
    #define CV_INLINE inline
#elif (defined WIN32 || defined WIN64) && !defined __GNUC__
    #define CV_INLINE __inline
#else
    #define CV_INLINE static
#endif
#endif /* CV_INLINE */

#if (defined WIN32 || defined WIN64) && defined CVAPI_EXPORTS
    #define CV_EXPORTS __declspec(dllexport)
#else
    #define CV_EXPORTS
#endif

#ifndef CVAPI
    #define CVAPI(rettype) CV_EXTERN_C CV_EXPORTS rettype CV_CDECL
#endif

// #if defined _MSC_VER || defined __BORLANDC__
// typedef __int64 int64;
// typedef unsigned __int64 uint64;
// #else
typedef long long int64;
//typedef unsigned long long uint64;
// #endif

// #ifndef HAVE_IPL
typedef unsigned char uchar;
// typedef unsigned short ushort;
// #endif

typedef signed char schar;

/* CvArr* is used to pass arbitrary
 * array-like data structures
 * into functions where the particular
 * array type is recognized at runtime:
 */
typedef void CvArr;

typedef union Cv32suf
{
    int i;
    unsigned u;
    float f;
}
Cv32suf;



/****************************************************************************************\
*                             Common macros and inline functions                         *
\****************************************************************************************/


#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif


// /* absolute value without jumps */
// #ifndef __cplusplus
// #define  CV_IABS(a)     (((a) ^ ((a) < 0 ? -1 : 0)) - ((a) < 0 ? -1 : 0))
// #else
// #define  CV_IABS(a)     abs(a)
// #endif
// #define  CV_CMP(a,b)    (((a) > (b)) - ((a) < (b)))
// #define  CV_SIGN(a)     CV_CMP((a),0)

// // CV_INLINE  int  cvRound( double value )
// // {
// // #if CV_SSE2
// //     __m128d t = _mm_load_sd( &value );
// //     return _mm_cvtsd_si32(t);
// // #elif defined WIN32 && !defined WIN64 && defined _MSC_VER
// //     int t;
// //     __asm
// //     {
// //         fld value;
// //         fistp t;
// //     }
// //     return t;
// // #elif (defined HAVE_LRINT) || (defined WIN64 && !defined EM64T && defined CV_ICC)
// //     return (int)lrint(value);
// // #else
//     /*
//      the algorithm was taken from Agner Fog's optimization guide
//      at http://www.agner.org/assem
//      */
// //     Cv64suf temp;
// //     temp.f = value + 6755399441055744.0;
// //     return (int)temp.u;
// // #endif
// // }


// // CV_INLINE  int  cvFloor( double value )
// // {
// // #if CV_SSE2
// //     __m128d t = _mm_load_sd( &value );
// //     int i = _mm_cvtsd_si32(t);
// //     return i - _mm_movemask_pd(_mm_cmplt_sd(t,_mm_cvtsi32_sd(t,i)));
// // #else
// //     int temp = cvRound(value);
// //     Cv32suf diff;
// //     diff.f = (float)(value - temp);
// //     return temp - (diff.i < 0);
// // #endif
// // }



/****************************************************************************************\
*                                  Image type (IplImage)                                 *
\****************************************************************************************/

// #ifndef HAVE_IPL

/*
 * The following definitions (until #endif)
 * is an extract from IPL headers.
 * Copyright (c) 1995 Intel Corporation.
 */
#define IPL_DEPTH_SIGN 0x80000000

#define IPL_DEPTH_1U     1
#define IPL_DEPTH_8U     8
#define IPL_DEPTH_16U   16
#define IPL_DEPTH_32F   32

#define IPL_DEPTH_8S  (IPL_DEPTH_SIGN| 8)
#define IPL_DEPTH_16S (IPL_DEPTH_SIGN|16)
#define IPL_DEPTH_32S (IPL_DEPTH_SIGN|32)

#define IPL_DATA_ORDER_PIXEL  0
#define IPL_DATA_ORDER_PLANE  1

#define IPL_ORIGIN_TL 0
//#define IPL_ORIGIN_BL 1



typedef struct _IplImage
{
    int  nSize;             /* sizeof(IplImage) */
    int  ID;                /* version (=0)*/
    int  nChannels;         /* Most of OpenCV functions support 1,2,3 or 4 channels */
    int  alphaChannel;      /* Ignored by OpenCV */
    int  depth;             /* Pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16S,
                               IPL_DEPTH_32S, IPL_DEPTH_32F and IPL_DEPTH_64F are supported.  */
    char colorModel[4];     /* Ignored by OpenCV */
    char channelSeq[4];     /* ditto */
    int  dataOrder;         /* 0 - interleaved color channels, 1 - separate color channels.
                               cvCreateImage can only create interleaved images */
    int  origin;            /* 0 - top-left origin,
                               1 - bottom-left origin (Windows bitmaps style).  */
    int  align;             /* Alignment of image rows (4 or 8).
                               OpenCV ignores it and uses widthStep instead.    */
    int  width;             /* Image width in pixels.                           */
    int  height;            /* Image height in pixels.                          */
    struct _IplROI *roi;    /* Image ROI. If NULL, the whole image is selected. */
    struct _IplImage *maskROI;      /* Must be NULL. */
    void  *imageId;                 /* "           " */
    struct _IplTileInfo *tileInfo;  /* "           " */
    int  imageSize;         /* Image data size in bytes
                               (==image->height*image->widthStep
                               in case of interleaved data)*/
    char *imageData;        /* Pointer to aligned image data.         */
    int  widthStep;         /* Size of aligned image row in bytes.    */
    int  BorderMode[4];     /* Ignored by OpenCV.                     */
    int  BorderConst[4];    /* Ditto.                                 */
    char *imageDataOrigin;  /* Pointer to very origin of image data
                               (not necessarily aligned) -
                               needed for correct deallocation */
}
IplImage;

typedef struct _IplTileInfo IplTileInfo;

typedef struct _IplROI
{
    int  coi; /* 0 - no COI (all channels are selected), 1 - 0th channel is selected ...*/
    int  xOffset;
    int  yOffset;
    int  width;
    int  height;
}
IplROI;



#define IPL_IMAGE_HEADER 1
#define IPL_IMAGE_DATA   2
#define IPL_IMAGE_ROI    4

// #endif /*HAVE_IPL*/



#define CV_IS_IMAGE_HDR(img) \
    ((img) != NULL && ((const IplImage*)(img))->nSize == sizeof(IplImage))



/* for storing double-precision
   floating point data in IplImage's */
#define IPL_DEPTH_64F  64



/****************************************************************************************\
*                                  Matrix type (CvMat)                                   *
\****************************************************************************************/

#define CV_CN_MAX     64
#define CV_CN_SHIFT   3
#define CV_DEPTH_MAX  (1 << CV_CN_SHIFT)

#define CV_8U   0
#define CV_8S   1
#define CV_16U  2
#define CV_16S  3
#define CV_32S  4
#define CV_32F  5
#define CV_64F  6
#define CV_USRTYPE1 7

#define CV_MAKETYPE(depth,cn) ((depth) + (((cn)-1) << CV_CN_SHIFT))
#define CV_MAKE_TYPE CV_MAKETYPE

#define CV_8UC1 CV_MAKETYPE(CV_8U,1)
// #define CV_8UC2 CV_MAKETYPE(CV_8U,2)
// #define CV_8UC3 CV_MAKETYPE(CV_8U,3)
// #define CV_8UC4 CV_MAKETYPE(CV_8U,4)
// #define CV_8UC(n) CV_MAKETYPE(CV_8U,(n))

#define CV_8SC1 CV_MAKETYPE(CV_8S,1)
// #define CV_8SC2 CV_MAKETYPE(CV_8S,2)
// #define CV_8SC3 CV_MAKETYPE(CV_8S,3)
// #define CV_8SC4 CV_MAKETYPE(CV_8S,4)
// #define CV_8SC(n) CV_MAKETYPE(CV_8S,(n))

// #define CV_16UC1 CV_MAKETYPE(CV_16U,1)
// #define CV_16UC2 CV_MAKETYPE(CV_16U,2)
// #define CV_16UC3 CV_MAKETYPE(CV_16U,3)
// #define CV_16UC4 CV_MAKETYPE(CV_16U,4)
// #define CV_16UC(n) CV_MAKETYPE(CV_16U,(n))

// #define CV_16SC1 CV_MAKETYPE(CV_16S,1)
// #define CV_16SC2 CV_MAKETYPE(CV_16S,2)
// #define CV_16SC3 CV_MAKETYPE(CV_16S,3)
// #define CV_16SC4 CV_MAKETYPE(CV_16S,4)
// #define CV_16SC(n) CV_MAKETYPE(CV_16S,(n))

// #define CV_32SC1 CV_MAKETYPE(CV_32S,1)
#define CV_32SC2 CV_MAKETYPE(CV_32S,2)
// #define CV_32SC3 CV_MAKETYPE(CV_32S,3)
// #define CV_32SC4 CV_MAKETYPE(CV_32S,4)
// #define CV_32SC(n) CV_MAKETYPE(CV_32S,(n))

// #define CV_32FC1 CV_MAKETYPE(CV_32F,1)
#define CV_32FC2 CV_MAKETYPE(CV_32F,2)
// #define CV_32FC3 CV_MAKETYPE(CV_32F,3)
// #define CV_32FC4 CV_MAKETYPE(CV_32F,4)
// #define CV_32FC(n) CV_MAKETYPE(CV_32F,(n))

// #define CV_64FC1 CV_MAKETYPE(CV_64F,1)
// #define CV_64FC2 CV_MAKETYPE(CV_64F,2)
// #define CV_64FC3 CV_MAKETYPE(CV_64F,3)
// #define CV_64FC4 CV_MAKETYPE(CV_64F,4)
// #define CV_64FC(n) CV_MAKETYPE(CV_64F,(n))

// #define CV_AUTO_STEP  0x7fffffff
// #define CV_WHOLE_ARR  cvSlice( 0, 0x3fffffff )

#define CV_MAT_CN_MASK          ((CV_CN_MAX - 1) << CV_CN_SHIFT)
#define CV_MAT_CN(flags)        ((((flags) & CV_MAT_CN_MASK) >> CV_CN_SHIFT) + 1)
#define CV_MAT_DEPTH_MASK       (CV_DEPTH_MAX - 1)
#define CV_MAT_DEPTH(flags)     ((flags) & CV_MAT_DEPTH_MASK)
#define CV_MAT_TYPE_MASK        (CV_DEPTH_MAX*CV_CN_MAX - 1)
#define CV_MAT_TYPE(flags)      ((flags) & CV_MAT_TYPE_MASK)
#define CV_MAT_CONT_FLAG_SHIFT  14
#define CV_MAT_CONT_FLAG        (1 << CV_MAT_CONT_FLAG_SHIFT)
#define CV_IS_MAT_CONT(flags)   ((flags) & CV_MAT_CONT_FLAG)
// #define CV_IS_CONT_MAT          CV_IS_MAT_CONT
// #define CV_MAT_TEMP_FLAG_SHIFT  15
// #define CV_MAT_TEMP_FLAG        (1 << CV_MAT_TEMP_FLAG_SHIFT)
// #define CV_IS_TEMP_MAT(flags)   ((flags) & CV_MAT_TEMP_FLAG)

#define CV_MAGIC_MASK       0xFFFF0000
#define CV_MAT_MAGIC_VAL    0x42420000
// #define CV_TYPE_NAME_MAT    "opencv-matrix"

typedef struct CvMat
{
    int type;
    int step;

    /* for internal use only */
    int* refcount;
    int hdr_refcount;

    union
    {
        uchar* ptr;
        short* s;
        int* i;
        float* fl;
        double* db;
    } data;

// #ifdef __cplusplus
//     union
//     {
//         int rows;
//         int height;
//     };

//     union
//     {
//         int cols;
//         int width;
//     };
// #else
    int rows;
    int cols;
//#endif

}
CvMat;


#define CV_IS_MAT_HDR(mat) \
    ((mat) != NULL && \
    (((const CvMat*)(mat))->type & CV_MAGIC_MASK) == CV_MAT_MAGIC_VAL && \
    ((const CvMat*)(mat))->cols > 0 && ((const CvMat*)(mat))->rows > 0)

#define CV_IS_MAT(mat) \
    (CV_IS_MAT_HDR(mat) && ((const CvMat*)(mat))->data.ptr != NULL)

#define CV_IS_MASK_ARR(mat) \
    (((mat)->type & (CV_MAT_TYPE_MASK & ~CV_8SC1)) == 0)


/* 0x3a50 = 11 10 10 01 01 00 00 ~ array of log2(sizeof(arr_type_elem)) */
#define CV_ELEM_SIZE(type) \
    (CV_MAT_CN(type) << ((((sizeof(size_t)/4+1)*16384|0x3a50) >> CV_MAT_DEPTH(type)*2) & 3))




/****************************************************************************************\
*                       Multi-dimensional dense array (CvMatND)                          *
\****************************************************************************************/

#define CV_MATND_MAGIC_VAL    0x42430000
// #define CV_TYPE_NAME_MATND    "opencv-nd-matrix"

#define CV_MAX_DIM            32
//#define CV_MAX_DIM_HEAP       (1 << 16)

typedef struct CvMatND
{
    int type;
    int dims;

    int* refcount;
    int hdr_refcount;

    union
    {
        uchar* ptr;
        float* fl;
        double* db;
        int* i;
        short* s;
    } data;

    struct
    {
        int size;
        int step;
    }
    dim[CV_MAX_DIM];
}
CvMatND;

#define CV_IS_MATND_HDR(mat) \
    ((mat) != NULL && (((const CvMatND*)(mat))->type & CV_MAGIC_MASK) == CV_MATND_MAGIC_VAL)

#define CV_IS_MATND(mat) \
    (CV_IS_MATND_HDR(mat) && ((const CvMatND*)(mat))->data.ptr != NULL)




/****************************************************************************************\
*                      Other supplementary data type definitions                         *
\****************************************************************************************/

/*************************************** CvRect *****************************************/

typedef struct CvRect
{
    int x;
    int y;
    int width;
    int height;
}
CvRect;



/******************************* CvPoint and variants ***********************************/

typedef struct CvPoint
{
    int x;
    int y;
}
CvPoint;



// CV_INLINE  
CvPoint  cvPoint( int x, int y );
// {
//     CvPoint p;

//     p.x = x;
//     p.y = y;

//     return p;
// }


typedef struct CvPoint2D32f
{
    float x;
    float y;
}
CvPoint2D32f;





/******************************** CvSize's & CvBox **************************************/

typedef struct
{
    int width;
    int height;
}
CvSize;

//CV_INLINE  
CvSize  cvSize( int width, int height );
// {
//     CvSize s;

//     s.width = width;
//     s.height = height;

//     return s;
// }




/************************************* CvSlice ******************************************/

typedef struct CvSlice
{
    int  start_index, end_index;
}
CvSlice;

//CV_INLINE  
CvSlice  cvSlice( int start, int end );
// {
//     CvSlice slice;
//     slice.start_index = start;
//     slice.end_index = end;

//     return slice;
// }

#define CV_WHOLE_SEQ_END_INDEX 0x3fffffff
#define CV_WHOLE_SEQ  cvSlice(0, CV_WHOLE_SEQ_END_INDEX)


/************************************* CvScalar *****************************************/

typedef struct CvScalar
{
    double val[4];
}
CvScalar;



//CV_INLINE  
CvScalar  cvScalarAll( double val0123 );
// {
//     CvScalar scalar;
//     scalar.val[0] = val0123;
//     scalar.val[1] = val0123;
//     scalar.val[2] = val0123;
//     scalar.val[3] = val0123;
//     return scalar;
// }

/****************************************************************************************\
*                                   Dynamic Data structures                              *
\****************************************************************************************/

/******************************** Memory storage ****************************************/

typedef struct CvMemBlock
{
    struct CvMemBlock*  prev;
    struct CvMemBlock*  next;
}
CvMemBlock;

#define CV_STORAGE_MAGIC_VAL    0x42890000

typedef struct CvMemStorage
{
    int signature;
    CvMemBlock* bottom;           /* First allocated block.                   */
    CvMemBlock* top;              /* Current memory block - top of the stack. */
    struct  CvMemStorage* parent; /* We get new blocks from parent as needed. */
    int block_size;               /* Block size.                              */
    int free_space;               /* Remaining free space in current block.   */
}
CvMemStorage;


typedef struct CvMemStoragePos
{
    CvMemBlock* top;
    int free_space;
}
CvMemStoragePos;


/*********************************** Sequence *******************************************/

typedef struct CvSeqBlock
{
    struct CvSeqBlock*  prev; /* Previous sequence block.                   */
    struct CvSeqBlock*  next; /* Next sequence block.                       */
    int    start_index;       /* Index of the first element in the block +  */
                              /* sequence->first->start_index.              */
    int    count;             /* Number of elements in the block.           */
    schar* data;              /* Pointer to the first element of the block. */
}
CvSeqBlock;


#define CV_TREE_NODE_FIELDS(node_type)                               \
    int       flags;             /* Miscellaneous flags.     */      \
    int       header_size;       /* Size of sequence header. */      \
    struct    node_type* h_prev; /* Previous sequence.       */      \
    struct    node_type* h_next; /* Next sequence.           */      \
    struct    node_type* v_prev; /* 2nd previous sequence.   */      \
    struct    node_type* v_next  /* 2nd next sequence.       */

/*
   Read/Write sequence.
   Elements can be dynamically inserted to or deleted from the sequence.
*/
#define CV_SEQUENCE_FIELDS()                                              \
    CV_TREE_NODE_FIELDS(CvSeq);                                           \
    int       total;          /* Total number of elements.            */  \
    int       elem_size;      /* Size of sequence element in bytes.   */  \
    schar*    block_max;      /* Maximal bound of the last block.     */  \
    schar*    ptr;            /* Current write pointer.               */  \
    int       delta_elems;    /* Grow seq this many at a time.        */  \
    CvMemStorage* storage;    /* Where the seq is stored.             */  \
    CvSeqBlock* free_blocks;  /* Free blocks list.                    */  \
    CvSeqBlock* first;        /* Pointer to the first sequence block. */

typedef struct CvSeq
{
    CV_SEQUENCE_FIELDS()
}
CvSeq;

// #define CV_TYPE_NAME_SEQ             "opencv-sequence"
// #define CV_TYPE_NAME_SEQ_TREE        "opencv-sequence-tree"

/*************************************** Set ********************************************/
/*
  Set.
  Order is not preserved. There can be gaps between sequence elements.
  After the element has been inserted it stays in the same place all the time.
  The MSB(most-significant or sign bit) of the first field (flags) is 0 iff the element exists.
*/
#define CV_SET_ELEM_FIELDS(elem_type)   \
    int  flags;                         \
    struct elem_type* next_free;

typedef struct CvSetElem
{
    CV_SET_ELEM_FIELDS(CvSetElem)
}
CvSetElem;

#define CV_SET_FIELDS()      \
    CV_SEQUENCE_FIELDS()     \
    CvSetElem* free_elems;   \
    int active_count;

typedef struct CvSet
{
    CV_SET_FIELDS()
}
CvSet;


#define CV_SET_ELEM_IDX_MASK   ((1 << 26) - 1)
#define CV_SET_ELEM_FREE_FLAG  (1 << (sizeof(int)*8-1))




/*********************************** Chain/Countour *************************************/

typedef struct CvChain
{
    CV_SEQUENCE_FIELDS()
    CvPoint  origin;
}
CvChain;

#define CV_CONTOUR_FIELDS()  \
    CV_SEQUENCE_FIELDS()     \
    CvRect rect;             \
    int color;               \
    int reserved[3];

typedef struct CvContour
{
    CV_CONTOUR_FIELDS()
}
CvContour;

// typedef CvContour CvPoint2DSeq;

/****************************************************************************************\
*                                    Sequence types                                      *
\****************************************************************************************/

#define CV_SEQ_MAGIC_VAL             0x42990000

#define CV_IS_SEQ(seq) \
    ((seq) != NULL && (((CvSeq*)(seq))->flags & CV_MAGIC_MASK) == CV_SEQ_MAGIC_VAL)

#define CV_SET_MAGIC_VAL             0x42980000

#define CV_SEQ_ELTYPE_BITS           9
#define CV_SEQ_ELTYPE_MASK           ((1 << CV_SEQ_ELTYPE_BITS) - 1)

#define CV_SEQ_ELTYPE_POINT          CV_32SC2  /* (x,y) */
#define CV_SEQ_ELTYPE_CODE           CV_8UC1   /* freeman code: 0..7 */
#define CV_SEQ_ELTYPE_GENERIC        0


#define CV_SEQ_KIND_BITS        3
#define CV_SEQ_KIND_MASK        (((1 << CV_SEQ_KIND_BITS) - 1)<<CV_SEQ_ELTYPE_BITS)

/* types of sequences */
#define CV_SEQ_KIND_GENERIC     (0 << CV_SEQ_ELTYPE_BITS)
#define CV_SEQ_KIND_CURVE       (1 << CV_SEQ_ELTYPE_BITS)


#define CV_SEQ_FLAG_SHIFT       (CV_SEQ_KIND_BITS + CV_SEQ_ELTYPE_BITS)

/* flags for curves */
#define CV_SEQ_FLAG_CLOSED     (1 << CV_SEQ_FLAG_SHIFT)

#define CV_SEQ_FLAG_HOLE       (8 << CV_SEQ_FLAG_SHIFT)


#define CV_SEQ_POLYLINE        (CV_SEQ_KIND_CURVE  | CV_SEQ_ELTYPE_POINT)
#define CV_SEQ_POLYGON         (CV_SEQ_FLAG_CLOSED | CV_SEQ_POLYLINE )

/* chain-coded curves */
#define CV_SEQ_CHAIN           (CV_SEQ_KIND_CURVE  | CV_SEQ_ELTYPE_CODE)
#define CV_SEQ_CHAIN_CONTOUR   (CV_SEQ_FLAG_CLOSED | CV_SEQ_CHAIN)



#define CV_SEQ_ELTYPE( seq )   ((seq)->flags & CV_SEQ_ELTYPE_MASK)
#define CV_SEQ_KIND( seq )     ((seq)->flags & CV_SEQ_KIND_MASK )


#define CV_IS_SEQ_CLOSED( seq )     (((seq)->flags & CV_SEQ_FLAG_CLOSED) != 0)

#define CV_IS_SEQ_HOLE( seq )       (((seq)->flags & CV_SEQ_FLAG_HOLE) != 0)

/* type checking macros */
#define CV_IS_SEQ_POINT_SET( seq ) \
    ((CV_SEQ_ELTYPE(seq) == CV_32SC2 || CV_SEQ_ELTYPE(seq) == CV_32FC2))



#define CV_IS_SEQ_POLYLINE( seq )   \
    (CV_SEQ_KIND(seq) == CV_SEQ_KIND_CURVE && CV_IS_SEQ_POINT_SET(seq))

#define CV_IS_SEQ_POLYGON( seq )   \
    (CV_IS_SEQ_POLYLINE(seq) && CV_IS_SEQ_CLOSED(seq))

#define CV_IS_SEQ_CHAIN( seq )   \
    (CV_SEQ_KIND(seq) == CV_SEQ_KIND_CURVE && (seq)->elem_size == 1)



#define CV_IS_SEQ_CHAIN_CONTOUR( seq ) \
    (CV_IS_SEQ_CHAIN( seq ) && CV_IS_SEQ_CLOSED( seq ))


/****************************************************************************************/
/*                            Sequence writer & reader                                  */
/****************************************************************************************/

#define CV_SEQ_WRITER_FIELDS()                                     \
    int          header_size;                                      \
    CvSeq*       seq;        /* the sequence written */            \
    CvSeqBlock*  block;      /* current block */                   \
    schar*       ptr;        /* pointer to free space */           \
    schar*       block_min;  /* pointer to the beginning of block*/\
    schar*       block_max;  /* pointer to the end of block */

typedef struct CvSeqWriter
{
    CV_SEQ_WRITER_FIELDS()
}
CvSeqWriter;


#define CV_SEQ_READER_FIELDS()                                      \
    int          header_size;                                       \
    CvSeq*       seq;        /* sequence, beign read */             \
    CvSeqBlock*  block;      /* current block */                    \
    schar*       ptr;        /* pointer to element be read next */  \
    schar*       block_min;  /* pointer to the beginning of block */\
    schar*       block_max;  /* pointer to the end of block */      \
    int          delta_index;/* = seq->first->start_index   */      \
    schar*       prev_elem;  /* pointer to previous element */


typedef struct CvSeqReader
{
    CV_SEQ_READER_FIELDS()
}
CvSeqReader;



#define CV_WRITE_SEQ_ELEM( elem, writer )             \
{                                                     \
    assert( (writer).seq->elem_size == sizeof(elem)); \
    if( (writer).ptr >= (writer).block_max )          \
    {                                                 \
        cvCreateSeqBlock( &writer);                   \
    }                                                 \
    assert( (writer).ptr <= (writer).block_max - sizeof(elem));\
    memcpy((writer).ptr, &(elem), sizeof(elem));      \
    (writer).ptr += sizeof(elem);                     \
}


/* Move reader position forward: */
#define CV_NEXT_SEQ_ELEM( elem_size, reader )                 \
{                                                             \
    if( ((reader).ptr += (elem_size)) >= (reader).block_max ) \
    {                                                         \
        cvChangeSeqBlock( &(reader), 1 );                     \
    }                                                         \
}



/* Read element and move read position forward: */
#define CV_READ_SEQ_ELEM( elem, reader )                       \
{                                                              \
    assert( (reader).seq->elem_size == sizeof(elem));          \
    memcpy( &(elem), (reader).ptr, sizeof((elem)));            \
    CV_NEXT_SEQ_ELEM( sizeof(elem), reader )                   \
}




#define CV_READ_CHAIN_POINT( _pt, reader )                              \
{                                                                       \
    (_pt) = (reader).pt;                                                \
    if( (reader).ptr )                                                  \
    {                                                                   \
        CV_READ_SEQ_ELEM( (reader).code, (reader));                     \
        assert( ((reader).code & ~7) == 0 );                            \
        (reader).pt.x += (reader).deltas[(int)(reader).code][0];        \
        (reader).pt.y += (reader).deltas[(int)(reader).code][1];        \
    }                                                                   \
}

#endif /*_CXCORE_TYPES_H_*/




#ifndef _CVTYPES_H_
#define _CVTYPES_H_

#ifndef SKIP_INCLUDES
  #include <assert.h>
  #include <stdlib.h>
#endif

/*
Internal structure that is used for sequental retrieving contours from the image.
It supports both hierarchical and plane variants of Suzuki algorithm.
*/
typedef struct _CvContourScanner* CvContourScanner;

/* contour retrieval mode */
#define CV_RETR_EXTERNAL 0
#define CV_RETR_LIST     1


/* contour approximation method */  
#define CV_CHAIN_CODE               0
#define CV_CHAIN_APPROX_NONE        1
#define CV_CHAIN_APPROX_SIMPLE      2
#define CV_CHAIN_APPROX_TC89_L1     3
#define CV_CHAIN_APPROX_TC89_KCOS   4
#define CV_LINK_RUNS                5

/* Freeman chain reader state */
typedef struct CvChainPtReader
{
    CV_SEQ_READER_FIELDS()
    char      code;
    CvPoint   pt;
    schar     deltas[8][2];
}
CvChainPtReader;

#endif /*_CVTYPES_H_*/

/* The header is mostly for internal use and it is likely to change.
   It contains some macro definitions that are used in cxcore, cv, cvaux
   and, probably, other libraries. If you need some of this functionality,
   the safe way is to copy it into your code and rename the macros.
*/
#ifndef _CXCORE_MISC_H_
#define _CXCORE_MISC_H_



//#include <limits.h>
// #ifdef _OPENMP
// #include "omp.h"
// #endif

/****************************************************************************************\
*                              Compile-time tuning parameters                            *
\****************************************************************************************/

/* default image row align (in bytes) */
#define  CV_DEFAULT_IMAGE_ROW_ALIGN  4


/* maximum size of dynamic memory buffer.
   cvAlloc reports an error if a larger block is requested. */
#define  CV_MAX_ALLOC_SIZE    (((size_t)1 << (sizeof(size_t)*8-2)))

/* the alignment of all the allocated buffers */
#define  CV_MALLOC_ALIGN    32

/* default alignment for dynamic data strucutures, resided in storages. */
#define  CV_STRUCT_ALIGN    ((int)sizeof(double))

/* default storage block size */
#define  CV_STORAGE_BLOCK_SIZE   ((1<<16) - 128)



/****************************************************************************************\
*                                  Common declarations                                   *
\****************************************************************************************/



#define CV_IMPL CV_EXTERN_C

#define CV_DBG_BREAK() { volatile int* crashMe = 0; *crashMe = 0; }



#define  CV_ORIGIN_TL  0
#define  CV_ORIGIN_BL  1


#define  CV_TOGGLE_FLT(x) ((x)^((int)(x) < 0 ? 0x7fffffff : 0))


//CV_INLINE void* cvAlignPtr( const void* ptr, int align=32 )
// CV_INLINE 
void* cvAlignPtr( const void* ptr, int align);
// {
//     //if(align ==0 ) align = 32;//fix
//     assert( (align & (align-1)) == 0 );
//     return (void*)( ((size_t)ptr + align - 1) & ~(size_t)(align-1) );
// }

// CV_INLINE 
int cvAlign( int size, int align );
// {
//     assert( (align & (align-1)) == 0 && size < INT_MAX );
//     return (size + align - 1) & -align;
// }

// // CV_INLINE  
CvSize  cvGetMatSize( const CvMat* mat );
// // {
// //     CvSize size = { mat->cols, mat->rows };//fix 
// //     return size;
// // }

// #define  CV_DESCALE(x,n)     (((x) + (1 << ((n)-1))) >> (n))



#define CV_MEMCPY_INT( dst, src, len )                                              \
{                                                                                   \
    size_t _icv_memcpy_i_, _icv_memcpy_len_ = (len);                                \
    int* _icv_memcpy_dst_ = (int*)(dst);                                            \
    const int* _icv_memcpy_src_ = (const int*)(src);                                \
    assert( ((size_t)_icv_memcpy_src_&(sizeof(int)-1)) == 0 &&                      \
            ((size_t)_icv_memcpy_dst_&(sizeof(int)-1)) == 0 );                      \
                                                                                    \
    for(_icv_memcpy_i_=0;_icv_memcpy_i_<_icv_memcpy_len_;_icv_memcpy_i_++)          \
        _icv_memcpy_dst_[_icv_memcpy_i_] = _icv_memcpy_src_[_icv_memcpy_i_];        \
}




/****************************************************************************************\
*                     Structures and macros for integration with IPP                     *
\****************************************************************************************/

/* IPP-compatible return codes */
typedef enum CvStatus
{
    CV_BADMEMBLOCK_ERR          = -113,
    CV_INPLACE_NOT_SUPPORTED_ERR= -112,
    CV_UNMATCHED_ROI_ERR        = -111,
    CV_NOTFOUND_ERR             = -110,
    CV_BADCONVERGENCE_ERR       = -109,

    CV_BADDEPTH_ERR             = -107,
    CV_BADROI_ERR               = -106,
    CV_BADHEADER_ERR            = -105,
    CV_UNMATCHED_FORMATS_ERR    = -104,
    CV_UNSUPPORTED_COI_ERR      = -103,
    CV_UNSUPPORTED_CHANNELS_ERR = -102,
    CV_UNSUPPORTED_DEPTH_ERR    = -101,
    CV_UNSUPPORTED_FORMAT_ERR   = -100,

    CV_BADARG_ERR      = -49,  //ipp comp
    CV_NOTDEFINED_ERR  = -48,  //ipp comp

    CV_BADCHANNELS_ERR = -47,  //ipp comp
    CV_BADRANGE_ERR    = -44,  //ipp comp
    CV_BADSTEP_ERR     = -29,  //ipp comp

    CV_BADFLAG_ERR     =  -12,
    CV_DIV_BY_ZERO_ERR =  -11, //ipp comp
    CV_BADCOEF_ERR     =  -10,

    CV_BADFACTOR_ERR   =  -7,
    CV_BADPOINT_ERR    =  -6,
    CV_BADSCALE_ERR    =  -4,
    CV_OUTOFMEM_ERR    =  -3,
    CV_NULLPTR_ERR     =  -2,
    CV_BADSIZE_ERR     =  -1,
    CV_NO_ERR          =   0,
    CV_OK              =   CV_NO_ERR
}
CvStatus;

#define CV_ERROR_FROM_STATUS( result )                \
    CV_ERROR( cvErrorFromIppStatus( result ), "OpenCV function failed" )

#define IPPI_CALL( Func )                                              \
{                                                                      \
      CvStatus  ippi_call_result;                                      \
      ippi_call_result = Func;                                         \
                                                                       \
      if( ippi_call_result < 0 )                                       \
            CV_ERROR_FROM_STATUS( (ippi_call_result));                 \
}


#endif /*_CXCORE_MISC_H_*/
