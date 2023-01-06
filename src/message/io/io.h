#ifndef __MESAGE_IO_H__
#define __MESSAGE_IO_H__

#include "../message.h"

int message_write(int descriptor, Message *message);
int message_read(int descriptor, Message *message);


#endif
