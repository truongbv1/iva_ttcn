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

static struct
{
    Cv_iplCreateImageHeader  createHeader;
    Cv_iplAllocateImageData  allocateData;
    Cv_iplDeallocate  deallocate;
    Cv_iplCreateROI  createROI;
    Cv_iplCloneImage  cloneImage;
}
CvIPL;



/****************************************************************************************\
*                               CvMat creation and basic operations                      *
\****************************************************************************************/




static void icvCheckHuge( CvMat* arr )
{
    if( (int64)arr->step*arr->rows > INT_MAX )
        arr->type &= ~CV_MAT_CONT_FLAG;
}




// Initializes CvMat header, allocated by the user
CV_IMPL CvMat*
cvInitMatHeader( CvMat* arr, int rows, int cols,
                 int type, void* data, int step )
{
    CV_FUNCNAME( "cvInitMatHeader" );
    
    __BEGIN__;

    int mask, pix_size, min_step;

    if( !arr )
        CV_ERROR_FROM_CODE( CV_StsNullPtr );

    if( (unsigned)CV_MAT_DEPTH(type) > CV_DEPTH_MAX )
        CV_ERROR_FROM_CODE( CV_BadNumChannels );

    if( rows <= 0 || cols <= 0 )
        CV_ERROR( CV_StsBadSize, "Non-positive cols or rows" );
 
    type = CV_MAT_TYPE( type );
    arr->type = type | CV_MAT_MAGIC_VAL;
    arr->rows = rows;
    arr->cols = cols;
    arr->data.ptr = (uchar*)data;
    arr->refcount = 0;
    arr->hdr_refcount = 0;

    mask = (arr->rows <= 1) - 1;
    pix_size = CV_ELEM_SIZE(type);
    min_step = arr->cols*pix_size & mask;

    if( step != CV_AUTOSTEP && step != 0 )
    {
        if( step < min_step )
            CV_ERROR_FROM_CODE( CV_BadStep );
        arr->step = step & mask;
    }
    else
    {
        arr->step = min_step;
    }

    arr->type = CV_MAT_MAGIC_VAL | type |
                (arr->step == min_step ? CV_MAT_CONT_FLAG : 0);

    icvCheckHuge( arr );

    __END__;

    return arr;
}




/****************************************************************************************\
*                          Common for multiple array types operations                    *
\****************************************************************************************/

