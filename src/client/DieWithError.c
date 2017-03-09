/*  DieWithError.c
  Command_Line_Chat
  Created by Raymond Feng on 2/21/17.
  Copyright © 2017 Raymond Feng. All rights reserved.
*/ 

#include <stdio.h>
#include <stdlib.h>

void DieWithError(char *errorMessage){
    perror(errorMessage);
    exit(1);
}
