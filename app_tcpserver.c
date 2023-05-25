/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

//            char replyMsg[64];
//            uint16_t replyMsgSize, remoteCoreId, remoteCoreEndPt;
//            replyMsgSize = sizeof(replyMsg);
//            int32_t status = RPMessage_recv(
//                &gAckReplyMsgObject,
//                replyMsg,
//                &replyMsgSize,
//                &remoteCoreId,
//                &remoteCoreEndPt,
//                SystemP_WAIT_FOREVER);

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <string.h>
#include <stdio.h>
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include <kernel/dpl/ClockP.h>
#include "enet_apputils.h"
#include "foc.h"
#include "inverterPacket.h"
#include "FreeRTOS.h"
#include "task.h"

#include "Tcp_Server_IPC.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define APP_SERVER_PORT  (8888)

static const uint8_t APP_CLIENT_TX_HEADER[] = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/event-stream\r\n\r\n";
static const uint8_t APP_CLIENT_TX_MOCK_DATA[] = "data: { \"msg\": \"Hello From Stream!\" }\n\n";

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void AppTcp_echoPckt();

static void AppTcp_ServerTask(void *arg);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void AppTcp_echoPckt(struct netconn *pClientConn)
{
    struct netbuf *buf;
    void *data;
    u16_t len;
    err_t err;

    while ((err = netconn_recv(pClientConn, &buf)) == ERR_OK)
    {
        err = netconn_write(pClientConn, APP_CLIENT_TX_HEADER, sizeof(APP_CLIENT_TX_HEADER)-1, NETCONN_COPY);
        if (err != ERR_OK)
        {
            printf("tcpecho: netconn_write: error \"%s\"\r\n", lwip_strerr(err));
            break;
        }
        do
        {
            // ClockP_usleep(300000);


            char messageBuffer[sizeof(InverterStreamPacket_t)] = {0};
            uint16_t replyMsgSize = sizeof(InverterStreamPacket_t);
            uint16_t remoteCoreId, remoteCoreEndPt;

            int32_t status = RPMessage_recv(
                &gAckReplyMsgObject,
                messageBuffer,
                &replyMsgSize,
                &remoteCoreId,
                &remoteCoreEndPt,
                SystemP_WAIT_FOREVER);

            if (status != SystemP_SUCCESS) continue;

            char stringMessage[512];
            int stringLen = stringifyInverterPacket(messageBuffer, stringMessage);

            err = netconn_write(pClientConn, stringMessage, stringLen, NETCONN_COPY);
            if (err != ERR_OK)
            {
                printf("tcpecho: netconn_write: error \"%s\"\r\n", lwip_strerr(err));
                break;
            }

        } while (1);
    }
}

/**
 * Router
 */
static void HandleTcpConnection(void *arg)
{
    struct netconn *clientConnection = (struct netconn*) arg;
    err_t err;

    while(1)
    {
        ClockP_usleep(300000);
        char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, world!";
        err = netconn_write(clientConnection, response, sizeof(response)-1, NETCONN_COPY);
        if (err != ERR_OK)
        {
            printf("HandleTcpConnection: netconn_write: error \"%s\"\r\n", lwip_strerr(err));
            break;
        }
    }

    // ConnMgm
    printf("Closed client connection %p\r\n", clientConnection);
    netconn_close(clientConnection);
    netconn_delete(clientConnection);

    // MemMgm
    vPortFree(clientConnection);
    vTaskDelete(NULL);
}


/**
 * Ovdje se:
 *  1. Prihvaca nova konekcija
 *  2. Alocira se memorija u heapu za state nove konekcije
 *  3. isntancira se novi thread sa connection handlerom kojemu se daje ponter na state konekcije
 *
 *  Trenutno je max broj konekcija limitiran u lwip konfiguraciji na 5
 *  macro je: MEMP_NUM_TCP_PCB
 */
static void AppTcp_ServerTask(void *arg)
{
    // Konekcija u listening stateu
    struct netconn *pConn = NULL;
    err_t err;
    LWIP_UNUSED_ARG(arg);

    pConn = netconn_new(NETCONN_TCP);
    netconn_bind(pConn, IP_ADDR_ANY, APP_SERVER_PORT);
    LWIP_ERROR("tcpecho: invalid conn\r\n", (pConn != NULL), return;);

    /* Tell connection to go into listening mode. */
    netconn_listen(pConn);

    while (1)
    {

        struct netconn *newClientConnection = (struct netconn*) pvPortMalloc(sizeof(struct netconn));

        // Dohvat nove konekcije, blocking call
        err = netconn_accept(pConn, &newClientConnection);
        printf("accepted new connection %p\r\n", newClientConnection);

        /* Process the new connection. */
        if (err < ERR_OK)
        {
            DebugP_log("Unable to accept connection: errno %d\r\n", err);

            // MemMgm
            vPortFree(newClientConnection);
            continue;
        }

        // AppTcp_echoPckt(pClientConn);
        sys_thread_new("hTcp", HandleTcpConnection, newClientConnection, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
    }

    printf("Closed listen connection %p\r\n", pConn);
    /* Close connection and discard connection identifier. */
    netconn_close(pConn);
    netconn_delete(pConn);

}

void AppTcp_startServer()
{
    sys_thread_new("AppTcp_ServerTask", AppTcp_ServerTask, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}
/*-----------------------------------------------------------------------------------*/
