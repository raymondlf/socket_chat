//
//  client.c
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
#include <unistd.h>

#define BUFFER_SIZE 1024
#define USERNAME_LIMIT 64
#define PASSWORD_LIMIT 64
#define CONNECTED 1
#define DISCONNECTED 0
#define ONLINE 1
#define OFFLINE 0
#define MAX_PENDING 1

void ConnectToServer();
void Login();
void GetUserList();
void SendMessage();
void GetMessage();
void InitChat();
void Chat();
void ResetBuffer(char *);

char option[2];
char usr_status[2];
char username[USERNAME_LIMIT];
char password[PASSWORD_LIMIT];
char targetname[USERNAME_LIMIT];


unsigned short conn_status;
unsigned short status;

int sock;
int localSock, targetSock;
struct sockaddr_in serverAddr;
unsigned short localPort, targetPort;
struct sockaddr_in localAddr, targetAddr;
unsigned short serverPort;
/* pointer does not work well with scanf */
char ipAddr[16];
char* serverIP = ipAddr;

unsigned int dataLength;
unsigned int targetLength, localLength;

char buffer[BUFFER_SIZE];
char* ptr; // pointer to manipulate buffer

void DieWithError(char *errorMessage);
void inChat();

int main(int argc, char* args[]){
    
    /* setup initial user status to offline*/
    ptr = &usr_status[0];
    strcpy(ptr, "0\0");
    
    /* setup buffer */
    
    //display option
    printf("Welcome. Select from one of the options below by entering a digit:\n\n");
    while(1){// loop forever until exit
        memset(buffer, 0, sizeof(buffer));
        printf("------------------------------------------------\n");
        printf("\t1. Connect to the server\n");
        printf("\t2. Get the user list\n");
        printf("\t3. Send a message\n");
        printf("\t4. Get my message\n");
        printf("\t5. Initiate a chat with my friend\n");
        printf("\t6. Chat with my friend\n");
        printf("\t0. exit application\n");
        printf("------------------------------------------------\n");
        printf("> ");
        short choice;
        scanf("%hd", &choice);
        switch (choice) {
            case 0: // exit app
                if(sock > 0){
                    ResetBuffer(buffer);
                    buffer[0] = '7';
                    if(send(sock, buffer, BUFFER_SIZE, 0) < 0)
                        DieWithError("send() failed on exit");
                    close(sock);
                    ResetBuffer(buffer);
                }
                return 0;
            case 1: // connect to server
                ConnectToServer();
                ResetBuffer(buffer);
                break;
            case 2:
                GetUserList();
                ResetBuffer(buffer);
                break;
            case 3:
                SendMessage();
                ResetBuffer(buffer);
                break;
            case 4:
                GetMessage();
                ResetBuffer(buffer);
                break;
            case 5:
                InitChat();
                ResetBuffer(buffer);
                break;
            case 6:
                Chat();
                ResetBuffer(buffer);
                break;
            default:
                printf("input error\n");
                break;
        }
    }
    return 0;
}

