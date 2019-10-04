    // FFmpeg
    #include <libavutil/timestamp.h>
    #include <libavdevice/avdevice.h>
    #include <libavformat/avio.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
    #include <libavutil/time.h>
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>

    //#include <rtscamkit.h>
    //#include <rtsavapi.h>
    //#include <rtsvideo.h>

    // image processing
    #include "../inc/cxcore.h"
    #include "../inc/cxtypes.h"
    	
    #include <sys/stat.h> //check modification : file
	// list of rectangles
    typedef struct list {		
        struct list* prev;
        struct list* next;
        int id;
        int status;
        int level;
        int angle;
        CvRect rect;        
    }list;

    int get_size(list * Rects);    
    void print_list_rect(list* Rects);
    void push(list** Rects, CvRect rect);
    int remove_by_index(list** Rects, int index);
    void release(list **Rects);
    CvRect rect_max(CvRect rect1, CvRect rect2);
    void connect_nearby_rects(list** Rects, int delta_w, int delta_h);
    


   

    /********************* motion detection ******************
    *
    *********************************************************/
	int data_stream(AVFormatContext** infoContext, char* devName);
    void create_background(IplImage* currentFrame, IplImage* background);    
    void find_foreground(IplImage* grayImage, IplImage* grayBackground, IplImage* foreground, int valThresh, int learningRate);    
    int motion_detect(IplImage* rawImage, IplImage* grayBackground, list** rects, int learningRate);


    /********************** object tracking ******************
    *
    *********************************************************/
    int find_object_direction(CvPoint p1, CvPoint p2);
    double overlap_box(CvRect box1, CvRect box2);
    int object_tracking(list* rects, list** listTrack);


    /********************** line crossing ********************
    * 
    *********************************************************/
    void get_minmax_line(CvPoint* p1, CvPoint* p2, CvPoint* pminLine, CvPoint* pmaxLine);
    void direction_line_crossing(CvPoint* p1, CvPoint* p2);
	//int setup_line_input(char* filename, CvPoint* p1, CvPoint* p2, CvPoint* pminLine, CvPoint* pmaxLine, int options);
    void line_crossing(CvPoint p1, CvPoint p2, CvPoint pminLine, CvPoint pmaxLine, list* listTrack, int options);
    

    
    
    /******************* intrusion detection ****************
    *
    ********************************************************/
    int point_polygon_test(int nvert, int *vertx, int *verty, int testx, int testy);
    int intrusion(list* rects, int nvert, int *vertx, int *verty);

    