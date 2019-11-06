#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
// msgq
#include <sys/ipc.h>
#include <sys/msg.h>
// socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
//check modification : file
//#include <sys/stat.h> 

/***************************** message queue *******************************
*
****************************************************************************/
#define PERMS 0644
struct my_msgbuf {
   long mtype;
   char mtext[200];
};

extern int msqid;
int msgq_send(int msqid, char* messages) ;
int rm_msgq(int msqid);

/******************************* socket ************************************
*
****************************************************************************/
extern int socket_flag;
extern char* hostname;
extern char* port;

int send_server();

/********************************* json ************************************
*
****************************************************************************/