#include "message.h"
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Implementations */

void message_create(Message *message)
{
    message->pairs = NULL;
    message->pairs_amount = 0;
}


void message_add_pair(Message *message, char *key, char *value)
{
    int i = message->pairs_amount;

    if(i == 0)
        message->pairs = (Pair *) malloc(sizeof(Pair));
    else
        message->pairs = realloc(message->pairs, sizeof(Pair) * (i + 1));

    pair_create(&(message->pairs[i]), key, value);
    message->pairs_amount++;
}


char *message_get(Message *message, char *key)
{
    for(int i = 0; i < message->pairs_amount; i++)
    {
        if(strcmp(key, message->pairs[i].key) == 0)
            return message->pairs[i].value;
    }

    return NULL;
}


void message_delete(Message *message)
{
    for(int i = 0; i < message->pairs_amount; i++)
        pair_delete(&(message->pairs[i]));

    free(message->pairs);
}


char *message_repr(Message *message)
{
    char *result_str = strdup("");

    for(int i = 0; i < message->pairs_amount; i++)
    {
        char *pair_str = pair_repr(&(message->pairs[i]));
        size_t new_str_len = strlen(result_str) + strlen(pair_str) + 2;

        result_str = realloc(result_str, new_str_len);
        strcat(result_str, pair_str);
        strcat(result_str, ";");

        free(pair_str);
    }

    return result_str;
}


bool message_empty(Message *message)
{
    return message->pairs_amount == 0;
}


int message_from_string(Message *message, char *str)
{
    int pos = 0;
    size_t str_len = strlen(str);

    message_create(message);

    while(pos < str_len)
    {
        int pair_str_len;
        char *substr;
        Pair pair;

        pair_str_len = strcspn(str + pos, ";");

        if(pair_str_len == str_len - pos)
        {
            errno = MESSAGE_INVALID_FORMAT;
            printf("FormatMessageStringError: symbol ';' doesn't exist. String: %s\n", str);
            return -1;
        }

        substr = (char *) malloc(pair_str_len + 1);
        memset(substr, '\0', pair_str_len + 1);
        memcpy(substr, str + pos, pair_str_len);
        pair_from_string(&pair, substr);
        message_add_pair(message, pair.key, pair.value);

        pos += pair_str_len + 1;

        free(substr);
        pair_delete(&pair);
    }

    return 0;
}