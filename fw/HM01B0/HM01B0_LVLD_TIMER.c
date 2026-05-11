/*
 * HM01B0_LVLD_TIMER.h
 *
 *  Created on: April 25, 2019
 *      Author: Ali Najafi
 */

#include "HM01B0_LVLD_TIMER.h"
#include "ble_manager.h"
#include "HM01B0_SPI.h"
#include "HM01B0_BLE_DEFINES.h"
#include "gpio.h"
#include "timers.h"

// #define LVLD_TIMER_VALUE 2800 // the second number in multiplication is equal to 64/cam_mclk_freq; if cam_mcl_freq=8MHz => 8

#define CAM_MCLK_FREQ_MHZ 8
#ifdef QQVGA
  #define LVLD_TIMER_VALUE 2500 // the second number in multiplication is equal to 64/cam_mclk_freq; if cam_mcl_freq=8MHz => 8
#else
  #define LVLD_TIMER_VALUE 5000 // the second number in multiplication is equal to 64/cam_mclk_freq; if cam_mcl_freq=8MHz => 8
#endif



const nrf_drv_timer_t TIMER_LVLD = NRF_DRV_TIMER_INSTANCE(4);

void lvld_timer_enable(void)
{
  nrf_drv_timer_enable(&TIMER_LVLD);
}

void lvld_timer_disable(void)
{
  nrf_drv_timer_disable(&TIMER_LVLD);
}

/**
 * @brief Handler for timer events.
 */
void timer_lvld_event_handler(nrf_timer_event_t event_type, void* p_context)
{
  nrf_gpio_pin_clear(CAM_SPI_CS_OUT);
  lvld_timer_disable();
  // if (line_count < IMAGE_HEIGHT)
  // {
  //   lvld_timer_disable();
  //   NRF_GPIO->OUTSET = 1UL << CAM_SPI_CS_OUT;
//   spiSlaveSetBuffersBackWithLineCount(line_count);
  //   nrf_drv_gpiote_in_event_enable(CAM_LINE_VALID, true);
  // }
  // else
  // {
  //   nrf_drv_gpiote_in_event_disable(CAM_LINE_VALID);
  //   nrf_drv_gpiote_in_event_disable(CAM_FRAME_VALID);
  //   lvld_timer_disable();
  //   NRF_GPIO->OUTSET = 1UL << CAM_SPI_CS_OUT;
  //   NRF_LOG_RAW_INFO("%d lvld event done\n", systemTimeGetMs());
  //   hm_set_capture_done();
  // }
}

void lvld_timer_init(void)
{
  uint32_t err_code = NRF_SUCCESS;
  //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
  nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
  err_code = nrf_drv_timer_init(&TIMER_LVLD, &timer_cfg, timer_lvld_event_handler);
  APP_ERROR_CHECK(err_code);

  nrf_drv_timer_extended_compare(
      &TIMER_LVLD, NRF_TIMER_CC_CHANNEL4, LVLD_TIMER_VALUE, NRF_TIMER_SHORT_COMPARE4_CLEAR_MASK, true);
}

/** @} */
