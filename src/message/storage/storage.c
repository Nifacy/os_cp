#include "storage.h"
#include "../../support/support.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


int message_storage_init(MessageStorage *storage, char *filepath)
{
    storage->add = fopen(filepath, "a+");
    strcpy(storage->path, filepath);
 
    if(storage->add == NULL)
        return -1;

    return 0;
}


void message_storage_clear(MessageStorage *storage)
{
    FILE *file = fopen(storage->path, "w");
    fclose(file);
    fclose(storage->add);
    storage->add = fopen(storage->path, "a+");
}


int message_storage_add(MessageStorage *storage, MessageRecord *record)
{
    int result = fprintf(storage->add, "\"%s,%s,%s\"\n", record->from, record->to, record->content);
    fclose(storage->add);
    storage->add = fopen(storage->path, "a+");

    if(result < 0)
        return -1;

    return 0;
}


MessageReader message_storage_select(MessageStorage *storage)
{
    MessageReader reader;

    reader.file = fopen(storage->path, "r");
    reader.have_info = true;

    return reader;
}


int message_reader_next(MessageReader *reader, MessageRecord *record)
{
    if(!reader->have_info) return -1;

    if(feof(reader->file))
    {
        reader->have_info = false;
        return -1;
    }

    char line[1024];
    fscan_string(reader->file, line);

    int i = strcspn(line, ",");

    if(i == strlen(line))
        return -1;

    int j = i + 1 + strcspn(line + i + 1, ",");

    if(j == strlen(line))
        return -1;

    line[i] = '\0';
    line[j] = '\0';

    strcpy(record->from, line);
    strcpy(record->to, line + i + 1);
    strcpy(record->content, line + j + 1);

    return 0;
}
