/*
 *  Copyright (c) Texas Instruments Incorporated 2022
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * \file ti_enet_lwipif.c
 *
 * \brief This file contains enet Lwip interface layer implementation for driver callback.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <assert.h>
#include "ti_enet_config.h"
#include "ti_enet_lwipif.h"
#include <lwip/tcpip.h>

#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/HwiP.h>

#include <enet.h>
#include <networking/enet/core/include/per/cpsw.h>
#include <networking/enet/utils/include/enet_appmemutils_cfg.h>
#include <networking/enet/utils/include/enet_apputils.h>
#include <networking/enet/utils/include/enet_appmemutils.h>
#include <networking/enet/utils/include/enet_appboardutils.h>
#include <networking/enet/utils/include/enet_appsoc.h>
#include <networking/enet/utils/include/enet_apprm.h>
#include <networking/enet/core/lwipif/inc/pbufQ.h>

#include <lwip2lwipif.h>
#include <custom_pbuf.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#if ENET_CFG_IS_ON(CPSW_CSUM_OFFLOAD_SUPPORT)
#if !LWIP_CHECKSUM_CTRL_PER_NETIF
#error "LWIP_CHECKSUM_CTRL_PER_NETIF is not set in lwipopts.h"
#endif
#endif

#define ENETLWIP_PACKET_POLL_PERIOD_US (1000U)

#define ENETLWIP_APP_POLL_PERIOD       (500U)
/*! \brief RX packet task stack size */
#define LWIPIF_RX_PACKET_TASK_STACK    (1024U)

/*! \brief TX packet task stack size */
#define LWIPIF_TX_PACKET_TASK_STACK    (1024U)

/*! \brief Links status poll task stack size */
#if (_DEBUG_ == 1)
#define LWIPIF_POLL_TASK_STACK         (3072U)
#else
#define LWIPIF_POLL_TASK_STACK         (1024U)
#endif

#define OS_TASKPRIHIGH              8U

#define LWIPIF_RX_PACKET_TASK_PRI      (OS_TASKPRIHIGH)

#define LWIPIF_TX_PACKET_TASK_PRI      (OS_TASKPRIHIGH)

#define LWIP_POLL_TASK_PRI             (OS_TASKPRIHIGH - 1U)


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
uint8_t gPktRxTaskStack[LWIPIF_RX_PACKET_TASK_STACK] __attribute__ ((aligned(sizeof(long long))));
uint8_t gPktTxTaskStack[LWIPIF_TX_PACKET_TASK_STACK] __attribute__ ((aligned(sizeof(long long))));
uint8_t gPollTaskStack[LWIPIF_POLL_TASK_STACK] __attribute__ ((aligned(sizeof(long long))));

/*!* Handle to Rx semaphore, on which the rxTask awaits for notification
* of used packets available.
*/
SemaphoreP_Object rxPktSem;
/*!* Handle to Tx semaphore, on which the txTask awaits for notification
* of used packets available.
*/SemaphoreP_Object txPktSem;
/*!
* Handle to Polling task semaphore, on which the pollTask awaits for notification
* of used packets available.
*/
SemaphoreP_Object pollSem;

/*!
* Handle to Rx task, whose job it is to receive packets used by the hardware
* and give them to the stack, and return freed packets back to the hardware.
*/
    TaskP_Object rxTask;
/*! Handle to Tx task whose job is to retrieve packets consumed by the hardware and
*  give them to the stack */
    TaskP_Object txTask;
/*! Handle to polling task whose job is to retrieve packets consumed by the hardware and
*  give them to the stack */
    TaskP_Object pollTask;

/*
 * Handle to counting shutdown semaphore, which all subtasks created in the
 * open function must post before the close operation can complete.
 */
SemaphoreP_Object shutDownSemObj;
/** Boolean to indicate shutDownFlag status of translation layer.*/
volatile bool shutDownFlag;
/*
 * Clock handle for triggering the packet Rx notify
 */
    ClockP_Object pollLinkClkObj;

static struct netif gNetif[ENET_SYSCFG_NETIF_COUNT];

/* For Cpdma Rx scatter-gather implies that #rxpkts = #rxbuffers = #rxpbufs
   For Udma's static Rx scatter-gather, #rxbuffers = #rxpbufs = 4 * #rxpkts */
