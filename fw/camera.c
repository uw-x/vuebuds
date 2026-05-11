#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <nrfx.h>

#include "HM01B0_CAPTURE.h"
#include "HM01B0_SPI.h"
#include "camera.h"
#include "gpio.h"

uint32_t image_size;
static bool cameraInitialized = false;

void cameraInit(void)
{
  if (!cameraInitialized) {
    hm_reset_capture_done();

    // TODO: ENABLE CAMERA POWER HERE
    // something like // cameraEnablePower();

    hm_peripheral_init();

    // TODO: This initialized the SPI slave, we should move this to when the BLE connection establishes
    hm_peripheral_connected_init();

    NRF_LOG_INFO("[camera] initialized");
    cameraInitialized = true;
  } else {
    NRF_LOG_INFO("[camera] already initialized, skipping");
  }
}

void cameraDeInit(void)
{
  cameraInitialized = false;

  gpioDisable(CAM_MCLK_IN_FROM_MCU);
  gpioDisable(CAM_INT);
  gpioDisable(CAM_LINE_VALID);
  gpioDisable(CAM_FRAME_VALID);
  gpioDisable(CAM_PCLK_OUT_TO_MCU);
  gpioDisable(CAM_D0);
  gpioDisable(CAM_D1);
  gpioDisable(CAM_D2);
  gpioDisable(CAM_D3);

  hm_peripheral_uninit();
  hm_clk_enable(false);
  // TODO: Fill this out
}

void cameraCaptureFrame(void)
{
  NRF_LOG_RAW_INFO("[cam] starting capture...\n");
  hm_single_capture_spi_832();
  NRF_LOG_RAW_INFO("[cam] Capture complete: size %i bytes\n", spiSlaveGetBytesReceived());
}

void cameraStartStream(void)
{
  NRF_LOG_RAW_INFO("Starting stream...\n");
  hm_single_capture_spi_832_stream();
}

uint32_t cameraGetFrameBuffer(uint8_t** frame)
{
  return spiSlaveGetRxBuffer(frame);
}

void cameraReadyNextFrame(void)
{
  spiSlaveSetBuffers();
  spiSlaveClearByteCounters();

  /*Camera values initialized*/
  hm_reset_capture_done();

  /*Enable the FRAME VALID interrupt*/
  nrf_drv_gpiote_in_event_enable(CAM_FRAME_VALID, true);
}

uint32_t cameraGetLines(uint8_t** lines)
{
  return spiSlaveGetRxBufferStreaming(lines);
}

uint32_t cameraGetBytesReceived(void)
{
  return spiSlaveGetBytesReceived();
}

void cameraReadyForMoreData(void)
{
  spiSlaveClearEventQueued();
}

void cameraEnableStandbyMode(bool standby)
{
  NRF_LOG_INFO("[camera] standby:%d", standby);
  hm_standby_mode(standby);
}