/*
 * HM01B0_LVLD_TIMER.h
 *
 *  Created on: April 7, 2019
 *      Author: Ali
 */

#ifndef HM01B0_LVLD_TIMER_H_
#define HM01B0_LVLD_TIMER_H_

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_drv_timer.h"
#include "app_error.h"
#include "HM01B0_BLE_DEFINES.h"
#include "HM01B0_GPIO.h"
#include "HM01B0_CAPTURE.h"

void lvld_timer_enable(void);
void lvld_timer_disable(void);
void lvld_timer_init(void);

#endif /* HM01B0_LVLD_TIMER_H_ */