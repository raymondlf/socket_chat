//
//  server.c
//  Command_Line_Chat
//
//  Created by Raymond Feng on 2/17/17.
//  Copyright © 2017 Raymond Feng. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "structs.h"

#define BUFFER_SIZE 1024
#define USERNAME_LIMIT 64
#define PASSWORD_LIMIT 64
#define CONNECTED 1
#define DISCONNECTED 0

#define MAX_PENDING 5
#define MAX_USERS 10

#define LOGIN 0
#define GETLIST 1
#define SEND 2
#define GET 3

struct usrList userList;
int serverSock;
int clientSock; // one socket for each customers
struct sockaddr_in serverAddr;
struct sockaddr_in clientAddr;
unsigned short serverPort;
unsigned int clientLength; // client address struct length
char* ptr; // pointer to manipulate buffer

void DieWithError(char *errorMessage);
void HandleTCPClient(int, struct usrList*);

int main(int argc, char* args[]){
    struct usr Alice;
    struct usr Bob;
    Alice.name = "alice";
    Alice.password = "1234";
    Alice.numMessage = 0;
    Bob.name = "bob";
    Bob.password = "1234";
    Bob.numMessage = 0;
    userList.numUsr = 2;
    userList.usr[0] = Alice;
    userList.usr[1] = Bob;
    
    /* test arguments */
    if(argc != 2){
        serverPort = 8000;
    }
    else{
        serverPort = atoi(args[1]);
    }

    
    /* Create socket for incoming connections */
    if((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("sockeet() failed");
    
    /* Construct local address structure */
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(serverPort);
    
    /* Bind to the local address */
    if(bind(serverSock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0){
        DieWithError("bind() failed");
    }
    
    /* Mark the socket so it will listen for incoming connections */
    if(listen(serverSock, MAX_PENDING) < 0){
        DieWithError("listen() failed");
    }
    
    while(1){ /* run forever */
        printf("Server on wait\n");
        printf("*************************************************\n");
        
        /* Set the size of the in-out parameter */
        clientLength = sizeof(clientAddr);
        
        /* accept connection request */
        if ((clientSock = accept(serverSock, (struct sockaddr*) &clientAddr, &clientLength)) < 0) {
            DieWithError("accept() failed");
        }
        
        printf("Handling client %s\n", inet_ntoa(clientAddr.sin_addr));
        
        HandleTCPClient(clientSock, &userList);
        
        printf("Finished handling %s\n", inet_ntoa(clientAddr.sin_addr));
    }
}
