/*
 *  Copyright (c) Texas Instruments Incorporated 2020
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

/**
 *  \file ti_enet_soc.c
 *
 *  \brief This file contains enet soc config related functionality.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <enet.h>
#include <enet_cfg.h>
#include <priv/mod/cpsw_ale_priv.h>
#include <priv/mod/cpsw_cpts_priv.h>
#include <priv/mod/cpsw_hostport_priv.h>
#include <priv/mod/cpsw_macport_priv.h>
#include <priv/mod/cpsw_stats_priv.h>
#include <priv/mod/mdio_priv.h>
#include <priv/mod/cpsw_clks.h>
#include <priv/core/enet_rm_priv.h>
#include <include/core/enet_utils.h>
#include <include/core/enet_osal.h>
#include <include/core/enet_soc.h>
#include <include/core/enet_per.h>
#include <include/per/cpsw.h>
#include <src/dma/cpdma/enet_cpdma_priv.h>
#include <priv/per/cpsw_cpdma_priv.h>
#include <utils/include/enet_appsoc.h>
#include <drivers/hw_include/cslr_soc.h>
#include <kernel/dpl/CpuIdP.h>
#include "ti_enet_config.h"


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Number of CPSW interrupts: RxThreh, Rx, Tx and Misc(CPTS, MDIO and stats) */
#define CPSW_INTR_NUM                                (4U)

/* ENET_CTRL_MODE value for different MII modes */
#define CPSW_ENET_CTRL_MODE_RMII                     (1U)
#define CPSW_ENET_CTRL_MODE_RGMII                    (2U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/*!
 * \brief Interrupt "SoC connection" configuration.
 */
typedef struct EnetSoc_IntrCfg_s
{
    /*! Id of the interrupt as defined in the Enet Per driver */
    uint32_t intrId;

    /*! Core Interrupt number */
    uint16_t coreIntNum;

    /*! Interrupt's trigger sensitivity for ARM corepac as
     *  @ref OSAL_armGicTrigType_t.
     */
    uint16_t triggerType;
} EnetSoc_IntrCfg;

/*!
 * \brief CPSW SoC configuration.
 *
 * SoC-level configuration information for the CPSW driver.
 */
typedef struct CpswSoc_Cfg_s
{
    /*! CPSW main clock (CPPI_ICLK) frequency in Hz. CPPI packet streaming
     * interface clock
     * Note: Clock frequency variable needs to be uint64_t as PMLIB API takes
     * clkRate input as uint64_t */
    uint64_t cppiClkFreqHz;

    /*! CPSW interrupts */
    EnetSoc_IntrCfg intrs[CPSW_INTR_NUM];

    /* Tx Ch Count*/
    uint32_t txChCount;

    /* Rx Ch Count */
    uint32_t rxChCount;

    /* CPTS Hardware Push Event Count */
    uint32_t cptsHwPushCount;
} CpswSoc_Cfg;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static const EnetSoc_IntrCfg *EnetSoc_findIntrCfg(Enet_Type enetType,
                                                  uint32_t instId,
                                                  uint32_t intrId);

static uint32_t EnetSoc_getMcuEnetControl(Enet_MacPort macPort,
                                          uint32_t *modeSel);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* -------------------------------- CPSW 3G --------------------------------- */

