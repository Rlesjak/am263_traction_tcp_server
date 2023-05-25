/*
 * Tcp_Server_IPC.h
 *
 *  Created on: 17. svi 2023.
 *      Author: Robi
 */

#ifndef TCP_SERVER_IPC_H_
#define TCP_SERVER_IPC_H_

#include <drivers/ipc_rpmsg.h>

RPMessage_Object gAckReplyMsgObject;

void setup_IPC();
void IPC_SendMessage();
void IPC_ReceveMessage();


#endif /* TCP_SERVER_IPC_H_ */
