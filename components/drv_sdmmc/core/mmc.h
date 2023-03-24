/*
 * The Clear BSD License
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _FSL_MMC_H_
#define _FSL_MMC_H_

#include "sdmmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @addtogroup MMCCARD
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define OCR_BUSY            0x80000000
#define OCR_HCS             0x40000000
#define OCR_VOLTAGE_MASK    0x007FFF80
#define OCR_ACCESS_MODE     0x60000000

#define MMC_VDD_165_195     0x00000080  /* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21       0x00000100  /* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22       0x00000200  /* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23       0x00000400  /* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24       0x00000800  /* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25       0x00001000  /* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26       0x00002000  /* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27       0x00004000  /* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28       0x00008000  /* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29       0x00010000  /* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30       0x00020000  /* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31       0x00040000  /* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32       0x00080000  /* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33       0x00100000  /* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34       0x00200000  /* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35       0x00400000  /* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36       0x00800000  /* VDD voltage 3.5 ~ 3.6 */


/*! @brief MMC card flags */
enum _mmc_card_flag {
    kMMC_SupportHighSpeed26MHZFlag = (1U << 0U),           /*!< Support high speed 26MHZ */
    kMMC_SupportHighSpeed52MHZFlag = (1U << 1U),           /*!< Support high speed 52MHZ */
    kMMC_SupportHighSpeedDDR52MHZ180V300VFlag = (1 << 2U), /*!< ddr 52MHZ 1.8V or 3.0V */
    kMMC_SupportHighSpeedDDR52MHZ120VFlag = (1 << 3U),     /*!< DDR 52MHZ 1.2V */
    kMMC_SupportHS200200MHZ180VFlag = (1 << 4U),           /*!< HS200 ,200MHZ,1.8V */
    kMMC_SupportHS200200MHZ120VFlag = (1 << 5U),           /*!< HS200, 200MHZ, 1.2V */
    kMMC_SupportHS400DDR200MHZ180VFlag = (1 << 6U),        /*!< HS400, DDR, 200MHZ,1.8V */
    kMMC_SupportHS400DDR200MHZ120VFlag = (1 << 7U),        /*!< HS400, DDR, 200MHZ,1.2V */
    kMMC_SupportHighCapacityFlag = (1U << 8U),             /*!< Support high capacity */
    kMMC_SupportAlternateBootFlag = (1U << 9U),            /*!< Support alternate boot */
    kMMC_SupportDDRBootFlag = (1U << 10U),                 /*!< support DDR boot flag*/
    kMMC_SupportHighSpeedBootFlag = (1U << 11U),           /*!< support high speed boot flag*/
};

/*!
 * @brief mmc card state
 *
 * Define the card structure including the necessary fields to identify and describe the card.
 */
typedef struct _mmc_card {
    SDMMCHOST_CONFIG host; /*!< Host information */

    bool isHostReady;                /*!< Use this flag to indicate if need host re-init or not*/
    bool noInteralAlign;             /*!< use this flag to disable sdmmc align. If disable, sdmmc will not make sure the
                           data buffer address is word align, otherwise all the transfer are align to low level driver */
    bool forceUseBuffer;            /*!< use this flag to fore using internal buffer for data transfer in cache system */
    uint32_t *forceBuffer;          /*!< force Buffer Point */
    uint32_t busClock_Hz;            /*!< MMC bus clock united in Hz */
    uint32_t relativeAddress;        /*!< Relative address of the card */
    bool enablePreDefinedBlockCount; /*!< Enable PRE-DEFINED block count when read/write */
    uint32_t flags;                  /*!< Capability flag in _mmc_card_flag */
    uint32_t rawCid[4U];             /*!< Raw CID content */
    uint32_t rawCsd[4U];             /*!< Raw CSD content */
    uint32_t rawExtendedCsd[MMC_EXTENDED_CSD_BYTES / 4U]; /*!< Raw MMC Extended CSD content */
    uint32_t ocr;                                         /*!< Raw OCR content */
    mmc_cid_t cid;                                        /*!< CID */
    mmc_csd_t csd;                                        /*!< CSD */
    mmc_extended_csd_t extendedCsd;                       /*!< Extended CSD */
    uint32_t block_size;                                   /*!< Card block size */
    uint32_t userPartitionBlocks;                         /*!< Card total block number in user partition */
    uint32_t bootPartitionBlocks;                         /*!< Boot partition size united as block size */
    uint32_t eraseGroupBlocks;                            /*!< Erase group size united as block size */
    mmc_access_partition_t currentPartition;              /*!< Current access partition */
    mmc_voltage_window_t hostVoltageWindowVCCQ;           /*!< Host IO voltage window */
    mmc_voltage_window_t hostVoltageWindowVCC; /*!< application must set this value according to board specific */
    mmc_high_speed_timing_t busTiming;         /*!< indicate the current work timing mode*/
    mmc_data_bus_width_t busWidth;             /*!< indicate the current work bus width */
} mmc_card_t;