LWIP_MEMPOOL_DECLARE(RX_POOL, ENET_SYSCFG_TOTAL_NUM_RX_PKT, sizeof(Rx_CustomPbuf), "Rx Custom Pbuf pool");
/* These must be sufficient for total number of rx pbufs and tx packets */
pbufNode gFreePbufArr[ENET_SYSCFG_TOTAL_NUM_RX_PKT * 2];
/* ========================================================================== */
/*                            Function Declaration                            */
/* ========================================================================== */

void LwipifEnetApp_startSchedule(struct netif *netif);
void LwipifEnetApp_createRxPktHandlerTask(struct netif *netif);

void LwipifEnetApp_createTxPktHandlerTask(struct netif *netif);

static void LwipifEnetApp_rxPacketTask(void *arg);

static void LwipifEnetApp_txPacketTask(void *arg);

static void LwipifEnetApp_poll(void *arg);

static void LwipifEnetApp_postPollLink(ClockP_Object *clkObj, void *arg);

static err_t LwipifEnetApp_createPollTask (struct netif *netif);


void LwipifEnetApp_netifOpen(uint32_t netifIdx, const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip4_addr_t *gw)
{
    if(netifIdx < ENET_SYSCFG_NETIF_COUNT)
    {
#if ENET_CFG_IS_ON(CPSW_CSUM_OFFLOAD_SUPPORT)

        /* Enable all Flags except below checksumflags. */
        const uint32_t lwIPChcksumDisableFlags = 0U | NETIF_CHECKSUM_GEN_TCP | NETIF_CHECKSUM_GEN_UDP;
        const uint32_t lwIPChcksumSetFlags = (NETIF_CHECKSUM_ENABLE_ALL & ~lwIPChcksumDisableFlags);
#endif
        netif_add(&gNetif[netifIdx],
                    ipaddr, 
                    netmask,
                    gw,
                    NULL,
                    LWIPIF_LWIP_init, 
                    tcpip_input
                    );

        if(netifIdx == ENET_SYSCFG_DEFAULT_NETIF_IDX)
        {
            netif_set_default(&gNetif[netifIdx]);
        }
#if ENET_CFG_IS_ON(CPSW_CSUM_OFFLOAD_SUPPORT)
        NETIF_SET_CHECKSUM_CTRL(&gNetif[netifIdx], lwIPChcksumSetFlags);
#endif
    }
    else
    {
        DebugP_log("ERROR: NetifIdx is out of valid range!\r\n");
        EnetAppUtils_assert(FALSE);
    }
}

void LwipifEnetApp_netifClose(uint32_t netifIdx)
{
    netif_remove(&gNetif[netifIdx]);
}

struct netif * LwipifEnetApp_getNetifFromId(uint32_t netifIdx)
{
    struct netif * pNetif = NULL;
    if(netifIdx < ENET_SYSCFG_NETIF_COUNT)
    {
        pNetif = &gNetif[netifIdx];
    }
    else{
            DebugP_log("ERROR: NetifIdx is out of valid range!\r\n");
            EnetAppUtils_assert(FALSE);
    }

    return pNetif;
}

void LwipifEnetAppCb_getNetifInfo(struct netif *netif,
                                  LwipifEnetAppIf_GetHandleNetifInfo *outArgs)
{
    outArgs->numRxChannels = ENET_SYSCFG_RX_FLOWS_NUM;
    outArgs->numTxChannels = ENET_SYSCFG_TX_CHANNELS_NUM;
    outArgs->isDirected = false;
}