CpswSoc_Cfg gEnetSoc_cpsw3gSocCfg =
{
    /* As per the clocking specification the mcu_sysclk0 is 1000MHz with
     * fixed /3 divider. */
    .cppiClkFreqHz = 200000000LU,
    .intrs =
    {
        {   /* RX_THRESH - CPDMA Rx Thresh Interrupt */
            .intrId     = CPSW_INTR_RX_THRESH,
            .coreIntNum = CSLR_R5FSS0_CORE1_INTR_CPSW0_TH_THRESH_INTR,
            .triggerType= ENETOSAL_ARM_GIC_TRIG_TYPE_EDGE,
        },
        {   /* RX_PEND - CPDMA Rx pending interrupt */
            .intrId     = CPSW_INTR_RX_PEND,
            .coreIntNum = CSLR_R5FSS0_CORE1_INTR_CPSW0_TH_INTR,
            .triggerType= ENETOSAL_ARM_GIC_TRIG_TYPE_EDGE,
        },
        {   /* TX_PEND - CPDMA Tx pending interrupt */
            .intrId     = CPSW_INTR_TX_PEND,
            .coreIntNum = CSLR_R5FSS0_CORE1_INTR_CPSW0_FH_INTR,
            .triggerType= ENETOSAL_ARM_GIC_TRIG_TYPE_EDGE,
        },
        {   /* MISC_PEND - Miscellaneous interrupt for CPTS, MDIO and Stats*/
            .intrId     = CPSW_INTR_MISC_PEND,
            .coreIntNum = CSLR_R5FSS0_CORE1_INTR_CPSW0_MISC_INTR,
            .triggerType= ENETOSAL_ARM_GIC_TRIG_TYPE_EDGE,
        },
    },
    .txChCount       = ENET_CPDMA_CPSW_MAX_TX_CH, /* TODO: set to 8 eventually */
    .rxChCount       = ENET_CPDMA_CPSW_MAX_RX_CH, /* TODO: set to 8 eventually */
    .cptsHwPushCount = 4U,
};

/* CPSW_3G MAC port template */
#define ENET_SOC_CPSW3G_MACPORT(n)                                    \
{                                                                     \
    .enetMod =                                                        \
    {                                                                 \
        .name       = "cpsw3G.macport" #n,                            \
        .physAddr   = (CSL_CPSW0_U_BASE + CPSW_NU_OFFSET),         \
        .physAddr2  = (CSL_CPSW0_U_BASE + CPSW_SGMII0_OFFSET(0U)), \
        .features   = (ENET_FEAT_BASE |                               \
                       CPSW_MACPORT_FEATURE_EST |                     \
                       CPSW_MACPORT_FEATURE_SGMII |                   \
                       CPSW_MACPORT_FEATURE_INTERVLAN),               \
        .errata     = ENET_ERRATA_NONE,                               \
        .open       = &CpswMacPort_open,                              \
        .rejoin     = &CpswMacPort_rejoin,                            \
        .ioctl      = &CpswMacPort_ioctl,                             \
        .close      = &CpswMacPort_close,                             \
    },                                                                \
    .macPort = ENET_MAC_PORT_ ## n,                                   \
}

/* CPSW_3G MAC ports */
CpswMacPort_Obj gEnetSoc_cpsw3gMacObj[] =
{
    ENET_SOC_CPSW3G_MACPORT(1),
    ENET_SOC_CPSW3G_MACPORT(2),
};


