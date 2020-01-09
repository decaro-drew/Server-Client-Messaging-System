
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

void error(char *msg)
{
    perror(msg);
    exit(-1);
}


int main(int argc, char const *argv[]) 

{ 

    
    //INPUT ERROR CHECKING
    // If the user didn't enter enough arguments, complain and exit

    if (argc != 3)
	{
		fprintf(stderr, "Incorrect number of arguments.\nFirst enter an IP address or a hostname. Second enter a port number of a server to connect to.\n");
		return -1;
	}



    int sockfd = 0, valread; 
    struct sockaddr_in serv_addr; 
    // char msg[1024];// = "this is the message from client sent to the server";
    char buffer[1024] = {0};


    //Build time
    
    /** If the user gave enough arguments, try to use them to get a port number and address **/

    char* userIPHostInput = argv[1];

    char* userPortInput = argv[2];

    // convert the text representation of the port number given by the user to an int

    int portno = atoi(userPortInput);



    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(portno); 
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, userIPHostInput, &serv_addr.sin_addr)<=0) 
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    printf("> ");
    scanf("%[^\n]", &buffer);

    char arg1[1024];
    char arg2[1024];
    char bytes[1024];
    char tmp;
    while (strcmp(buffer, "quit") != 0) {
      if(strcmp(buffer, "create") == 0){
	memset(buffer, 0, 1024);
	printf("Okay, what would like to name the new box?\n");
	printf("create:> ");
	scanf("%c", &tmp);
	scanf("%[^\n]", &arg2);
	memcpy(arg1, "CREAT " , 6);
	strcat(arg1, arg2);
   	strcat(buffer, arg1);
	memset(arg1, 0, 1024);
        send(sockfd , buffer , strlen(buffer) , 0 ); 
	memset(buffer, 0, 1024);
        valread = read( sockfd , buffer, 1024); 
	if(strcmp(buffer, "OK!") == 0){
	  printf("Success! Message box '%s' was created\n", arg2);
	}
	else if(strcmp(buffer, "EXIST") == 0)
	  printf("Error. Message box '%s' already exists\n", arg2);
	else{
	  printf("Error. Message box name must be between 5 and 25 characters long and start with an alphabetic character\n");
	memset(arg2, 0, 1024);}
      }
      else if(strcmp(buffer, "delete") == 0){
	printf("Okay, delete what box?\n");
	printf("delete:> ");
	scanf("%c", &tmp);
	scanf("%[^\n]", &arg2);
	memcpy(arg1, "DELBX " , 6);
	strcat(arg1, arg2);
	memset(buffer, 0, 1024);
	strcat(buffer, arg1);
	send(sockfd , buffer , strlen(buffer) , 0 ); 
	memset(buffer, 0, 1024);
        valread = read( sockfd , buffer, 1024); 
	if(strcmp(buffer, "OK!") == 0)
	  printf("Success! Message box '%s' was deleted\n", arg2);
    	else if(strcmp(buffer, "NEXST") == 0)
	  printf("Error. Message box '%s' does not exist\n", arg2);
	else if(strcmp(buffer, "OPEND") == 0)
	  printf("Error. Message box is currently open\n");
	else if(strcmp(buffer, "NOTMT") == 0)
	  printf("Error. Attempting to delete a message box that still has messages\n");
	else if(strcmp(buffer, "WHAT?") == 0)
	  printf("Error. Command is in some way broken or malformed\n");
      }
      else if(strcmp(buffer, "open") == 0){
	printf("Okay, open which message box?\n");
	printf("open:> ");
	scanf("%c", &tmp);
	scanf("%[^\n]", &arg2);
	printf("arg is: %s", arg2);
	memcpy(arg1, "OPNBX " , 6);
	strcat(arg1, arg2);
	memset(buffer, 0, 1024);
	strcat(buffer, arg1);
	send(sockfd , buffer , strlen(buffer) , 0 ); 
	memset(buffer, 0, 1024);
        valread = read( sockfd , buffer, 1024); 
	if(strcmp(buffer, "OK!") == 0)
	  printf("Success! Message box '%s' was opened\n", arg2);
    	else if(strcmp(buffer, "NEXST") == 0)
	  printf("Error. Message box '%s' does not exist\n", arg2);
	else if(strcmp(buffer, "OPEND") == 0)
	  printf("Error. Message box is currently open\n");
	else if(strcmp(buffer, "ANOPN") == 0)
	  printf("Error. Close current message box before opening a new one\n");
	else if(strcmp(buffer, "WHAT?") == 0)
	  printf("Error. Command is in some way broken or malformed");
	memset(arg2, 0, 1024);
	memset(arg1, 0, 1024);
      }
      else if(strcmp(buffer, "put") == 0){
	printf("Okay, what message do you want to put?\n");
	printf("put:> ");
	scanf("%c", &tmp);
	scanf("%[^\n]", &arg2);
	sprintf(bytes, "%d", strlen(arg2));
	memcpy(arg1, "PUTMG!" , 6);
	strcat(arg1, bytes);
	strcat(arg1, "!");
	strcat(arg1, arg2);
	memset(buffer, 0, 1024);
	strcat(buffer, arg1);
	printf("buffer is now: %s\n", buffer);
	send(sockfd , buffer , strlen(buffer) , 0 ); 
	memset(buffer, 0, 1024);
        valread = read( sockfd , buffer, 1024); 

        if(strcmp(buffer, "NOOPN") == 0)
	  printf("Error. There is no message box open\n", arg2);
	else if(strcmp(buffer, "WHAT?") == 0)
	  printf("Error. Command is in some way broken or malformed\n");
	else{
	  printf("Success! The message has been added\n", arg2);
	}
	memset(arg2, 0, 1024);
	memset(arg1, 0, 1024);
	memset(bytes, 0, 1024);
      }
      else if(strcmp(buffer, "close") == 0){
	printf("Okay, close what box?\n");
	printf("close:> ");
	scanf("%c", &tmp);
	scanf("%[^\n]", &arg2);
	memcpy(arg1, "CLSBX " , 6);
	strcat(arg1, arg2);
	memset(buffer, 0, 1024);
	strcat(buffer, arg1);
	printf("buffer is now: %s\n", buffer);	
	send(sockfd , buffer , strlen(buffer) , 0 ); 
	memset(buffer, 0, 1024);
        valread = read( sockfd , buffer, 1024); 
	if(strcmp(buffer, "OK!") == 0)
	  printf("Success! The message has been added\n", arg2);
        else if(strcmp(buffer, "NOOPN") == 0)
	  printf("Error. Message box '%s' is not open\n", arg2);
	else if(strcmp(buffer, "WHAT?") == 0)
	  printf("Error. Command is in some way broken or malformed\n");
      }
      else if(strcmp(buffer, "next") == 0){
	memset(buffer, 0, 1024);
	memcpy(buffer, "NXTMG", 5);
        send(sockfd , buffer , strlen(buffer) , 0 ); 
	memset(buffer, 0, 1024);
	valread = read( sockfd , buffer, 1024);
	if(strcmp(buffer, "EMPTY") == 0)
	  printf("Error. There are no messages left in this message box\n");
	else if(strcmp(buffer, "NOOPN") == 0)
	  printf("Error. You have no message box open\n");
	else if(strcmp(buffer, "WHAT?") == 0)
	  printf("Error. Your command is in somve way broken or malformed\n");
	else
	  printf("%s\n", buffer);
      }
      else if(strcmp(buffer, "help") == 0){
	printf("quit\n");
	printf("create\n");
	printf("delete\n");
	printf("open\n");
	printf("close\n");
	printf("next\n");
	printf("put\n");
      }

      //for any misunderstood input
      else{
	send(sockfd, buffer, strlen(buffer), 0);
	memset(buffer, 0, 1024);
	valread = read( sockfd , buffer, 1024);
	if(strcmp(buffer, "WHAT?") == 0){
	  printf("That is not a command, for a command list enter 'help'\n");
      	}
	else if(strcmp(buffer, "badmsg") == 0){
		error("client disconnected and shut down. connect via HELLO next time.");	
	}
	else if(strcmp(buffer, "HELLO DUMBv0 ready!") == 0){
		printf("HELLO DUMBv0 ready!\n");	
	}
	}
	
      memset(buffer, 0, 1024);
      printf("> ");
      scanf("%c", &tmp);
      scanf("%[^\n]", &buffer);
    }
    send(sockfd , buffer , strlen(buffer) , 0 ); 

    return 0; 
} 