void LwipifEnetAppCb_getEnetLwipIfInstInfo(LwipifEnetAppIf_GetEnetLwipIfInstInfo *outArgs)
{
    EnetPer_AttachCoreOutArgs attachInfo;
    EnetApp_HandleInfo handleInfo;
    Enet_Type enetType;
    uint32_t instId;

    uint32_t coreId = EnetSoc_getCoreId();


    EnetApp_getEnetInstInfo(&enetType,
                            &instId);

    EnetApp_acquireHandleInfo(enetType, instId, &handleInfo);
    EnetApp_coreAttach(enetType,instId, coreId, &attachInfo);

    outArgs->hEnet         = handleInfo.hEnet;
    outArgs->hostPortRxMtu = attachInfo.rxMtu;
    ENET_UTILS_ARRAY_COPY(outArgs->txMtu, attachInfo.txMtu);
    outArgs->isPortLinkedFxn = &EnetApp_isPortLinked;
    outArgs->timerPeriodUs   = ENETLWIP_PACKET_POLL_PERIOD_US;

    outArgs->maxNumNetif = ENET_SYSCFG_NETIF_COUNT;
    outArgs->numRxChannels = ENET_SYSCFG_RX_FLOWS_NUM;
    outArgs->numTxChannels = ENET_SYSCFG_TX_CHANNELS_NUM;

    outArgs->pPbufInfo = gFreePbufArr;
    outArgs->pPbufInfoSize = sizeof(gFreePbufArr)/sizeof(pbufNode);
    LWIP_MEMPOOL_INIT(RX_POOL);


#if ENET_CFG_IS_ON(CPSW_CSUM_OFFLOAD_SUPPORT)
    int32_t status;
    /* Confirm HW checksum offload is enabled when LWIP chksum offload is enabled */
        Enet_IoctlPrms prms;
        bool csumOffloadFlg;
        ENET_IOCTL_SET_OUT_ARGS(&prms, &csumOffloadFlg);
        ENET_IOCTL(handleInfo.hEnet,
                   coreId,
                   ENET_HOSTPORT_IS_CSUM_OFFLOAD_ENABLED,
                   &prms,
                   status);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("() Failed to get checksum offload info: %d\r\n", status);
        }

        EnetAppUtils_assert(true == csumOffloadFlg);
#endif
}

void LwipifEnetAppCb_getTxHandleInfo(LwipifEnetAppIf_GetTxHandleInArgs *inArgs,
                                     LwipifEnetAppIf_TxHandleInfo *outArgs)
{
    Enet_Type enetType;
    uint32_t instId, i;
    EnetDma_Pkt *pPktInfo;
    EnetApp_HandleInfo handleInfo;
    EnetApp_GetTxDmaHandleOutArgs  txChInfo;
    EnetApp_GetDmaHandleInArgs     txInArgs;

    EnetApp_getEnetInstInfo(&enetType,
                            &instId);
    EnetApp_acquireHandleInfo(enetType, instId, &handleInfo);

    /* Open TX channel */
    txInArgs.cbArg     = inArgs->cbArg;
    txInArgs.notifyCb  = inArgs->notifyCb;

    EnetApp_getTxDmaHandle(inArgs->chId,
                          &txInArgs,
                          &txChInfo);

    outArgs->hTxChannel = txChInfo.hTxCh;
    outArgs->txChNum = txChInfo.txChNum;
    outArgs->numPackets = txChInfo.maxNumTxPkts;
    outArgs->disableEvent = true;

    /* Initialize the DMA free queue */
    EnetQueue_initQ(inArgs->pktInfoQ);

    for (i = 0U; i < txChInfo.maxNumTxPkts; i++)
    {
        /* Initialize Pkt info Q from allocated memory */
        pPktInfo = EnetMem_allocEthPktInfoMem(&inArgs->cbArg,
                                              ENETDMA_CACHELINE_ALIGNMENT);

        EnetAppUtils_assert(pPktInfo != NULL);
        ENET_UTILS_SET_PKT_APP_STATE(&pPktInfo->pktState, ENET_PKTSTATE_APP_WITH_FREEQ);
        EnetQueue_enq(inArgs->pktInfoQ, &pPktInfo->node);

    }

}