/* CPSW 3G Peripheral */
Cpsw_Obj gEnetSoc_cpsw3g =
{
    .enetPer =
    {
        .name         = "cpsw3g",
        .enetType     = ENET_CPSW_3G,
        .instId       = 0U,
        .magic        = ENET_NO_MAGIC,
        .physAddr     = (CSL_CPSW0_U_BASE + CPSW_NU_OFFSET),
        .physAddr2    = (CSL_CPSW0_U_BASE + CPSW_NUSS_OFFSET),
        .features     = (ENET_FEAT_BASE |
                         CPSW_FEATURE_EST | 
                         CPSW_FEATURE_INTERVLAN),
        .errata       = ENET_ERRATA_NONE,
        .initCfg      = &Cpsw_initCfg,
        .open         = &Cpsw_open,
        .rejoin       = &Cpsw_rejoin,
        .ioctl        = &Cpsw_ioctl,
        .poll         = &Cpsw_poll,
        .convertTs    = NULL,
        .periodicTick = &Cpsw_periodicTick,
        .registerEventCb = NULL,
        .unregisterEventCb = NULL,
        .close        = &Cpsw_close,
    },

    /* Host port module object */
    .hostPortObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.hostport",
            .physAddr   = (CSL_CPSW0_U_BASE + CPSW_NU_OFFSET),
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &CpswHostPort_open,
            .rejoin     = &CpswHostPort_rejoin,
            .ioctl      = &CpswHostPort_ioctl,
            .close      = &CpswHostPort_close,
        }
    },

    /* MAC port module objects */
    .macPortObj = gEnetSoc_cpsw3gMacObj,
    .macPortNum = ENET_ARRAYSIZE(gEnetSoc_cpsw3gMacObj),

    /* ALE module object */
    .aleObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.ale",
            .physAddr   = (CSL_CPSW0_U_BASE + CPSW_ALE_OFFSET),
            .features   = (CPSW_ALE_FEATURE_FLOW_PRIORITY |
                           CPSW_ALE_FEATURE_IP_HDR_WHITELIST),
            .errata     = ENET_ERRATA_NONE,
            .open       = &CpswAle_open,
            .rejoin     = &CpswAle_rejoin,
            .ioctl      = &CpswAle_ioctl,
            .close      = &CpswAle_close,
        },
    },

    /* CPTS module object */
    .cptsObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.cpts",
            .physAddr   = (CSL_CPSW0_U_BASE + CPSW_CPTS_OFFSET),
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &CpswCpts_open,
            .rejoin     = &CpswCpts_rejoin,
            .ioctl      = &CpswCpts_ioctl,
            .close      = &CpswCpts_close,
        },
    },

    /* MDIO module object */
    .mdioObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.mdio",
            .physAddr   = (CSL_CPSW0_U_BASE + CPSW_MDIO_OFFSET),
            .features   = MDIO_FEATURE_CLAUSE45,
            .errata     = ENET_ERRATA_NONE,
            .open       = &Mdio_open,
            .rejoin     = &Mdio_rejoin,
            .ioctl      = &Mdio_ioctl,
            .close      = &Mdio_close,
        },
    },

    /* Statistics module object */
    .statsObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.stats",
            .physAddr   = (CSL_CPSW0_U_BASE + CPSW_NU_OFFSET),
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &CpswStats_open,
            .rejoin     = &CpswStats_rejoin,
            .ioctl      = &CpswStats_ioctl,
            .close      = &CpswStats_close,
        },
    },

    /* RM module object */
    .rmObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.rm",
            .physAddr   = 0U,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &EnetRm_open,
            .rejoin     = &EnetRm_rejoin,
            .ioctl      = &EnetRm_ioctl,
            .close      = &EnetRm_close,
        },
    },
};

/* ---------------------------- Enet Peripherals ---------------------------- */

Enet_Obj gEnetSoc_perObj[] =
{
    /* CPSW_3G Enet driver/peripheral */
    {
        .enetPer = &gEnetSoc_cpsw3g.enetPer,
    },
};

