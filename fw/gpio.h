// bsp
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

#define FF 1
#if FF
  #define BUTTON_PIN                      NRF_GPIO_PIN_MAP(0, 15)
  #define CAM_PCLK_OUT_TO_MCU             NRF_GPIO_PIN_MAP(0, 19) // CAM_PCLK (SPI CLK)
  #define CAM_D0                          NRF_GPIO_PIN_MAP(0, 20)              // CAM_D0
#else
  #define BUTTON_PIN                      NRF_GPIO_PIN_MAP(0, 24) // button 3 on dev board
  #define CAM_PCLK_OUT_TO_MCU             NRF_GPIO_PIN_MAP(1, 2) // CAM_PCLK (SPI CLK)
  #define CAM_D0                          NRF_GPIO_PIN_MAP(1, 3)              // CAM_D0
#endif

// SPI (IMU)
#define SPI_SCK_PIN                       NRF_GPIO_PIN_MAP(0, 5)
#define SPI_MOSI_PIN                      NRF_GPIO_PIN_MAP(0, 17)
#define SPI_MISO_PIN                      NRF_GPIO_PIN_MAP(0, 13)
#define SPI_CS_PIN                        NRF_GPIO_PIN_MAP(0, 3)

// IMU
#define IMU_INT1_PIN                      NRF_GPIO_PIN_MAP(0, 2)
#define IMU_INT2_PIN                      NRF_GPIO_PIN_MAP(0, 31)
#define ACCEL_EN_PIN                      NRF_GPIO_PIN_MAP(0, 30)
#define ACCEL_INT1_PIN                    NRF_GPIO_PIN_MAP(1, 9) // dummy value for compilation

// LED
#define LED1_PIN                          NRF_GPIO_PIN_MAP(0, 7)                // 0 to turn on, 1 to turn off
#define LED2_PIN                          NRF_GPIO_PIN_MAP(0, 4)                // 0 to turn on, 1 to turn off

// DEBUG_UART
#define UART_TX_PIN                       NRF_GPIO_PIN_MAP(0, 6)
#define UART_RX_PIN                       NRF_GPIO_PIN_MAP(0, 8)

// I2C
#define I2C_SCL_PIN                       NRF_GPIO_PIN_MAP(0, 27)
#define I2C_SDA_PIN                       NRF_GPIO_PIN_MAP(0, 26)

// CAMERA (TODO: Revisit these, getting it to compile for now)
#define CAM_POWER                         NRF_GPIO_PIN_MAP(0, 18)  // Unused
#define CAM_SPI_CS_OUT                    NRF_GPIO_PIN_MAP(0, 12)  // Goes to CAM_SPI_CS_IN
#define CAM_SPI_CS_IN                     NRF_GPIO_PIN_MAP(0, 14)  // Listens on CAM_SPI_CS_IN
#define CAM_FRAME_VALID                   NRF_GPIO_PIN_MAP(0, 29)  // Input
#define CAM_LINE_VALID                    NRF_GPIO_PIN_MAP(0, 11)  // Input
#define CAM_INT                           NRF_GPIO_PIN_MAP(0, 9)   // Input
#define CAM_MCLK_IN_FROM_MCU              NRF_GPIO_PIN_MAP(1, 8)

// #define CAM_MCLK_IN_FROM_MCU              NRF_GPIO_PIN_MAP(0, 15)

#define CAM_D1                            NRF_GPIO_PIN_MAP(0, 21)
#define CAM_D2                            NRF_GPIO_PIN_MAP(0, 22)
#define CAM_D3                            NRF_GPIO_PIN_MAP(0, 23)

// Wrapper
#define GPIO_INTERRUPT_CONFIG_RISING  GPIOTE_CONFIG_IN_SENSE_LOTOHI(true)
#define GPIO_INTERRUPT_CONFIG_FALLING GPIOTE_CONFIG_IN_SENSE_HITOLO(true)
#define GPIO_INTERRUPT_CONFIG_TOGGLE  GPIOTE_CONFIG_IN_SENSE_TOGGLE(true)

#define gpioPin_t                            nrfx_gpiote_pin_t
#define gpioOutput_t                         nrf_drv_gpiote_out_config_t

#define gpioInput_t                          nrf_drv_gpiote_in_config_t
#define gpioInputEnable(pin, config, handler)  nrf_drv_gpiote_in_init(pin, config, handler)
#define gpioInterruptEnable(pin)             nrf_drv_gpiote_in_event_enable(pin, true)
#define gpioInterruptDisable(pin)            nrf_drv_gpiote_in_event_disable(pin)
#define gpioRead(pin)                        nrf_gpio_pin_read(pin)

void gpioInit(void);
void gpioOutputEnable(gpioPin_t pin);
void gpioDisable(gpioPin_t pin);
void gpioWrite(gpioPin_t pin, uint8_t value);
bool buttonPressed(void);
