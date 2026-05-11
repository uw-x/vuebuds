/*
 * mustard
 * maruchi kim
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <nrfx.h>

#include "app_util_platform.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "sensorsim.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_clock.h"
#include "ble_manager.h"
#include "timers.h"
#include "event.h"
#include "gpio.h"
#include "accel.h"
#include "spi.h"
#include "i2c.h"
#include "flash.h"
#include "cli.h"
#include "camera.h"
#include "main.h"
#include "pmu.h"
#include "boards.h"
#include "nrf_ppi.h"
#include "nrf_timer.h"
#include "HM01B0_BLE_DEFINES.h"

APP_TIMER_DEF(imuTimer);
APP_TIMER_DEF(buttonReleaseTimer);
APP_TIMER_DEF(chargeTimer);
static uint8_t imuBuffer[12] = {0};
static bool bleRetry = false;
static uint32_t expectedBufferCount = 0;
static uint8_t frameCount = 0;
static bool streaming = false;
static bool chargingDisplayed = false;
static bool charging = false;
static bool pmuDiscovered = false;

static uint8_t metadataIndex = 0;
static uint8_t metadata[180] = { 0 };


void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static void imuTimerCallback(void * p_context)
{
  eventQueuePush(EVENT_IMU_SAMPLE_DATA);
}

static void buttonReleaseTimerCallback(void * p_context)
{
  eventQueuePush(EVENT_STOP_SENSORS);
}

static void chargeTimerCallback(void * p_context)
{
  // disable LED after 8 seconds
  gpioWrite(LED1_PIN, 1);
}

void powerEnterSleepMode(void)
{
  ret_code_t err_code;

  NRF_LOG_RAW_INFO("%08d [power] powering off...\n", systemTimeGetMs());

  delayMs(1);

  // Prepare wakeup buttons.
  nrf_gpio_cfg_sense_set(BUTTON_PIN, NRF_GPIO_PIN_SENSE_LOW);

  cameraDeInit();

  // LEDs
  gpioDisable(LED1_PIN);
  gpioDisable(LED2_PIN);

  // Turn off PMIC power rails SBB0, SBB1 and LDO
  // Do not turn of nRF rail (1.8V = SBB2)
  // MAX77650_setADE_SBB0(true);
  MAX77650_setEN_SBB0(0b100); // turn off

  MAX77650_setADE_SBB1(true); // enable active discharge for faster turn-off
  MAX77650_setEN_SBB1(0b100); // turn off

  MAX77650_setEN_LDO(0b100); // turn off

  // Set main bias to low power
  MAX77650_setSBIA_LPM(true);

  // disable I2C
  gpioDisable(I2C_SCL_PIN);
  gpioDisable(I2C_SDA_PIN);

  // Disable remaining camera control pins not handled by cameraDeInit()
  gpioDisable(CAM_POWER);
  gpioDisable(CAM_SPI_CS_OUT);
  gpioInterruptDisable(CAM_SPI_CS_IN);
  gpioDisable(CAM_SPI_CS_IN);

  // Disable timers
  timersDeInit();

  // Go to system-off mode (this function will not return; wakeup will cause a reset).
  err_code = sd_power_system_off();
  APP_ERROR_CHECK(err_code);
}

static void bsp_event_handler(bsp_event_t event)
{
  ret_code_t err_code;

  switch (event)
  {
    case BSP_EVENT_SLEEP:
      // powerEnterSleepMode();
      break;

    case BSP_EVENT_DISCONNECT:
      err_code = bleDisconnect();
      if (err_code != NRF_ERROR_INVALID_STATE) { APP_ERROR_CHECK(err_code); }
      break;

    case BSP_EVENT_KEY_0:
      break;

    case BSP_EVENT_KEY_2:
      break;

    case BSP_EVENT_KEY_3:
      break;

    default:
      break;
  }
}

static void buttons_leds_init(void)
{
  ret_code_t err_code;

  err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
  APP_ERROR_CHECK(err_code);

  // Configure power off
  bsp_event_to_button_action_assign(USER_BUTTON, BSP_BUTTON_ACTION_RELEASE, BSP_EVENT_SLEEP);
}

static void logInit(void)
{
  ret_code_t err_code = NRF_LOG_INIT(app_timer_cnt_get);
  APP_ERROR_CHECK(err_code);
}

static void powerInit(void)
{
  ret_code_t err_code;
  err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);

  sd_power_dcdc_mode_set(true);

  pmuDiscovered = pmu_init();
}

static void idle(void)
{
  if (streaming && eventQueueEmpty()) {
    nrf_pwr_mgmt_run();
  } else if (NRF_LOG_PROCESS() == false && eventQueueEmpty()) {
    nrf_pwr_mgmt_run();
  }
}

static void mustardInit(void)
{
  gpioInit();
  // logInit();
  NRF_LOG_RAW_INFO("%08d [mustard] booting...\n", systemTimeGetMs());
  timersInit();
  ret_code_t err_code;
  err_code = app_timer_create(&buttonReleaseTimer, APP_TIMER_MODE_SINGLE_SHOT, buttonReleaseTimerCallback);
  err_code = app_timer_create(&chargeTimer, APP_TIMER_MODE_SINGLE_SHOT, chargeTimerCallback);
  APP_ERROR_CHECK(err_code);
  // cliInit();

  NRF_LOG_RAW_INFO("Press the Tab key to see all available commands.\n");

  eventQueueInit();

  i2cInit();

  powerInit();

  bleInit();
  bleAdvertisingStart();

  NRF_LOG_RAW_INFO("%08d [mustard] booted\n", systemTimeGetMs());
}

static void processQueue(void)
{
  if (!eventQueueEmpty()) {
    switch(eventQueueFront()) {

      case EVENT_BLE_DATA_STREAM_START:
        NRF_LOG_RAW_INFO("%08d [ble] stream start\n", systemTimeGetMs());
        // start camera stream here?
        break;

      case EVENT_BLE_DATA_STREAM_STOP:
      {
        NVIC_SystemReset();
        break;
      }

      case EVENT_CAMERA_STREAM_START:
      {
        cameraInit();
        cameraStartStream();

        streaming = true;
        // After stream starts disable unnecessary peripherals to save power
        // cliDeInit(); // Disables UART

        // PMU power optimizations for streaming mode
        if (pmuDiscovered) {
          MAX77650_setEN_LED_MSTR(false); // Disable LED driver to save power

          // Only suspend USB if not actively charging
          if (!pmuIsCharging()) {
            MAX77650_setUSBS(true); // Place USB/CHGIN in suspend mode
          }
        }

        break;
      }

      case EVENT_CAMERA_STREAM_STOP:
      {
        break;
      }

      case EVENT_CAMERA_CAPTURE_DONE:
      {
        NRF_LOG_RAW_INFO("%08d [cam] EVENT_CAMERA_CAPTURE_DONE\n", systemTimeGetMs());
        uint8_t *camData;
        uint32_t camDataLength = cameraGetFrameBuffer(&camData);
        bleSendData(camData, camDataLength);
        break;
      }

      case EVENT_CAMERA_READY_NEXT_FRAME:
      {
        NRF_LOG_RAW_INFO("%08d [cam] EVENT_CAMERA_READY_NEXT_FRAME\n", systemTimeGetMs());
        cameraReadyNextFrame();
        break;
      }

      case EVENT_BUTTON_STATE_CHANGED:
      {
        // Start IMU reads here, need to latch onto something to get regular spaced reads
        static uint32_t lastPressedTimeMs;
        static uint8_t buttonPressedCounter = 0;
        uint32_t currentTimeMs = systemTimeGetMs();
        bool pressed = buttonPressed();

        NRF_LOG_RAW_INFO("%08d [button] pressed:%d resetCounter:%d\n", currentTimeMs, pressed, buttonPressedCounter);

        if (pressed) {
          if ((currentTimeMs - lastPressedTimeMs) > BUTTON_DEBOUNCE_MS) {
            buttonPressedCounter = 0;
          }

          lastPressedTimeMs = currentTimeMs;
          ++buttonPressedCounter;
          cameraEnableStandbyMode(false);
        } else if (buttonPressedCounter >= 5) {
          NRF_LOG_RAW_INFO("%08d [main] trigger power down\n", systemTimeGetMs());
          buttonPressedCounter = 0; // reset this for debug
          eventQueuePush(EVENT_POWER_ENTER_SLEEP_MODE);
        } else {
          // button released
          app_timer_stop(buttonReleaseTimer);
        }

        break;
      }

      case EVENT_BLE_RADIO_START:
        // Event that fires whenever the radio starts up
        break;

      case EVENT_BLE_SEND_DATA_DONE:
        // BLE just finished, attempt to fill in more data
        // send();
        break;

      case EVENT_BLE_IDLE:
        // powerEnterSleepMode();
        break;

      case EVENT_TIMERS_ONE_SECOND_ELAPSED:
      {
        if (!streaming) {
          gpioWrite(LED2_PIN, 0);
          delayMs(1);
          gpioWrite(LED2_PIN, 1);
        }
        break;
      }

      case EVENT_BLE_DISCONNECTED:
        NVIC_SystemReset();
        break;

      case EVENT_POWER_ENTER_SLEEP_MODE:
        powerEnterSleepMode();
        break;

      case EVENT_STOP_SENSORS:
        cameraEnableStandbyMode(true);
        break;

      default:
        NRF_LOG_RAW_INFO("%08d [main] unhandled event:%d\n", systemTimeGetMs(), eventQueueFront());
        break;
    }

    eventQueuePop();
  }
}

int main(void)
{
  mustardInit();

  for (;;)
  {
    idle();
    processQueue();
    if (streaming) {
      bleService();
    } else {
      cliProcess();
    }
  }
}
