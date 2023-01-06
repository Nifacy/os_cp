#ifndef __MESSAGE_STORAGE_H__
#define __MESSAGE_STORAGE_H__


#include <stdio.h>
#include <stdbool.h>


#define PATH_SIZE 100
#define RECORD_STR_FIELD_SIZE 100
#define RECORD_CONTENT_SIZE 256


/* Types */

typedef struct {
    char path[PATH_SIZE];
    FILE *add;
} MessageStorage;


typedef struct {
    char from[RECORD_STR_FIELD_SIZE];
    char to[RECORD_STR_FIELD_SIZE];
    char content[RECORD_CONTENT_SIZE];
} MessageRecord;


typedef struct {
    FILE *file;
    bool have_info;
} MessageReader;


/* Methods */

int message_storage_init(MessageStorage *storage, char *filepath);
int message_storage_add(MessageStorage *storage, MessageRecord *record);
void message_storage_clear(MessageStorage *storage);
MessageReader message_storage_select(MessageStorage *storage);
int message_reader_next(MessageReader *reader, MessageRecord *record);


#endif