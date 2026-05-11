/*
 * HM01B0_SPI.h
 *
 *  Created on: Nov 20, 2018
 *      Author: Ali Najafi
 */

#ifndef HM01B0_SPI_H_
#define HM01B0_SPI_H_

/*  Standard C Included Files */
#include <stdbool.h>

#include "sdk_config.h"
#include "nrf_drv_spis.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "HM01B0_BLE_DEFINES.h"

#define SPI_SLAVE_BUS 2

// void spis_pin_set(void);

void spiSlaveSetRxDone(uint16_t value);
uint16_t spiSlaveGetRxLength(void);
void spiSlaveSetTransferDone(bool done);
bool spiSlaveGetTransferDone(void);
void spiSlaveInit(void);
void spiSlaveDeInit(void);
void spiSlaveSetBuffers(void);
void spiSlaveSetBuffersBackWithLineCount(uint32_t lineCount);
uint32_t spiSlaveGetRxBuffer(uint8_t **rxBuffer);
uint32_t spiSlaveGetRxBufferStreaming(uint8_t **rxBuffer);
void spiSlaveClearByteCounters(void);
uint32_t spiSlaveGetBytesReceived(void);
void spiSlaveClearEventQueued(void);

#endif /* HM01B0_SPI_H_ */