void LwipifEnetAppCb_getRxHandleInfo(LwipifEnetAppIf_GetRxHandleInArgs *inArgs,
                                     LwipifEnetAppIf_RxHandleInfo *outArgs)
{
    Enet_Type enetType;
    uint32_t instId, i;
    EnetDma_Pkt *pPktInfo;
    Rx_CustomPbuf *cPbuf;
    int32_t status;
    EnetApp_HandleInfo handleInfo;
    EnetPer_AttachCoreOutArgs attachInfo;
    uint32_t coreId          = EnetSoc_getCoreId();
    EnetApp_GetRxDmaHandleOutArgs  rxChInfo;
    EnetApp_GetDmaHandleInArgs     rxInArgs;
    uint32_t numCustomPbuf;
    uint32_t scatterSegments[] =
    {
        ENET_UTILS_ALIGN(384, ENETDMA_CACHELINE_ALIGNMENT) /* Keep this size aligned with R5F cacheline of 32B */
    };
    EnetApp_getEnetInstInfo(&enetType,
                            &instId);
    EnetApp_acquireHandleInfo(enetType, instId, &handleInfo);
    EnetApp_coreAttach(enetType,instId, coreId, &attachInfo);

    /* Open RX channel */
    rxInArgs.cbArg     = inArgs->cbArg;
    rxInArgs.notifyCb  = inArgs->notifyCb;

    EnetApp_getRxDmaHandle(inArgs->chId,
                          &rxInArgs,
                          &rxChInfo);
    numCustomPbuf = rxChInfo.maxNumRxPkts;
    outArgs->rxFlowIdx = rxChInfo.rxChNum;
    outArgs->hRxFlow  = rxChInfo.hRxCh;
    outArgs->numPackets  = rxChInfo.maxNumRxPkts;
    outArgs->disableEvent = true;
    if(rxChInfo.macAddressValid)
    {
        EnetUtils_copyMacAddr(&outArgs->macAddr[inArgs->chId][0U], rxChInfo.macAddr);
        EnetAppUtils_print("Host MAC address-%d : ",inArgs->chId);
        EnetAppUtils_printMacAddr(&outArgs->macAddr[inArgs->chId][0U]);
    }

    /* Initialize the DMA free queue */
    EnetQueue_initQ(inArgs->pReadyRxPktQ);
    EnetQueue_initQ(inArgs->pFreeRxPktInfoQ);
    pbufQ_init(inArgs->pFreePbufInfoQ);

    for (i = 0U; i < rxChInfo.maxNumRxPkts; i++)
    {

        pPktInfo = EnetMem_allocEthPkt(&inArgs->cbArg,
                           ENETDMA_CACHELINE_ALIGNMENT,
                           ENET_ARRAYSIZE(scatterSegments),
                           scatterSegments);
        EnetAppUtils_assert(pPktInfo != NULL);
        ENET_UTILS_SET_PKT_APP_STATE(&pPktInfo->pktState, ENET_PKTSTATE_APP_WITH_READYQ);

        /* Put all the filled pPktInfo into readyRxPktQ and submit to driver */
        EnetQueue_enq(inArgs->pReadyRxPktQ, &pPktInfo->node);
    }

    EnetQueue_verifyQCount(inArgs->pReadyRxPktQ);
    for (i = 0U; i < numCustomPbuf; i++)
    {
        /* Allocate the Custom Pbuf structures and put them in freePbufInfoQ */
        cPbuf = NULL;
        cPbuf = (Rx_CustomPbuf*)LWIP_MEMPOOL_ALLOC(RX_POOL);
        EnetAppUtils_assert(cPbuf != NULL);
        cPbuf->p.custom_free_function = custom_pbuf_free;
        cPbuf->customPbufArgs         = (Rx_CustomPbuf_Args)inArgs->cbArg;
        cPbuf->next                   = NULL;
        cPbuf->alivePbufCount         = 0U;
        cPbuf->orgBufLen              = 0U;
        cPbuf->orgBufPtr              = NULL;
        cPbuf->p.pbuf.flags          |= PBUF_FLAG_IS_CUSTOM;
        pbufQ_enQ(inArgs->pFreePbufInfoQ, &(cPbuf->p.pbuf));
    }

    if(ENET_SYSCFG_NETIF_COUNT > 1U)
    {
        for(uint32_t i =1U; i<ENET_SYSCFG_NETIF_COUNT; i++)
        {
            /* Allocating another mac addresses for number of netifs supported*/
            status = EnetAppUtils_allocMac(handleInfo.hEnet,
                                        attachInfo.coreKey,
                                        coreId,
                                        &outArgs->macAddr[i][0U]);
            EnetAppUtils_assert(ENET_SOK == status);
            EnetAppUtils_addHostPortEntry(handleInfo.hEnet, coreId,  &outArgs->macAddr[i][0U]);
            EnetAppUtils_print("Host MAC address-%d : ",i);
            EnetAppUtils_printMacAddr(&outArgs->macAddr[i][0U]);
        }
    }
}

