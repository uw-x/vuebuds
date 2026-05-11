/*
 * HM01B0_GPIO.c
 *
 *  Created on: Nov 20, 2018
 *      Author: Ali Najafi
 */
#include "HM01B0_GPIO.h"
#include "gpio.h"
#include "ble_manager.h"
#include "timers.h"

static uint8_t line_counter = 0; // only used for high res
void in_pin_handler_CAM_LINE_VALID(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  if (action == NRF_GPIOTE_POLARITY_HITOLO)
  { // End of line
    // NRF_LOG_RAW_INFO("in_pin_handler_CAM_LINE_VALID line_counter:%d\n", line_counter)
    if (line_counter < 119)
    {
      line_counter++;
    }

    if (line_counter == 119)
    {
      nrf_drv_gpiote_in_event_disable(CAM_LINE_VALID);
      // End of row 118 (0-indexed), start second transaction
      NRF_GPIO->OUTSET = 1UL << CAM_SPI_CS_OUT; // CS high
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();
      __NOP();


      NRF_GPIO->OUTCLR = 1UL << CAM_SPI_CS_OUT; // CS low
    }
  }
}

void in_pin_handler_CAM_FRAME_VALID(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  static bool start = true;

  if (gpioRead(CAM_FRAME_VALID) && start) {
    lvld_timer_enable();
    start = false;
#ifdef QQVGA
    nrf_drv_gpiote_in_event_disable(CAM_LINE_VALID);
#else
    line_counter = 0;
    nrf_drv_gpiote_in_event_enable(CAM_LINE_VALID, true);
#endif

    NRF_LOG_RAW_INFO("%08d image start\n", systemTimeGetMs());
  } else if (gpioRead(CAM_FRAME_VALID) == 0 && !start) {
    NRF_GPIO->OUTSET = 1UL << CAM_SPI_CS_OUT;
    nrf_drv_gpiote_in_event_disable(CAM_FRAME_VALID);
    start = true;
    NRF_LOG_RAW_INFO("%08d image end\n", systemTimeGetMs());
  }
}

void gpio_setting_uninit(void)
{
  nrf_drv_gpiote_in_uninit(CAM_FRAME_VALID);
  nrf_drv_gpiote_in_uninit(CAM_LINE_VALID);
}

void gpio_setting_init(void)
{
  ret_code_t err_code;

  /*toggling*/
  nrf_drv_gpiote_in_config_t in_config_frmvld = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
  in_config_frmvld.pull = NRF_GPIO_PIN_NOPULL;

  err_code = nrf_drv_gpiote_in_init(CAM_FRAME_VALID, &in_config_frmvld, in_pin_handler_CAM_FRAME_VALID);
  APP_ERROR_CHECK(err_code);

  /*Finds falling for line valid edge instead of just toggling*/
  nrf_drv_gpiote_in_config_t in_config_lnvld = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
  in_config_lnvld.pull = NRF_GPIO_PIN_NOPULL;

  err_code = nrf_drv_gpiote_in_init(CAM_LINE_VALID, &in_config_lnvld, in_pin_handler_CAM_LINE_VALID);
  APP_ERROR_CHECK(err_code);
}
