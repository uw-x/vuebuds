/*
 * HM01B0_CAPTURE.c
 *
 *  Created on: Nov 25, 2018
 *      Author: Ali Najafi
 */
#include <stdbool.h>

#include "HM01B0_CAPTURE.h"
#include "gpio.h"
#include "i2c.h"
#include "HM01B0_LVLD_TIMER.h"
#include "nrf_delay.h"
#include "ble_manager.h"
#include "timers.h"
#include "event.h"
#include "HM01B0_SPI.h"

uint32_t line_count = 0;
bool image_rd_done = 0;
static bool streaming;
uint16_t total_image_size;

void cameraPowerEnable(void)
{
  nrf_gpio_cfg(
    CAM_POWER,
    NRF_GPIO_PIN_DIR_OUTPUT,
    NRF_GPIO_PIN_INPUT_DISCONNECT,
    NRF_GPIO_PIN_NOPULL,
    NRF_GPIO_PIN_H0H1,
    NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_pin_clear(CAM_POWER);
  nrf_delay_ms(POR_DELAY);
  nrf_gpio_pin_set(CAM_POWER);
  nrf_delay_ms(POR_DELAY);
}

bool hm_get_capture_done(void)
{
  return image_rd_done;
}

void hm_reset_capture_done(void)
{
  line_count = 0;
  image_rd_done = false;
}

void hm_set_capture_done(void)
{
  if (!image_rd_done) {
    NRF_LOG_RAW_INFO("%08d [cam] IMAGE READ DONE %d\n", systemTimeGetMs(), spiSlaveGetBytesReceived());
    eventQueuePush(EVENT_CAMERA_CAPTURE_DONE);
    image_rd_done = true;
  }
}

void hm01b0_init(void){
  /*Test if camera is functional*/
  uint8_t version = i2cRead16(CAMERA_I2C_ADDR, REG_MODEL_ID_L);
  if(version != 0xB0){
    NRF_LOG_INFO("REG_MODEL_ID_L: %x \n", version);
    NRF_LOG_INFO("Camera version problem \n");
  }
  i2cWrite16(CAMERA_I2C_ADDR,  REG_MODE_SELECT, 0x00);//go to stand by mode

  /*Initialize and set high the SPI chip select*/
  nrf_gpio_cfg_output(CAM_SPI_CS_OUT);//Set up the chip select for SPI
  nrf_gpio_pin_set(CAM_SPI_CS_OUT);

  /*Camera settings initialization*/
  // hm01b0_init_fixed_rom_qvga_fixed();
  hm01b0_init_optimized_vlm();

  // arducam_default();
  // hm01b0_init_datasheet_default();
}

/*Deactivates the peripherals that change the power consumption*/
void hm_peripheral_uninit(void)
{
  spiSlaveDeInit();
  nrf_drv_timer_disable(&CAM_TIMER);
  CAM_TIMER.p_reg->TASKS_SHUTDOWN = 1;
}

void hm_peripheral_init(void)
{
  hm_clk_out(); // initializes and starts timer
  NRF_LOG_INFO("[camera] mclk out");

  delayMs(100);

  /*Initialize the GPIO settings: Frame valid for */
  gpio_setting_init();
  hm01b0_init();
  lvld_timer_init();

  #if(CAM_CLK_GATING == 1)
    nrf_drv_timer_disable(&CAM_TIMER);
  #endif
}

/*Activates the peripherals that change the power consumption, when connected and before taking picture*/
void hm_peripheral_connected_init(void){
  spiSlaveInit();
  spiSlaveSetBuffers();
}

void hm_single_capture_spi_832_stream(void){
  #if (CAM_CLK_GATING == 1)
  if (!streaming) {
    nrf_drv_timer_enable(&CAM_TIMER);
  }
  #endif

  line_count = 0;

  /*SPI registers initilization*/
  spiSlaveSetTransferDone(false);
  spiSlaveClearByteCounters();
  spiSlaveSetBuffers();

  /*Camera values initialized*/
  hm_reset_capture_done();

  /*Enable the FRAME VALID interrupt*/
  nrf_drv_gpiote_in_event_enable(CAM_FRAME_VALID, true);

  i2cWrite16(CAMERA_I2C_ADDR, REG_MODE_SELECT, STREAMING_MODE);

  if (!streaming) {
    streaming = true;
    NRF_LOG_RAW_INFO("%08d [cam] stream started\n", systemTimeGetMs());
  }
}


void hm_single_capture_spi_832(void)
{
  #if(CAM_CLK_GATING == 1)
  nrf_drv_timer_enable(&CAM_TIMER);
  #endif

  line_count = 0;

  /*SPI registers initilization*/
  spiSlaveSetTransferDone(false);
  spiSlaveClearByteCounters();
  spiSlaveSetBuffers();

  /* Camera values initialized */
  hm_reset_capture_done();

  /*Enable the FRAME VALID interrupt*/
  nrf_drv_gpiote_in_event_enable(CAM_FRAME_VALID, true);
  lvld_timer_enable();
  nrf_gpio_pin_clear(CAM_SPI_CS_OUT);

  i2cWrite16(CAMERA_I2C_ADDR, REG_MODE_SELECT, SINGLE_SHOT_MODE); // If we use the 0x03 mode for single capture, the power of camera stays high after capturing one frame
  // i2cWrite16(CAMERA_I2C_ADDR, REG_MODE_SELECT, STREAMING_MODE);   // If we use the 0x03 mode for single capture, the power of camera stays high after capturing one frame

  while (!hm_get_capture_done()) {
    NRF_LOG_PROCESS();
    __WFE();
  };
  while (!spiSlaveGetTransferDone()) {
    NRF_LOG_PROCESS();
    __WFE();
  };
}

void hm_standby_mode(bool standby)
{
  static bool currentStandby = false;
  if (standby) {
    i2cWrite16(CAMERA_I2C_ADDR, REG_MODE_SELECT, 0x00);
    delayMs(5);
    hm_clk_enable(false);
    // spiSlaveDeInit();
    // Clock gate
    currentStandby = true;
  } else if (currentStandby) {
    // spiSlaveInit();
    // spiSlaveSetBuffers();
    // spiSlaveClearByteCounters();
    hm_clk_enable(true);
    delayMs(5);
    i2cWrite16(CAMERA_I2C_ADDR, REG_MODE_SELECT, STREAMING_MODE);
    currentStandby = false;
  }
}