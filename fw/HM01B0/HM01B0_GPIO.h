/*
 * HM01B0_GPIO.h
 *
 *  Created on: Nov 20, 2018
 *      Author: Ali Najafi
 */

#ifndef HM01B0_GPIO_H_
#define HM01B0_GPIO_H_

/*  Standard C Included Files */
#include <stdbool.h>

#include "nrf.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "boards.h"
#include "HM01B0_SPI.h"
#include "HM01B0_LVLD_TIMER.h"
#include "HM01B0_CAPTURE.h"

void gpio_setting_init(void);
void gpio_setting_uninit(void);

/*!
* @brief Call back for PINT Pin interrupt 0-7.
*/
void in_pin_handler_CAM_LINE_VALID(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void in_pin_handler_CAM_FRAME_VALID(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

#endif /* HM01B0_GPIO_H_ */