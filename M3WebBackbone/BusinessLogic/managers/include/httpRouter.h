#ifndef HTTPROUTER_H
#define HTTPROUTER_H

#include <stdint.h>
#include <stddef.h>

size_t uxApplicationHTTPHandleRequestHook(const char *pcURLData, char *pcBuffer, size_t uxBufferLength);

#endif
