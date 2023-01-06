#include "connection/connection.h"
#include "message/message.h"
#include "message/io/io.h"
#include "support/support.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

PipeConnection callback;
PipeConnection session_input;
PipeConnection session_output;
char *login;

pthread_t handle_server_reply_thread;

/* Exit handlers */

void close_session()
{
    pthread_cancel(handle_server_reply_thread);
    pthread_join(handle_server_reply_thread, NULL);
    connection_close(&session_input);
    connection_close(&session_output);
}

void interrupt_handler(int sig) { exit(0); }


char *server_path(char *server_id, char *conn_local_name)
{
    size_t fullname_len = strlen(server_id) + strlen(conn_local_name) + 2;
    char *fullname = (char *) malloc(fullname_len);

    sprintf(fullname, "%s_%s", server_id, conn_local_name);
    return fullname;
}


void connect_to_server(char *server_id)
{
    PipeConnection auth;
    Message message;
    char *auth_name;

    auth_name = server_path(server_id, "auth");
    message_create(&message);
    
    if(!connection_exists(auth_name))
    {
        printf("Server doesn't exist\n");
        exit(2);
    }

    if(connection_connect(&auth, auth_name, WRITE) == -1)
    {
        printf("Server unactive\n");
        exit(2);
    }

    message_add_pair(&message, "callback", callback.name);
    message_add_pair(&message, "login", login);

    if(message_write(connection_descriptor(&auth), &message) == -1)
    {
        printf("[Client] Can't write data to server: %s", strerror(errno));
    }

    printf("[Client] Listen callback...\n");
    while(1)
    {
        Message recv_msg;
        message_read(connection_descriptor(&callback), &recv_msg);

        if(!message_empty(&recv_msg))
        {
            char *type = message_get(&recv_msg, "type");

            if(strcmp(type, "ok") == 0)
            {
                printf("[Client] Connected!\n");
                printf("[Client] Connecting to input/output pipes...\n");
                
                printf("\t1.Input ");
                connection_connect(&session_input, message_get(&recv_msg, "input"), WRITE);
                printf("[OK]\n");
                
                printf("\t2.Output ");
                connection_connect(&session_output, message_get(&recv_msg, "output"), READ);
                printf("[OK]\n");
            }

            if(strcmp(type, "error") == 0)
            {
                printf("[Client] Error: %s\n", message_get(&recv_msg, "message"));
                exit(2);
            }

            break;
        }

        message_delete(&recv_msg);
    }

    message_delete(&message);
    free(auth_name);
    connection_close(&callback);
}


char *get_server_id(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Server id not specified\n");
        exit(1);
    }

    return argv[1];
}


char *get_login(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("Login not specified\n");
        exit(1);
    }

    return argv[2];
}


void init_callback()
{
    char callback_name[256];
    int client_id = 0;

    while(1)
    {
        sprintf(callback_name, "client%d_callback", client_id);
        
        if(!connection_exists(callback_name))
            break;

        client_id++;
    }

    if(connection_create(&callback, callback_name, READ) == -1)
    {
        printf("[Client] Can't create callback connection: %s\n", strerror(errno));
        exit(1);
    }
}


void *handle_server_reply(void *vargp)
{
    printf("[Client] Waiting messages...\n");

    while(1)
    {
        Message reply;

        if(message_read(connection_descriptor(&session_output), &reply) == -1)
        {
            printf("[Client] Error: read from session input pipe\n");
            exit(1);
        }

        if(!message_empty(&reply))
        {
            char *msg_type = message_get(&reply, "type");

            if(strcmp(msg_type, "incoming_message") == 0)
                printf("[Client] '%s': %s\n", message_get(&reply, "sender"), message_get(&reply, "content"));
            
            else if(strcmp(msg_type, "history") == 0)
            {
                for(int i = 0; i < reply.pairs_amount; i++)
                {
                    Pair *pair = reply.pairs + i;

                    if(strcmp(pair->key, "_") == 0)
                    {
                        char *from;
                        char *to;
                        char *content;

                        int pos1 = strcspn(pair->value, ",");
                        int pos2 = pos1 + 1 + strcspn(pair->value + pos1 + 1, ",");

                        pair->value[pos1] = '\0';
                        pair->value[pos2] = '\0';

                        from = pair->value;
                        to = pair->value + (pos1 + 1);
                        content = pair->value + (pos2 + 1);

                        printf("%s\t%s\t%s\n", from, to, content);
                    }
                }
            }
        }

        message_delete(&reply);
    }
}


/* Actions */

void send_message()
{
    Message request;
    char target[256];
    char content[256];

    scanf("%s", target);
    fscan_string(stdin, content);

    message_create(&request);
    message_add_pair(&request, "type", "send");
    message_add_pair(&request, "target", target);
    message_add_pair(&request, "content", content);

    if(message_write(connection_descriptor(&session_input), &request) == -1)
        printf("[Client] Error: can't write to server input pipe\n");

    message_delete(&request);
}


void parse_filter_parameter(char *value, char *key, Message *msg)
{
    if(strcmp(value, "ALL") == 0) return;
    if(strcmp(value, "ME") == 0)
    {
        message_add_pair(msg, key, login);
        return;
    }
    message_add_pair(msg, key, value);
}


void get_history()
{
    Message request;
    char target_filter[256];
    char sender_filter[256];

    scanf("%s %s", sender_filter, target_filter);

    message_create(&request);
    message_add_pair(&request, "type", "history");

    parse_filter_parameter(target_filter, "target", &request);
    parse_filter_parameter(sender_filter, "sender", &request);

    if(message_write(connection_descriptor(&session_input), &request) == -1)
    {
        printf("[Client] Error: can't write to server input pipe\n");
        return;
    }

    message_delete(&request);
}


void exit_request()
{
    Message request;

    message_create(&request);
    message_add_pair(&request, "type", "exit");

    printf("[Client] Send exit request...\n");

    if(message_write(connection_descriptor(&session_input), &request) == -1)
    {
        printf("[Client] Error: can't write to server input pipe\n");
        return;
    }

    message_delete(&request);
    exit(0);
}


int main(int argc, char *argv[])
{
    char *server_id = get_server_id(argc, argv);
    login = get_login(argc, argv);

    signal(SIGINT, interrupt_handler);

    init_callback();
    connect_to_server(server_id);
    atexit(close_session);

    pthread_create(&handle_server_reply_thread, NULL, handle_server_reply, NULL);

    while(1)
    {
        char cmd_type[256];

        printf("> ");
        scanf("%s", cmd_type);

        if(strcmp(cmd_type, "send") == 0) send_message();
        else if(strcmp(cmd_type, "history") == 0) get_history();
        else if(strcmp(cmd_type, "exit") == 0) exit_request();
        else printf("[Client] Unknown command\n");
    }

    pthread_exit(NULL);
    return 0;
}