/*
 * HM01B0_SPI.c
 *
 *  Created on: Nov 30, 2018
 *      Author: Ali Najafi
 */
#include "HM01B0_CAPTURE.h"
#include "HM01B0_SPI.h"
#include "gpio.h"
#include "HM01B0_BLE_DEFINES.h"
#include "event.h"
#include "nrfx_spis_patch.h"
nrfx_spis_t spiSlaveInstance = NRF_DRV_SPIS_INSTANCE(SPI_SLAVE_BUS); /**< SPIS instance. */
static bool transferDone = false;
static uint8_t m_tx_buf[1] = {0};                                   /**< TX buffer. */
static uint8_t m_rx_buf[TOTAL_IMAGE_SIZE];                          /**< RX buffer  */
static uint16_t m_length_rx = IMAGE_WIDTH; /**< Transfer length. */ /**< Transfer length. */
static uint8_t m_length_tx = 0;                                     /**< Transfer length. */
static uint32_t bytesReceived = 0;
static uint32_t bytesSent = 0;
static uint32_t size_p = 0;
static bool eventQueued = false;
static uint8_t buffersReceived = 0;

nrfx_spis_config_t spiSlaveConfig = {
    .miso_pin = NRFX_SPIS_PIN_NOT_USED,
    .mosi_pin = CAM_D0,
    .sck_pin = CAM_PCLK_OUT_TO_MCU,
    .csn_pin = CAM_SPI_CS_IN,
    .mode = NRF_SPIS_MODE_0,
    .bit_order = NRF_SPIS_BIT_ORDER_MSB_FIRST,
    .csn_pullup = NRFX_SPIS_DEFAULT_CSN_PULLUP,
    .miso_drive = NRFX_SPIS_DEFAULT_MISO_DRIVE,
    .def = NRFX_SPIS_DEFAULT_DEF,
    .orc = NRFX_SPIS_DEFAULT_ORC,
    .irq_priority = NRFX_SPIS_DEFAULT_CONFIG_IRQ_PRIORITY,
    .irq_priority = 5,
};

uint16_t spiSlaveGetRxLength(void)
{
  return m_length_rx;
}
void spiSlaveSetTransferDone(bool done)
{
  transferDone = done;
}
bool spiSlaveGetTransferDone(void)
{
  return transferDone;
}
void spiSlaveEventHandler(nrf_drv_spis_event_t event)
{
  if (event.evt_type == NRF_DRV_SPIS_XFER_DONE)
  {
#ifdef QQVGA
    transferDone = true;
    bytesReceived += TOTAL_IMAGE_SIZE;
    hm_set_capture_done();
#else
    if (buffersReceived == 0)
    {
      NRF_LOG_RAW_INFO("spiSlaveEventHandler 1\n");
      buffersReceived = 1;
      APP_ERROR_CHECK(nrfx_spis_buffers_set(&spiSlaveInstance, m_tx_buf, m_length_tx, m_rx_buf + (IMAGE_WIDTH * 118), IMAGE_WIDTH * 121));
    }
    else
    {
      NRF_LOG_RAW_INFO("spiSlaveEventHandler 2\n");
      bytesReceived += TOTAL_IMAGE_SIZE;
      transferDone = true;
      hm_set_capture_done();
    }
#endif
  }
}

void spiSlaveSetBuffers(void)
{
  buffersReceived = 0;
  memset(m_rx_buf, 0, TOTAL_IMAGE_SIZE);
  // APP_ERROR_CHECK(nrfx_spis_buffers_set(&spiSlaveInstance, m_tx_buf, m_length_tx, m_rx_buf, m_length_rx));
#ifdef QQVGA
  APP_ERROR_CHECK(nrfx_spis_buffers_set(&spiSlaveInstance, m_tx_buf, m_length_tx, m_rx_buf, TOTAL_IMAGE_SIZE));
#else
  APP_ERROR_CHECK(nrfx_spis_buffers_set(&spiSlaveInstance, m_tx_buf, m_length_tx, m_rx_buf, IMAGE_WIDTH * 118));
#endif
}

void spiSlaveSetBuffersBackWithLineCount(uint32_t lineCount)
{
  APP_ERROR_CHECK(nrfx_spis_buffers_set(&spiSlaveInstance, m_tx_buf, m_length_tx, m_rx_buf + (lineCount * m_length_rx), m_length_rx));
}
void spiSlaveInit(void)
{
  APP_ERROR_CHECK(nrf_drv_spis_init(&spiSlaveInstance, &spiSlaveConfig, spiSlaveEventHandler));
  transferDone = false;
}
void spiSlaveDeInit(void)
{
  nrfx_spis_uninit(&spiSlaveInstance);
}
uint32_t spiSlaveGetRxBuffer(uint8_t **rxBuffer)
{
  *rxBuffer = m_rx_buf;
  return TOTAL_IMAGE_SIZE;
}
uint32_t spiSlaveGetRxBufferStreaming(uint8_t **rxBuffer)
{
  *rxBuffer = m_rx_buf + (bytesSent % TOTAL_IMAGE_SIZE);
  size_p = bytesReceived - bytesSent;
  if (((bytesSent % TOTAL_IMAGE_SIZE) + size_p) >= TOTAL_IMAGE_SIZE)
  {
    // send remainder of image and then start back at the front.
    size_p = TOTAL_IMAGE_SIZE - (bytesSent % TOTAL_IMAGE_SIZE);
    bytesSent += TOTAL_IMAGE_SIZE - (bytesSent % TOTAL_IMAGE_SIZE);
  }
  else
  {
    bytesSent = bytesSent + size_p;
  }
  NRF_LOG_RAW_INFO("bytesSent->%d\n", bytesSent);
  return size_p;
}
void spiSlaveClearByteCounters(void)
{
  bytesReceived = 0;
  bytesSent = 0;
}
uint32_t spiSlaveGetBytesReceived(void)
{
  return bytesReceived;
}
void spiSlaveClearEventQueued(void)
{
  eventQueued = false;
}