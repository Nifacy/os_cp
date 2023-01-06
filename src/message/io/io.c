#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


int message_write(int descriptor, Message *message)
{
    int res;
    char *message_str;

    message_str = message_repr(message);
    res = write(descriptor, message_str, strlen(message_str));
    res = write(descriptor, "\n", 1);

    if(res == -1)
    {
        printf("MessageWriteError: Can't write message");
        return -1;
    }

    free(message_str);
    return 0;
}


int message_read(int descriptor, Message *message)
{
    char *read_data;
    int read_bytes;
    int buff_size = 256;

    message_create(message);
    read_data = (char *) malloc(buff_size);
    memset(read_data, '\0', buff_size);
    read_bytes = 0;

    while(1)
    {
        char read_symb;
        int res;

        while(1)
        {
            res = read(descriptor, &read_symb, sizeof(read_symb));

            if(res == -1)
            {
                if(errno == EAGAIN) continue;
                printf("MessageReadError: Can't read data\n");
                free(read_data);
                return -1;
            }

            break;
        }

        if(read_bytes == buff_size)
        {
            buff_size += 256;
            read_data = realloc(read_data, buff_size);
        }

        read_data[read_bytes] = read_symb;
        read_bytes++;

        if(read_symb == '\0' || read_symb == '\n' || read_symb == EOF)
            break;
    }

    read_data[read_bytes - 1] = '\0';
    int res = message_from_string(message, read_data);

    free(read_data);
    if(res == -1) return -1;
    return 0;
}