void LwipifEnetAppCb_releaseTxHandle(LwipifEnetAppIf_ReleaseTxHandleInfo *releaseInfo)
{
    EnetApp_HandleInfo handleInfo;
    EnetPer_AttachCoreOutArgs attachInfo;
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    Enet_Type enetType;
    uint32_t instId;
    uint32_t coreId = EnetSoc_getCoreId();

    EnetApp_getEnetInstInfo(&enetType,
                            &instId);
    EnetApp_acquireHandleInfo(enetType, instId, &handleInfo);
    EnetApp_coreAttach(enetType,instId, coreId, &attachInfo);

    /* Close TX channel */
    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);
    EnetApp_closeTxDma(releaseInfo->txChNum,
                       handleInfo.hEnet,
                       attachInfo.coreKey,
                       coreId,
                       &fqPktInfoQ,
                       &cqPktInfoQ);
    releaseInfo->txFreePktCb(releaseInfo->txFreePktCbArg, &fqPktInfoQ, &cqPktInfoQ);
    EnetApp_coreDetach(enetType, instId, coreId, attachInfo.coreKey);
    EnetApp_releaseHandleInfo(enetType, instId);
}

void LwipifEnetAppCb_releaseRxHandle(LwipifEnetAppIf_ReleaseRxHandleInfo *releaseInfo)
{
    EnetApp_HandleInfo handleInfo;
    EnetPer_AttachCoreOutArgs attachInfo;
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    Enet_Type enetType;
    uint32_t instId;
    uint32_t coreId = EnetSoc_getCoreId();

    EnetApp_getEnetInstInfo(&enetType,
                            &instId);
    EnetApp_acquireHandleInfo(enetType, instId, &handleInfo);
    EnetApp_coreAttach(enetType,instId, coreId, &attachInfo);

    /* Close RX channel */
    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);
    EnetApp_closeRxDma(releaseInfo->rxChNum,
                       handleInfo.hEnet,
                       attachInfo.coreKey,
                       coreId,
                       &fqPktInfoQ,
                       &cqPktInfoQ);

    releaseInfo->rxFreePktCb(releaseInfo->rxFreePktCbArg, &fqPktInfoQ, &cqPktInfoQ);
    EnetApp_coreDetach(enetType, instId, coreId, attachInfo.coreKey);
    EnetApp_releaseHandleInfo(enetType, instId);
}

static err_t LwipifEnetApp_createPollTask(struct netif *netif)
{
    TaskP_Params params;
    int32_t status;
    ClockP_Params clkPrms;


    if (NULL != netif->state)
    {
        /*Initialize semaphore to call synchronize the poll function with a timer*/
        status = SemaphoreP_constructBinary(&pollSem, 0U);
        EnetAppUtils_assert(status == SystemP_SUCCESS);

        /* Initialize the poll function as a thread */
        TaskP_Params_init(&params);
        params.name = "Lwipif_Lwip_poll";
        params.priority       = LWIP_POLL_TASK_PRI;
        params.stack          = &gPollTaskStack[0U];
        params.stackSize      = sizeof(gPollTaskStack);
        params.args           = &(gNetif[ENET_SYSCFG_NETIF_COUNT - 1]);
        params.taskMain       = &LwipifEnetApp_poll;

        status = TaskP_construct(&pollTask, &params);
        EnetAppUtils_assert(status == SystemP_SUCCESS);

        ClockP_Params_init(&clkPrms);
        clkPrms.start     = 0;
        clkPrms.period    = ENETLWIP_APP_POLL_PERIOD;
        clkPrms.args      = &pollSem;
        clkPrms.callback  = &LwipifEnetApp_postPollLink;
        clkPrms.timeout   = ENETLWIP_APP_POLL_PERIOD;

        /* Creating timer and setting timer callback function*/
        status = ClockP_construct(&pollLinkClkObj,
                                  &clkPrms);
        if (status == SystemP_SUCCESS)
        {
            /* Set timer expiry time in OS ticks */
            ClockP_setTimeout(&pollLinkClkObj, ENETLWIP_APP_POLL_PERIOD);
            ClockP_start(&pollLinkClkObj);
        }
        else
        {
            EnetAppUtils_assert (status == SystemP_SUCCESS);
        }

        /* Filter not defined */
        /* Inform the world that we are operational. */
        EnetAppUtils_print("[LWIPIF_LWIP] Enet has been started successfully\r\n");

        return ERR_OK;
    }
    else
    {
        return ERR_BUF;
    }
}
/*
* create a function called postEvent[i]. each event, each postfxn.
*/
static void LwipifEnetApp_postSemaphore(void *pArg)
{
    SemaphoreP_Object *pSem = (SemaphoreP_Object *) pArg;
    SemaphoreP_post(pSem);
}

