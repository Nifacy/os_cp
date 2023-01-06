#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <stdbool.h>

/* Types */

typedef enum {
    WRITE,
    READ
} ConnectionMode;

typedef struct {
    char *name;
    int descriptor;
    ConnectionMode use_mode;
} PipeConnection;

/* Methods */

int connection_create(PipeConnection *connection, char *name, ConnectionMode mode);
int connection_connect(PipeConnection *connection, char *name, ConnectionMode mode);
int connection_descriptor(PipeConnection *connection);
bool connection_exists(char *name);
void connection_close(const PipeConnection *connection);

#endif