/*************************************************************************************************
 * API
 ************************************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name MMCCARD Function
 * @{
 */

/*!
 * @brief Initializes the MMC card and host.
 *
 * @param card Card descriptor.
 *
 * @retval kStatus_SDMMC_HostNotReady host is not ready.
 * @retval kStatus_SDMMC_GoIdleFailed Go idle failed.
 * @retval kStatus_SDMMC_SendOperationConditionFailed Send operation condition failed.
 * @retval kStatus_SDMMC_AllSendCidFailed Send CID failed.
 * @retval kStatus_SDMMC_SetRelativeAddressFailed Set relative address failed.
 * @retval kStatus_SDMMC_SendCsdFailed Send CSD failed.
 * @retval kStatus_SDMMC_CardNotSupport Card not support.
 * @retval kStatus_SDMMC_SelectCardFailed Send SELECT_CARD command failed.
 * @retval kStatus_SDMMC_SendExtendedCsdFailed Send EXT_CSD failed.
 * @retval kStatus_SDMMC_SetBusWidthFailed Set bus width failed.
 * @retval kStatus_SDMMC_SwitchHighSpeedFailed Switch high speed failed.
 * @retval kStatus_SDMMC_SetCardBlockSizeFailed Set card block size failed.
 * @retval kStatus_Success Operate successfully.
 */
status_t MMC_Init(mmc_card_t *card, void *user_data, uint32_t sdif);

/*!
 * @brief Deinitializes the card and host.
 *
 * @param card Card descriptor.
 */
void MMC_Deinit(mmc_card_t *card);

/*!
 * @brief intialize the card.
 *
 * @param card Card descriptor.
 *
 * @retval kStatus_SDMMC_HostNotReady host is not ready.
 * @retval kStatus_SDMMC_GoIdleFailed Go idle failed.
 * @retval kStatus_SDMMC_SendOperationConditionFailed Send operation condition failed.
 * @retval kStatus_SDMMC_AllSendCidFailed Send CID failed.
 * @retval kStatus_SDMMC_SetRelativeAddressFailed Set relative address failed.
 * @retval kStatus_SDMMC_SendCsdFailed Send CSD failed.
 * @retval kStatus_SDMMC_CardNotSupport Card not support.
 * @retval kStatus_SDMMC_SelectCardFailed Send SELECT_CARD command failed.
 * @retval kStatus_SDMMC_SendExtendedCsdFailed Send EXT_CSD failed.
 * @retval kStatus_SDMMC_SetBusWidthFailed Set bus width failed.
 * @retval kStatus_SDMMC_SwitchHighSpeedFailed Switch high speed failed.
 * @retval kStatus_SDMMC_SetCardBlockSizeFailed Set card block size failed.
 * @retval kStatus_Success Operate successfully.
 */
status_t MMC_CardInit(mmc_card_t *card);

/*!
 * @brief Deinitializes the card.
 *
 * @param card Card descriptor.
 */
void MMC_CardDeinit(mmc_card_t *card);

/*!
 * @brief initialize the host.
 *
 * This function deinitializes the specific host.
 *
 * @param card Card descriptor.
 */
status_t MMC_HostInit(mmc_card_t *card, void *user_data, uint32_t sdif);


/*!
 * @brief Deinitializes the host.
 *
 * This function deinitializes the host.
 *
 * @param card Card descriptor.
 */
void MMC_HostDeinit(mmc_card_t *card);

/*!
 * @brief reset the host.
 *
 * This function reset the specific host.
 *
 * @param host host descriptor.
 */
void MMC_HostReset(SDMMCHOST_CONFIG *host);

/*!
 * @brief Checks if the card is read-only.
 *
 * @param card Card descriptor.
 * @retval true Card is read only.
 * @retval false Card isn't read only.
 */
bool MMC_CheckReadOnly(mmc_card_t *card);

/*!
 * @brief Reads data blocks from the card.
 *
 * @param card Card descriptor.
 * @param buffer The buffer to save data.
 * @param startBlock The start block index.
 * @param block_count The number of blocks to read.
 * @retval kStatus_InvalidArgument Invalid argument.
 * @retval kStatus_SDMMC_CardNotSupport Card not support.
 * @retval kStatus_SDMMC_SetBlockCountFailed Set block count failed.
 * @retval kStatus_SDMMC_TransferFailed Transfer failed.
 * @retval kStatus_SDMMC_StopTransmissionFailed Stop transmission failed.
 * @retval kStatus_Success Operate successfully.
 */
