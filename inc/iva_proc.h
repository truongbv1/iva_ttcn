// FFmpeg
#include <libavutil/timestamp.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avio.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

// iva processing
#include "cxcore.h"
#include "cxtypes.h"

// json
#include <stdbool.h>
#include <json-c/json.h>

//check modification : file
#include <sys/time.h>
#include <sys/stat.h>


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

typedef struct motion_detection_iva
{
  int width;
  int height;
  bool enable_md;
  bool enable_mdr;
  int delta_w;
  int delta_h;
  int area_min;
  int varThresh;
	int learningRate;
  CvRect region_detection;
} motion_detection_iva;


typedef struct line_crossing_iva
{
  int width;
  int height;
  bool enable_lc;
  int sensitivity;
  int direction;  
  int startX;
  int startY;
  int endX;
  int endY;
  int maxX;
  int maxY;
  int minX;
  int minY;
} line_crossing_iva;

typedef struct intrusion_iva
{
  int width;
  int height;
  bool enable_its;
  int sensitivity;
  int direction;
  int nvert;
  int vertX[4];
  int vertY[4];  
} intrusion_iva;


// linked list
int get_size(list *Rects);
void print_list_rect(list *Rects);
void push(list **Rects, CvRect rect);
int remove_by_index(list **Rects, int index);
void release(list **Rects);
CvRect rect_max(CvRect rect1, CvRect rect2);
void connect_nearby_rects(list **Rects, int delta_w, int delta_h);


/************************ setup input ********************
*
*********************************************************/
int get_file_contents(const char *filename, char **outbuffer);
void print_json_object(struct json_object *jobj, const char *msg);
int get_cfg_iva(motion_detection_iva* md_cfg, line_crossing_iva* lc_cfg, intrusion_iva* its_cfg, char* filename);
int check_modification_file(const char *path, time_t* oldMTime);
int setup_cfg(char* filename_json, int widthResize, int heightResize, motion_detection_iva* md_cfg, line_crossing_iva* lc_cfg, intrusion_iva* its_cfg);


/********************* motion detection ******************
*
*********************************************************/
int data_stream(AVFormatContext **infoContext, char *devName);
void resize_image(IplImage *img, IplImage *img_resized);
void create_background(IplImage *currentFrame, IplImage *background);
void find_foreground(IplImage *grayImage, IplImage *grayBackground, IplImage *foreground, int varThresh, int learningRate);
int motion_detect(IplImage *grayImage, IplImage *grayBackground, list **listTrack, motion_detection_iva md_cfg);
int motion_detect_region(motion_detection_iva md_cfg, list *listTrack);
void normalize_motion_detection_region(motion_detection_iva* md_cfg, int widthResize, int heightResize);


/********************** object tracking ******************
*
*********************************************************/
int find_object_direction(CvPoint p1, CvPoint p2);
double overlap_box(CvRect box1, CvRect box2);
int object_tracking(list *rects, list **listTrack);


/********************** line crossing ********************
* 
*********************************************************/
void normalize_line_crossing(line_crossing_iva* lc_cfg, int widthResize, int heightResize);
void line_crossing(line_crossing_iva lc_cfg, list *listTrack);


/******************* intrusion detection ****************
*
********************************************************/
void normalize_intrusion_detection(intrusion_iva* its_cfg, int widthResize, int heightResize);
int point_polygon_test(intrusion_iva its_cfg, int testx, int testy);
int intrusion_detection(intrusion_iva its_cfg, list *rects);
