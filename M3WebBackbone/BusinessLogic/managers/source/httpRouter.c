#include "httpRouter.h"
#include "debugPrintFunctions.h"

uint32_t uxApplicationHTTPHandleRequestHook(char* url, char* fileName, uint32_t fileSize)
{
    printf("Http request received \r\n");

    return 0;
}