void ConnectToServer(){
    if(conn_status == CONNECTED){
        if(usr_status[0] == '0'){
            printf("Already connected. Please login.\n");
            Login();
            return;
        }
        else{
            printf("Already connected and login!\n");
            return;
        }
    }
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError(" socket () failed") ;
    printf("Please input server IP address\n");
    printf("(IPaddress) > ");
    scanf(" %16[^\n]", ipAddr);
    printf("Please input server port number\n");
    printf("Port number) > ");
    scanf("%hd", &serverPort);
    printf("------------------------------------------------\n");
    printf("Connecting to %s:%hd ...\n", serverIP, serverPort);
    
    /* Construct serverAddr struct */
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(serverPort);
    
    /* Establish connection to specified server */
    if(connect(sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0){
        DieWithError("connect() failed");
    }
    else{
        conn_status = CONNECTED; // update connection status
        printf("Connected!\n");
        printf("------------------------------------------------\n");
        
        Login();
    }
    ResetBuffer(buffer);
}

void Login(){
    ResetBuffer(buffer);
    printf("------------------------------------------------\n");
    printf("Please input user name\n");
    printf("(User name) > ");
    scanf(" %63[^\n]", username);
    printf("Please input password\n");
    printf("Password) > ");
    scanf(" %63[^\n]", password);
    
    /* construct buffer to send */
    // requestID: request to log in
    ptr = &buffer[0];
    strcpy(ptr, "0\0");
    // user name
    ptr = &buffer[4];
    strcpy(ptr, username);
    // password
    ptr = &buffer[68];
    strcpy(ptr, password);
    
    printf("Logging in ...\n");
    
    if(send(sock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("send() failed");
    }
    
    ResetBuffer(buffer);
    
    if(recv(sock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("recv() failed");
    }
    if((strcmp(buffer, "1")) == 0){ // login success
        usr_status[0] = '1';
        status = ONLINE;
        printf("Login success!\n");
        printf("------------------------------------------------\n");
    }
    else{
        printf("Login failed!\n");
        printf("------------------------------------------------\n");
    }
}

void GetUserList(){
    if(conn_status == DISCONNECTED){
        printf("Not connected to server.\n");
        return;
    }
    else if(status == OFFLINE){
        printf("Not logged in.\n");
        return;
    }
     // Request ID
    ptr = &buffer[0];
    strcpy(ptr, "1");
     // User status
    ptr = &buffer[2];
    strcpy(ptr, usr_status);
     // username
    ptr = &buffer[4];
    strcpy(ptr, username);

    if(send(sock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("send() failed in GetUserList()");
    }
    
    ResetBuffer(buffer);
    
    if(recv(sock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("recv() failed in GetUserList()");
    }
    
    /* print user list */
    ptr = &buffer[0];
    int last_index = atoi(ptr); // index of last user in userlist
    printf("There are totally %d user(s).\n", last_index + 1);
    for(int i = 0; i <= last_index; i++){
        ptr = &buffer[2 + 65 * i];
        printf("%s\n", ptr);
    }
}

void SendMessage(){
    if(conn_status == DISCONNECTED){
        printf("Not connected to server.\n");
        return;
    }
    else if(status == OFFLINE){
        printf("Not logged in.\n");
        return;
    }

    /* setup basic info in buffer */
    /* request ID */
    ptr = &buffer[0];
    strcpy(ptr, "2\0");
    /* user status */
    ptr = &buffer[2];
    strcpy(ptr, usr_status);
    /* user name */
    ptr = &buffer[4];
    strcpy(ptr, username);
    
    /* get recipient and message */
    printf("(Recipient) > ");
    scanf(" %63[^\n]", &buffer[68]);
    printf("(Message) > ");
    scanf(" %99[^\n]", &buffer[131]);

    if(send(sock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("send() failed in SendMessage()!");
    }
    ResetBuffer(buffer);
    
    /* wait for send result */
    if(recv(sock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("recv() failed in SendMessage()!");
    }
    if(strcmp(buffer, "1") == 0){
        printf("------------------------------------------------\n");
        printf("Message sent.\n");
        printf("------------------------------------------------\n");
    }
    else{
        printf("------------------------------------------------\n");
        printf("Failed. Please try again.\n");
        printf("Recipient might not be in the user list, or his/her mailbox is full. \n");
        printf("------------------------------------------------\n");
    }
}

void GetMessage(){
    if(conn_status == DISCONNECTED){
        printf("Not connected to server.\n");
        return;
    }
    else if(status == OFFLINE){
        printf("Not logged in.\n");
        return;
    }
    /* setup basic info in buffer */
    /* request ID */
    ptr = &buffer[0];
    strcpy(ptr, "3"); // request to get message
    /* user name */
    ptr = &buffer[2];
    strcpy(ptr, username);
    
    if(send(sock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("send() failed in GetMessage()");
    }
    ResetBuffer(buffer);
    
    /* wait for getMessage result */
    if(recv(sock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("recv() failed in SendMessage()!");
    }
    
    int lastIndex = (int) buffer[0];
    if(lastIndex < 48){
        printf("You don't have any message.\n");
        return;
    }
    ptr = &buffer[0];
    lastIndex = atoi(ptr);
    
    printf("You have %d message(s):\n", lastIndex + 1);
    
    /* receive and print message */
    for(int i = 0; i <= lastIndex; i++){
        ResetBuffer(buffer);
        if(recv(sock, buffer, BUFFER_SIZE, 0) < 0){
            DieWithError("recv() failed in SendMessage()!");
        }
        printf("%s\n", buffer);
    }
    printf("------------------------------------------------\n");
    return;
}

void InitChat(){
    /* disconnect to server */
    if(conn_status == CONNECTED){
        ResetBuffer(buffer);
        buffer[0] = '7';
        if(send(sock, buffer, BUFFER_SIZE, 0) < 0)
            DieWithError("send() failed on InitChat");
        close(sock);
        ResetBuffer(buffer);
        conn_status = DISCONNECTED;
        status = OFFLINE;
        memset(usr_status, 0, 2);
        strcpy(usr_status, "0");
    }

    /* create local socket and target socket for incoming chat */
    memset(username, 0, USERNAME_LIMIT);
    
    printf("Please input your name.\n");
    printf("(Your user name) > ");
    scanf(" %63[^\n]", username);
    
    printf("Please input your port number.\n");
    printf("(Port number) > ");
    scanf("%hd", &localPort);
    
    printf("Please input the name of the person you want to chat with\n");
    printf("(Target name) > ");
    scanf(" %63[^\n]", targetname);
    
    if((localSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("sockeet() failed in InitChat()");
    
    /* Construct local address structure */
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(localPort);
    
    /* Bind to the local address */
    if(bind(localSock, (struct sockaddr*) &localAddr, sizeof(localAddr)) < 0){
        DieWithError("bind() failed");
    }

    /* Mark the socket so it will listen for incoming connections */
    if(listen(localSock, MAX_PENDING) < 0){
        DieWithError("listen() failed");
    }

    /* Set the size of the in-out parameter */
    targetLength = sizeof(targetAddr);
    
    printf("Please wait for the other user to connect.\n");
    
    /* accept connection request */
    if ((targetSock = accept(localSock, (struct sockaddr*) &targetAddr, &targetLength)) < 0) {
        DieWithError("accept() failed");
    }
    
    // send init message
    char* initMessage = "Connection established. We can chat now.";
    ptr = &buffer[0];
    strcpy(ptr, "0");
    ptr = &buffer[2];
    strcpy(ptr, username);
    ptr = &buffer[66];
    strcpy(ptr, initMessage);
    if(send(targetSock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("send failed in InitChat()");
    }
    printf("%s > %s\n", username, ptr);
    printf("(Please start sending message.)\n");
    
    while(1){ /* run forever */
        /* send message */
        ResetBuffer(buffer);
        printf("%s > ", username);
        scanf(" %958[^\n]", &buffer[66]);
        ptr = &buffer[66];
        if(strcmp(ptr, "quit") == 0) { // user quit
            ResetBuffer(buffer);
            ptr = &buffer[0];
            strcpy(ptr, "1");
            if(send(targetSock, buffer, BUFFER_SIZE, 0) < 0){
                DieWithError("send failed in InitChat()");
            }
            close(localSock);
            return;
        }

        ptr = &buffer[0];
        strcpy(ptr, "0");
        ptr = &buffer[2];
        strcpy(ptr, username);
        
        
        if(send(targetSock, buffer, BUFFER_SIZE, 0) < 0){
            DieWithError("send() failed in InitChat()");
        }
        
        ResetBuffer(buffer);
        
        /* get message buffer */
        if(recv(targetSock, buffer, sizeof(buffer), 0) < 0){
            DieWithError("recv() failed");
        }
        ptr = &buffer[0];
        if(strcmp(ptr, "1") == 0){ // target user quit
            printf("The other user quit chatting.\n");
            close(localSock);
            return;
        }
        // print received message
        ptr = &buffer[66];
        printf("%s > %s\n", targetname, ptr);
        
    }

}

void Chat(){
    /* disconnect to server */
    if(conn_status == CONNECTED){
        ResetBuffer(buffer);
        buffer[0] = '7';
        if(send(sock, buffer, BUFFER_SIZE, 0) < 0)
            DieWithError("send() failed on exit");
        close(sock);
        ResetBuffer(buffer);
        conn_status = DISCONNECTED;
        status = OFFLINE;
        memset(usr_status, 0, 2);
        strcpy(usr_status, "0");
    }
    
    /* create local socket and target socket for incoming chat */
    
    memset(&username, 0, sizeof(username));
    
    printf("(Your user name) > ");
    scanf(" %63[^\n]", username);
    
    printf("(Target name) > ");
    scanf(" %63[^\n]", targetname);
    
    printf("(Target IP) > ");
    memset(ipAddr, 0, 16);
    scanf(" %16[^\n]", ipAddr); // pointer serverIP is pointing at the head of this array
    
    printf("(Target Port) > ");
    scanf("%hd", &targetPort);
    
    
    if ((localSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("sockeet() failed in InitChat()");
    
    
    /* Construct local address structure */
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(localPort);
    
    /* Construct target address structure */
    memset(&targetAddr, 0, sizeof(targetAddr));
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_addr.s_addr = inet_addr(serverIP);
    targetAddr.sin_port = htons(targetPort);
    
    
    if(connect(localSock, (struct sockaddr*) &targetAddr, sizeof(targetAddr)) < 0){
        DieWithError("connect() failed in Chat()");
    }
    
    
    /* receive init message */
    ResetBuffer(buffer);
    if(recv(localSock, buffer, BUFFER_SIZE, 0) < 0){
        DieWithError("recv() failed in InitChat()");
    }
    /* print init message */
    ptr = &buffer[66];
    printf("%s > %s\n", targetname, ptr);
    
    printf("Please wait for the next message.\n");
    ResetBuffer(buffer);
    
    /* get message buffer */
    if(recv(localSock, buffer, sizeof(buffer), 0) < 0){
        DieWithError("recv() failed");
    }
    
    ptr = &buffer[66];
    printf("%s > %s\n", targetname, ptr);
    
    printf("You can start sending message.\n");
    /* start chatting */
    while(1){ /* run forever */
        /* send message */
        ResetBuffer(buffer);
        printf("%s > ", username);
        scanf(" %958[^\n]", &buffer[66]);
        ptr = &buffer[66];
        if(strcmp(ptr, "quit") == 0) { // user quit
            ResetBuffer(buffer);
            ptr = &buffer[0];
            strcpy(ptr, "1");
            if(send(localSock, buffer, BUFFER_SIZE, 0) < 0){
                DieWithError("send failed in InitChat()");
            }
            close(localSock);
            return;
        }
        ptr = &buffer[0];
        strcpy(ptr, "0");
        ptr = &buffer[2];
        strcpy(ptr, username);
        if(send(localSock, buffer, BUFFER_SIZE, 0) < 0){
            DieWithError("send() failed in InitChat()");
        }
        
        ResetBuffer(buffer);
        
        /* get message buffer */
        if(recv(localSock, buffer, sizeof(buffer), 0) < 0){
            DieWithError("recv() failed");
        }
        ptr = &buffer[0];
        if(strcmp(ptr, "1") == 0){ // target user quit
            printf("The other user quit chatting.\n");
            close(localSock);
            return;
        }
        // print received message
        ptr = &buffer[66];
        printf("%s > %s\n", targetname, ptr);
        
    }

}

void ResetBuffer(char* target){
    memset(target, 0, BUFFER_SIZE);
}
