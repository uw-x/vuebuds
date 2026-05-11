#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <nrfx.h>

/* TWI instance ID. */
#if TWI0_ENABLED
#define TWI_INSTANCE_ID 0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID 1
#endif

#define CAMERA_I2C_ADDR 0x24

void i2cInit(void);
void i2cScan(void);
void i2cWrite16(uint8_t addr, uint16_t reg, uint8_t data);
void i2cWrite8(uint8_t addr, uint8_t reg, uint8_t data);
uint8_t i2cRead16(uint8_t addr, uint16_t reg);
uint8_t i2cRead8(uint8_t addr, uint8_t reg);
bool i2cScanForPmu(void);
void i2cEnable(bool enable);