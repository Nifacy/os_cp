#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "./pair/pair.h"
#include <stdbool.h>

/* Errors */

#define MESSAGE_INVALID_FORMAT 2;

/* Types */

typedef struct {
    Pair *pairs;
    int pairs_amount;
} Message;

/* Methods */

void message_create(Message *message);
int message_from_string(Message *message, char *str);
void message_delete(Message *message);

void message_add_pair(Message *message, char *key, char *value);
bool message_empty(Message *message);
char *message_get(Message *message, char *key);
char *message_repr(Message *message);


#endif