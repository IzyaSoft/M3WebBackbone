#include "httpRouter.h"
#include "debugPrintFunctions.h"

size_t uxApplicationHTTPHandleRequestHook( const char *pcURLData, char *pcBuffer, size_t uxBufferLength )
{
    printf("Http request received \r\n");

    return 0;
}
