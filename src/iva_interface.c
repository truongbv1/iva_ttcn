#include "iva_interface.h"
#include "iva_proc.h"

/***************************** message queue *******************************
*
****************************************************************************/
int msqid;
int msgq_send(int msqid, char* messages) 
{
  struct my_msgbuf buf;
  int len;
  sprintf(buf.mtext, "%s", messages);   
  len = strlen(buf.mtext);
  if (msgsnd(msqid, &buf, len + 1, 0) == -1) // +1 for '\0'
    perror("msgsnd");

  return 0;
}


int rm_msgq(int msqid)
{
  //IPC_RMID Removes the message queue immediately.
  if (msgctl(msqid, IPC_RMID, NULL) == -1)
  {
    perror("msgctl");
    exit(1);
  }
  printf("Removes the message queue\n");
  return 0;
}


/****************************** json file **********************************
*
****************************************************************************/
/**
 * Place the contents of the specified file into a memory buffer
 *
 * @param[in] filename The path and name of the file to read
 * @param[out] filebuffer A pointer to the contents in memory
 * @return status 0 success, 1 on failure
 */
int get_file_contents(const char *filename, char **outbuffer)
{
  FILE *file = NULL;
  long filesize;
  const int blocksize = 1;
  size_t readsize;
  char *filebuffer;

  // Open the file
  file = fopen(filename, "r");
  if (NULL == file)
  {
    printf("'%s' not opened\n", filename);
    exit(EXIT_FAILURE);
  }

  // Determine the file size
  fseek(file, 0, SEEK_END);
  filesize = ftell(file);
  rewind(file);

  // Allocate memory for the file contents
  filebuffer = (char *)malloc(sizeof(char) * filesize);
  *outbuffer = filebuffer;
  if (filebuffer == NULL)
  {
    fputs("malloc out-of-memory", stderr);
    exit(EXIT_FAILURE);
  }

  // Read in the file
  readsize = fread(filebuffer, blocksize, filesize, file);
  if (readsize != filesize)
  {
    fputs("didn't read file completely", stderr);
    exit(EXIT_FAILURE);
  }

  // Clean exit
  fclose(file);
  return EXIT_SUCCESS;
}


// Just a utility function
void print_json_object(struct json_object *jobj, const char *msg)
{
  printf("\n***************%s**************\n", msg);
  printf("%s", json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
  printf("\n********************************************************\n");
}


int get_cfg_iva(motion_detection_iva* md_cfg, line_crossing_iva* lc_cfg, intrusion_iva* its_cfg, char* filename)
{
  char *buffer = NULL;
  struct json_object *root_obj;
  struct json_object *data_iva_obj;
  struct json_object *rect_iva_obj;
  struct json_object *tmp_obj;
  get_file_contents(filename, &buffer);
  root_obj = json_tokener_parse(buffer);

  json_object_object_get_ex(root_obj, "data_iva", &data_iva_obj);
  json_object_object_get_ex(data_iva_obj, "rect_iva", &data_iva_obj);

  // region_md
  rect_iva_obj = json_object_array_get_idx(data_iva_obj, 0);  
  json_object_object_get_ex(rect_iva_obj, "resolution", &tmp_obj);
  json_object_object_get_ex(tmp_obj, "width", &tmp_obj);  
  md_cfg->width = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "resolution", &tmp_obj);
  json_object_object_get_ex(tmp_obj, "height", &tmp_obj);  
  md_cfg->height = json_object_get_int(tmp_obj);
  //json_object_object_get_ex(rect_iva_obj, "enable_md", &tmp_obj);
  //md_cfg->enable_md = json_object_get_boolean(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "enable_md", &tmp_obj);
  md_cfg->enable_mdr = json_object_get_boolean(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "startX", &tmp_obj);
  md_cfg->region_detection.x = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "startY", &tmp_obj);
  md_cfg->region_detection.y = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "endX", &tmp_obj);
  md_cfg->region_detection.width = json_object_get_int(tmp_obj) - md_cfg->region_detection.x;
  json_object_object_get_ex(rect_iva_obj, "endY", &tmp_obj);
  md_cfg->region_detection.height = json_object_get_int(tmp_obj)- md_cfg->region_detection.y;
  print_json_object(rect_iva_obj, "*** setup motion region ***");

  // line
  rect_iva_obj = json_object_array_get_idx(data_iva_obj, 1);  
  json_object_object_get_ex(rect_iva_obj, "resolution", &tmp_obj);
  json_object_object_get_ex(tmp_obj, "width", &tmp_obj);  
  lc_cfg->width = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "resolution", &tmp_obj);
  json_object_object_get_ex(tmp_obj, "height", &tmp_obj);  
  lc_cfg->height = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "enable_lc", &tmp_obj);
  lc_cfg->enable_lc = json_object_get_boolean(tmp_obj);
  //json_object_object_get_ex(rect_iva_obj, "sensitivity", &tmp_obj);
  //lc_cfg->sensitivity = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "direction", &tmp_obj);
  lc_cfg->direction = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "startX", &tmp_obj);
  lc_cfg->startX = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "startY", &tmp_obj);
  lc_cfg->startY = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "endX", &tmp_obj);
  lc_cfg->endX = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "endY", &tmp_obj);
  lc_cfg->endY = json_object_get_int(tmp_obj);
  print_json_object(rect_iva_obj, "*** setup line crossing ***");

  // intrusion
  rect_iva_obj = json_object_array_get_idx(data_iva_obj, 2);  
  json_object_object_get_ex(rect_iva_obj, "resolution", &tmp_obj);
  json_object_object_get_ex(tmp_obj, "width", &tmp_obj);  
  its_cfg->width = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "resolution", &tmp_obj);
  json_object_object_get_ex(tmp_obj, "height", &tmp_obj);  
  its_cfg->height = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "enable_its", &tmp_obj);
  its_cfg->enable_its = json_object_get_boolean(tmp_obj);
  //json_object_object_get_ex(rect_iva_obj, "sensitivity", &tmp_obj);
  //its_cfg->sensitivity = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "direction", &tmp_obj);
  its_cfg->direction = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "p_x1", &tmp_obj);
  its_cfg->vertX[0] = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "p_y1", &tmp_obj);
  its_cfg->vertY[0] = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "p_x2", &tmp_obj);
  its_cfg->vertX[1] = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "p_y2", &tmp_obj);
  its_cfg->vertY[1] = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "p_x3", &tmp_obj);
  its_cfg->vertX[2] = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "p_y3", &tmp_obj);
  its_cfg->vertY[2] = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "p_x4", &tmp_obj);
  its_cfg->vertX[3] = json_object_get_int(tmp_obj);
  json_object_object_get_ex(rect_iva_obj, "p_y4", &tmp_obj);
  its_cfg->vertY[3] = json_object_get_int(tmp_obj);
  its_cfg->nvert = 4;
  print_json_object(rect_iva_obj, " setup intrusion detection ");

  json_object_put(tmp_obj);
  json_object_put(rect_iva_obj);
  json_object_put(data_iva_obj);
  json_object_put(root_obj);
  free(buffer);
  return 0;
}

