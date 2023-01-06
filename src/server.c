#include "./connection/connection.h"
#include "message/message.h"
#include "message/io/io.h"
#include "session/storage/storage.h"
#include "message/storage/storage.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>


/* Server variables */

PipeConnection auth;
MessageStorage storage;
MessageStorage unwatched;
SessionStorage sessions;

pthread_t auth_thread;

void close_auth()
{
    printf("[Server] Closing auth pipe...\n");
    pthread_cancel(auth_thread);
    pthread_join(auth_thread, NULL);
    connection_close(&auth);
}


void close_sessions()
{
    printf("[Server] Closing sessions...\n");
    session_storage_delete(&sessions);
}


void interrupt_handler(int sig)
{
    exit(0);
}


char *get_server_id(int argc, char *argv[])
{
    if(argc < 2)
    {
        perror("Please, write server name as first argument\n");
        exit(1);
    }

    return argv[1];
}


char *server_path(char *server_id, char *conn_local_name)
{
    size_t fullname_len = strlen(server_id) + strlen(conn_local_name) + 2;
    char *fullname = (char *) malloc(fullname_len);

    sprintf(fullname, "%s_%s", server_id, conn_local_name);
    return fullname;
}


void launch_server(char *server_id)
{
    printf("[Server] Launching...\n");
    char *auth_connection_name = server_path(server_id, "auth");

    // 1. Launch auth pipe
    printf("\t1. Auth connection ");
    if(connection_exists(auth_connection_name))
    {
        printf("[Error]\n");
        exit(1);
    }
    else
    {
        printf(": Create ");
        if(connection_create(&auth, auth_connection_name, READ) == -1)
        {
            printf("[Error]\n");
            exit(1);
        }
    }
    printf("[OK]\n");
    atexit(close_auth);

    // 2. Init session storage
    printf("\t2. Session storage [OK]\n");
    session_storage_init(&sessions, server_id);
    atexit(close_sessions);

    // 3. Launch message database
    printf("\t3. Message storage ");
    if(message_storage_init(&storage, "messages.db") == -1)
    {
        printf("[Error]\n");
        exit(1);
    }
    printf("[OK]\n");

    // 4. Launch unwatched messages database
    printf("\t4. Unwatched messages storage ");
    if(message_storage_init(&unwatched, "unwatched.db") == -1)
    {
        printf("[Error]\n");
        exit(1);
    }
    printf("[OK]");

    free(auth_connection_name);
}


/* Actions */

void send_message_history(char *sender, char *target_filter, char *sender_filter)
{
    Message reply;
    ClientSession *sender_session;
    MessageReader reader;
    MessageRecord record;

    printf("[Server] Collecting message history of user '%s'...\n", sender);

    message_create(&reply);
    sender_session = session_storage_get(&sessions, sender);
    reader = message_storage_select(&storage);

    if(sender_session == NULL)
    {
        message_delete(&reply);
        printf("[Server] User '%s' offline\n", sender);
        return;
    }

    if(session_restore(sender_session) == -1)
    {
        message_delete(&reply);
        printf("[Server] Can't restore connection\n");
        return;
    }

    message_add_pair(&reply, "type", "history");
    
    while(message_reader_next(&reader, &record) != -1)
    {
        char *repr;
        size_t repr_size;

        if(sender_filter != NULL)
            if(strstr(record.from, sender_filter) == NULL)
                continue;

        if(target_filter != NULL)
            if(strstr(record.to, target_filter) == NULL)
                continue;

        repr_size = strlen(record.from) + strlen(record.to) + strlen(record.content) + 2;
        repr = (char *) malloc(repr_size);

        sprintf(repr, "%s,%s,%s", record.from, record.to, record.content);

        message_add_pair(&reply, "_", repr);

        free(repr);
    }

    if(message_write(connection_descriptor(&(sender_session->output)), &reply) == -1)
    {
        message_delete(&reply);
        printf("[Server] Can't write to output pipe\n");
        return;
    }

    printf("[Server] Sent!\n");
    message_delete(&reply);
}


void save_message(char *sender, char *target, char *content)
{
    printf("[Server] Save message...\n");

    MessageRecord record;
    strcpy(record.from, sender);
    strcpy(record.to, target);
    strcpy(record.content, content);
    
    if(message_storage_add(&storage, &record) == -1)
    {
        printf("[Server] Error: Can't add record to db. Reason: %s\n", strerror(errno));
        exit(1);
    }

    printf("[Server] Message saved\n");
}


