
#include "Tcp_Server_IPC.h"
#include <drivers/ipc_rpmsg.h>

// RPMessage_Object gAckReplyMsgObject;

#define RECEVE_ENDPOINT 12

void setup_IPC()
{
    RPMessage_CreateParams createParams;
    RPMessage_CreateParams_init(&createParams);
    createParams.localEndPt = RECEVE_ENDPOINT;
    RPMessage_construct(&gAckReplyMsgObject, &createParams);
}

void IPC_SendMessage()
{
    char sendMsg[64] = "hello, tcp!!!";
    char replyMsg[64];
    uint16_t replyMsgSize, remoteCoreId, remoteCoreEndPt;

    RPMessage_send(
        sendMsg, strlen(sendMsg),
        INVERTER_CORE_ID, INVERTER_ENDPOINT,
        RPMessage_getLocalEndPt(&gAckReplyMsgObject),
        100);
}
