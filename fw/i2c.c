#include <stdio.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "gpio.h"
#include "i2c.h"

#define I2C_ADDRESS_COUNT 127

static const nrf_drv_twi_t i2cHandle = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
static volatile bool transferDone = false;
static volatile bool addrNack = false;
static volatile bool dataNack = false;
static volatile bool callbackReceived = false;

static bool verifyWrite16(uint8_t addr, uint16_t reg, uint8_t data)
{
  uint8_t rxData = i2cRead16(addr, reg);
  return (rxData == data ? true : false);
}

static bool verifyWrite8(uint8_t addr, uint8_t reg, uint8_t data)
{
  uint8_t rxData = i2cRead8(addr, reg);
  return (rxData == data ? true : false);
}

static void resetFlags(void)
{
  transferDone = false;
  addrNack = false;
  dataNack = false;
  callbackReceived = false;
}

static void waitForTransfer(void)
{
  while (callbackReceived == false)
  {
    __WFE();
  };

  if (transferDone == false) {
    if (addrNack)
    {
      NRF_LOG_INFO("[i2c] addr nack");
    }
    else if (dataNack)
    {
      NRF_LOG_INFO("[i2c] data nack");
    }
    else
    {
      NRF_LOG_INFO("[i2c] unexpected error");
    }
  }
}

void i2cHandler(nrf_drv_twi_evt_t const *p_event, void *p_context)
{
  switch (p_event->type)
  {
    case NRF_DRV_TWI_EVT_DONE:
      if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
      {
        // No op
      }
      transferDone = true;
      break;
    case NRF_DRV_TWI_EVT_ADDRESS_NACK:
      addrNack = true;
      break;
    case NRF_DRV_TWI_EVT_DATA_NACK:
      dataNack = true;
      break;
    default:
      break;
  }
  callbackReceived = true;
}

void i2cInit(void)
{
  ret_code_t err_code;

  const nrf_drv_twi_config_t i2c_config = {
      .scl = I2C_SCL_PIN,
      .sda = I2C_SDA_PIN,
      .frequency = NRF_DRV_TWI_FREQ_100K,
      .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
      .clear_bus_init = false
  };

  err_code = nrf_drv_twi_init(&i2cHandle, &i2c_config, i2cHandler, NULL);
  APP_ERROR_CHECK(err_code);

  // nrf_drv_twi_enable(&i2cHandle);
  transferDone = false;
}

void i2cEnable(bool enable) {
  if (enable) {
    nrf_drv_twi_enable(&i2cHandle);
  } else {
    nrf_drv_twi_disable(&i2cHandle);
  }
}

void i2cScan(void)
{
  i2cEnable(true);
  ret_code_t err_code;
  uint8_t sample_data;
  bool detected_device = false;

  for (uint8_t address = 1; address <= I2C_ADDRESS_COUNT; address++)
  {
    resetFlags();
    err_code = nrf_drv_twi_rx(&i2cHandle, address, &sample_data, sizeof(sample_data));
    APP_ERROR_CHECK(err_code);
    while (callbackReceived == false)
    {
      __WFE();
    };

    if (transferDone)
    {
      detected_device = true;
      NRF_LOG_INFO("TWI device detected at address 0x%x.", address);
    }
  }

  if (!detected_device)
  {
    NRF_LOG_INFO("No device was found.");
  }
  i2cEnable(false);
}

bool i2cScanForPmu(void)
{
  i2cEnable(true);
  ret_code_t err_code;
  uint8_t sample_data;
  bool detected_device = false;
  for (uint8_t address = 0x40; address <= 0x48; address++)
  {
    resetFlags();
    err_code = nrf_drv_twi_rx(&i2cHandle, address, &sample_data, sizeof(sample_data));
    APP_ERROR_CHECK(err_code);
    while (callbackReceived == false)
    {
      __WFE();
    };

    if (transferDone)
    {
      detected_device = true;
      NRF_LOG_INFO("TWI device detected at address 0x%x.", address);
    }
  }
  i2cEnable(false);

  return detected_device;
}

// 8bit registers
void i2cWrite8(uint8_t addr, uint8_t reg, uint8_t data)
{
  i2cEnable(true);
  ret_code_t err_code;
  uint8_t bytes[2] = {reg, data};
  resetFlags();
  err_code = nrf_drv_twi_tx(&i2cHandle, addr, bytes, sizeof(bytes), false);
  APP_ERROR_CHECK(err_code);
  waitForTransfer();

  if (verifyWrite8(addr, reg, data) == false)
  {
    NRF_LOG_INFO("[i2c] failed to write");
  }

  i2cEnable(false);
}

uint8_t i2cRead8(uint8_t addr, uint8_t reg)
{
  i2cEnable(true);
  ret_code_t err_code;
  uint8_t rxData;

  resetFlags();
  err_code = nrf_drv_twi_tx(&i2cHandle, addr, &reg, 1, false);
  APP_ERROR_CHECK(err_code);
  waitForTransfer();

  resetFlags();
  err_code = nrf_drv_twi_rx(&i2cHandle, addr, &rxData, sizeof(rxData));
  APP_ERROR_CHECK(err_code);
  waitForTransfer();
  i2cEnable(false);
  return rxData;
};

// 16 bit registers
void i2cWrite16(uint8_t addr, uint16_t reg, uint8_t data)
{
  i2cEnable(true);
  ret_code_t err_code;
  uint8_t bytes[3] = {0};
  bytes[0] = (reg >> 8) & 0xFF;
  bytes[1] = reg & 0xFF;
  bytes[2] = data;

  resetFlags();
  err_code = nrf_drv_twi_tx(&i2cHandle, addr, bytes, sizeof(bytes), false);
  APP_ERROR_CHECK(err_code);
  waitForTransfer();

  if (verifyWrite16(addr, reg, data) == false) {
    NRF_LOG_INFO("[i2c] failed to write addr:0x%x reg:0x%04x data:0x%04x", addr, reg, data);
  }
  i2cEnable(false);
}

uint8_t i2cRead16(uint8_t addr, uint16_t reg)
{
  i2cEnable(true);
  ret_code_t err_code;
  uint8_t rxData;
  uint8_t bytes[2] = {0};
  bytes[0] = (reg >> 8) & 0xFF;
  bytes[1] = reg & 0xFF;

  resetFlags();
  err_code = nrf_drv_twi_tx(&i2cHandle, addr, bytes, sizeof(bytes), false);
  APP_ERROR_CHECK(err_code);
  waitForTransfer();

  resetFlags();
  err_code = nrf_drv_twi_rx(&i2cHandle, addr, &rxData, sizeof(rxData));
  APP_ERROR_CHECK(err_code);
  waitForTransfer();
  i2cEnable(false);

  return rxData;
}