void send_message(char *sender, char *target, char *content)
{
    ClientSession *target_session;
    Message target_message;

    message_create(&target_message);
    message_add_pair(&target_message, "type", "incoming_message");
    message_add_pair(&target_message, "sender", sender);
    message_add_pair(&target_message, "target", target);
    message_add_pair(&target_message, "content", content);

    printf("[Server] Send message to '%s'\n", target);

    if(session_storage_exists(&sessions, target))
    {
        target_session = session_storage_get(&sessions, target);

        if(session_restore(target_session) != -1)
        {
            if(message_write(connection_descriptor(session_output(target_session)), &target_message) == -1)
            {
                printf("[Server] Can't write to output pipe\n");
                exit(1);
            }
        }
        else
        {
            printf("[Server] Can't restore connection\n");
            exit(1);
        }
    }
    else
    {
        printf("[Server] User '%s' offline\n", target);
        MessageRecord record;
        strcpy(record.from, sender);
        strcpy(record.to, target);
        strcpy(record.content, content);
        message_storage_add(&unwatched, &record);
    }

    message_delete(&target_message);
}


void close_session(char *sender)
{
    printf("[Server] Close connection with '%s'\n", sender);
    session_storage_remove(&sessions, sender);
}


void check_unwatched_messages(char *login, int output)
{
    MessageReader reader;
    MessageStorage buffer;
    MessageRecord record;

    reader = message_storage_select(&unwatched);
    message_storage_init(&buffer, "buffer.db");

    while(message_reader_next(&reader, &record) != -1)
    {
        if(strcmp(record.to, login) == 0)
            send_message(record.from, record.to, record.content);
        else
            message_storage_add(&buffer, &record);
    }

    reader = message_storage_select(&buffer);
    message_storage_clear(&unwatched);

    while(message_reader_next(&reader, &record) != -1)
        message_storage_add(&unwatched, &record);

    message_storage_clear(&buffer);
}


/* Threads */

void *handle_user_requests(void *vargp)
{
    char *login = (char *) vargp;
    ClientSession *session = session_storage_get(&sessions, login);
    int input_desc = connection_descriptor(session_input(session));
    int output_desc = connection_descriptor(session_output(session));

    printf("[Server] Checking unwatched dmessages...\n");

    check_unwatched_messages(login, output_desc);

    printf("[Server] Waiting messages from '%s'...\n", login); 

    while(1)
    {
        Message request;

        if(message_read(input_desc, &request) == -1)
        {
            printf("[Server] Can't read message from '%s' input pipe. Reason: %s\n", login, strerror(errno));
            printf("\tDescriptor: %d\n", connection_descriptor(session_input(session)));
            exit(1);
        }

        if(!message_empty(&request))
        {
            printf("[Server] Handle message from '%s'...\n", session_login(session));
            char *type = message_get(&request, "type");

            if(strcmp(type, "send") == 0)
            {
                char *target = message_get(&request, "target");
                char *content = message_get(&request, "content");

                send_message(login, target, content);
                save_message(login, target, content);
            }
            else if(strcmp(type, "history") == 0)
            {
                char *target_filter = message_get(&request, "target");
                char *sender_filter = message_get(&request, "sender");
                send_message_history(login, target_filter, sender_filter);
            }
            else if(strcmp(type, "exit") == 0)
            {
                close_session(login);
                break;
            }
        }

        message_delete(&request);
    }

    free(login);
    return NULL;
}

void *auth_users(void *vargp)
{
    printf("[Server] Waiting messages...\n");

    while(1)
    {
        Message message;

        if(message_read(connection_descriptor(&auth), &message) == -1)
        {
            printf("[Server] Error: can't read from auth pipe. Reason: %s\n", strerror(errno));
            break;
        }

        if(!message_empty(&message))
        {
            printf("[Server] Got auth request\n");
            
            PipeConnection callback;
            Message reply;
            char *callback_name;
            char *login;

            callback_name = message_get(&message, "callback");
            login = message_get(&message, "login");

            if(connection_connect(&callback, callback_name, WRITE) == -1)
            {
                printf("[Server] Error: can't connect to callback pipe. Reason: %s\n", strerror(errno));
                break;
            }

            message_create(&reply);

            if(session_storage_exists(&sessions, login))
            {
                message_add_pair(&reply, "type", "error");
                message_add_pair(&reply, "message", "login taken");
            }
            else
            {
                ClientSession *new_session = session_storage_create(&sessions, login);
                message_add_pair(&reply, "type", "ok");
                message_add_pair(&reply, "input", new_session->input.name);
                message_add_pair(&reply, "output", new_session->output.name);

                pthread_t handle_thread;

                pthread_create(&handle_thread, NULL, handle_user_requests, strdup(login));
            }
    
            message_write(connection_descriptor(&callback), &reply);
            connection_close(&callback);
            message_delete(&reply);
        }

        message_delete(&message);
    }

    return NULL;
}

/* Main */

int main(int argc, char *argv[])
{
    signal(SIGINT, interrupt_handler);

    char *server_id = get_server_id(argc, argv);
    Message message;

    launch_server(server_id);

    pthread_create(&auth_thread, NULL, auth_users, NULL);

    pthread_exit(NULL);
    return 0;
}
