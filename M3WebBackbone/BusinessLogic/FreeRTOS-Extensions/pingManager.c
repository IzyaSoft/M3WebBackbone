
#include <stdint.h>
#include "pingManager.h"
#include "queue.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_UDP_IP.h"


/* If ipconfigSUPPORT_OUTGOING_PINGS is set to 1 in FreeRTOSIPConfig.h then
vApplicationPingReplyHook() is called by the IP stack when the stack receives a
ping reply.*/
void vApplicationPingReplyHook(ePingReplyStatus_t eStatus, uint16_t usIdentifier)
{
    //switch(eStatus)
    //{
        //case eSuccess    :
            /* A valid ping reply has been received.  Post the sequence number
            on the queue that is read by the vSendPing() function below.  Do
            not wait more than 10ms trying to send the message if it cannot be
            sent immediately because this function is called from the IP stack
            task - blocking in this function will block the IP stack. */
            //xQueueSend(xPingReplyQueue, &usIdentifier, 10 / portTICK_PERIOD_MS);
            //break;

        //case eInvalidChecksum :
        //case eInvalidData :
            /* A reply was received but it was not valid. */
            //break;
    //}
}


//BaseType_t vSendPing(const int8_t *pcIPAddress)
//{
    //uint16_t usRequestSequenceNumber, usReplySequenceNumber;
    //uint32_t ulIPAddress;

    /* The pcIPAddress parameter holds the destination IP address as a string in
    decimal dot notation (for example, "192.168.0.200").  Convert the string into
    the required 32-bit format. */
    //ulIPAddress = FreeRTOS_inet_addr( pcIPAddress );

    /* Send a ping containing 8 data bytes.  Wait (in the Blocked state) a
    maximum of 100ms for a network buffer into which the generated ping request
    can be written and sent. */
    //usRequestSequenceNumber = FreeRTOS_SendPingRequest( ulIPAddress, 8, 100 / portTICK_PERIOD_MS );

    //if( usRequestSequenceNumber == pdFAIL )
    //{
        /* The ping could not be sent because a network buffer could not be
        obtained within 100ms of FreeRTOS_SendPingRequest() being called. */
    //}
    //else
    //{
        /* The ping was sent.  Wait 200ms for a reply.  The sequence number from
        each reply is sent from the vApplicationPingReplyHook() on the
        xPingReplyQueue queue (this is not standard behaviour, but implemented in
        the example function above).  It is assumed the queue was created before
        this function was called! */
        //if(xQueueReceive(xPingReplyQueue, &usReplySequenceNumber, 200 / portTICK_PERIOD_MS) == pdPASS )
        //{
            /* A ping reply was received.  Was it a reply to the ping just sent? */
            //if( usRequestSequenceNumber == usReplySequenceNumber )
            //{
                /* This was a reply to the request just sent. */
            //}
        //}
    //}
//}
