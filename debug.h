// helper function to print debugging information, but only on DEBUG build
#ifndef MYSH_DEBUG_HEADER
#define MYSH_DEBUG_HEADER

#include <stdarg.h>
#include <stdio.h>

void debug(const char* format, ...)
{
#ifdef DEBUG
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	printf("\n");
	va_end(args);
#endif
}

#endif
