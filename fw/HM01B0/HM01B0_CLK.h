/*
 * HM01B0_CLK.h
 *
 *  Created on: Nov 25, 2018
 *      Author: Ali Najafi
 */

#ifndef HM01B0_CLK_H_
#define HM01B0_CLK_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_gpiote.h"

#include "boards.h"
#include "app_error.h"

#include "HM01B0_BLE_DEFINES.h"

static nrf_drv_timer_t CAM_TIMER = NRF_DRV_TIMER_INSTANCE(3);

void hm_clk_out(void);
void hm_clk_enable(bool enable);

#endif /* HM01B0_CLK_H_ */