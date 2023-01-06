#include "support.h"
#include <string.h>
#include <stdlib.h>


void fscan_string(FILE *stream, char *buff)
{
    size_t read_symbols = 0;
    
    fscanf(stream, "%s", buff);
    read_symbols = strlen(buff);
    
    if(buff[0] == '"')
    {
        memcpy(buff, buff + 1, read_symbols);
        read_symbols--;
        
        while(buff[read_symbols - 1] != '"')
        {
            buff[read_symbols] = ' ';
            fscanf(stream, "%s", buff + read_symbols + 1);
            read_symbols = strlen(buff);
        }

        buff[read_symbols - 1] = '\0';
    }
}