// check last modification of file
int check_modification_file(const char *path, time_t* oldMTime)
{
  struct stat fileStat;
  int err = stat(path, &fileStat);
  if (err != 0)
  {
      perror(" [check_modification_file] stat");
      exit(errno);
  }
  if(fileStat.st_mtime != *oldMTime) {
    *oldMTime = fileStat.st_mtime;
    return 1;
  }
  return 0;
}

// motion detection region
void normalize_motion_detection_region(motion_detection_iva* md_cfg, int widthResize, int heightResize)
{
  md_cfg->region_detection.x = (md_cfg->region_detection.x)*heightResize/(md_cfg->height) - 2*heightResize/9;
  md_cfg->region_detection.y = (md_cfg->region_detection.y)*heightResize/(md_cfg->height);
  md_cfg->region_detection.width = (md_cfg->region_detection.width)*heightResize/(md_cfg->height);
  md_cfg->region_detection.height = (md_cfg->region_detection.height)*heightResize/(md_cfg->height);
  if (md_cfg->region_detection.x < 0) md_cfg->region_detection.x = 0;
}

// line crossing
void normalize_line_crossing(line_crossing_iva* lc_cfg, int widthResize, int heightResize)
{
  // set rawsize(16:9) to rezize(4:3) of frame
  // X_resize = X_server.(H_resize/H_server) - (H_resize/2).(16/9 - 4/3)
  // X_resize = X_server.(H_resize/H_server) - 2.H_resize/9
  lc_cfg->startX = (lc_cfg->startX)*heightResize/(lc_cfg->height) - 2*heightResize/9;
  lc_cfg->endX = (lc_cfg->endX)*heightResize/(lc_cfg->height) - 2*heightResize/9;
  if (lc_cfg->startX < 0) lc_cfg->startX = 0;
  if (lc_cfg->endX < 0) lc_cfg->endX = 0;
  // Y_resize = Y_server.H_resize/H_server
  lc_cfg->startY = (lc_cfg->startY)*heightResize/(lc_cfg->height);
  lc_cfg->endY = (lc_cfg->endY)*heightResize/(lc_cfg->height);
  // set default direction
  if (((lc_cfg->startX < lc_cfg->endX) && (lc_cfg->startY > lc_cfg->endY)) ||
    ((lc_cfg->startX > lc_cfg->endX) && (lc_cfg->startY >= lc_cfg->endY)) ||
    (lc_cfg->startY > lc_cfg->endY))
  {
    // swap point_1 and point_2
    int tempX = lc_cfg->startX;
    int tempY = lc_cfg->startY;
    lc_cfg->startX = lc_cfg->endX;
    lc_cfg->startY = lc_cfg->endY;
    lc_cfg->endX = tempX;
    lc_cfg->endY = tempY;
  }

  // set minmax line
  lc_cfg->minX = MIN(lc_cfg->startX, lc_cfg->endX);
  lc_cfg->minY = MIN(lc_cfg->startY, lc_cfg->endY);  
  lc_cfg->maxX = MAX(lc_cfg->startX, lc_cfg->endX);
  lc_cfg->maxY = MAX(lc_cfg->startY, lc_cfg->endY);   

  // x1 = x2 || y1 = y2
  if ((lc_cfg->maxX - lc_cfg->minX) < 30)
  {
    lc_cfg->minX -= 10;
    lc_cfg->maxX += 10;
  }
  if ((lc_cfg->maxY - lc_cfg->minY) < 20)
  {
    lc_cfg->minY -= 8;
    lc_cfg->maxY += 8;
  }
}


