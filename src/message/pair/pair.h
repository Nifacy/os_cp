#ifndef __MESSAGE_PAIR_H__
#define __MESSAGE_PAIR_H__


/* Errors */

#define PAIR_INVALID_FORMAT 1

/* Types */

typedef struct {
    char* key;
    char* value;
} Pair;

/* Methods */

void pair_create(Pair *pair, char *key, char *value);
void pair_delete(Pair *pair);
char *pair_repr(Pair *pair);
int pair_from_string(Pair *pair, char *str);

#endif