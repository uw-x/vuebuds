/*
 * HM01B0_CLK.h
 *
 *  Created on: Nov 25, 2018
 *      Author: Ali Najafi
 */
#include <nrfx.h>
#include "nordic_common.h"
#include "nrf.h"

#include "HM01B0_CLK.h"
#include "HM01B0_CAPTURE.h"
#include "gpio.h"
#include "nrf_drv_ppi.h"

uint32_t *timer0_prescaler_address = (uint32_t *)0x40009510; // address for timer1 prescaler
uint32_t timer0_prescaler0 = 0x00000000;

static void timer_dummy_handler(nrf_timer_event_t event_type, void *p_context) {}

void hm_clk_out(void)
{
  ret_code_t err_code;

  err_code = nrf_drv_ppi_init();
  APP_ERROR_CHECK(err_code);

  nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
  err_code = nrf_drv_timer_init(&CAM_TIMER, &timer_cfg, timer_dummy_handler);
  APP_ERROR_CHECK(err_code);

  (*timer0_prescaler_address) = timer0_prescaler0;

  uint32_t compare_evt_addr;
  uint32_t gpiote_task_addr;
  nrf_ppi_channel_t ppi_channel;
  nrf_drv_gpiote_out_config_t config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(false);

  err_code = nrf_drv_gpiote_out_init(CAM_MCLK_IN_FROM_MCU, &config);
  APP_ERROR_CHECK(err_code);

  nrf_drv_timer_extended_compare(&CAM_TIMER, (nrf_timer_cc_channel_t)0, 1UL, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);

  err_code = nrf_drv_ppi_channel_alloc(&ppi_channel);
  APP_ERROR_CHECK(err_code);

  compare_evt_addr = nrf_drv_timer_event_address_get(&CAM_TIMER, NRF_TIMER_EVENT_COMPARE0);
  gpiote_task_addr = nrf_drv_gpiote_out_task_addr_get(CAM_MCLK_IN_FROM_MCU);

  err_code = nrf_drv_ppi_channel_assign(ppi_channel, compare_evt_addr, gpiote_task_addr);
  APP_ERROR_CHECK(err_code);

  err_code = nrf_drv_ppi_channel_enable(ppi_channel);
  APP_ERROR_CHECK(err_code);

  nrf_drv_gpiote_out_task_enable(CAM_MCLK_IN_FROM_MCU);
  nrf_gpio_cfg(CAM_MCLK_IN_FROM_MCU, NRF_GPIO_PIN_DIR_OUTPUT,
               NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
               NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);

  nrf_drv_timer_enable(&CAM_TIMER);
}

void hm_clk_enable(bool enable)
{
  if (enable) {
    nrf_drv_timer_enable(&CAM_TIMER);
  } else {
    nrf_drv_timer_disable(&CAM_TIMER);
  }
}