status_t MMC_ReadBlocks(mmc_card_t *card, uint8_t *buffer, uint32_t startBlock, uint32_t block_count);

/*!
 * @brief Writes data blocks to the card.
 *
 * @param card Card descriptor.
 * @param buffer The buffer to save data blocks.
 * @param startBlock Start block number to write.
 * @param block_count Block count.
 * @retval kStatus_InvalidArgument Invalid argument.
 * @retval kStatus_SDMMC_NotSupportYet Not support now.
 * @retval kStatus_SDMMC_SetBlockCountFailed Set block count failed.
 * @retval kStatus_SDMMC_WaitWriteCompleteFailed Send status failed.
 * @retval kStatus_SDMMC_TransferFailed Transfer failed.
 * @retval kStatus_SDMMC_StopTransmissionFailed Stop transmission failed.
 * @retval kStatus_Success Operate successfully.
 */
status_t MMC_WriteBlocks(mmc_card_t *card, const uint8_t *buffer, uint32_t startBlock, uint32_t block_count);

/*!
 * @brief Erases groups of the card.
 *
 * Erase group is the smallest erase unit in MMC card. The erase range is [startGroup, endGroup].
 * The size of group is depend on info in CSD get from eMMC chip.
 *
 * @param  card Card descriptor.
 * @param  startGroup Start group number.
 * @param  endGroup End group number.
 * @retval kStatus_InvalidArgument Invalid argument.
 * @retval kStatus_SDMMC_WaitWriteCompleteFailed Send status failed.
 * @retval kStatus_SDMMC_TransferFailed Transfer failed.
 * @retval kStatus_Success Operate successfully.
 */
status_t MMC_EraseGroups(mmc_card_t *card, uint32_t startGroup, uint32_t endGroup);

/*!
 * @brief Selects the partition to access.
 *
 * @param card Card descriptor.
 * @param partitionNumber The partition number.
 * @retval kStatus_SDMMC_ConfigureExtendedCsdFailed Configure EXT_CSD failed.
 * @retval kStatus_Success Operate successfully.
 */
status_t MMC_SelectPartition(mmc_card_t *card, mmc_access_partition_t partitionNumber);

/*!
 * @brief Configures the boot activity of the card.
 *
 * @param card Card descriptor.
 * @param config Boot configuration structure.
 * @retval kStatus_SDMMC_NotSupportYet Not support now.
 * @retval kStatus_SDMMC_ConfigureExtendedCsdFailed Configure EXT_CSD failed.
 * @retval kStatus_SDMMC_ConfigureBootFailed Configure boot failed.
 * @retval kStatus_Success Operate successfully.
 */
status_t MMC_SetBootConfig(mmc_card_t *card, const mmc_boot_config_t *config);

/*!
 * @brief MMC card start boot.
 *
 * @param card Card descriptor.
 * @param mmcConfig mmc Boot configuration structure.
 * @param buffer address to recieve data.
 * @param hostConfig host boot configurations.
 * @retval kStatus_Fail fail.
 * @retval kStatus_SDMMC_TransferFailed transfer fail.
 * @retval kStatus_SDMMC_GoIdleFailed reset card fail.
 * @retval kStatus_Success Operate successfully.
 */
status_t MMC_StartBoot(mmc_card_t *card,
                       const mmc_boot_config_t *mmcConfig,
                       uint8_t *buffer,
                       SDMMCHOST_BOOT_CONFIG *hostConfig);

/*!
 * @brief MMC card set boot configuration write protect.
 *
 * @param card Card descriptor.
 * @param wp write protect value.
 */
status_t MMC_SetBootConfigWP(mmc_card_t *card, uint8_t wp);

/*!
 * @brief MMC card continous read boot data.
 *
 * @param card Card descriptor.
 * @param buffer buffer address.
 * @param hostConfig host boot configurations.
 */
status_t MMC_ReadBootData(mmc_card_t *card, uint8_t *buffer, SDMMCHOST_BOOT_CONFIG *hostConfig);

/*!
 * @brief MMC card stop boot mode.
 *
 * @param card Card descriptor.
 * @param bootMode boot mode.
 */
status_t MMC_StopBoot(mmc_card_t *card, uint32_t bootMode);

/*!
 * @brief MMC card set boot partition write protect.
 *
 * @param card Card descriptor.
 * @param bootPartitionWP boot partition write protect value.
 */
status_t MMC_SetBootPartitionWP(mmc_card_t *card, mmc_boot_partition_wp_t bootPartitionWP);

/* @} */
#if defined(__cplusplus)
}
#endif
/*! @} */
#ifdef __cplusplus
}
#endif

#endif /* _FSL_MMC_H_*/
