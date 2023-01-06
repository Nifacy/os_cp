#include "pair.h"
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* Implementations */

void pair_create(Pair *pair, char *key, char *value)
{
    pair->key = strdup(key);
    pair->value = strdup(value);
}


void pair_delete(Pair *pair)
{
    free(pair->key);
    free(pair->value);
}


char *pair_repr(Pair *pair)
{
    size_t repr_str_len;
    char *repr_str;

    repr_str_len = strlen(pair->key) + strlen(pair->value) + 1;
    repr_str = (char *) malloc(repr_str_len + 1);

    sprintf(repr_str, "%s:%s", pair->key, pair->value);
    return repr_str;
}


int pair_from_string(Pair *pair, char *str)
{
    char *key;
    char *value;
    int key_length;
    int value_length;
    int str_length;
    int bracket_pos;

    bracket_pos = strcspn(str, ":");
    str_length = strlen(str);
    key_length = bracket_pos;
    value_length = str_length - key_length - 1;

    if(key_length == str_length)
    {
        errno = PAIR_INVALID_FORMAT;
        printf("FormatPairStringError: symbol ':' doesn't exist. String: %s\n", str);
        return -1;
    }

    key = (char *) malloc(key_length + 1);
    value = (char *) malloc(value_length + 1);
    
    memset(key, '\0', key_length + 1);
    memset(value, '\0', value_length + 1);

    memcpy(key, str, key_length);
    memcpy(value, str + key_length + 1, value_length);

    pair->key = key;
    pair->value = value;

    return 0;
}
