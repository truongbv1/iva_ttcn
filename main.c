#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
// thread
#include <pthread.h>
// iva processing
#include "iva_proc.h"
// message queue , socket, cfg
#include "iva_interface.h"

#ifndef DEBUG
#define DEBUG 1
#endif // DEBUG

// raw video53 640x480
#define WIDTH 640
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

#define DELAY_SEND 3
#define DELAY_END 20

#define RUN_ALL 0
#define RUN_MD 1
#define RUN_LC 2
#define RUN_ID 3

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

    int number = 0;
    int option = 0; // all or MD or LC or ID   
    list *listTrack = NULL;  // list rectangle-object
    char* filename_json = "/usr/conf/iva.json";  
    time_t oldMTime; // last modified

    // default for motion detection 
    motion_detection_iva md_cfg;
    md_cfg.delta_w = 1;
    md_cfg.delta_h = 2;
    md_cfg.area_min = 2;
    md_cfg.varThresh = 20;
	md_cfg.learningRate = 30;	
	int delayEventMD = 0;
	int eventFlagMD = 0;
	int delayEndMD = 0;
	int endFlagMD = 0;

	// line crossing
	line_crossing_iva lc_cfg;

	// intrusion detection    
	intrusion_iva its_cfg;
	int intrusionDetectFlag = 0;
    int delayEventIntrusion = 0;

	if(argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"?") == 0))
	{
		printf("\n\n**************************  help  **************************\n");
		printf("[options]\n"); 
		printf("-m : \tmotion detection\n"); 
		printf("-l : \tmotion detection and line crossing\n");
		printf("-i : \tmotion detection and intrusion detection\n");
		printf("[parameters]\n"); 
    	printf("--varThresh : \tthreshold value of foreground and background (default = %d)\n", md_cfg.varThresh);
    	printf("--delta_w : \tdelta-width for connect nearby rectangle (default = %d)\n", md_cfg.delta_w);
    	printf("--delta_h : \tdelta-height for connect nearby rectangle (default = %d)\n", md_cfg.delta_h);
    	printf("--area_min : \tarea min for object deteted (default = %d)\n", md_cfg.area_min);
    	printf("--learningRate : \tlearning rate for update background (default = %d)\n", md_cfg.learningRate);
    	printf("--hostname :\t static ip of server (default = %s)\n", hostname);
    	printf("--port : \tport connect to server (default = %s)\n", port);
    	//printf("--camera : \tin (indoor) or out (outdoor)\n");
    	printf("usage:\n%s (default)\n", argv[0]);
    	printf("%s [options] [[parameters] value] ...vv..\n", argv[0]);
    	printf("\n**************************  ****  **************************\n\n");
    	return 0;
	}
	
	// setup
	check_modification_file(filename_json, &oldMTime);
	setup_cfg(filename_json, _WIDTH, _HEIGHT, &lc_cfg, &its_cfg);	
	if(lc_cfg.enable_lc == 0 && its_cfg.enable_its == 0) option = RUN_MD;
	if(lc_cfg.enable_lc == 1 && its_cfg.enable_its == 0) option = RUN_LC;
	if(lc_cfg.enable_lc == 0 && its_cfg.enable_its == 1) option = RUN_ID; 


	int i = 1;
	for(i =1; i< argc;i++){
		if(strcmp(argv[i],"-m")==0){option = RUN_MD; continue;}
		if(strcmp(argv[i],"-l")==0){option = RUN_LC; continue;}
		if(strcmp(argv[i],"-i")==0){option = RUN_ID; continue;}

		if(strcmp(argv[i],"--varThresh")==0){md_cfg.varThresh = atoi(argv[++i]); continue;}
		if(strcmp(argv[i],"--delta_w")==0)	{md_cfg.delta_w = atoi(argv[++i]); continue;}
		if(strcmp(argv[i],"--delta_h")==0)	{md_cfg.delta_h = atoi(argv[++i]); continue;}
		if(strcmp(argv[i],"--area_min")==0)	{md_cfg.area_min = atoi(argv[++i]); continue;}
		if(strcmp(argv[i],"--learningRate")==0)	{md_cfg.learningRate = atoi(argv[++i]); continue;}
		if(strcmp(argv[i],"--hostname")==0)	{hostname = argv[++i]; continue;}
		if(strcmp(argv[i],"--port")==0)		{port = argv[++i]; continue;}
		//if(strcmp(argv[i],"--camera")==0)	{op = argv[++i]; continue;}
		printf("unknown? argument %s\n", argv[i]);
		return 0;			
	}
	
	printf("\n\n**************** setup motion detection ****************\n");
	printf("use -help or ?\n");
	if(option == RUN_ALL) printf("option : \tDefault (MD+LC+ID)\n");
	if(option == RUN_MD) printf("option : \tMotion detection\n");
	if(option == RUN_LC) printf("option : \tLine crossing\n");
	if(option == RUN_ID) printf("option : \tIntrusion detection\n");
	printf("varThresh = \t%d\n", md_cfg.varThresh);
    printf("delta_w = \t%d\n", md_cfg.delta_w);
    printf("delta_h = \t%d\n", md_cfg.delta_h);
    printf("area_min = \t%d\n", md_cfg.area_min);
    printf("learningRate = \t%d\n", md_cfg.learningRate);
    printf("hostname = \t%s\n", hostname);
    printf("port = \t\t%s\n", port);
    printf("********************************************************\n\n");
	
	int lr_const = md_cfg.learningRate;

	/******************************* socket ************************************
	*
	****************************************************************************/
	int enable_sk = 0;
    if (hostname != NULL && port != NULL)
    {
	    // thread	    
	    enable_sk = 1;
	    pthread_t thread_socket;	    
	    pthread_create(&thread_socket, NULL, &send_server, NULL); 	    
	    int err = pthread_detach(thread_socket);
		if (err) debug("Failed to detach Thread : %s\n",strerror(err)); 
    } 

	/***************************** message queue *******************************
	*
	****************************************************************************/
	//int msqid;	
	key_t key = 1236; // default
	// system("touch msgq.txt");
	// if ((key = ftok("msgq.txt", 'B')) == -1)
	// {
	// 	perror("ftok");
	// 	exit(1);
	// }
	if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1)
	{
		perror("msgget");
		exit(1);
	}
	printf("created message queue\n");
	

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
            if (motion_detect(grayImage, grayBackground, &listTrack, md_cfg) == 1)
			{
				delayEndMD = 0;
				if (!eventFlagMD)
				{
					if (delayEventMD == DELAY_SEND)
					{
						eventFlagMD = 1;
						endFlagMD = 1;
						printf("-------------> Motion detected <------------\n");
						msgq_send(msqid, "MD"); // send "MD" to message queue
					}
					else delayEventMD++;
				}
				else
				{
					// line crossing *******************************************************
					if(option == RUN_ALL || option == RUN_LC)
					{
						line_crossing(lc_cfg, listTrack);
					} // *******************************************************************
					

					// intrusion detection *************************************************
					if(option == RUN_ALL || option == RUN_ID)
					{					
						if (intrusion_detection(its_cfg, listTrack) == 1)
						{
					        if (!intrusionDetectFlag)
					        {
					            if (delayEventIntrusion == 5)
					            {					                
					                intrusionDetectFlag = 1;
					                delayEventIntrusion = 0;
					                printf("------------> intrusion detected <----------\n");
					                msgq_send(msqid, "ID"); // send "ID" to message queue
					            }
					            else delayEventIntrusion++;
					        }
					    }
					    else
					    {					 
					        if (intrusionDetectFlag)
				        	{
				        		delayEventIntrusion--;
					    		if(delayEventIntrusion == -5)
					    		{				        			
				        			intrusionDetectFlag = 0;
				        			delayEventIntrusion = 0;
				        			printf("---------------> End intrusion <------------\n");
				        		}
				        	}
					        
					    }
					} // ********************************************************************

				    // socket: send server *****************
				    if(enable_sk) 
				    {
			    		list *current = listTrack;
					    while (current != NULL)
					    {
					        push(&rects_sk, current->rect);
					        current = current->next;
					    }			    	
				    	socket_flag = 0;				    	
				    }
				    // *************************************
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
						msgq_send(msqid, "EndMD"); // send "EndMD" to message queue
					}		
					else delayEndMD++;
				}
			}

            if (md_cfg.learningRate > 0) md_cfg.learningRate--;
            else 
            {
            	md_cfg.learningRate = lr_const;

            	// check last modification of cfg-file , 
            	// update cfg when update background (default is 30 frame)
	            if(check_modification_file(filename_json, &oldMTime))
	            {	
	            	printf("%s file was modified\n", filename_json);
	            	setup_cfg(filename_json, _WIDTH, _HEIGHT, &lc_cfg, &its_cfg);	
					if(lc_cfg.enable_lc == 0 && its_cfg.enable_its == 0) option = RUN_MD;
					if(lc_cfg.enable_lc == 1 && its_cfg.enable_its == 0) option = RUN_LC;
					if(lc_cfg.enable_lc == 0 && its_cfg.enable_its == 1) option = RUN_ID;
	            }
            }

            
        }
        else create_background(grayImage, grayBackground);

        if (number < 256)
        {
            if (number == 255) printf("-------------->done!\ndetection:\n");
            number++;
        }

        av_free_packet(&inPacket);
    }

    msgq_send(msqid, "end");// closed retrieving message from queue
    //system("rm msgq.txt"); 	// remove msgq.txt (key)
    rm_msgq(msqid);			// remove message queue
    avformat_close_input(&infoContext);
    cvReleaseImage(&img);
    cvReleaseImage(&grayImage);
    cvReleaseImage(&grayBackground);
    return 0;
}
