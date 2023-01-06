#include "storage.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#define FULL_LOGIN_SIZE 100


void __login_full_name(char *result, char *server_id, char *login)
{
    sprintf(result, "%s_%s", server_id, login);
}


void session_storage_init(SessionStorage *storage, char *server_id)
{
    storage->sessions = NULL;
    storage->sessions_amount = 0;
    strcpy(storage->server_id, server_id);
}


bool session_storage_exists(SessionStorage *storage, char *login)
{
    char full_login[FULL_LOGIN_SIZE];

    __login_full_name(full_login, storage->server_id, login);

    for(int i = 0; i < storage->sessions_amount; i++)
        if(strcmp(storage->sessions[i].login, full_login) == 0)
            return true;

    return false;
}


ClientSession *session_storage_create(SessionStorage *storage, char *login)
{
    char full_login[FULL_LOGIN_SIZE];

    __login_full_name(full_login, storage->server_id, login);

    if(storage->sessions_amount == 0)
        storage->sessions = (ClientSession *) malloc(sizeof(ClientSession));
    else
        storage->sessions = (ClientSession *) realloc(storage->sessions, sizeof(ClientSession) * (storage->sessions_amount + 1));

    session_init(storage->sessions + storage->sessions_amount, full_login);
    storage->sessions_amount++;

    return storage->sessions + (storage->sessions_amount - 1);
}


ClientSession *session_storage_get(SessionStorage *storage, char *login)
{
    char full_login[FULL_LOGIN_SIZE];

    __login_full_name(full_login, storage->server_id, login);

    for(int i = 0; i < storage->sessions_amount; i++)
        if(strcmp(storage->sessions[i].login, full_login) == 0)
            return storage->sessions + i;

    return NULL;
}


void session_storage_remove(SessionStorage *storage, char *login)
{
    char full_login[FULL_LOGIN_SIZE];

    __login_full_name(full_login, storage->server_id, login);

    for(int i = 0; i < storage->sessions_amount; i++)
    {
        if(strcmp(storage->sessions[i].login, full_login) == 0)
        {
            ClientSession *session = &(storage->sessions[i]);
            ClientSession *arr;

            session_close(session);
            
            if(storage->sessions_amount > 1)
            {
                arr = (ClientSession *) malloc(sizeof(ClientSession) * (storage->sessions_amount - 1));
                memcpy(arr, storage->sessions, sizeof(ClientSession) * i);
                memcpy(arr + i, storage->sessions + i + 1, sizeof(ClientSession) * (storage->sessions_amount - i - 1));
            } else arr = NULL;

            free(storage->sessions);
            
            storage->sessions_amount--;
            storage->sessions = arr;

            break;
        }
    }
}


void session_storage_delete(SessionStorage *storage)
{
    for(int i = 0; i < storage->sessions_amount; i++)
        session_close(storage->sessions + i);
    
    free(storage->sessions);
}


SessionIterator session_storage_iter(SessionStorage *storage)
{
    SessionIterator iterator = {.cursor = 0, .storage = storage};
    return iterator;
}


int session_storage_next(SessionIterator *iterator, SessionPair *pair)
{
    if(iterator->cursor >= iterator->storage->sessions_amount)
        return -1;

    ClientSession *session = iterator->storage->sessions + iterator->cursor;
    pair->login = session->login + (strlen(iterator->storage->server_id) + 1);
    pair->session = session;

    iterator->cursor++;

    return 0;
}
