#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

// socket
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <errno.h>
// thread
#include <pthread.h>

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
                printf("\n[%s:%s %d] START \n", __FILE__, __func__, __LINE__);                                                           \
                printf(__VA_ARGS__);                                                                                                     \
                printf("\n END \n");                                                                                                     \
            }                                                                                                                            \
        } while (0);                                                                                                                     \
    }

static void Termination(int sign)
{
    g_exit = 1;
}

//#define DELAY_SEND 10
//#define DELAY_END 20
#define DELAY_SEND 5
#define DELAY_END 20

int main(int argc, char *argv[])
{   

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
    int number = 0;
    int learningRate = 15;
    int varThresh = 15;
    int delta_w = 2;
    int delta_h = 3;
    int area_min = 2;
    // motion detection
	int delayEventMD = 0;
	int eventFlagMD = 0;
	int delayEndMD = 0;
	int endFlagMD = 0;

    // line crossing
    CvPoint p1, p2, pminLine, pmaxLine;
    char* op = NULL;    
    int direction = 0; // 1 : A->B , -1: B->A, 0: A<=>B
    //direction_line_crossing(&p1, &p2);
    //get_minmax_line(&p1, &p2, &pminLine, &pmaxLine);

    // intrusion detection
    int nvert = 4; // 5 vertices
    int vertx[] = {57, 70, 80, 62};
    int verty[] = {40, 37, 45, 50};
    int intrusionDetectFlag = 0;
    int delayEventIntrusion = 0;

	if(argc == 2 && strcmp(argv[1],"-help") == 0)
	{
		printf("\n\n**************************  help  **************************\n");
		printf("default: %s\n\n", argv[0]);
    	printf("--varThresh : threshold value of foreground and background (default = %d)\n", varThresh);
    	printf("--delta_w : delta-width for connect nearby rectangle (default = %d)\n", delta_w);
    	printf("--delta_h : delta-height for connect nearby rectangle (default = %d)\n", delta_h);
    	printf("--area_min : area min for object deteted (default = %d)\n", area_min);
    	printf("--learningRate : learning rate for update background (default = %d)\n", learningRate);
    	printf("--hostname : static ip of server (default = %s)\n", hostname);
    	printf("--port : port connect to server (default = %s)\n", port);
    	printf("--camera : i-indoor or o-outdoor\n");
    	printf("usage:\n%s --parameter value --parameter value ...vv..\n", argv[0]);
    	printf("\n**************************  ****  **************************\n\n");
    	return 0;
	}
	else 
	{
		int i = 1;
		for(i =1; i< argc;i++){
			if(i%2 != 0) {
				if(strcmp(argv[i],"--varThresh")==0){varThresh = atoi(argv[i+1]); continue;}
				if(strcmp(argv[i],"--delta_w")==0)	{delta_w = atoi(argv[i+1]); continue;}
				if(strcmp(argv[i],"--delta_h")==0)	{delta_h = atoi(argv[i+1]); continue;}
				if(strcmp(argv[i],"--area_min")==0)	{area_min = atoi(argv[i+1]); continue;}
				if(strcmp(argv[i],"--learningRate")==0)	{learningRate = atoi(argv[i+1]); continue;}
				if(strcmp(argv[i],"--hostname")==0)	{hostname = argv[i+1]; continue;}
				if(strcmp(argv[i],"--port")==0)		{port = argv[i+1]; continue;}
				if(strcmp(argv[i],"--camera")==0)	{op = argv[i+1];}
			}
		}
		
		printf("\n\n**************** setup ******************\n");
		printf("use -help\n");
		printf("varThresh = %d\n", varThresh);
	    printf("delta_w = %d\n", delta_w);
	    printf("delta_h = %d\n", delta_h);
	    printf("area_min = %d\n", area_min);
	    printf("learningRate = %d\n", learningRate);
	    printf("hostname = %s\n", hostname);
	    printf("port = %s\n", port);
	    printf("*****************************************\n\n");
	}
	

	if(op != NULL)
	{
		if(strcmp(op,"i")==0)
		{
			p1.x = 21;
			p1.y = 28;
			p2.x = 43;
			p2.y = 34;
			vertx[0] = 18;vertx[1] = 41;vertx[2] = 35;vertx[3] = 7;
			verty[0] = 33;verty[1] = 38;verty[2] = 50;verty[3] = 41;
		}
		if(strcmp(op,"o")==0)
		{
			p1.x = 50;
			p1.y = 24;
			p2.x = 70;
			p2.y = 38;
			vertx[0] = 37;vertx[1] = 61;vertx[2] = 72;vertx[3] = 38;
			verty[0] = 43;verty[1] = 42;verty[2] = 56;verty[3] = 57;
		}
	}     	

    direction_line_crossing(&p1, &p2);
    get_minmax_line(&p1, &p2, &pminLine, &pmaxLine);


	/******************************* socket ************************************
	*
	****************************************************************************/

    if (hostname != NULL && port != NULL)
    {
	    // thread	    
	    pthread_t thread_socket;	    
	    pthread_create(&thread_socket, NULL, &send_server, NULL); 	    
	    int err = pthread_detach(thread_socket);
		if (err) debug("Failed to detach Thread : %s\n",strerror(err)); 
    } 


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

    /*************************** processing ************************************/    
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
            if (motion_detect(grayImage, grayBackground, &listTrack, learningRate, varThresh, delta_w, delta_h, area_min) == 1)
			{
				delayEndMD = 0;
				if (!eventFlagMD)
				{
					if (delayEventMD == DELAY_SEND)
					{
						eventFlagMD = 1;
						endFlagMD = 1;
						printf("-------------> Motion detected <------------\n");
					}
					else delayEventMD++;
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

				    // send server
				    if(argc > 5) 
				    {
			    		list *current = listTrack;
					    while (current != NULL)
					    {
					        push(&rects_sk, current->rect);
					        current = current->next;
					    }					    	
				    	//printf("main ..\n");	
				    	socket_flag = 0;			    	
				    	
				    }
				}
			}
			else
			{
				delayEventMD = 0;
				if (endFlagMD)
				{	
					if (delayEndMD == DELAY_END)
					{
						eventFlagMD = 0;
						endFlagMD = 0;
						printf("---------------> End motion <---------------\n");
					}		
					else delayEndMD++;
				}
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
    //close(sockfd); //Dong socket 
    avformat_close_input(&infoContext);
    cvReleaseImage(&img);
    cvReleaseImage(&grayImage);
    cvReleaseImage(&grayBackground);
    return 0;
}