EnetCpdma_DrvObj gEnetSoc_dmaObj[ENET_ARRAYSIZE(gEnetSoc_perObj)] = 
{
    [0] = 
    {
        .enetType   = ENET_CPSW_3G,
        .cpdmaRegs  = (CSL_CpdmaRegs *)(CSL_CPSW0_U_BASE + CPSW_CPDMA_OFFSET),
        .cpswSsRegs = (CSL_Xge_cpsw_ss_sRegs *)(CSL_CPSW0_U_BASE + CPSW_NUSS_OFFSET),
        .features = (ENET_FEAT_BASE | ENET_CPDMA_CHANNEL_OVERRIDE),
    },
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EnetSoc_init(void)
{
    EnetDma_Handle hDma = NULL;
    uint32_t size;
    uintptr_t cpswDescMem;

    hDma = &gEnetSoc_dmaObj[0U];
    memset(hDma, 0, sizeof(*hDma));
    hDma->initFlag   = true;
    hDma->enetType   = ENET_CPSW_3G;
    hDma->cpdmaRegs  = (CSL_CpdmaRegs *)(CSL_CPSW0_U_BASE + CPSW_CPDMA_OFFSET);
    hDma->cpswSsRegs = (CSL_Xge_cpsw_ss_sRegs *)(CSL_CPSW0_U_BASE + CPSW_NUSS_OFFSET);
    EnetSoc_getCppiDescInfo(ENET_CPSW_3G,
                            0,
                           &cpswDescMem,
                           &size);
    hDma->cppiRamBase = (void *)cpswDescMem;
    Enet_devAssert((sizeof(EnetCpdma_cppiDesc) <= ENETDMA_CACHELINE_ALIGNMENT),
                    "CPDMA desc size higher than expected: %d Bytes", sizeof(EnetCpdma_cppiDesc));
    hDma->maxBds = size/sizeof(EnetCpdma_cppiDesc);
    hDma->features = (ENET_FEAT_BASE | ENET_CPDMA_CHANNEL_OVERRIDE);

    /* Nothing to do */
    return ENET_SOK;
}

void EnetSoc_deinit(void)
{

}

EnetDma_Handle EnetSoc_getDmaHandle(Enet_Type enetType,
                                    uint32_t instId)
{
    EnetDma_Handle hDma = NULL;

    switch (enetType)
    {
        case ENET_CPSW_3G:
            if (instId == 0U)
            {
                hDma = &gEnetSoc_dmaObj[0U];
            }
            break;

        default:
            break;
    }

    return hDma;
}

Enet_Handle EnetSoc_getEnetHandleByIdx(uint32_t idx)
{
    Enet_Handle hEnet = NULL;

    if (idx < ENET_ARRAYSIZE(gEnetSoc_perObj))
    {
        hEnet = &gEnetSoc_perObj[idx];
    }

    return hEnet;
}

Enet_Handle EnetSoc_getEnetHandle(Enet_Type enetType,
                                  uint32_t instId)
{
    Enet_Handle hEnet = NULL;

    switch (enetType)
    {
        case ENET_CPSW_3G:
            if (instId == 0U)
            {
                hEnet = &gEnetSoc_perObj[0U];
            }
            break;

        default:
            break;
    }

    return hEnet;
}

bool EnetSoc_isCoreAllowed(Enet_Type enetType,
                           uint32_t instId,
                           uint32_t coreId)
{
    return true;
}

uint32_t EnetSoc_getEnetNum(void)
{
    return ENET_ARRAYSIZE(gEnetSoc_perObj);
}

uint32_t EnetSoc_getMacPortMax(Enet_Type enetType,
                               uint32_t instId)
{
    uint32_t numPorts = 0U;

    if ((enetType == ENET_CPSW_3G) && (instId == 0U))
    {
        numPorts = gEnetSoc_cpsw3g.macPortNum;
    }
    else
    {
        Enet_devAssert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
    }

    return numPorts;
}

uint32_t EnetSoc_getCoreId(void)
{
    uint32_t coreId = CSL_CORE_ID_R5FSS0_1;

    return coreId;
}

uint32_t EnetSoc_getClkFreq(Enet_Type enetType,
                            uint32_t instId,
                            uint32_t clkId)
{
    uint32_t freq = 0U;

    if (clkId == CPSW_CPPI_CLK)
    {
        if ((enetType == ENET_CPSW_3G) && (instId == 0U))
        {
            freq = gEnetSoc_cpsw3gSocCfg.cppiClkFreqHz;
        }
        else
        {
            Enet_devAssert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
        }
    }
    else
    {
        Enet_devAssert(false, "Invalid clk id %u\n", clkId);
    }

    return freq;
}

static const EnetSoc_IntrCfg *EnetSoc_findIntrCfg(Enet_Type enetType,
                                                  uint32_t instId,
                                                  uint32_t intrId)
{
    const EnetSoc_IntrCfg *intrCfg =  NULL;
    const EnetSoc_IntrCfg *intrs = NULL;
    uint32_t numIntrs = 0U;
    uint32_t i;

    if ((enetType == ENET_CPSW_3G) && (instId == 0U))
    {
        intrs    = gEnetSoc_cpsw3gSocCfg.intrs;
        numIntrs = ENET_ARRAYSIZE(gEnetSoc_cpsw3gSocCfg.intrs);
    }
    else
    {
        Enet_devAssert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
    }

    for (i = 0U; i < numIntrs; i++)
    {
        if (intrs[i].intrId == intrId)
        {
            intrCfg = &intrs[i];
            break;
        }
    }

    Enet_devAssert(intrCfg != NULL, "No config found for intr %u (eneType=%u instId=%u)\n", intrId, enetType, instId);

    return intrCfg;
}

int32_t EnetSoc_setupIntrCfg(Enet_Type enetType,
                             uint32_t instId,
                             uint32_t intrId)
{
    int32_t status = ENET_SOK;

    return status;
}

int32_t EnetSoc_releaseIntrCfg(Enet_Type enetType,
                               uint32_t instId,
                               uint32_t intrId)
{
    int32_t status = ENET_SOK;

    return status;
}

uint32_t EnetSoc_getIntrNum(Enet_Type enetType,
                            uint32_t instId,
                            uint32_t intrId)
{
    const EnetSoc_IntrCfg *intrCfg;
    uint32_t intrNum = 0U;

    intrCfg = EnetSoc_findIntrCfg(enetType, instId, intrId);
    Enet_devAssert(intrCfg != NULL,
                   "per%u.%u: Failed to get config for intr %u\n", enetType, instId, intrId);

    if (intrCfg != NULL)
    {
        intrNum = intrCfg->coreIntNum;
    }

    return intrNum;
}

uint32_t EnetSoc_getIntrTriggerType(Enet_Type enetType,
                                    uint32_t instId,
                                    uint32_t intrId)
{
    const EnetSoc_IntrCfg *intrCfg;
    uint32_t intrTrigType = ENETOSAL_ARM_GIC_TRIG_TYPE_LEVEL;

    intrCfg = EnetSoc_findIntrCfg(enetType, instId, intrId);
    Enet_devAssert(intrCfg != NULL,
                   "per%u.%u: Failed to get config for intr %u\n", enetType, instId, intrId);

    if (intrCfg != NULL)
    {
        intrTrigType = (uint32_t) intrCfg->triggerType;
    }

    return intrTrigType;
}




int32_t EnetSoc_getEFusedMacAddrs(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                  uint32_t *num)
{
    CSL_top_ctrlRegs *mmrRegs;
    uint32_t val;

    if (*num >= 1U)
    {
        mmrRegs = (CSL_top_ctrlRegs *)(uintptr_t)CSL_TOP_CTRL_U_BASE;

        val = CSL_REG32_RD(&mmrRegs->MAC_ID0);
        macAddr[0][5] = (uint8_t)((val & 0x000000FFU) >> 0U);
        macAddr[0][4] = (uint8_t)((val & 0x0000FF00U) >> 8U);
        macAddr[0][3] = (uint8_t)((val & 0x00FF0000U) >> 16U);
        macAddr[0][2] = (uint8_t)((val & 0xFF000000U) >> 24U);

        val = CSL_REG32_RD(&mmrRegs->MAC_ID1);
        macAddr[0][1] = (uint8_t)((val & 0x000000FFU) >> 0U);
        macAddr[0][0] = (uint8_t)((val & 0x0000FF00U) >> 8U);

        *num = 1U;
    }

    return ENET_SOK;
}

uint32_t EnetSoc_getMacPortCaps(Enet_Type enetType,
                                uint32_t instId,
                                Enet_MacPort macPort)
{
    uint32_t linkCaps = 0U;

    switch (enetType)
    {
        case ENET_CPSW_3G:
            if (macPort <= ENET_MAC_PORT_2)
            {
                linkCaps = (ENETPHY_LINK_CAP_HD10 | ENETPHY_LINK_CAP_FD10 |
                            ENETPHY_LINK_CAP_HD100 | ENETPHY_LINK_CAP_FD100 |
                            ENETPHY_LINK_CAP_FD1000);
            }
            break;

        default:
            Enet_devAssert(false, "Invalid peripheral type: %u\n", enetType);
            break;
    }

    return linkCaps;
}

int32_t EnetSoc_getMacPortMii(Enet_Type enetType,
                              uint32_t instId,
                              Enet_MacPort macPort,
                              EnetMacPort_Interface *mii)
{
    EnetMac_LayerType *enetLayer = &mii->layerType;
    EnetMac_SublayerType *enetSublayer = &mii->sublayerType;
    uint32_t modeSel = CPSW_ENET_CTRL_MODE_RGMII;
    int32_t status = ENET_EFAIL;

    switch (enetType)
    {
        case ENET_CPSW_3G:
            status = EnetSoc_getMcuEnetControl(macPort, &modeSel);
            break;

        default:
            Enet_devAssert(false, "Invalid peripheral type: %u\n", enetType);
            break;
    }

    if (status == ENET_SOK)
    {
        switch (modeSel)
        {
            /* RMII */
            case CPSW_ENET_CTRL_MODE_RMII:
                *enetLayer    = ENET_MAC_LAYER_MII;
                *enetSublayer = ENET_MAC_SUBLAYER_REDUCED;
                break;

            /* RGMII */
            case CPSW_ENET_CTRL_MODE_RGMII:
                *enetLayer    = ENET_MAC_LAYER_GMII;
                *enetSublayer = ENET_MAC_SUBLAYER_REDUCED;
                break;

            default:
                status = ENET_EINVALIDPARAMS;
                break;
        }
    }

    return status;
}

static uint32_t EnetSoc_getMcuEnetControl(Enet_MacPort macPort,
                                          uint32_t *modeSel)
{
    CSL_mss_ctrlRegs *mssCtrlRegs = (CSL_mss_ctrlRegs *)CSL_MSS_CTRL_U_BASE;
    int32_t status = ENET_SOK;

    switch (macPort)
    {
        case ENET_MAC_PORT_1:
            *modeSel = CSL_FEXT(mssCtrlRegs->CPSW_CONTROL,MSS_CTRL_CPSW_CONTROL_PORT1_MODE_SEL);
            break;

        case ENET_MAC_PORT_2:
            *modeSel = CSL_FEXT(mssCtrlRegs->CPSW_CONTROL,MSS_CTRL_CPSW_CONTROL_PORT2_MODE_SEL);
            break;

        default:
            status = ENET_EINVALIDPARAMS;
            break;
    }

    if (status == ENET_SOK)
    {
        switch (*modeSel)
        {
            case CPSW_ENET_CTRL_MODE_RMII:
            case CPSW_ENET_CTRL_MODE_RGMII:
                break;

            default:
                status = ENET_EINVALIDPARAMS;
                break;
        }
    }

    return status;
}

int32_t EnetSoc_validateQsgmiiCfg(Enet_Type enetType,
                                  uint32_t instId)
{
    int32_t status = ENET_EFAIL;

    return status;
}

int32_t EnetSoc_mapPort2QsgmiiId(Enet_Type enetType,
                                 uint32_t instId,
                                 Enet_MacPort macPort,
                                 uint32_t *qsgmiiId)
{
    int32_t status = ENET_EFAIL;

    return status;
}

uint32_t EnetSoc_getRxFlowCount(Enet_Type enetType,
                               uint32_t instId)
{
    return gEnetSoc_cpsw3gSocCfg.rxChCount;
}

uint32_t EnetSoc_getTxChPeerId(Enet_Type enetType,
                               uint32_t instId,
                               uint32_t chNum)
{
    return 0U;
}

uint32_t EnetSoc_getRxChPeerId(Enet_Type enetType,
                               uint32_t instId,
                               uint32_t chIdx)
{
    return 0U;
}

uint32_t EnetSoc_getTxChCount(Enet_Type enetType,
                               uint32_t instId)
{
    return gEnetSoc_cpsw3gSocCfg.txChCount;
}
/*!
 *  \brief CPSW3G default configuration
 *
 *   Note: If user wishes to change the Resource Partition the following
 *   things must be considered:
 *   1. Sum of numTxCh allocated to each core should not exceed 8.
 *   2. Sum of numRxFlows allocated to each core should not exceed 7 (not 8),
 *      as one Rx flow is reserved to the master core.
 *
 */
static EnetRm_ResPrms gEnetAppRmDefCfg_3G =
{
    .coreDmaResInfo =
    {
        [0] =
        {
            .coreId        = CSL_CORE_ID_R5FSS0_1,
            .numTxCh       = ENET_SYSCFG_TX_CHANNELS_NUM,    /* numTxCh */
            .numRxFlows    = ENET_SYSCFG_RX_FLOWS_NUM,    /* numRxFlows */
            .numMacAddress = 4U,    /* numMacAddress */
        },
    },
    .numCores = 1U,
};

const EnetRm_ResPrms *EnetAppRm_getResPartInfo(Enet_Type enetType)
{
    EnetRm_ResPrms *rmInitPrms = NULL;

    switch (enetType)
    {
        case ENET_CPSW_3G:
        {
            rmInitPrms = &gEnetAppRmDefCfg_3G;
            break;
        }
        default:
        {
            rmInitPrms = NULL;
        }
    }
    if (rmInitPrms != NULL)
    {
        /* Update coreid to the local core */
        rmInitPrms->coreDmaResInfo[0].coreId = EnetSoc_getCoreId();
    }

    return(rmInitPrms);
}

/* Cores IOCTL Privileges */

static const EnetRm_IoctlPermissionTable gEnetAppIoctlPermission_3G =
{
    .defaultPermittedCoreMask = (ENET_BIT(CSL_CORE_ID_R5FSS0_0) |
                                 ENET_BIT(CSL_CORE_ID_R5FSS0_1) |
                                 ENET_BIT(CSL_CORE_ID_R5FSS1_0) |
                                 ENET_BIT(CSL_CORE_ID_R5FSS1_1)),
    .numEntries = 0,
};

const EnetRm_IoctlPermissionTable *EnetAppRm_getIoctlPermissionInfo(Enet_Type enetType)
{
    const EnetRm_IoctlPermissionTable *ioctlPerm = NULL;

    switch (enetType)
    {
        case ENET_CPSW_3G:
        {
            ioctlPerm = &gEnetAppIoctlPermission_3G;
            break;
        }

        default:
        {
            ioctlPerm = NULL;
            break;
        }
    }

    return(ioctlPerm);
}

uint32_t EnetSoC_mapApp2CpdmaCoreId(uint32_t appCoreId)
{
    CSL_ArmR5CPUInfo cpuInfo;
    uint32_t coreId = appCoreId;

    CSL_armR5GetCpuID(&cpuInfo);
    if (cpuInfo.grpId == (uint32_t)CSL_ARM_R5_CLUSTER_GROUP_ID_0)
    {
        if (cpuInfo.cpuID == CSL_ARM_R5_CPU_ID_0)
        {
            coreId = CSL_CORE_ID_R5FSS0_0;
        }
        if (cpuInfo.cpuID == CSL_ARM_R5_CPU_ID_1)
        {
            coreId = CSL_CORE_ID_R5FSS0_1;
        }
    }
    if (cpuInfo.grpId == (uint32_t)CSL_ARM_R5_CLUSTER_GROUP_ID_1)
    {
        if (cpuInfo.cpuID == CSL_ARM_R5_CPU_ID_0)
        {
             coreId = CSL_CORE_ID_R5FSS1_0;
        }
        if (cpuInfo.cpuID == CSL_ARM_R5_CPU_ID_1)
        {
            coreId = CSL_CORE_ID_R5FSS1_1;
        }
    }

    return coreId;
}
