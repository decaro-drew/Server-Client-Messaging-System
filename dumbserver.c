
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "mailbox.h"

pthread_mutex_t lock;


threadlist* thread;

void error(char *msg)
{
    perror(msg);
    exit(1);
}
box* head;
void* chkMalloc(size_t sz){
	void* mem = malloc(sz);
	if(mem == NULL){
	  exit(1);
	}	
	return mem;
}

int addBox(char* name){
  box* newest = chkMalloc(sizeof(*newest));
  strcpy(newest->name, name);
  if(head == NULL){
    head = newest;
    return 0;
  }
  box* ptr = head;
  if(strcmp(ptr->name, name) == 0){
    return 1;
  }
  int count = 0;
  while(ptr->next != NULL){
    ptr = ptr->next;
    if(strcmp(ptr->name, name) == 0){
      return 1;
    }
    count++;
  }
  ptr->next = newest;

  return 0;
  
}
int closeBox(char* name, char* open){
  box* ptr = head;
  
    if(strcmp("", open) == 0){
      return 1;
    }
    
    if(strcmp(name, open) != 0){
      return 1;
    }
  
  
  while(ptr != NULL){
    if(strcmp(ptr->name, name) == 0){
      if(ptr->open == 1){
	ptr->open = 0;
	return 0;
      }
      else{
	
	return 1;
      }
    }
    ptr = ptr->next;
  }
  return 1;
}
int delBox(char* name, char* open){
  
  
    
    if(strcmp(name, open) == 0){
      return 2;//OPEND
    }
  
  box* ptr = head;
  box* tail = NULL;
  while(ptr != NULL){

      if(strcmp(ptr->name, name) == 0)
      {//match found

	      if(ptr->open == 1){//open
		return 2;//OPEND
	      }
	      else if(ptr->m != NULL){//not open but has messages
		return 3;//NOTMT
	      }
	      else {//not open and empty, ready for deletion
		if (tail == NULL){
			
			head = ptr-> next;
			return 0;//OK!
		}else {

			tail->next = ptr->next;
			return 0;//OK!
		}

	      }

     }

    tail = ptr;
    ptr = ptr->next;

  }
  return 1;//NEXST
}

int openBox(char* name, char* open){
  box* ptr = head;
  while(ptr != NULL){
    if(strcmp(ptr->name, open) == 0){
      return 1;
    }
    ptr = ptr->next;
  }
  ptr = head;
  while(ptr != NULL){
    if(strcmp(ptr->name, name) == 0){
      if(ptr->open == 1){
	return 2;
      }
      else{
	ptr->open = 1;
	return 0;
      }
    }
    ptr = ptr->next;
  }
  return 3;
}

int addMsg(char* name, char* message){
  box* ptr = head;
  while(ptr!= NULL){
    if(strcmp(ptr->name, name) == 0){
      break;
    }
    else
      ptr = ptr->next;
  }
  if(ptr == NULL){
    return 1;
  }
  msg* mptr = chkMalloc(sizeof(*mptr));
  mptr->message = malloc(sizeof(message));
  if(ptr->m == NULL){
    strcpy(mptr->message, message);
    ptr->m = mptr;
    return 0;
  }
  mptr = ptr->m;
  while(mptr->next != NULL)
    mptr = mptr->next;
  msg* final = chkMalloc(sizeof(*final));
  final->message = malloc(sizeof(message));
  strcpy(final->message, message);
  mptr->next = final;
  return 0;


}

