//
//  structs.h
//  Command_Line_Chat
//
//  Created by Raymond Feng on 2/22/17.
//  Copyright © 2017 Raymond Feng. All rights reserved.
//

#ifndef structs_h
#define structs_h

#define MAX_USERS 10
#define MAX_MESSAGE 10
/* user struct */
struct usr{
    char* name;
    char* password;
    unsigned int numMessage;
    char message[MAX_MESSAGE][1024];
};

struct usrList{
    struct usr usr[MAX_USERS];
    int numUsr;
};
#endif /* structs_h */
