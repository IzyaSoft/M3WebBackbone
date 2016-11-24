#ifndef HTTPROUTER_H
#define HTTPROUTER_H

#include <stdint.h>

uint32_t uxApplicationHTTPHandleRequestHook(char* url, char* fileName, uint32_t fileSize);

#endif
