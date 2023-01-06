#include "connection.h"
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>


/* Support functions */

char *path_generate(char *name)
{
    const char *path_tmp = "/tmp/%s.fifo";
    size_t result_str_size = strlen(name) + strlen(path_tmp);
    char *result_str = (char *) malloc(result_str_size + 1);

    sprintf(result_str, path_tmp, name);
    return result_str;
}

int get_use_file_mode(ConnectionMode mode)
{
    switch(mode)
    {
        case READ: return O_RDONLY;
        case WRITE: return O_WRONLY;
    }
}

/* Implementations */

int connection_create(PipeConnection *connection, char *name, ConnectionMode mode)
{
    char *connection_path;
    int descriptor;

    connection_path = path_generate(name);
    
    connection->name = strdup(name);
    connection->descriptor = -1;
    connection->use_mode = mode;

    if(mkfifo(connection_path, 0777) == -1)
        return -1;

    free(connection_path);
    
    return 0;
}


int connection_connect(PipeConnection *connection, char *name, ConnectionMode mode)
{
    char *connection_filepath;
    int descriptor;

    connection_filepath = path_generate(name);
    descriptor = open(connection_filepath, get_use_file_mode(mode));
    
    connection->name = strdup(name);
    connection->descriptor = descriptor;
    connection->use_mode = mode;

    if(descriptor == -1)
        return -1;

    return 0;
}


bool connection_exists(char *name)
{
    char *path;
    bool result;

    path = path_generate(name);
    result = access(path, F_OK) == 0;

    free(path);
    return result;
}


void connection_close(const PipeConnection *connection)
{
    char *connection_filepath;

    connection_filepath = path_generate(connection->name);

    close(connection->descriptor);
    remove(connection_filepath);

    free(connection->name);
    free(connection_filepath);
}


int connection_descriptor(PipeConnection *connection)
{
    if(connection->descriptor == -1)
    {
        char *filepath = path_generate(connection->name);

        connection->descriptor = open(filepath, get_use_file_mode(connection->use_mode));
        printf("[ConnectionLib] Generate descriptor from '%s': %d\n", connection->name, connection->descriptor);
    
        free(filepath);
    }
    
    return connection->descriptor;
}
