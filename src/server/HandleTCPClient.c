//
//  HandleTCPClient.c
//  Command_Line_Chat
//
//  Created by Raymond Feng on 2/21/17.
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
#define SUCCESS 1
#define FAILED 0

#define MAX_PENDING 5
#define MAX_USERS 10
#define MAX_MESSAGES 10

/* request ID */
#define LOGIN 0
#define GETLIST 1
#define SEND 2
#define GET 3
#define BREAK 7

void DieWithError(char* errorMessage);
void ResetBuffer(char*);
void Login(int clientSock, char *buffer, struct usrList*);
void GetList(int clientSock, char *buffer, struct usrList*);
void SendMessage(int clientSock, char *buffer, struct usrList*); //something is wrong
void GetMessage(int clientSock, char *buffer, struct usrList *userList);


void HandleTCPClient(int clientSock, struct usrList *userList){
    char* ptr;
    char buffer[BUFFER_SIZE];
    while(1){
        ResetBuffer(buffer);
        /* get message buffer */
        if(recv(clientSock, buffer, BUFFER_SIZE, 0) < 0){
            DieWithError("recv() failed");
        }
        /* get requestID */
        ptr = &buffer[0];
        int request = atoi(ptr);
        switch (request) {
            case LOGIN:
                printf("------------------------------------------------\n");
                printf("login request received!\n");
                Login(clientSock, buffer, userList);
                ResetBuffer(buffer);
                printf("------------------------------------------------\n");
                break;
            case GETLIST:
                printf("------------------------------------------------\n");
                printf("Get user list request received!\n");
                GetList(clientSock, buffer, userList);
                ResetBuffer(buffer);
                printf("------------------------------------------------\n");
                break;
            case SEND:
                printf("------------------------------------------------\n");
                printf("Send message request received!\n");
                SendMessage(clientSock, buffer, userList);
                ResetBuffer(buffer);
                printf("------------------------------------------------\n");
                break;
            case GET:
                printf("------------------------------------------------\n");
                printf("Get message request received!\n");
                GetMessage(clientSock, buffer, userList);
                ResetBuffer(buffer);
                printf("------------------------------------------------\n");
                break;
            case BREAK:
                printf("------------------------------------------------\n");
                printf("client disconnected\n");
                if(clientSock > 0)
                    close(clientSock);
                ResetBuffer(buffer);
                printf("------------------------------------------------\n");
                return;
            default:
                DieWithError("Invalid request!");
        }
    }
    return;
}

void Login(int clientSock, char *buffer, struct usrList *userList){
    char* ptr;
    char username[64];
    char password[64];
    ptr = &buffer[4];
    strcpy(username, ptr);
    ptr = &buffer[68];
    strcpy(password, ptr);
    printf("Loging in %s ...\n", username);
    
    for(int i = 0; i < userList->numUsr; i++){
        if(strcmp(username, userList->usr[i].name) == 0){ // user found
            if(strcmp(password, userList->usr[i].password) == 0){ //check password
                printf("%s login success!\n", username);
                printf("------------------------------------------------\n");
                ResetBuffer(buffer);
                buffer[0] = '1';
                if(send(clientSock, buffer, BUFFER_SIZE, 0) < 0){
                    DieWithError("send() failed in Login()");
                }
                return;
            }
            else{
                printf("Wrong password\n");
                printf("------------------------------------------------\n");
                ResetBuffer(buffer);
                buffer[0] = '0';
                if(send(clientSock, buffer, BUFFER_SIZE, 0) < 0){
                    DieWithError("send() failed in Login()");
                }
                return;
            }
        }
    }
    printf("User %s not found.\n", username);
    ResetBuffer(buffer);
    buffer[0] = '0';
    if(send(clientSock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("send() failed in Login()");
    }
}

void GetList(int clientSock, char *buffer, struct usrList *userList){
    printf("In GegList()\n");
    ResetBuffer(buffer);
    
    char* ptr;
    char last_index = (char) (48 + userList->numUsr - 1);
    ptr = &buffer[0];
    *ptr = last_index;
    
    /* put usernames on buffer. 65 bytes each. reserve last byte for \0 */
    for(int i = 0; i < userList->numUsr; i++){
        ptr = &buffer[2 + i * 65];
        strcpy(ptr, userList->usr[i].name);
    }
    
    if(send(clientSock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("send() failed in GetList()");
    }
}

void SendMessage(int clientSock, char *buffer, struct usrList *userList){
    printf("In SendMessage()\n");
    char* ptr;
    char username[64];
    char recipient[64];
    ptr = &buffer[4];
    strcpy(username, ptr);
    ptr = &buffer[68];
    strcpy(recipient, ptr);
    
    for(int i = 0; i < userList->numUsr; i++){
        if(strcmp(recipient, userList->usr[i].name) == 0){ // recipient user found
            if(userList->usr[i].numMessage < MAX_MESSAGE){ // check if mailbox is full
                ptr = &buffer[131];
                char *mailbox = userList->usr[i].message[userList->usr[i].numMessage];
                memset(mailbox, 0, BUFFER_SIZE);
                strcpy(mailbox, ptr);
                userList->usr[i].numMessage++;
                ResetBuffer(buffer);
                buffer[0] = '1';
                if(send(clientSock, buffer, BUFFER_SIZE, 0) < 0){
                    DieWithError("send() failed in SendMessage()");
                }
                return;
            }
            else{ // mailbox full
                printf("%s mailbox full.\n", recipient);
                ResetBuffer(buffer);
                buffer[0] = '0';
                if(send(clientSock, buffer, BUFFER_SIZE, 0) < 0){
                    DieWithError("send() failed in SendMessage()");
                }
                return;
            }
        }
    }
    printf("%s not found.\n", recipient); // user not found
    ResetBuffer(buffer);
    buffer[0] = '0';
    if(send(clientSock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("send() failed in SendMessage()");
    }
    return;
}

void GetMessage(int clientSock, char *buffer, struct usrList *userList){
    printf("In GetMessage()\n");
    /* fetch username from buffer */
    char* ptr;
    int usrID = 0;
    int lastIndex = -1;
    char username[64];
    ptr = &buffer[2];
    strcpy(username, ptr);
    
    for(int i = 0; i < userList->numUsr; i++){
        if(strcmp(username, userList->usr[i].name) == 0)
            usrID = i;
    }
    /* notify the user of the last message index */
    ResetBuffer(buffer);
    lastIndex = userList->usr[usrID].numMessage - 1;
    buffer[0] = (char) (lastIndex + 48);
    if(send(clientSock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("send() failed in SendMessage()");
    }
    
    /* send message */
    for(int i = 0; i <= lastIndex; i++){
        ResetBuffer(buffer);
        strcpy(buffer, userList->usr[usrID].message[i]);
        if(send(clientSock, buffer, BUFFER_SIZE, 0) < 0){
            DieWithError("send() failed in SendMessage()");
        }
    }
    
    /* clearing message list */
    for(int i = 0; i <= lastIndex; i++){
        ResetBuffer(userList->usr[usrID].message[i]);
        userList->usr[usrID].numMessage = 0;
    }
}

void ResetBuffer(char* target){
    memset(target, 0, BUFFER_SIZE);
}
