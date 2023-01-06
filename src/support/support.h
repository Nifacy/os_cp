#ifndef __SUPPORT_H__
#define __SUPPORT_H__


#include <stdio.h>


void fscan_string(FILE *stream, char *buff);
/*
	Считывает с канала строку следующего формата: "<word> <word> ... <word>"

	:param stream: канал, с которого происходит считывание строки
	:param buff: буфер, в который заносится считанная строка
*/


#endif
