#ifndef __SESSION_STORAGE_H__
#define __SESSION_STORAGE_H__


#include "../session.h"
#include <stdbool.h>


#define SERVER_STR_SIZE 50


/* Types */

typedef struct {
    ClientSession *sessions;
    int sessions_amount;
    char server_id[SERVER_STR_SIZE];
} SessionStorage;


typedef struct {
    int cursor;
    SessionStorage *storage;
} SessionIterator;


typedef struct {
    char *login;
    ClientSession *session;
} SessionPair;

/* Methods */

void session_storage_init(SessionStorage *storage, char *server_id);
void session_storage_delete(SessionStorage *storage);

bool session_storage_exists(SessionStorage *storage, char *login);
ClientSession *session_storage_create(SessionStorage *storage, char *login);
ClientSession *session_storage_get(SessionStorage *storage, char *login);
void session_storage_remove(SessionStorage *storage, char *login);

SessionIterator session_storage_iter(SessionStorage *storage);
int session_storage_next(SessionIterator *iterator, SessionPair *pair);

#endif