void LwipifEnetApp_startSchedule(struct netif *netif
    )
{
    uint32_t status;
    status = SemaphoreP_constructBinary(&txPktSem, 0U);
    EnetAppUtils_assert(status == SystemP_SUCCESS);

    status = SemaphoreP_constructBinary(&rxPktSem, 0U);
    EnetAppUtils_assert(status == SystemP_SUCCESS);

    Enet_notify_t rxNotify =
        {
           .cbFxn = &LwipifEnetApp_postSemaphore, //gives different cb fxn for different events.
           .cbArg = &rxPktSem //
        };
    Enet_notify_t txNotify =
        {
                .cbFxn = &LwipifEnetApp_postSemaphore,
                .cbArg = &txPktSem
        };

    LWIPIF_LWIP_setNotifyCallbacks(netif, &rxNotify, &txNotify);
    /* Initialize Tx task*/
    LwipifEnetApp_createTxPktHandlerTask(netif);

    /* Initialize Rx Task*/
    LwipifEnetApp_createRxPktHandlerTask(netif);

    /* Initialize Polling task*/
    LwipifEnetApp_createPollTask(netif);
}

void LwipifEnetApp_createRxPktHandlerTask(struct netif *netif)
{
    TaskP_Params params;
    int32_t status;

    /* Create RX packet task */
    TaskP_Params_init(&params);
    params.name = "LwipifEnetApp_RxPacketTask";
    params.priority       = LWIPIF_RX_PACKET_TASK_PRI;
    params.stack          = &gPktRxTaskStack[0U];
    params.stackSize      = sizeof(gPktRxTaskStack);
    params.args           = netif;
    params.taskMain       = &LwipifEnetApp_rxPacketTask;

    status = TaskP_construct(&rxTask , &params);
    EnetAppUtils_assert(status == SystemP_SUCCESS);
}

void LwipifEnetApp_createTxPktHandlerTask(struct netif *netif)
{
    TaskP_Params params;
    int32_t status;

    /* Create TX packet task */
    TaskP_Params_init(&params);
    params.name = "LwipifEnetApp_TxPacketTask";
    params.priority       = LWIPIF_TX_PACKET_TASK_PRI;
    params.stack          = &gPktTxTaskStack[0U];
    params.stackSize      = sizeof(gPktTxTaskStack);
    params.args           = netif;
    params.taskMain       = &LwipifEnetApp_txPacketTask;

    status = TaskP_construct(&txTask , &params);
    EnetAppUtils_assert(status == SystemP_SUCCESS);
}

static void LwipifEnetApp_rxPacketTask(void *arg)
{
    struct netif *netif = (struct netif *) arg;
    while (!shutDownFlag)
    {
        /* Wait for the Rx ISR to notify us that packets are available with data */
        SemaphoreP_pend(&rxPktSem, SystemP_WAIT_FOREVER);
        if (shutDownFlag)
        {
            /* This translation layer is shutting down, don't give anything else to the stack */
            break;
        }

        LWIPIF_LWIP_rxPktHandler(netif);
    }

    /* We are shutting down, notify that we are done */
    SemaphoreP_post(&shutDownSemObj);
}

static void LwipifEnetApp_txPacketTask(void *arg)
{
    struct netif *netif = (struct netif *) arg;
    while (!shutDownFlag)
    {
        /*
         * Wait for the Tx ISR to notify us that empty packets are available
         * that were used to send data
         */
        SemaphoreP_pend(&txPktSem, SystemP_WAIT_FOREVER);
        LWIPIF_LWIP_txPktHandler(netif);
    }

    /* We are shutting down, notify that we are done */
    SemaphoreP_post(&shutDownSemObj);
}

static void LwipifEnetApp_poll(void *arg)
{
    /* Call the driver's periodic polling function */
    volatile bool flag = 1;
    struct netif* netif = (struct netif*) arg;

    while (flag)
    {
        SemaphoreP_pend(&pollSem, SystemP_WAIT_FOREVER);
        sys_lock_tcpip_core();
        LWIPIF_LWIP_periodic_polling(netif);
        sys_unlock_tcpip_core();
    }
}

static void LwipifEnetApp_postPollLink(ClockP_Object *clkObj, void *arg)
{
    if(arg != NULL)
    {
        SemaphoreP_Object *hpollSem = (SemaphoreP_Object *) arg;
        SemaphoreP_post(hpollSem);
    }
}



