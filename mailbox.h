#ifndef MAILBOX_H
#define MAILBOX_H

//messages for message box
typedef struct _msg{

    char* message;
    struct msg* next;

}msg;


//box to hold messages and connect to other boxes
typedef struct _box{

    char name[1024];
    int open; //0 closed, 1 open
    msg* m;
    struct box* next;

}box;

//arguments to send to thread for client accept

typedef struct _cliArgs{

    int sockfd;
    struct sockaddr_in address;

}cliArgs;

typedef struct _threadlist{
  pthread_t thread;
  struct threadlist* next;

}threadlist;
  



#endif
