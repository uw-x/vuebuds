#include "i2c.h"
#include "gpio.h"
#include "camera.h"
#include "HM01B0_SPI.h"
#include "HM01B0_CLK.h"
#include "spi.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_cli.h"
#include "nrf_cli_types.h"
#include "nrf_cli_libuarte.h"
#include "timers.h"
#include "spi.h"
#include "pmu.h"

#define CLI_EXAMPLE_LOG_QUEUE_SIZE (6)

NRF_CLI_LIBUARTE_DEF(m_cli_libuarte_transport, 512, 512);
NRF_CLI_DEF(m_cli_libuarte,
            "mustard] ",
            &m_cli_libuarte_transport.transport,
            '\r',
            CLI_EXAMPLE_LOG_QUEUE_SIZE);


// I2C COMMANDS
static void cmd_i2c_scan(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  i2cScan();
}

static void cmd_i2c(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  ASSERT(p_cli);
  ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);

  if ((argc == 1) || nrf_cli_help_requested(p_cli))
  {
    nrf_cli_help_print(p_cli, NULL, 0);
    return;
  }

  if (argc != 2)
  {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\n", argv[0]);
    return;
  }

  nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\n", argv[0], argv[1]);
}

static uint16_t pixel = 0;

static void cmd_cam_capture(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  cameraInit();
  cameraStartStream();
  pixel = 0;
}

static void cmd_cam_print(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  uint8_t* camData;
  uint16_t camDataLength = 0;

  camDataLength = spiSlaveGetRxBuffer(&camData);

  bool dataExists = false;
  uint16_t dataExistsAtIndex = 0;
  for (uint16_t i = 0; i < camDataLength; i++) {
    if (camData[i] != 0) {
      dataExists = true;
    }
  }

  if (dataExists) {
    for (int i = pixel; i < pixel + 100; i++) {
      NRF_LOG_RAW_INFO("%d ", camData[i]);
    }
  } else {
    NRF_LOG_RAW_INFO("No data");
  }

  pixel += 100;

  NRF_LOG_RAW_INFO("\n");
}

static void cmd_cam_clk(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  hm_clk_out();
}

// CAMERA COMMANDS
static void cmd_cam(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  ASSERT(p_cli);
  ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);

  if ((argc == 1) || nrf_cli_help_requested(p_cli))
  {
    nrf_cli_help_print(p_cli, NULL, 0);
    return;
  }

  if (argc != 2)
  {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\n", argv[0]);
    return;
  }

  nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\n", argv[0], argv[1]);
}

static void cmd_spis_receive(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  static bool initialized = false;
  if (initialized == false) {
    spiSlaveInit();
    initialized = true;
  }

  spiSlaveSetBuffers();
}

static void cmd_spis(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  ASSERT(p_cli);
  ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);

  if ((argc == 1) || nrf_cli_help_requested(p_cli))
  {
    nrf_cli_help_print(p_cli, NULL, 0);
    return;
  }

  if (argc != 2)
  {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\n", argv[0]);
    return;
  }

  nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\n", argv[0], argv[1]);
}

////////////////////////////////////////////////////////////////////////////////
// PMU COMMANDS

static void cmd_pmu_init(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  pmu_init();
}

static void cmd_pmu_read_reg(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  uint8_t regAddr = (uint8_t)strtol(argv[1], NULL, 16);
  uint8_t regVal = MAX77650_read_register(regAddr);
  NRF_LOG_RAW_INFO("reg 0x%x = 0x%x\n", regAddr, regVal);
}

static void cmd_pmu_write_reg(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  uint8_t regAddr = (uint8_t)strtol(argv[1], NULL, 16);
  uint8_t regVal = (uint8_t)strtol(argv[2], NULL, 16);
  NRF_LOG_RAW_INFO("reg 0x%x = 0x%x\n", regAddr, regVal);
  MAX77650_write_register(regAddr, regVal);

}

