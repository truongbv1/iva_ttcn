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

#define DELAY_SEND 4
#define DELAY_END 100
#define DELAY_END_MDR 80

#define RUN_ALL 1
#define RUN_MD 2
#define RUN_MR 3
#define RUN_LC 4
#define RUN_ID 5


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

    int frameNumber = 0;
    int option = 0; 			// all or MD or LC or ID   
    list *listTrack = NULL;  	// list rectangle-object
    char* filename_json = "/usr/conf/iva.json";  
    time_t oldMTime; 			// last modified of file-iva.json
    int countCheckFile = 0;

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
	// motion detection region
	int delayEventMDR = 0;
	int eventFlagMDR = 0;
	int delayEndMDR = 0;
	int endFlagMDR = 0;	
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
		printf("-r : \tmotion region detection\n"); 
		printf("-l : \tline crossing\n");
		printf("-i : \tintrusion detection\n");
		printf("-a : \tall: MD, MDR, LC, ID\n");
		printf("[parameters]\n"); 
    	printf("--varThresh : \tthreshold value of foreground and background (default = %d)\n", md_cfg.varThresh);
    	printf("--delta_w : \tdelta-width for connect nearby rectangle (default = %d)\n", md_cfg.delta_w);
    	printf("--delta_h : \tdelta-height for connect nearby rectangle (default = %d)\n", md_cfg.delta_h);
    	printf("--area_min : \tarea min for object deteted (default = %d)\n", md_cfg.area_min);
    	printf("--learningRate : \tlearning rate for update background (default = %d)\n", md_cfg.learningRate);
    	printf("--hostname :\t static ip of server (default = %s)\n", hostname);
    	printf("--port : \tport connect to server (default = %s)\n", port);
    	//printf("--camera : \tin (indoor) or out (outdoor)\n");
    	printf("usage:\n%s (default of file-cfg)\n", argv[0]);
    	printf("%s [options] [[parameters] value] ...vv..\n", argv[0]);
    	printf("\n**************************  ****  **************************\n\n");
    	return 0;
	}
	
	// setup
	check_modification_file(filename_json, &oldMTime);
	setup_cfg(filename_json, _WIDTH, _HEIGHT, &md_cfg, &lc_cfg, &its_cfg);	
	
	int i = 1;
	for(i =1; i< argc;i++){
		if(strcmp(argv[i],"-m")==0){option = RUN_MD; continue;}
		if(strcmp(argv[i],"-r")==0){option = RUN_MR; continue;}
		if(strcmp(argv[i],"-l")==0){option = RUN_LC; continue;}
		if(strcmp(argv[i],"-i")==0){option = RUN_ID; continue;}
		if(strcmp(argv[i],"-a")==0){option = RUN_ALL; continue;}

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

	switch(option)
	{
		case RUN_MD: md_cfg.enable_mdr = 0; lc_cfg.enable_lc = 0; its_cfg.enable_its = 0; break;
		case RUN_MR: md_cfg.enable_mdr = 1; lc_cfg.enable_lc = 0; its_cfg.enable_its = 0; break;
		case RUN_LC: md_cfg.enable_mdr = 0; lc_cfg.enable_lc = 1; its_cfg.enable_its = 0; break;
		case RUN_ID: md_cfg.enable_mdr = 0; lc_cfg.enable_lc = 0; its_cfg.enable_its = 1; break;
		case RUN_ALL: md_cfg.enable_mdr= 1; lc_cfg.enable_lc = 1; its_cfg.enable_its = 1; break;
	}

	printf("\n\n**************** setup motion detection ****************\n");
	printf("use -help or ?\n");
	if(option == RUN_ALL) printf("option : \tDefault (MDR + LC + ID)\n");
	if(option == RUN_MD) printf("option : \tMotion detection\n");
	if(option == RUN_MR) printf("option : \tMotion region detection\n");
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
    AVFormatContext *infoContext = NULL; 	//avformat.h
    AVPacket inPacket;
    signal(SIGINT, Termination);
    signal(SIGTERM, Termination); 			//avcodec.h
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

        if (frameNumber >= 256)
        {        	
            if (motion_detect(grayImage, grayBackground, &listTrack, md_cfg) == 1)
			{
				// MD Region  
				if(md_cfg.enable_mdr)
				{				
					if (motion_detect_region(md_cfg, listTrack))
					{			
						delayEndMDR = 0;
						if (!eventFlagMDR)
						{
							if (delayEventMDR == DELAY_SEND)
							{
								eventFlagMDR = 1;
								endFlagMDR = 1;															
								//printf("-------------> Motion detected region <------------\n");
								msgq_send(msqid, "MDR"); // send "MD" to message queue							
							}
							else delayEventMDR++;
						}
					}					
					else
					{
						delayEventMDR = 0;
						if (endFlagMDR)
						{	
							if (delayEndMDR == DELAY_END_MDR)
							{
								eventFlagMDR = 0;
								endFlagMDR = 0;
								//printf("---------------> End motion region <---------------\n");
								msgq_send(msqid, "EndMDR"); // send "EndMD" to message queue
							}		
							else delayEndMDR++;
						}
					}
				}


				//  MD - LC, ID
				delayEndMD = 0;
				if (!eventFlagMD)
				{
					if (delayEventMD == DELAY_SEND)
					{
						eventFlagMD = 1;
						endFlagMD = 1;
						if(md_cfg.enable_md)
						{							
							//printf("-------------> Motion detected <------------\n");
							msgq_send(msqid, "MD"); // send "MD" to message queue
						}
					}
					else delayEventMD++;
				}
				else
				{
					// line crossing *******************************************************
					if(lc_cfg.enable_lc)
					{
						line_crossing(lc_cfg, listTrack);
					} // *******************************************************************
					

					// intrusion detection *************************************************
					if(its_cfg.enable_its)
					{					
						if (intrusion_detection(its_cfg, listTrack) == 1)
						{
					        if (!intrusionDetectFlag)
					        {
					            if (delayEventIntrusion == 5)
					            {					                
					                intrusionDetectFlag = 1;
					                delayEventIntrusion = 0;
					                //printf("------------> intrusion detected <----------\n");
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
					    		if(delayEventIntrusion == -30)
					    		{				        			
				        			intrusionDetectFlag = 0;
				        			delayEventIntrusion = 0;
				        			//printf("---------------> End intrusion <------------\n");
				        		}
				        	}
					        
					    }
					} // ********************************************************************
					

				    // socket: send server *****************
				    if(enable_sk) 
				    {
				    	if(!socket_flag)
				    	{					    		
				    		int n_sk = get_size(listTrack);				    		
				    		list *rs = listTrack;			    		
						    while (n_sk > 0)
						    {
						        push(&rects_sk, rs->rect);
						        rs = rs->next;
						        n_sk--;
						    }						 
						    socket_flag = 1;					    
				    	}				    	
				    }// ************************************

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
						//printf("---------------> End motion <---------------\n");
						msgq_send(msqid, "EndMD"); // send "EndMD" to message queue
					}		
					else delayEndMD++;
				}
			}

            if (md_cfg.learningRate > 0) md_cfg.learningRate--;
            else md_cfg.learningRate = lr_const;
            
        }
        else create_background(grayImage, grayBackground);

        if(countCheckFile >= 2000)
        {
        	// check last modification of cfg-file , 
        	// update cfg when update background (default is 30 frame)
            if(check_modification_file(filename_json, &oldMTime))
            {	
            	printf("%s file was modified\n", filename_json);
            	setup_cfg(filename_json, _WIDTH, _HEIGHT, &md_cfg, &lc_cfg, &its_cfg);	
				switch(option)
				{
					case RUN_MD: md_cfg.enable_mdr = 0; lc_cfg.enable_lc = 0; its_cfg.enable_its = 0; break;
					case RUN_MR: md_cfg.enable_mdr = 1; lc_cfg.enable_lc = 0; its_cfg.enable_its = 0; break;
					case RUN_LC: md_cfg.enable_mdr = 0; lc_cfg.enable_lc = 1; its_cfg.enable_its = 0; break;
					case RUN_ID: md_cfg.enable_mdr = 0; lc_cfg.enable_lc = 0; its_cfg.enable_its = 1; break;
					case RUN_ALL: md_cfg.enable_mdr= 1; lc_cfg.enable_lc = 1; its_cfg.enable_its = 1; break;
				}
            }
            countCheckFile = 0;
        }
        else countCheckFile++;

        if (frameNumber < 256)
        {
            if (frameNumber == 255) printf("-------------->done!\ndetection:\n");
            frameNumber++;
        }

        av_free_packet(&inPacket);
    }

    //msgq_send(msqid, "end");// closed retrieving message from queue
    //system("rm msgq.txt"); 	// remove msgq.txt (key)
    rm_msgq(msqid);			// remove message queue
    avformat_close_input(&infoContext);
    cvReleaseImage(&img);
    cvReleaseImage(&grayImage);
    cvReleaseImage(&grayBackground);
    return 0;
}