char* getNext(char* open){
  char result[1024];
  box* ptr = head;
  while(ptr!= NULL){
    if(strcmp(ptr->name, open) == 0){
      break;
    }
    else
      ptr = ptr->next;
  }
  if(ptr->m == NULL){
    strcpy(result, "EMPTY");
    return result;
  }

    msg* m = ptr->m;
    strcpy(result, m->message);
    ptr->m = m->next;
    return result;
  

}
void clientPortal(void* args) 
{
	int sockfd = ((cliArgs*) args) -> sockfd;
	struct sockaddr_in address = ((cliArgs*) args) -> address;
	char str[INET_ADDRSTRLEN];
	
	inet_ntop(AF_INET, &(address.sin_addr), str, INET_ADDRSTRLEN);
	char buffer[1024] = {0};
	char open[1024] = {0};
	char *ok = "OK!";
	int valread;

	
	valread = read( sockfd , buffer, 1024);
	int length = strlen(buffer);
	char tok[5];
	int i = 0;
		while(i<5){
		    tok[i] = buffer[i];
		    i++;
		  }
		  tok[i]='\0';
	int c = 0;
	char arg[1024];
	while(strcmp(tok, "GDBYE") != 0){

		time_t rawtime = time(NULL);
		struct tm *ptm = localtime(&rawtime);
	

		if(strcmp(tok, "CREAT") == 0){
		  pthread_mutex_lock(&lock);

			while(c<length-5){
			  arg[c] = buffer[6+c];
			  c++;
			}
			arg[c]='\0';
		       	memset(buffer, 0, 1024);
			if(strlen(arg) > 25 || strlen(arg) <5){
			  strcpy(buffer, "WHAT?");
			  fprintf(stderr,"%02d%02d %d Dec %s ER:WHAT?\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
			}
			else if((arg[0]>='a' && arg[0]<='z') || (arg[0]>='A' && arg[0]<='Z')){
			  
			  int a = addBox(arg);
			  if(a == 1){
			    strcpy(buffer, "EXIST");
			    fprintf(stderr,"%02d%02d %d Dec %s ER:EXIST\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
			  }
			  else{
			    strcpy(buffer, "OK!");
			    printf("%02d%02d %d Dec %s CREAT\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
			  }

			}else{
			  strcpy(buffer, "WHAT?");
fprintf(stderr,"%02d%02d %d Dec %s ER:WHAT\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
			}
			  send(sockfd, buffer, strlen(buffer), 0);
			  memset(arg, 0, 1024);
			  c = 0;
			  pthread_mutex_unlock(&lock);

		}
		else if(strcmp(tok, "OPNBX") == 0){
			pthread_mutex_lock(&lock);

		  while(c<length-5){
		    arg[c] = buffer[6+c];
		    c++;
		  }
		  arg[c]='\0';
		  c = 0;
		  if(strlen(arg) > 25 || strlen(arg) <5){
			  strcpy(buffer, "WHAT?");
  fprintf(stderr,"%02d%02d %d Dec %s ER:WHAT?\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday, str);
		  }
		  else if((arg[0]>='a' && arg[0]<='z') || (arg[0]>='A' && arg[0]<='Z')){
		    int a = openBox(arg, open);
		    if(a == 1){
		      strcpy(buffer, "ANOPN");
		     fprintf(stderr,"%02d%02d %d Dec %s ER:ANOPN\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		    }
		    else if(a == 2){
		      strcpy(buffer, "OPEND");
 fprintf(stderr,"%02d%02d %d Dec %s ER:OPEND\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		    }
		    else if(a == 3){
		      strcpy(buffer, "NEXST");
		       fprintf(stderr,"%02d%02d %d Dec %s ER:NEXST\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		    }
		    else{
		      strcpy(buffer, "OK!");
		      printf("%02d%02d %d Dec %s OPNBX\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		      memset(open, 0, 1024);
		      strcpy(open, arg);
		    }
		    
		  }else{
		    strcpy(buffer, "WHAT?");
		    printf("%02d%02d %d Dec %s ER:WHAT?\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday, str);
		  }
		  send(sockfd, buffer, strlen(buffer), 0);
			pthread_mutex_unlock(&lock);

		}	
		else if(strcmp(tok, "NXTMG") == 0){
		      // next message
			pthread_mutex_lock(&lock);

		  char next[1024];
		  if(strlen(open) == 0){
		    printf("%02d%02d %d Dec %s NOOPN\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		    strcpy(buffer, "NOOPN");

		  }
		  else{
		    strcpy(next, getNext(open));
		    if(strcmp(next, "EMPTY") == 0){
		      printf("%02d%02d %d Dec %s EMPTY\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		      strcpy(buffer, "EMPTY");
		    }
		    else{
		      char b[1024];
		      sprintf(b, "%d", strlen(next));
		      memcpy(buffer, "OK!" ,3);
		      strcat(buffer, b);
		      strcat(buffer, "!");
		      strcat(buffer, next);
		      
		    }
		    memset(next, 0, 1024);
			send(sockfd, buffer, strlen(buffer), 0);
			printf("%02d%02d %d Dec %s NEXTMG\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		  }
			pthread_mutex_unlock(&lock);

		}
		else if(strcmp(tok, "PUTMG") == 0){
			pthread_mutex_lock(&lock);

		  int x=0;
		  int count=0;
		  while(x<strlen(buffer)){
		    if(buffer[x] == '!'){
		      count++;
		    }
		    x++;
		  }
		  char argX[1024];
		  int z = 0;
		  if(strlen(open) == 0){
		    printf("%02d%02d %d Dec %s ER:NOOPN\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		    strcpy(buffer, "NOOPN");
		  }
		  else if(count > 0){
		    while(c<length-5){
		      arg[c] = buffer[6+c];
		      c++;
		      if(buffer[6+c] == '!'){
			c++;
			while(z<length-5-c){
			  argX[z] = buffer[6+c+z];
			  z++;
			}
			break;
		      }
		    }
		  
		      arg[c]='\0';
		      argX[z] ='\0';		      c = 0;
		      z = 0;
		      if(strlen(arg) == 0 || strlen(argX) == 0 || atoi(arg) != strlen(argX)){
			printf("%02d%02d %d Dec %s ER:WHAT\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
			strcpy(buffer, "WHAT?");
		      }
		      else{
			int put = addMsg(open, argX);
			if(put == 0){
			  printf("%02d%02d %d Dec %s PUTMG\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
			  strcpy(buffer, "OK!");
			}
		      }
		  }
	        

		
		  else{
		    printf("%02d%02d %d Dec %s ER:WHAT\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		    strcpy(buffer, "WHAT?");
		  }
		  memset(argX, 0, 1024);
		  memset(arg, 0, 1024);
		  send(sockfd, buffer, strlen(buffer), 0);
			pthread_mutex_unlock(&lock);

		}
		else if(strcmp(tok, "DELBX") == 0){
		    pthread_mutex_lock(&lock);
			while(c<length-5){
		    	arg[c] = buffer[6+c];
		    	c++;
		  	}
		  	arg[c]='\0';
			printf("arg is: %s\n", arg);
			int a;
			if(strlen(arg) < 25 || strlen(arg) > 5){

				if((arg[0]>='a' && arg[0]<='z') || (arg[0]>='A' && arg[0]<='Z')){
			
					a = delBox(arg, open);
				}
				else{
					a = 4;
				}
			}
			else{

				a = 4;
			}

			if(a == 1){
		      	strcpy(buffer, "NEXST");
		      	fprintf(stderr,"%02d%02d %d Dec %s ER:NEXST\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
			send(sockfd, buffer, strlen(buffer), 0);
		    	}
			else if (a == 2){
			strcpy(buffer, "OPEND");
		      	fprintf(stderr,"%02d%02d %d Dec %s ER:OPEND\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
			send(sockfd, buffer, strlen(buffer), 0);
			}
			else if(a == 3){
		        strcpy(buffer, "NOTMT");
		        fprintf(stderr,"%02d%02d %d Dec %s ER:NEXST\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		    	}
			else if(a == 4){
			strcpy(buffer, "WHAT?");
		    	fprintf(stderr,"%02d%02d %d Dec %s ER:WHAT?\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday, str);
			send(sockfd, buffer, strlen(buffer), 0);
			}
			else{
		      	strcpy(buffer, "OK!");
		      	printf("%02d%02d %d Dec %s DELBX\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
			send(sockfd, ok, strlen(ok), 0);
		    	}
			pthread_mutex_unlock(&lock);
		
		}
		else if(strcmp(tok, "CLSBX") == 0){
		    //close box
		        pthread_mutex_lock(&lock);
			while(c<length-5){
		    	arg[c] = buffer[6+c];
		    	c++;
		  	}
		  	arg[c]='\0';
			printf("arg is: %s\n", arg);
			int a;
			if(strlen(arg) < 25 || strlen(arg) > 5){

				if((arg[0]>='a' && arg[0]<='z') || (arg[0]>='A' && arg[0]<='Z')){
			
					a = closeBox(arg, open);
				}
				else{
					a = 3;
				}
			}
			else{

				a = 3;
			}

			if(a == 1){
		      	strcpy(buffer, "NOOPN");
		      	fprintf(stderr,"%02d%02d %d Dec %s ER:NOOPN\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
			send(sockfd, buffer, strlen(buffer), 0);
		    	}
			else if(a == 3){
			strcpy(buffer, "WHAT?");
		    	fprintf(stderr,"%02d%02d %d Dec %s ER:WHAT?\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday, str);
			send(sockfd, buffer, strlen(buffer), 0);
			}
			else{
		      	strcpy(buffer, "OK!");
		      	printf("%02d%02d %d Dec %s CLSBX\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday,str);
		      	memset(open, 0, 1024);
			send(sockfd, ok, strlen(ok), 0);
		    	}
			pthread_mutex_unlock(&lock);
		}
		else{
		  pthread_mutex_lock(&lock);
			strcpy(buffer, "WHAT?");
		    	fprintf(stderr,"%02d%02d %d Dec %s ER:WHAT?\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday, str);
			send(sockfd, buffer, strlen(buffer), 0);
			pthread_mutex_unlock(&lock);
		}
		memset(buffer, 0, 1024);
		memset(tok, 0, 5);
		valread = read(sockfd, buffer, 1024);
		length = strlen(buffer);
		i = 0;
		while(i<5){
		    tok[i] = buffer[i];
		    i++;
		  }
		  tok[i]='\0';
        

	}


}
void cliAccept(void *args)
{
	int sockfd = ((cliArgs*) args) -> sockfd;
	struct sockaddr_in address = ((cliArgs*) args) -> address;
	char str[INET_ADDRSTRLEN];
	int addrlen = sizeof(address);
	int newsockfd, valread;
	char buffer[1024] = {0};
	char *msg = "HELLO DUMBv0 ready!";
	char *badmsg = "badmsg";

	
	
	while ((newsockfd = accept(sockfd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) != -1) 
	{
		inet_ntop(AF_INET, &(address.sin_addr), str, INET_ADDRSTRLEN);
		time_t rawtime = time(NULL);
		struct tm *ptm = localtime(&rawtime);
		printf("%02d%02d %d Dec %s connected\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday, str);
		
		valread = read( newsockfd , buffer, 1024);
		if(strcmp(buffer, "HELLO") == 0)
		{	
			pthread_t topass;
			printf("%02d%02d %d Dec %s HELLO\n", ptm->tm_hour, ptm->tm_min, ptm->tm_mday, str);		
			send(newsockfd, msg, strlen(msg), 0);
			//int *socktopass = &newsockfd;
			cliArgs *args = (cliArgs*) malloc (sizeof(cliArgs));
			args->sockfd = newsockfd;
			args->address = address;
			pthread_create( &topass,NULL, clientPortal, (void*) args);
	        
		
		}
		else 
		{
			send(newsockfd, badmsg, strlen(badmsg), 0);
		}
	}

	
	pthread_exit(0);
}

int main(int argc, char const *argv[]) 
{ 

	//input check

	if (argc != 2)
	{
		fprintf(stderr, "Provide port number, and port number only\n");	
		return -1;
	}

	int portno = atoi(argv[1]);

	if (portno < 4000)
	{
		fprintf(stderr, "The port number should be strictly greater than 4K (not base-10).\n");
		return -1;
	}


	//socket building
	pthread_mutex_init(&lock, NULL);
	int opt = 1;
	struct sockaddr_in address;
	int sockfd;//, newsockfd, valread;
	int addrlen = sizeof(address);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    	{ 
        	error("socket"); 
    	} 

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    	{ 
        	error("setsockopt"); 
    	}

 
    	address.sin_family = AF_INET; 
    	address.sin_addr.s_addr = INADDR_ANY; 
    	address.sin_port = htons(portno); 


	if (bind(sockfd, (struct sockaddr *)&address, sizeof(address))<0) 
    	{ 
        	error("bind");  
    	}
 
    	if (listen(sockfd, 3) < 0) 
    	{ 
        	error("listen");  
    	} 

    	printf("listening...\n");


	//thread it out to accept client

	pthread_t clientAccept;

	cliArgs *args = (cliArgs*) malloc (sizeof(cliArgs));
	
	args->sockfd = sockfd;
	args->address = address;	
	
	pthread_create(&clientAccept, NULL, cliAccept, (void*) args);



	pthread_join(clientAccept, NULL);
	pthread_mutex_destroy(&lock);
	free(args);
	return 0;
}
