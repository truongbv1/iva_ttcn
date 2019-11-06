// FFmpeg
#include <libavutil/timestamp.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avio.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

// image processing
#include "cxcore.h"
#include "cxtypes.h"

// list of rectangles
typedef struct list
{
    struct list *prev;
    struct list *next;
    int id;
    int status;
    int trackLevel;
    int crossLevel;
    int angle;
    CvRect rect;
} list;

extern list *rects_sk; //list for socket

int get_size(list *Rects);
void print_list_rect(list *Rects);
void push(list **Rects, CvRect rect);
int remove_by_index(list **Rects, int index);
void release(list **Rects);
CvRect rect_max(CvRect rect1, CvRect rect2);
void connect_nearby_rects(list **Rects, int delta_w, int delta_h);

/********************* motion detection ******************
*
*********************************************************/
int data_stream(AVFormatContext **infoContext, char *devName);
void resize_image(IplImage *img, IplImage *img_resized);
void create_background(IplImage *currentFrame, IplImage *background);
void find_foreground(IplImage *grayImage, IplImage *grayBackground, IplImage *foreground, int valThresh, int learningRate);
int motion_detect(IplImage *grayImage, IplImage *grayBackground, list **listTrack, int learningRate, int valThresh, int delta_w, int delta_h, int area_min);

/********************** object tracking ******************
*
*********************************************************/
int find_object_direction(CvPoint p1, CvPoint p2);
double overlap_box(CvRect box1, CvRect box2);
int object_tracking(list *rects, list **listTrack);

/********************** line crossing ********************
* 
*********************************************************/
void get_minmax_line(CvPoint *p1, CvPoint *p2, CvPoint *pminLine, CvPoint *pmaxLine);
void direction_line_crossing(CvPoint *p1, CvPoint *p2);
//int setup_line_input(char* filename, CvPoint* p1, CvPoint* p2, CvPoint* pminLine, CvPoint* pmaxLine, int options);
void line_crossing(CvPoint p1, CvPoint p2, CvPoint pminLine, CvPoint pmaxLine, list *listTrack, int options);

/******************* intrusion detection ****************
*
********************************************************/
int point_polygon_test(int nvert, int *vertx, int *verty, int testx, int testy);
int intrusion(list *rects, int nvert, int *vertx, int *verty);
