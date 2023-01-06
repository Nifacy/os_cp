#ifndef __SESSION_H__
#define __SESSION_H__


#include "../connection/connection.h"


#define LOGIN_STR_SIZE 50


/* Types */

typedef struct {
    char login[LOGIN_STR_SIZE];
    PipeConnection input;
    PipeConnection output;
} ClientSession;

/* Methods */

int session_init(ClientSession *session, char *login);
int session_restore(ClientSession *session);
void session_close(ClientSession *session);

PipeConnection *session_input(ClientSession *session);
PipeConnection *session_output(ClientSession *session);
char *session_login(ClientSession *session);

#endif