void normalize_intrusion_detection(intrusion_iva* its_cfg, int widthResize, int heightResize)
{
  int i = 0;
  for(i = 0; i< its_cfg->nvert; i++){
      // X_resize = X_server.(H_resize/H_server) - 2.H_resize/9
      its_cfg->vertX[i] = (its_cfg->vertX[i])*heightResize/(its_cfg->height) - 2*heightResize/9;
      if (its_cfg->vertX[i] < 0) its_cfg->vertX[i] = 0;
      // Y_resize = Y_server.H_resize/H_server
      its_cfg->vertY[i] = (its_cfg->vertY[i])*heightResize/(its_cfg->height);
  }
}

int setup_cfg(char* filename_json, int widthResize, int heightResize, motion_detection_iva* md_cfg, line_crossing_iva* lc_cfg, intrusion_iva* its_cfg){
  
  // read file and get cfg-iva
  get_cfg_iva(md_cfg, lc_cfg, its_cfg, filename_json);

  // setup region motion-detection
  normalize_motion_detection_region(md_cfg, widthResize, heightResize);

  // setup input-line
  normalize_line_crossing(lc_cfg, widthResize, heightResize);

  // setup input-intrusion
  normalize_intrusion_detection(its_cfg, widthResize, heightResize);
  return 0;
}



/******************************* socket ************************************
*
****************************************************************************/
list *rects_sk = NULL;
int socket_flag = 0;

char* hostname = NULL;
char* port = NULL;

int send_server()
{
   
    int sockfd, portno, n;
    struct sockaddr_in serv_addr; //Cau truc chua dia chi server ma client can biet de ket noi toi

    char sendbuff[256];
    //char recvbuff[1024];

    portno = atoi(port); //Chuyen cong dich vu thanh so nguyen
    
    //create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        printf("\n Error : Could not create socket \n");      
    
    memset(&serv_addr, '0', sizeof(serv_addr));

    //Thiet lap dia chi cua server de ket noi den
    serv_addr.sin_family = AF_INET;        //Mac dinh
    serv_addr.sin_port = htons(portno);    //Cong dich vu   
    //Dia chi ip/domain may chu
    if(inet_pton(AF_INET, hostname, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }
    //Goi ham connect de thuc hien mot ket noi den server
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }
       
    printf("socket ..\n");    
    memset(sendbuff, 0, 256); //Khoi tao buffer
    char *p = sendbuff;
    int size_r = 0;
    int scale = 12; // 1280x720
    int frame_number = 0;

    while(1){

      if(rects_sk != NULL && socket_flag == 1){
        frame_number++;
        if(frame_number == 10000) frame_number = 0;
        list *current = rects_sk;          
        for (current = rects_sk; current != NULL; current = current->next)
        { 
          memset(sendbuff, 0, 256); 
          sprintf(p, "%d,%d,%d,%d,%d.",frame_number, scale*current->rect.x + 160, scale*current->rect.y, scale*current->rect.width, scale*current->rect.height);
          //printf("%s\n", p);
          n = write(sockfd, sendbuff, strlen(sendbuff));
          if (n < 0) printf("ERROR writing notification to socket");
        }

        release(&rects_sk);
        usleep(20000);
        socket_flag = 0;
      }
      else usleep(20000);
    }  
    
    close(sockfd); //Dong socket
    return 0;
}