// Allocates underlying array data
CV_IMPL void
cvCreateData( CvArr* arr )
{
    CV_FUNCNAME( "cvCreateData" );
    
    __BEGIN__;

    if( CV_IS_MAT_HDR( arr ))
    {
        size_t step, total_size;
        CvMat* mat = (CvMat*)arr;
        step = mat->step;

        if( mat->data.ptr != 0 )
            CV_ERROR( CV_StsError, "Data is already allocated" );

        if( step == 0 )
            step = CV_ELEM_SIZE(mat->type)*mat->cols;

        total_size = step*mat->rows + sizeof(int) + CV_MALLOC_ALIGN;
        CV_CALL( mat->refcount = (int*)cvAlloc( (size_t)total_size ));
        mat->data.ptr = (uchar*)cvAlignPtr( mat->refcount + 1, CV_MALLOC_ALIGN );
        *mat->refcount = 1;
    }
    else if( CV_IS_IMAGE_HDR(arr))
    {
        IplImage* img = (IplImage*)arr;

        if( img->imageData != 0 )
            CV_ERROR( CV_StsError, "Data is already allocated" );

        if( !CvIPL.allocateData )
        {
            CV_CALL( img->imageData = img->imageDataOrigin = 
                        (char*)cvAlloc( (size_t)img->imageSize ));
        }
        else
        {
            int depth = img->depth;
            int width = img->width;

            if( img->depth == IPL_DEPTH_32F || img->nChannels == 64 )
            {
                img->width *= img->depth == IPL_DEPTH_32F ? sizeof(float) : sizeof(double);
                img->depth = IPL_DEPTH_8U;
            }

            CvIPL.allocateData( img, 0, 0 );

            img->width = width;
            img->depth = depth;
        }
    }
    else if( CV_IS_MATND_HDR( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;
        int i;
        size_t total_size = CV_ELEM_SIZE(mat->type);

        if( mat->data.ptr != 0 )
            CV_ERROR( CV_StsError, "Data is already allocated" );

        if( CV_IS_MAT_CONT( mat->type ))
        {
            total_size = (size_t)mat->dim[0].size*(mat->dim[0].step != 0 ?
                         mat->dim[0].step : total_size);
        }
        else
        {
            for( i = mat->dims - 1; i >= 0; i-- )
            {
                size_t size = (size_t)mat->dim[i].step*mat->dim[i].size;

                if( total_size < size )
                    total_size = size;
            }
        }
        
        CV_CALL( mat->refcount = (int*)cvAlloc( total_size +
                                        sizeof(int) + CV_MALLOC_ALIGN ));
        mat->data.ptr = (uchar*)cvAlignPtr( mat->refcount + 1, CV_MALLOC_ALIGN );
        *mat->refcount = 1;
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;
}





// Deallocates array's data
CV_IMPL void
cvReleaseData( CvArr* arr )
{
    CV_FUNCNAME( "cvReleaseData" );
    
    __BEGIN__;

    if( CV_IS_MAT_HDR( arr ) || CV_IS_MATND_HDR( arr ))
    {
        CvMat* mat = (CvMat*)arr;
        cvDecRefData( mat );
    }
    else if( CV_IS_IMAGE_HDR( arr ))
    {
        IplImage* img = (IplImage*)arr;

        if( !CvIPL.deallocate )
        {
            char* ptr = img->imageDataOrigin;
            img->imageData = img->imageDataOrigin = 0;
            cvFree( &ptr );
        }
        else
        {
            CvIPL.deallocate( img, IPL_IMAGE_DATA );
        }
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;
}






/****************************************************************************************\
*                             Conversion to CvMat or IplImage                            *
\****************************************************************************************/

// convert array (CvMat or IplImage) to CvMat
CV_IMPL CvMat*
cvGetMat( const CvArr* array, CvMat* mat,
          int* pCOI, int allowND )
{
    CvMat* result = 0;
    CvMat* src = (CvMat*)array;
    int coi = 0;
    
    CV_FUNCNAME( "cvGetMat" );

    __BEGIN__;

    if( !mat || !src )
        CV_ERROR( CV_StsNullPtr, "NULL array pointer is passed" );

    if( CV_IS_MAT_HDR(src))
    {
        if( !src->data.ptr )
            CV_ERROR( CV_StsNullPtr, "The matrix has NULL data pointer" );
        
        result = (CvMat*)src;
    }
    else if( CV_IS_IMAGE_HDR(src) )
    {
        const IplImage* img = (const IplImage*)src;
        int depth, order;

        if( img->imageData == 0 )
            CV_ERROR( CV_StsNullPtr, "The image has NULL data pointer" );

        depth = icvIplToCvDepth( img->depth );
        if( depth < 0 )
            CV_ERROR_FROM_CODE( CV_BadDepth );

        order = img->dataOrder & (img->nChannels > 1 ? -1 : 0);

        if( img->roi )
        {
            if( order == IPL_DATA_ORDER_PLANE )
            {
                int type = depth;

                if( img->roi->coi == 0 )
                    CV_ERROR( CV_StsBadFlag,
                    "Images with planar data layout should be used with COI selected" );

                CV_CALL( cvInitMatHeader( mat, img->roi->height,
                                   img->roi->width, type,
                                   img->imageData + (img->roi->coi-1)*img->imageSize +
                                   img->roi->yOffset*img->widthStep +
                                   img->roi->xOffset*CV_ELEM_SIZE(type),
                                   img->widthStep ));
            }
            else /* pixel order */
            {
                int type = CV_MAKETYPE( depth, img->nChannels );
                coi = img->roi->coi;

                if( img->nChannels > CV_CN_MAX )
                    CV_ERROR( CV_BadNumChannels,
                        "The image is interleaved and has over CV_CN_MAX channels" );

                CV_CALL( cvInitMatHeader( mat, img->roi->height, img->roi->width,
                                          type, img->imageData +
                                          img->roi->yOffset*img->widthStep +
                                          img->roi->xOffset*CV_ELEM_SIZE(type),
                                          img->widthStep ));
            }
        }
        else
        {
            int type = CV_MAKETYPE( depth, img->nChannels );

            if( order != IPL_DATA_ORDER_PIXEL )
                CV_ERROR( CV_StsBadFlag, "Pixel order should be used with coi == 0" );

            CV_CALL( cvInitMatHeader( mat, img->height, img->width, type,
                                      img->imageData, img->widthStep ));
        }

        result = mat;
    }
    else if( allowND && CV_IS_MATND_HDR(src) )
    {
        CvMatND* matnd = (CvMatND*)src;
        int i;
        int size1 = matnd->dim[0].size, size2 = 1;
        
        if( !src->data.ptr )
            CV_ERROR( CV_StsNullPtr, "Input array has NULL data pointer" );

        if( !CV_IS_MAT_CONT( matnd->type ))
            CV_ERROR( CV_StsBadArg, "Only continuous nD arrays are supported here" );

        if( matnd->dims > 2 )
            for( i = 1; i < matnd->dims; i++ )
                size2 *= matnd->dim[i].size;
        else
            size2 = matnd->dims == 1 ? 1 : matnd->dim[1].size;

        mat->refcount = 0;
        mat->hdr_refcount = 0;
        mat->data.ptr = matnd->data.ptr;
        mat->rows = size1;
        mat->cols = size2;
        mat->type = CV_MAT_TYPE(matnd->type) | CV_MAT_MAGIC_VAL | CV_MAT_CONT_FLAG;
        mat->step = size2*CV_ELEM_SIZE(matnd->type);
        mat->step &= size1 > 1 ? -1 : 0;

        icvCheckHuge( mat );
        result = mat;
    }
    else
    {
        CV_ERROR( CV_StsBadFlag, "Unrecognized or unsupported array type" );
    }

    __END__;

    if( pCOI )
        *pCOI = coi;

    return result;
}


/****************************************************************************************\
*                               IplImage-specific functions                              *
\****************************************************************************************/



static  void
icvGetColorModel( int nchannels, const char** colorModel, const char** channelSeq )
{
    static const char* tab[][2] =
    {
        {"GRAY", "GRAY"},
        {"",""},
        {"RGB","BGR"},
        {"RGB","BGRA"}
    };

    nchannels--;
    *colorModel = *channelSeq = "";

    if( (unsigned)nchannels <= 3 )
    {
        *colorModel = tab[nchannels][0];
        *channelSeq = tab[nchannels][1];
    }
}


// create IplImage header
CV_IMPL IplImage *
cvCreateImageHeader( CvSize size, int depth, int channels )
{
    IplImage *img = 0;

    CV_FUNCNAME( "cvCreateImageHeader" );

    __BEGIN__;

    if( !CvIPL.createHeader )
    {
        CV_CALL( img = (IplImage *)cvAlloc( sizeof( *img )));
        CV_CALL( cvInitImageHeader( img, size, depth, channels, IPL_ORIGIN_TL,
                                    CV_DEFAULT_IMAGE_ROW_ALIGN ));
    }
    else
    {   
        const char *colorModel, *channelSeq;

        icvGetColorModel( channels, &colorModel, &channelSeq );

        img = CvIPL.createHeader( channels, 0, depth, (char*)colorModel, (char*)channelSeq,
                                  IPL_DATA_ORDER_PIXEL, IPL_ORIGIN_TL,
                                  CV_DEFAULT_IMAGE_ROW_ALIGN,
                                  size.width, size.height, 0, 0, 0, 0 );
    }

    __END__;

    if( cvGetErrStatus() < 0 && img )
        cvReleaseImageHeader( &img );

    return img;
}


// create IplImage header and allocate underlying data
CV_IMPL IplImage *
cvCreateImage( CvSize size, int depth, int channels )
{
    IplImage *img = 0;

    CV_FUNCNAME( "cvCreateImage" );

    __BEGIN__;

    CV_CALL( img = cvCreateImageHeader( size, depth, channels ));
    assert( img );
    CV_CALL( cvCreateData( img ));

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseImage( &img );

    return img;
}


// initalize IplImage header, allocated by the user
CV_IMPL IplImage*
cvInitImageHeader( IplImage * image, CvSize size, int depth,
                   int channels, int origin, int align )
{
    IplImage* result = 0;

    CV_FUNCNAME( "cvInitImageHeader" );

    __BEGIN__;

    const char *colorModel, *channelSeq;

    if( !image )
        CV_ERROR( CV_HeaderIsNull, "null pointer to header" );

    memset( image, 0, sizeof( *image ));
    image->nSize = sizeof( *image );

    CV_CALL( icvGetColorModel( channels, &colorModel, &channelSeq ));
    strncpy( image->colorModel, colorModel, 4 );
    strncpy( image->channelSeq, channelSeq, 4 );

    if( size.width < 0 || size.height < 0 )
        CV_ERROR( CV_BadROISize, "Bad input roi" );

    if( (depth != (int)IPL_DEPTH_1U && depth != (int)IPL_DEPTH_8U &&
         depth != (int)IPL_DEPTH_8S && depth != (int)IPL_DEPTH_16U &&
         depth != (int)IPL_DEPTH_16S && depth != (int)IPL_DEPTH_32S &&
         depth != (int)IPL_DEPTH_32F && depth != (int)IPL_DEPTH_64F) ||
         channels < 0 )
        CV_ERROR( CV_BadDepth, "Unsupported format" );
    if( origin != CV_ORIGIN_BL && origin != CV_ORIGIN_TL )
        CV_ERROR( CV_BadOrigin, "Bad input origin" );

    if( align != 4 && align != 8 )
        CV_ERROR( CV_BadAlign, "Bad input align" );

    image->width = size.width;
    image->height = size.height;

    if( image->roi )
    {
        image->roi->coi = 0;
        image->roi->xOffset = image->roi->yOffset = 0;
        image->roi->width = size.width;
        image->roi->height = size.height;
    }

    image->nChannels = MAX( channels, 1 );
    image->depth = depth;
    image->align = align;
    image->widthStep = (((image->width * image->nChannels *
         (image->depth & ~IPL_DEPTH_SIGN) + 7)/8)+ align - 1) & (~(align - 1));
    image->origin = origin;
    image->imageSize = image->widthStep * image->height;

    result = image;

    __END__;

    return result;
}


CV_IMPL void
cvReleaseImageHeader( IplImage** image )
{
    CV_FUNCNAME( "cvReleaseImageHeader" );

    __BEGIN__;

    if( !image )
        CV_ERROR( CV_StsNullPtr, "" );

    if( *image )
    {
        IplImage* img = *image;
        *image = 0;
        
        if( !CvIPL.deallocate )
        {
            cvFree( &img->roi );
            cvFree( &img );
        }
        else
        {
            CvIPL.deallocate( img, IPL_IMAGE_HEADER | IPL_IMAGE_ROI );
        }
    }
    __END__;
}


CV_IMPL void
cvReleaseImage( IplImage ** image )
{
    CV_FUNCNAME( "cvReleaseImage" );

    __BEGIN__

    if( !image )
        CV_ERROR( CV_StsNullPtr, "" );

    if( *image )
    {
        IplImage* img = *image;
        *image = 0;
        
        cvReleaseData( img );
        cvReleaseImageHeader( &img );
    }

    __END__;
}


// cxalloc.c
// default <malloc>
static void*
icvDefaultAlloc( size_t size, void* not) //fix not
{
    char *ptr, *ptr0 = (char*)malloc(
        (size_t)(size + CV_MALLOC_ALIGN*((size >= 4096) + 1) + sizeof(char*)));

    if( !ptr0 )
        return 0;

    // align the pointer
    ptr = (char*)cvAlignPtr(ptr0 + sizeof(char*) + 1, CV_MALLOC_ALIGN);
    *(char**)(ptr - sizeof(char*)) = ptr0;

    return ptr;
}


// default <free>
static int
icvDefaultFree( void* ptr, void* not)// fix not
{
    // Pointer must be aligned by CV_MALLOC_ALIGN
    if( ((size_t)ptr & (CV_MALLOC_ALIGN-1)) != 0 )
        return CV_BADARG_ERR;
    free( *((char**)ptr - 1) );

    return CV_OK;
}


// pointers to allocation functions, initially set to default
static CvAllocFunc p_cvAlloc = icvDefaultAlloc;
static CvFreeFunc p_cvFree = icvDefaultFree;
static void* p_cvAllocUserData = 0;




CV_IMPL  void*  cvAlloc( size_t size )
{
    void* ptr = 0;
    
    CV_FUNCNAME( "cvAlloc" );

    __BEGIN__;

    if( (size_t)size > CV_MAX_ALLOC_SIZE )
        CV_ERROR( CV_StsOutOfRange,
                  "Negative or too large argument of cvAlloc function" );

    ptr = p_cvAlloc( size, p_cvAllocUserData );
    if( !ptr )
        CV_ERROR( CV_StsNoMem, "Out of memory" );

    __END__;

    return ptr;
}


CV_IMPL  void  cvFree_( void* ptr )
{
    CV_FUNCNAME( "cvFree_" );

    __BEGIN__;

    if( ptr )
    {
        CVStatus status = p_cvFree( ptr, p_cvAllocUserData );
        if( status < 0 )
            CV_ERROR( status, "Deallocation error" );
    }

    __END__;
}

/* End of file. */
// cxtypes.h
CvPoint  cvPoint( int x, int y )
{
    CvPoint p;

    p.x = x;
    p.y = y;

    return p;
}

CvSize  cvSize( int width, int height )
{
    CvSize s;

    s.width = width;
    s.height = height;

    return s;
}


CvSlice  cvSlice( int start, int end )
{
    CvSlice slice;
    slice.start_index = start;
    slice.end_index = end;

    return slice;
}

CvScalar  cvScalarAll( double val0123 )
{
    CvScalar scalar;
    scalar.val[0] = val0123;
    scalar.val[1] = val0123;
    scalar.val[2] = val0123;
    scalar.val[3] = val0123;
    return scalar;
}

// cxcore.h
void  cvDecRefData( CvArr* arr )
{
    if( CV_IS_MAT( arr ))
    {
        CvMat* mat = (CvMat*)arr;
        mat->data.ptr = NULL;
        if( mat->refcount != NULL && --*mat->refcount == 0 )
            cvFree( &mat->refcount );
        mat->refcount = NULL;
    }
    else if( CV_IS_MATND( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;
        mat->data.ptr = NULL;
        if( mat->refcount != NULL && --*mat->refcount == 0 )
            cvFree( &mat->refcount );
        mat->refcount = NULL;
    }
}



// cxmisc.h

void* cvAlignPtr( const void* ptr, int align)
{
    //if(align ==0 ) align = 32;//fix
    assert( (align & (align-1)) == 0 );
    return (void*)( ((size_t)ptr + align - 1) & ~(size_t)(align-1) );
}

int cvAlign( int size, int align )
{
    assert( (align & (align-1)) == 0 && size < INT_MAX );
    return (size + align - 1) & -align;
}

CvSize  cvGetMatSize( const CvMat* mat )
{
    CvSize size = { mat->cols, mat->rows };//fix 
    return size;
}

/* End of file. */
