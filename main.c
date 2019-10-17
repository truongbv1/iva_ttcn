#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

//resize
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>

#include "./image_proc.h"

#ifndef DEBUG
#define DEBUG 1
#endif // DEBUG

//video51 1280x720
//#define _WIDTH	1280
//#define _HEIGHT	720

// raw video53 640x480
#define WIDTH 640 // for video53
#define HEIGHT 480
// video resized 80x60
#define _WIDTH 80
#define _HEIGHT 60
#define _IMAGE_SIZE _WIDTH *_HEIGHT

static int g_exit = 0;

#define debug(...)                                                                                                                       \
    {                                                                                                                                    \
        do                                                                                                                               \
        {                                                                                                                                \
            if ((DEBUG))                                                                                                                 \
            {                                                                                                                            \
                printf("\n\n[%s:%s %d]=================START=========================================\n", __FILE__, __func__, __LINE__); \
                printf(__VA_ARGS__);                                                                                                     \
                printf("\n\n=========================================END=============================================\n\n");             \
            }                                                                                                                            \
        } while (0);                                                                                                                     \
    }

static void Termination(int sign)
{
    g_exit = 1;
}

int main(int argc, char *argv[])
{

    /*************************** get stream ************************************
	*
	* get data stream from dev/video53 and save to buffer
	****************************************************************************/
    int ret = 0;
    AVFormatContext *infoContext = NULL; //avformat.h
    AVPacket inPacket;
    signal(SIGINT, Termination);
    signal(SIGTERM, Termination); //avcodec.h
    ret = data_stream(&infoContext, "/dev/video53");

    /********************************* iva *************************************
	* motion detection
	* object tracking
	* line crossing
	* intrusion detection
	****************************************************************************/
    IplImage *img = cvCreateImage(cvSize(WIDTH, HEIGHT), 8, 1);
    IplImage *grayImage = cvCreateImage(cvSize(_WIDTH, _HEIGHT), 8, 1);
    IplImage *grayBackground = cvCreateImage(cvSize(_WIDTH, _HEIGHT), 8, 1);
    //resize

    //FILE* pFile = NULL;
    list *listTrack = NULL;
    int number = 0, learningRate = 30;
    // motion detection
    int motionDetectFlag = 0;
    int delayEventMotion = 0;

    // line crossing
    CvPoint p1, p2, pminLine, pmaxLine;
    p1.x = 18;
    p1.y = 25;
    p2.x = 66;
    p2.y = 39;
    int direction = 0; // 1 : A->B , -1: B->A, 0: A<=>B
    direction_line_crossing(&p1, &p2);
    get_minmax_line(&p1, &p2, &pminLine, &pmaxLine);

    // intrusion detection
    int nvert = 5; // 5 vertices
    int vertx[] = {5, 10, 20, 20, 20};
    int verty[] = {50, 40, 40, 10, 10};
    int intrusionDetectFlag = 0;
    int delayEventIntrusion = 0;

    // run
    printf("----------> create background! <----------\n...\n");
    while (!g_exit)
    {
        ret = av_read_frame(infoContext, &inPacket);
        if (ret < 0)
        {
            av_free_packet(&inPacket);
            continue;
        }

        img->imageData = (char *)inPacket.data;
        resize_image(img, grayImage);

        if (number >= 256)
        {
            if (motion_detect(grayImage, grayBackground, &listTrack, learningRate) == 1)
            {
                if (!motionDetectFlag)
                {
                    if (delayEventMotion == 5)
                    {
                        printf("-------------> Motion detected <------------\n");
                        motionDetectFlag = 1;
                        delayEventMotion = 0;
                    }
                    else delayEventMotion++;
                }

                else
                {
                    // line crossing
                    line_crossing(p1, p2, pminLine, pmaxLine, listTrack, direction);

                    // intrusion detection
                    if (intrusion(listTrack, nvert, vertx, verty) == 1)
                    {
                        if (!intrusionDetectFlag)
                        {
                            if (delayEventIntrusion == 5)
                            {
                                printf("------------> intrusion detected <----------\n");
                                intrusionDetectFlag = 1;
                                delayEventIntrusion = 0;
                            }
                            else delayEventIntrusion++;
                        }
                    }
                    else
                    {
                        if (intrusionDetectFlag) printf("---------------> End intrusion <------------\n");
                        intrusionDetectFlag = 0;
                    }
                }
            }
            else
            {
                if (motionDetectFlag) printf("---------------> End motion <---------------\n");
                motionDetectFlag = 0;
            }

            if (learningRate > 0) learningRate--;
            else learningRate = 30;
        }
        else create_background(grayImage, grayBackground);

        if (number < 256)
        {
            if (number == 255) printf("-------------->done!\ndetection:\n");
            number++;
        }

        av_free_packet(&inPacket);
    }
    avformat_close_input(&infoContext);
    cvReleaseImage(&img);
    cvReleaseImage(&grayImage);
    cvReleaseImage(&grayBackground);
    return 0;
}
