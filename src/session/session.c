#include "session.h"
#include <string.h>
#include <stdio.h>


#define SESSION_CONECTION_NAME_SIZE 100


void __get_connection_name(char *result, char *login, char *type)
{
    sprintf(result, "%s_%s", login, type);
}


int session_init(ClientSession *session, char *login)
{
    char input_name[SESSION_CONECTION_NAME_SIZE];
    char output_name[SESSION_CONECTION_NAME_SIZE];

    __get_connection_name(input_name, login, "input");
    __get_connection_name(output_name, login, "output");

    int res1 = connection_create(&(session->input), input_name, READ);
    int res2 = connection_create(&(session->output), output_name, WRITE);
    strcpy(session->login, login);

    if(res1 == -1 || res2 == -1)
        return -1;

    return 0;
}


int session_restore(ClientSession *session)
{
    char output_name[SESSION_CONECTION_NAME_SIZE];

    if(session->output.descriptor == -1)
    {
        __get_connection_name(output_name, session->login, "output");
        int res = connection_connect(&(session->output), output_name, WRITE);
        return res;
    }

    return 0;
}


void session_close(ClientSession *session)
{
    connection_close(&(session->input));
    connection_close(&(session->output));
    strcpy(session->login, "");
}


PipeConnection *session_input(ClientSession *session)
{
    return &(session->input);
}


PipeConnection *session_output(ClientSession *session)
{
    return &(session->output);
}


char *session_login(ClientSession *session)
{
    return session->login;
}