static void cmd_pmu_setVoltages(nrf_cli_t const *p_cli, size_t argc, char **argv)
{

  MAX77650_setIP_SBB0(0b11);  //Limit output of SBB0 to 500mA
  MAX77650_setTV_SBB0(0b101000); //Set output Voltage of SBB0 to 1.8V
  MAX77650_setADE_SBB0(0b0); //Disable Active Discharge at SBB0 Output
  MAX77650_setEN_SBB0(0b110); //Enable SBB0 is on irrespective of FPS whenever the on/off controller is in its "On via Software" or "On via On/Off Controller" states

  MAX77650_setIP_SBB1(0b11);  //Limit output of SBB1 to 500mA
  MAX77650_setTV_SBB1(0b100000); //Set output Voltage of SBB1 to 1.2V
  MAX77650_setADE_SBB1(0b0); //Disable Active Discharge at SBB1 Output
  MAX77650_setEN_SBB1(0b110); //Enable SBB1 is on irrespective of FPS whenever the on/off controller is in its "On via Software" or "On via On/Off Controller" states

  MAX77650_setIP_SBB2(0b11);  //Limit output of SBB2 to 500mA
  MAX77650_setTV_SBB2(0b110010); //Set output Voltage of SBB2 to 3.3V
  MAX77650_setADE_SBB2(0b0); //Disable Active Discharge at SBB2 Output
  MAX77650_setEN_SBB2(0b110); //Enable SBB2 is on irrespective of FPS whenever the on/off controller is in its "On via Software" or "On via On/Off Controller" states

  MAX77650_setTV_LDO(0x24); //Set output Voltage of LDO to 1.8V
  MAX77650_setEN_LDO(true);

  NRF_LOG_RAW_INFO("SIMO Buck-Boost Voltage Channel 0: ");
  NRF_LOG_RAW_INFO(NRF_LOG_FLOAT_MARKER " V\n", NRF_LOG_FLOAT(MAX77650_getTV_SBB0() * 0.025 + 0.8));

  NRF_LOG_RAW_INFO("SIMO Buck-Boost Voltage Channel 2: ");
  if (!(MAX77650_getDIDM())){
    NRF_LOG_RAW_INFO(NRF_LOG_FLOAT_MARKER " V\n", NRF_LOG_FLOAT(MAX77650_getTV_SBB2() * 0.05 + 0.8));
  }
  else{
    NRF_LOG_RAW_INFO(NRF_LOG_FLOAT_MARKER " V\n", NRF_LOG_FLOAT(MAX77650_getTV_SBB2() * 0.05 + 2.4));
  }


}


static void cmd_pmu(nrf_cli_t const *p_cli, size_t argc, char **argv)
{
  ASSERT(p_cli);
  ASSERT(p_cli->p_ctx && p_cli->p_iface && p_cli->p_name);

  if ((argc == 1) || nrf_cli_help_requested(p_cli))
  {
    nrf_cli_help_print(p_cli, NULL, 0);
    return;
  }

  if (argc != 2)
  {
    nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: bad parameter count\n", argv[0]);
    return;
  }

  nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "%s: unknown parameter: %s\n", argv[0], argv[1]);
}
////////////////////////////////////////////////////////////////////////////////
void cliInit(void)
{
  ret_code_t ret;

  cli_libuarte_config_t libuarte_config;
  libuarte_config.tx_pin = UART_TX_PIN;
  libuarte_config.rx_pin = UART_RX_PIN;
  libuarte_config.baudrate = NRF_UARTE_BAUDRATE_115200;
  libuarte_config.parity = NRF_UARTE_PARITY_EXCLUDED;
  libuarte_config.hwfc = NRF_UARTE_HWFC_DISABLED;
  ret = nrf_cli_init(&m_cli_libuarte, &libuarte_config, true, true, NRF_LOG_SEVERITY_INFO);

  APP_ERROR_CHECK(ret);
  ret = nrf_cli_start(&m_cli_libuarte);
  APP_ERROR_CHECK(ret);
}

void cliProcess(void)
{
  nrf_cli_process(&m_cli_libuarte);
}

void cliDeInit(void)
{
  APP_ERROR_CHECK(nrf_cli_uninit(&m_cli_libuarte));
  gpioDisable(UART_TX_PIN);
  gpioDisable(UART_RX_PIN);
}

NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_spis){
    NRF_CLI_CMD(receive, NULL, "spiBus, data, length", cmd_spis_receive),
    NRF_CLI_SUBCMD_SET_END};
NRF_CLI_CMD_REGISTER(spis, &m_sub_spis, "spis", cmd_spis);

NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_i2c){
    NRF_CLI_CMD(scan, NULL, "Print all entered parameters.", cmd_i2c_scan),
    NRF_CLI_SUBCMD_SET_END};
NRF_CLI_CMD_REGISTER(i2c, &m_sub_i2c, "i2c", cmd_i2c);

NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_cam){
    NRF_CLI_CMD(clk, NULL, "Print all entered parameters.", cmd_cam_clk),
    NRF_CLI_CMD(capture, NULL, "Print all entered parameters.", cmd_cam_capture),
    NRF_CLI_CMD(print, NULL, "Print all entered parameters.", cmd_cam_print),
    NRF_CLI_SUBCMD_SET_END};
NRF_CLI_CMD_REGISTER(cam, &m_sub_cam, "camera", cmd_cam);


NRF_CLI_CREATE_STATIC_SUBCMD_SET(m_sub_pmu){
    NRF_CLI_CMD(init, NULL, "Print all entered parameters.", cmd_pmu_init),
    NRF_CLI_CMD(read, NULL, "read register (hex) from pmu", cmd_pmu_read_reg),
    NRF_CLI_CMD(write, NULL, "write register (hex) from pmu", cmd_pmu_write_reg),
    NRF_CLI_CMD(setVoltage, NULL, "set Voltage", cmd_pmu_setVoltages),
    NRF_CLI_SUBCMD_SET_END};

NRF_CLI_CMD_REGISTER(pmu, &m_sub_pmu, "pmu", cmd_pmu);

