#include "interface_iva.h"
#include "image_proc.h"

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

    char sendbuff[1024];
    //char recvbuff[1024];

    portno = atoi(port); //Chuyen cong dich vu thanh so nguyen
    //portno = 9000;
    //Tao socket
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
    memset(sendbuff, 0, 1024); //Khoi tao buffer
    char *p = sendbuff;
    int size_r = 0;
    int scale = 1; // 1280
    //int delta_scale = 160; // for 1280
    
    //char *s = "123 22 232 232,34 200 343 545,546 456 250 54";
    while(1){

       if(rects_sk != NULL && socket_flag == 0){

         memset(sendbuff, 0, 1024);
          list *current = rects_sk;
          for (current = rects_sk; current != NULL; current = current->next)
          {
            size_r = strlen(sendbuff);
            if (size_r < 1024){
               sprintf(p + size_r, "%d %d %d %d,", scale*current->rect.x, scale*current->rect.y, scale*current->rect.width, scale*current->rect.height);
            }  
          }
          
          //printf("%s\n",sendbuff);         
          n = write(sockfd, sendbuff, strlen(sendbuff));
          if (n < 0) printf("ERROR writing notification to socket");

          release(&rects_sk);
         socket_flag =1;
      }

       usleep(100);
    }    
    
    close(sockfd); //Dong socket
    return 0;
}