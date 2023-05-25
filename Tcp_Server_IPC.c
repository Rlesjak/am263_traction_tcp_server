
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
        CSL_CORE_ID_R5FSS0_0, 13,
        RPMessage_getLocalEndPt(&gAckReplyMsgObject),
        100);
}

void IPC_ReceveMessage()
{
    char sendMsg[64] = "hello, tcp!!!";
    char replyMsg[64];
    uint16_t replyMsgSize, remoteCoreId, remoteCoreEndPt;
    /* set 'replyMsgSize' to size of recv buffer,
     * after return `replyMsgSize` contains actual size of valid data in recv buffer
     */
    replyMsgSize = sizeof(replyMsg); /*  */
    RPMessage_recv(&gAckReplyMsgObject,
        replyMsg, &replyMsgSize,
        &remoteCoreId, &remoteCoreEndPt,
        SystemP_WAIT_FOREVER);
}
