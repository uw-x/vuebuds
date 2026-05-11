#include <stdio.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "gpio.h"
#include "pmu.h"
#include "i2c.h"
#include "timers.h"

#define MAX77650_debug false

// Target Voltages
#define TV_SBB0 0x3F // 2.8V CNFG_SBB0_A (this only goes up to 2.35)
#define TV_SBB1 0x38 // 1.5V = 0x38  (can only go down to 2.4V) CNFG_SBB1_A
#define TV_SBB2 0x14 // 1.8V CNFG_SBB2_A
#define TV_LDO  0x50 // 2.35V

// Charging Configuration
#define CHG_CV    0x18  // 4.2 V fast-charge constant voltage.(CNFG_CHG_G: 0x1E)
#define CHG_CC    0x01  // 15 mA fast-charge constant current.(CNFG_CHG_E: 0x1C)
#define CHGIN_LIM 0b100 // 475 mA charge input current limit.(CNFG_CHG_B: 0x19)
#define VSYS_REG  0x08  // 4.3 V System Regulation Voltage  (CNFG_CHG_D: 0x1B)

uint8_t MAX77650_read_register(uint8_t ADDR){
    return i2cRead8(MAX77651_I2C_ADDRESS, ADDR);
}

void MAX77650_write_register(uint8_t ADDR, uint8_t data){
    i2cWrite8(MAX77651_I2C_ADDRESS, ADDR, data);
}

bool MAX77650_getDIDM(void){
  return ((MAX77650_read_register(MAX77650_STAT_GLBL_ADDR) >> 6) & 0b00000011);
}

bool MAX77650_getLDO_DropoutDetector(void){ //Returns wheather the LDO is in Dropout or not; Return Value: 0=LDO is not in dropout; 1=LDO is in dropout
  uint8_t retval;
  retval = ((MAX77650_read_register(MAX77650_STAT_GLBL_ADDR) >> 5) & 0b00000001);
  return retval;
}

bool MAX77650_getThermalAlarm1(void){ //Returns Thermal Alarm (TJA1 limit); Return Value: 0=Tj<TJA1; 1=Tj>TJA1
  return ((MAX77650_read_register(MAX77650_STAT_GLBL_ADDR) >> 3) & 0b00000001);
}

bool MAX77650_getThermalAlarm2(void){ //Returns Therma2 Alarm (TJA2 limit); Return Value: 0=Tj<TJA2; 1=Tj>TJA2
  return ((MAX77650_read_register(MAX77650_STAT_GLBL_ADDR) >> 4) & 0b00000001);
}

bool MAX77650_getDebounceStatusnEN0(void){ //Returns Debounced Status of nEN input; Return Value: 0=nEN0 is not active; 1=nEN0 is active
  return ((MAX77650_read_register(MAX77650_STAT_GLBL_ADDR) >> 2) & 0b00000001);
}

bool MAX77650_getDebounceStatusPWR_HLD(void){ //Returns Debounced Status of PWR_HLD input; Return Value: 0=logic low; 1=logic high
  return ((MAX77650_read_register(MAX77650_STAT_GLBL_ADDR) >> 1) & 0b00000001);
}

uint8_t MAX77650_getERCFLAG(void){ //Returns the ERCFLAG Register 0x04
 return MAX77650_read_register(MAX77650_ERCFLAG_ADDR);
}

uint8_t MAX77650_getChipID(void){ // Return Value regaring OTP parts of the MAX77650
  return (MAX77650_read_register(MAX77650_CID_ADDR) & 0b00001111);
}

bool MAX77650_VCHGIN_MIN_STAT(void){ //Returns Minimum Input Voltage Regulation Loop Status; Return Value: 0=no event; 1=The minimum CHGIN voltage regulation loop has engaged to regulate VCHGIN >= VCHGIN-MIN
  return ((MAX77650_read_register(MAX77650_STAT_CHG_A_ADDR) >> 6) & 0b00000001);
}

bool MAX77650_getICHGIN_LIM_STAT(void){ //Returns Input Current Limit Loop Status; Return Value: 0=no event; 1=The CHGIN current limit loop has engaged to regulate ICHGIN <= ICHGIN-LIM
  return ((MAX77650_read_register(MAX77650_STAT_CHG_A_ADDR) >> 5) & 0b00000001);
}

bool MAX77650_getVSYS_MIN_STAT(void){ //Returns Minimum System Voltage Regulation Loop Status; Return Value: 0=no event; 1=The minimum system voltage regulation loop is engaged to regulate VSYS >= VSYS-MIN
  return ((MAX77650_read_register(MAX77650_STAT_CHG_A_ADDR) >> 4) & 0b00000001);
}

bool MAX77650_getTJ_REG_STAT(void){ //Returns Maximum Junction Temperature Regulation Loop Status; Return Value: 0=no event; 1=The maximum junction temperature regulation loop has engaged to regulate the junction temperature less than TJ-REG
  return ((MAX77650_read_register(MAX77650_STAT_CHG_A_ADDR) >> 3) & 0b00000001);
}

uint8_t MAX77650_getTHM_DTLS(void){ //Returns Battery Temperature Details; Return Value: Battery Temperature Details
  return (MAX77650_read_register(MAX77650_STAT_CHG_A_ADDR) & 0b00000111);
}

bool pmuIsCharging(void)
{
  uint8_t val = ((MAX77650_read_register(MAX77650_STAT_CHG_B_ADDR) >> 2) & 0b11);
  return (val == 0x3) ? true : false;
}

uint8_t MAX77650_getCHG_DTLS(void){ //Returns Charger Details
  return ((MAX77650_read_register(MAX77650_STAT_CHG_B_ADDR) >> 4) & 0b00001111);
}

uint8_t MAX77650_getCHGIN_DTLS(void){ //Returns CHGIN Status Details
  return ((MAX77650_read_register(MAX77650_STAT_CHG_B_ADDR) >> 2) & 0b00000011);
}

bool MAX77650_getCHG(void){ //Returns Quick Charger Status
  return ((MAX77650_read_register(MAX77650_STAT_CHG_B_ADDR) >> 1) & 0b00000001);
}

bool MAX77650_getTIME_SUS(void){ //Returns Time Suspend Indicator
  return (MAX77650_read_register(MAX77650_STAT_CHG_B_ADDR) & 0b00000001);
}



uint8_t MAX77650_getLED_FS0(void){ //Returns LED0 Full Scale Range
  return ((MAX77650_read_register(MAX77650_CNFG_LED0_A_ADDR) >> 6) & 0b00000011);
}

bool MAX77650_getINV_LED0(void){ //Returns LED0 Invert
  return ((MAX77650_read_register(MAX77650_CNFG_LED0_A_ADDR) >> 5) & 0b00000001);
}

uint8_t MAX77650_getBRT_LED0(void){ //Returns LED0 Brightness Control
  return (MAX77650_read_register(MAX77650_CNFG_LED0_A_ADDR) & 0b00011111);
}

uint8_t MAX77650_getP_LED0(void){ //Returns LED0 Period Settings
  return ((MAX77650_read_register(MAX77650_CNFG_LED0_B_ADDR) >> 4) & 0b00001111);
}

uint8_t MAX77650_getD_LED0(void){ //Returns LED0 On Duty-Cycle Settings
  return (MAX77650_read_register(MAX77650_CNFG_LED0_B_ADDR) & 0b00001111);
}



uint8_t MAX77650_getLED_FS1(void){ //Returns LED1 Full Scale Range
  return ((MAX77650_read_register(MAX77650_CNFG_LED1_A_ADDR) >> 6) & 0b00000011);
}

bool MAX77650_getINV_LED1(void){ //Returns LED1 Invert
  return ((MAX77650_read_register(MAX77650_CNFG_LED1_A_ADDR) >> 5) & 0b00000001);
}

uint8_t MAX77650_getBRT_LED1(void){ //Returns LED1 Brightness Control
  return (MAX77650_read_register(MAX77650_CNFG_LED1_A_ADDR) & 0b00011111);
}

uint8_t MAX77650_getP_LED1(void){ //Returns LED1 Period Settings
  return ((MAX77650_read_register(MAX77650_CNFG_LED1_B_ADDR) >> 4) & 0b00001111);
}

uint8_t MAX77650_getD_LED1(void){ //Returns LED1 On Duty-Cycle Settings
  return (MAX77650_read_register(MAX77650_CNFG_LED1_B_ADDR) & 0b00001111);
}

uint8_t MAX77650_getLED_FS2(void){ //Returns LED0 Full Scale Range
  return ((MAX77650_read_register(MAX77650_CNFG_LED2_A_ADDR) >> 6) & 0b00000011);
}

bool MAX77650_getINV_LED2(void){ //Returns LED2 Invert
  return ((MAX77650_read_register(MAX77650_CNFG_LED2_A_ADDR) >> 5) & 0b00000001);
}

uint8_t MAX77650_getBRT_LED2(void){ //Returns LED2 Brightness Control
  return (MAX77650_read_register(MAX77650_CNFG_LED2_A_ADDR) & 0b00011111);
}

uint8_t MAX77650_getP_LED2(void){ //Returns LED2 Period Settings
  return ((MAX77650_read_register(MAX77650_CNFG_LED2_B_ADDR) >> 4) & 0b00001111);
}

uint8_t MAX77650_getD_LED2(void){ //Returns LED2 On Duty-Cycle Settings
  return (MAX77650_read_register(MAX77650_CNFG_LED2_B_ADDR) & 0b00001111);
}

bool MAX77650_getBOK(void){ //Returns Main Bias Okay Status Bit
  return ((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) >> 6) & 0b00000001);
}

bool MAX77650_getSBIA_LPM(void){ //Returns Main Bias Low-Power Mode software request
  return ((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) >> 5) & 0b00000001);
}

bool MAX77650_getSBIA_EN(void){ //Returns Main Bias Enable Software Request
  return ((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) >> 4) & 0b00000001);
}

bool MAX77650_getnEN_MODE(void){ //Returns nEN Input (ONKEY) Default Configuration Mode
  return ((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) >> 3) & 0b00000001);
}

bool MAX77650_getDBEN_nEN(void){ //Returns Debounce Timer Enable for the nEN Pin
  return ((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) >> 2) & 0b00000001);
}

uint8_t MAX77650_getSFT_RST(void){ //Returns Software Reset Functions.
  return (MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) & 0b00000011);
}


uint8_t MAX77650_getINT_GLBL(void){ //Returns Interrupt Status Register 0x00
  return MAX77650_read_register(MAX77650_INT_GLBL_ADDR);
}

uint8_t MAX77650_getINT_M_GLBL(void){ //Returns Global Interrupt Mask Register
   return (MAX77650_read_register(MAX77650_INTM_GLBL_ADDR) & 0b01111111);
}

bool MAX77650_getDBEN_GPI(void){ //Returns General Purpose Input Debounce Timer Enable
   return ((MAX77650_read_register(MAX77650_CNFG_GPIO_ADDR) >> 4) & 0b00000001);
}
bool MAX77650_getDO(void){ //Returns General Purpose Output Data Output
   return ((MAX77650_read_register(MAX77650_CNFG_GPIO_ADDR) >> 3) & 0b00000001);
}
bool MAX77650_getDRV(void){ //Returns General Purpose Output Driver Type
   return ((MAX77650_read_register(MAX77650_CNFG_GPIO_ADDR) >> 2) & 0b00000001);
}
bool MAX77650_getDI(void){ //Returns GPIO Digital Input Value
   return ((MAX77650_read_register(MAX77650_CNFG_GPIO_ADDR) >> 1) & 0b00000001);
}
bool MAX77650_getDIR(void){ //Returns GPIO Direction
   return (MAX77650_read_register(MAX77650_CNFG_GPIO_ADDR) & 0b00000001);
}

//Charger Interrupt Status Register 0x01
uint8_t MAX77650_getINT_CHG(void){ //Returns Charger Interrupt Status Register 0x01
 return MAX77650_read_register(MAX77650_INT_CHG_ADDR);
}

//Charger Interrupt Mask Register 0x07
uint8_t MAX77650_getINT_M_CHG(void){ //Returns Global Interrupt Mask Register
   return (MAX77650_read_register(MAX77650_INT_M_CHG_ADDR) & 0b01111111);
}

//Charger Configuration Register A 0x18
uint8_t MAX77650_getTHM_HOT(void){ //Returns the VHOT JEITA Temperature Threshold
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_A_ADDR) >> 6) & 0b00000011);
}
uint8_t MAX77650_getTHM_WARM(void){ //Returns the VWARM JEITA Temperature Threshold
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_A_ADDR) >> 4) & 0b00000011);
}
uint8_t MAX77650_getTHM_COOL(void){ //Returns the VCOOL JEITA Temperature Threshold
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_A_ADDR) >> 2) & 0b00000011);
}
uint8_t MAX77650_getTHM_COLD(void){ //Returns the VCOLD JEITA Temperature Threshold
  return (MAX77650_read_register(MAX77650_CNFG_CHG_A_ADDR) & 0b00000011);
}

//Charger Configuration Register B 0x19
uint8_t MAX77650_getVCHGIN_MIN(void){ //Returns the Minimum CHGIN regulation voltage (VCHGIN-MIN)
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_B_ADDR) >> 5) & 0b00000111);
}
uint8_t MAX77650_getICHGIN_LIM(void){ //Returns the CHGIN Input Current Limit (ICHGIN-LIM)
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_B_ADDR) >> 2) & 0b00000111);
}
bool MAX77650_getI_PQ(void){ //Returns the prequalification charge current (IPQ) as a percentage of IFAST-CHG
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_B_ADDR) >> 1) & 0b00000001);
}
bool MAX77650_getCHG_EN(void){ //Returns Charger Enable
  return (MAX77650_read_register(MAX77650_CNFG_CHG_B_ADDR) & 0b00000001);
}

//Charger Configuration Register C 0x1A
uint8_t MAX77650_getCHG_PQ(void){ //Returns the Battery prequalification voltage threshold (VPQ)
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_C_ADDR) >> 5) & 0b00000111);
}
uint8_t MAX77650_getI_TERM(void){ //Returns the Charger Termination Current (ITERM). I_TERM[1:0] sets the charger termination current as a percentage of the fast charge current IFAST-CHG.
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_C_ADDR) >> 3) & 0b00000011);
}
uint8_t MAX77650_getT_TOPOFF(void){ //Returns the Topoff timer value (tTO)
  return (MAX77650_read_register(MAX77650_CNFG_CHG_C_ADDR) & 0b00000111);
}

//Charger Configuration Register D 0x1B
uint8_t MAX77650_getTJ_REG(void){ //Returns the die junction temperature regulation point, TJ-REG
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_D_ADDR) >> 5) & 0b00000111);
}
uint8_t MAX77650_getVSYS_REG(void){ //Returns the System voltage regulation (VSYS-REG)
  return (MAX77650_read_register(MAX77650_CNFG_CHG_D_ADDR) & 0b00011111);
}

//Charger Configuration Register E 0x1C
uint8_t MAX77650_getCHG_CC(void){ //Returns the fast-charge constant current value, IFAST-CHG.
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_E_ADDR) >> 2) & 0b00111111);
}
uint8_t MAX77650_getT_FAST_CHG(void){ //Returns the fast-charge safety timer, tFC.
  return (MAX77650_read_register(MAX77650_CNFG_CHG_E_ADDR) & 0b00000011);
}

//Charger Configuration Register F 0x1D
uint8_t MAX77650_getCHG_CC_JEITA(void){ //Returns the IFAST-CHG_JEITA for when the battery is either cool or warm as defined by the TCOOL and TWARM temperature thresholds. This register is a don't care if the battery temperature is normal.
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_F_ADDR) >> 2) & 0b00111111);
}
uint8_t MAX77650_getTHM_EN(void){ //Returns the Thermistor enable bit
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_F_ADDR) >> 1) & 0b00000001);
}

//Charger Configuration Register G 0x1E
uint8_t MAX77650_getCHG_CV(void){ //Returns the fast-charge battery regulation voltage, VFAST-CHG.
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_G_ADDR) >> 2) & 0b00111111);
}
bool MAX77650_getUSBS(void){ //this bit places CHGIN in USB suspend mode
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_G_ADDR) >> 1) & 0b00000001);
}

//Charger Configuration Register H 0x1F
uint8_t MAX77650_getCHG_CV_JEITA(void){ //Returns the modified VFAST-CHG for when the battery is either cool or warm as defined by the TCOOL and TWARM temperature thresholds. This register is a don't care if the battery temperature is normal.
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_H_ADDR) >> 2) & 0b00111111);
}

//Charger Configuration Register I 0x20
uint8_t MAX77650_getIMON_DISCHG_SCALE(void){ //Returns the Selects the battery discharge current full-scale current value This 4-bit configuration starts at 7.5mA and ends at 300mA.
  return ((MAX77650_read_register(MAX77650_CNFG_CHG_I_ADDR) >> 4) & 0b00001111);
}
uint8_t MAX77650_getMUX_SEL(void){ //Returns the analog channel to connect to AMUX configuration
  return (MAX77650_read_register(MAX77650_CNFG_CHG_I_ADDR) & 0b00001111);
}

//LDO Configuration Register A 0x38
uint8_t MAX77650_getTV_LDO(void){ //Returns the LDO Target Output Voltage
  return (MAX77650_read_register(MAX77650_CNFG_LDO_A_ADDR) & 0b01111111);
}

//LDO Configuration Register B 0x39
bool MAX77650_getADE_LDO(void){ //Returns LDO active-Discharge Enable
  return ((MAX77650_read_register(MAX77650_CNFG_LDO_B_ADDR) >> 3) & 0b00000001);
}
uint8_t MAX77650_getEN_LDO(void){ //Enable Control for LDO.
  return (MAX77650_read_register(MAX77650_CNFG_LDO_B_ADDR) & 0b00000111);
}

//SIMO Buck-Boost Global Configuration Register 0x28
bool MAX77650_getMRT_OTP(void){ //Returns Manual Reset Time Configuration
  return ((MAX77650_read_register(MAX77650_CNFG_SBB_TOP_ADDR) >> 6) & 0b00000001);
}
bool MAX77650_getSBIA_LPM_DEF(void){ //Returns Default voltage of the SBIA_LPM bit
  return ((MAX77650_read_register(MAX77650_CNFG_SBB_TOP_ADDR) >> 5) & 0b00000001);
}
bool MAX77650_getDBNC_nEN_DEF(void){ //Returns Default Value of the DBNC_nEN bit
  return ((MAX77650_read_register(MAX77650_CNFG_SBB_TOP_ADDR) >> 4) & 0b00000001);
}
uint8_t MAX77650_getDRV_SBB(void){ //Returns the SIMO Buck-Boost (all channels) Drive Strength Trim.
  return (MAX77650_read_register(MAX77650_CNFG_SBB_TOP_ADDR) & 0b00000011);
}

//SIMO Buck-Boost 0 Configuration Register A 0x29
uint8_t MAX77650_getIP_SBB0(void){ //Returns the SIMO Buck-Boost Channel 1 Peak Current Limit
  return ((MAX77650_read_register(MAX77650_CNFG_SBB0_A_ADDR) >> 6) & 0b00000011);
}
uint8_t MAX77650_getTV_SBB0(void){ //Returns the SIMO Buck-Boost Channel 0 Target Output Voltage
  return (MAX77650_read_register(MAX77650_CNFG_SBB0_A_ADDR) & 0b00111111);
}

//SIMO Buck-Boost 0 Configuration Register B 0x2A
bool MAX77650_getADE_SBB0(void){ //Returns SIMO Buck-Boost Channel 0 Active-Discharge Enable
  return ((MAX77650_read_register(MAX77650_CNFG_SBB0_B_ADDR) >> 3) & 0b00000001);
}
uint8_t MAX77650_getEN_SBB0(void){ //Returns the Enable Control for SIMO Buck-Boost Channel 0.
  return (MAX77650_read_register(MAX77650_CNFG_SBB0_B_ADDR) & 0b00000111);
}

//SIMO Buck-Boost 1 Configuration Register A 0x2B
uint8_t MAX77650_getIP_SBB1(void){ //Returns the SIMO Buck-Boost Channel 1 Peak Current Limit
  return ((MAX77650_read_register(MAX77650_CNFG_SBB1_A_ADDR) >> 6) & 0b00000011);
}
uint8_t MAX77650_getTV_SBB1(void){ //Returns the SIMO Buck-Boost Channel 1 Target Output Voltage
  return (MAX77650_read_register(MAX77650_CNFG_SBB1_A_ADDR) & 0b00111111);
}

//SIMO Buck-Boost 1 Configuration Register B 0x2C
bool MAX77650_getADE_SBB1(void){ //Returns SIMO Buck-Boost Channel 1 Active-Discharge Enable
  return ((MAX77650_read_register(MAX77650_CNFG_SBB1_B_ADDR) >> 3) & 0b00000001);
}
uint8_t MAX77650_getEN_SBB1(void){ //Returns the Enable Control for SIMO Buck-Boost Channel 1.
  return (MAX77650_read_register(MAX77650_CNFG_SBB1_B_ADDR) & 0b00000111);
}

//SIMO Buck-Boost 2 Configuration Register A 0x2D
uint8_t MAX77650_getIP_SBB2(void){ //Returns the SIMO Buck-Boost Channel 2 Peak Current Limit
  return ((MAX77650_read_register(MAX77650_CNFG_SBB2_A_ADDR) >> 6) & 0b00000011);
}
uint8_t MAX77650_getTV_SBB2(void){ //Returns the SIMO Buck-Boost Channel 2 Target Output Voltage
  return (MAX77650_read_register(MAX77650_CNFG_SBB2_A_ADDR) & 0b00111111);
}

//SIMO Buck-Boost 2 Configuration Register B 0x2E
bool MAX77650_getADE_SBB2(void){ //Returns SIMO Buck-Boost Channel 2 Active-Discharge Enable
  return ((MAX77650_read_register(MAX77650_CNFG_SBB2_B_ADDR) >> 3) & 0b00000001);
}
uint8_t MAX77650_getEN_SBB2(void){ //Returns the Enable Control for SIMO Buck-Boost Channel 2.
  return (MAX77650_read_register(MAX77650_CNFG_SBB2_B_ADDR) & 0b00000111);
}

bool MAX77650_getCLK_64_S(void){ //Returns 64Hz Clock Status
  return ((MAX77650_read_register(MAX77650_CNFG_LED_TOP_ADDR) >> 1) & 0b00000001);
}
bool MAX77650_getEN_LED_MSTR(void){ //Returns Master LED Enable Bit
  return (MAX77650_read_register(MAX77650_CNFG_LED_TOP_ADDR) & 0b00000001);
}


//CID Register
uint8_t MAX77650_getCID(void){ // Returns the OTP programmed Chip Identification Code
  return (MAX77650_read_register(MAX77650_CID_ADDR) & 0b00001111);
}


//***** Register writes *****

bool MAX77650_setTV_LDO(uint8_t target_val){ //Sets the LDO Target Output Voltage
  MAX77650_write_register(MAX77650_CNFG_LDO_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_LDO_A_ADDR) & 0b10000000) | ((target_val & 0b01111111) << 0)));
  if(MAX77650_getTV_LDO()==target_val)
   return true;
  else
   return false;
}

//LDO Configuration Register B 0x39
bool MAX77650_setADE_LDO(uint8_t target_val){ //Sets LDO active-Discharge Enable
  MAX77650_write_register(MAX77650_CNFG_LDO_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_LDO_B_ADDR) & 0b11110111) | ((target_val & 0b00000001) << 3)));
  if(MAX77650_getADE_LDO()==target_val)
   return true;
  else
   return false;
}

bool MAX77650_setEN_LDO(uint8_t target_val){ //Enable LDO
  MAX77650_write_register(MAX77650_CNFG_LDO_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_LDO_B_ADDR) & 0b11111000) | ((target_val & 0b00000111) << 0)));
  if(MAX77650_getEN_LDO()==target_val)
   return true;
  else
   return false;
}

//Charger Configuration Register A 0x18
bool MAX77650_setTHM_HOT(uint8_t target_val){ //Returns the VHOT JEITA Temperature Threshold
  MAX77650_write_register(MAX77650_CNFG_CHG_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_A_ADDR) & 0b00111111) | ((target_val & 0b00000011) << 6)));
  if(MAX77650_getTHM_HOT()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setTHM_WARM(uint8_t target_val){ //Returns the VWARM JEITA Temperature Threshold
  MAX77650_write_register(MAX77650_CNFG_CHG_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_A_ADDR) & 0b11001111) | ((target_val & 0b00000011) << 4)));
  if(MAX77650_getTHM_WARM()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setTHM_COOL(uint8_t target_val){ //Returns the VCOOL JEITA Temperature Threshold
  MAX77650_write_register(MAX77650_CNFG_CHG_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_A_ADDR) & 0b11110011) | ((target_val & 0b00000011) << 2)));
  if(MAX77650_getTHM_COOL()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setTHM_COLD(uint8_t target_val){ //Returns the VCOLD JEITA Temperature Threshold
  MAX77650_write_register(MAX77650_CNFG_CHG_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_A_ADDR) & 0b11111100) | ((target_val & 0b00000011) << 0)));
  if(MAX77650_getTHM_COLD()==target_val)
   return true;
  else
   return false;
}

//Charger Configuration Register B 0x19
bool MAX77650_setVCHGIN_MIN(uint8_t target_val){ //Returns the Minimum CHGIN regulation voltage (VCHGIN-MIN)
  MAX77650_write_register(MAX77650_CNFG_CHG_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_B_ADDR) & 0b00011111) | ((target_val & 0b00000111) << 5)));
  if(MAX77650_getVCHGIN_MIN()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setICHGIN_LIM(uint8_t target_val){ //Returns the CHGIN Input Current Limit (ICHGIN-LIM)
  MAX77650_write_register(MAX77650_CNFG_CHG_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_B_ADDR) & 0b11100011) | ((target_val & 0b00000111) << 2)));
  if(MAX77650_getICHGIN_LIM()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setI_PQ(bool target_val){ //Returns the prequalification charge current (IPQ) as a percentage of IFAST-CHG
  MAX77650_write_register(MAX77650_CNFG_CHG_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_B_ADDR) & 0b11111101) | ((target_val & 0b00000001) << 1)));
  if(MAX77650_getI_PQ()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setCHG_EN(bool target_val){ //Returns Charger Enable
  MAX77650_write_register(MAX77650_CNFG_CHG_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_B_ADDR) & 0b11111110) | ((target_val & 0b00000001) << 0)));
  if(MAX77650_getCHG_EN()==target_val)
   return true;
  else
   return false;
}

//Charger Configuration Register C 0x1A
bool MAX77650_setCHG_PQ(uint8_t target_val){ //Returns the Battery prequalification voltage threshold (VPQ)
  MAX77650_write_register(MAX77650_CNFG_CHG_C_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_C_ADDR) & 0b00011111) | ((target_val & 0b00000111) << 5)));
  if(MAX77650_getCHG_PQ()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setI_TERM(uint8_t target_val){ //Returns the Charger Termination Current (ITERM). I_TERM[1:0] sets the charger termination current as a percentage of the fast charge current IFAST-CHG.
  MAX77650_write_register(MAX77650_CNFG_CHG_C_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_C_ADDR) & 0b11100111) | ((target_val & 0b00000011) << 3)));
  if(MAX77650_getI_TERM()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setT_TOPOFF(uint8_t target_val){ //Returns the Topoff timer value (tTO)
  MAX77650_write_register(MAX77650_CNFG_CHG_C_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_C_ADDR) & 0b11111000) | ((target_val & 0b00000111) << 0)));
  if(MAX77650_getT_TOPOFF()==target_val)
   return true;
  else
   return false;
}

//Charger Configuration Register D 0x1B
bool MAX77650_setTJ_REG(uint8_t target_val){ //Returns the die junction temperature regulation point, TJ-REG
  MAX77650_write_register(MAX77650_CNFG_CHG_D_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_D_ADDR) & 0b00011111) | ((target_val & 0b00000111) << 5)));
  if(MAX77650_getTJ_REG()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setVSYS_REG(uint8_t target_val){ //Returns the System voltage regulation (VSYS-REG)
  MAX77650_write_register(MAX77650_CNFG_CHG_D_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_D_ADDR) & 0b11100000) | ((target_val & 0b00011111) << 0)));
  if(MAX77650_getVSYS_REG()==target_val)
   return true;
  else
   return false;
}

//Charger Configuration Register E 0x1C
bool MAX77650_setCHG_CC(uint8_t target_val){ //Returns the fast-charge constant current value, IFAST-CHG.
  MAX77650_write_register(MAX77650_CNFG_CHG_E_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_E_ADDR) & 0b00000011) | ((target_val & 0b00111111) << 2)));
  if(MAX77650_getCHG_CC()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setT_FAST_CHG(uint8_t target_val){ //Returns the fast-charge safety timer, tFC.
  MAX77650_write_register(MAX77650_CNFG_CHG_E_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_E_ADDR) & 0b11111100) | ((target_val & 0b00000011) << 0)));
  if(MAX77650_getT_FAST_CHG()==target_val)
   return true;
  else
   return false;
}

//Charger Configuration Register F 0x1D
bool MAX77650_setCHG_CC_JEITA(uint8_t target_val){ //Returns the IFAST-CHG_JEITA for when the battery is either cool or warm as defined by the TCOOL and TWARM temperature thresholds. This register is a don't care if the battery temperature is normal.
  MAX77650_write_register(MAX77650_CNFG_CHG_F_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_F_ADDR) & 0b00000011) | ((target_val & 0b00111111) << 2)));
  if(MAX77650_getCHG_CC_JEITA()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setTHM_EN(bool target_val){ //Returns the Thermistor enable bit
  MAX77650_write_register(MAX77650_CNFG_CHG_F_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_F_ADDR) & 0b11111101) | ((target_val & 0b00000001) << 1)));
  if(MAX77650_getTHM_EN()==target_val)
   return true;
  else
   return false;
}

//Charger Configuration Register G 0x1E
bool MAX77650_setCHG_CV(uint8_t target_val){ //Returns the fast-charge battery regulation voltage, VFAST-CHG.
  MAX77650_write_register(MAX77650_CNFG_CHG_G_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_G_ADDR) & 0b00000011) | ((target_val & 0b00111111) << 2)));
  if(MAX77650_getCHG_CV()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setUSBS(bool target_val){ //this bit places CHGIN in USB suspend mode
  MAX77650_write_register(MAX77650_CNFG_CHG_G_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_G_ADDR) & 0b11111101) | ((target_val & 0b00000001) << 1)));
  if(MAX77650_getUSBS()==target_val)
   return true;
  else
   return false;
}

//Charger Configuration Register H 0x1F
bool MAX77650_setCHG_CV_JEITA(uint8_t target_val){ //Returns the modified VFAST-CHG for when the battery is either cool or warm as defined by the TCOOL and TWARM temperature thresholds. This register is a don't care if the battery temperature is normal.
  MAX77650_write_register(MAX77650_CNFG_CHG_H_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_H_ADDR) & 0b00000011) | ((target_val & 0b00111111) << 2)));
  if(MAX77650_getCHG_CV_JEITA()==target_val)
   return true;
  else
   return false;
}

//Charger Configuration Register I 0x20
bool MAX77650_setIMON_DISCHG_SCALE(uint8_t target_val){ //Returns the Selects the battery discharge current full-scale current value This 4-bit configuration starts at 7.5mA and ends at 300mA.
  MAX77650_write_register(MAX77650_CNFG_CHG_I_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_I_ADDR) & 0b00001111) | ((target_val & 0b00001111) << 4)));
  if(MAX77650_getIMON_DISCHG_SCALE()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setMUX_SEL(uint8_t target_val){ //Returns the analog channel to connect to AMUX configuration
  MAX77650_write_register(MAX77650_CNFG_CHG_I_ADDR,((MAX77650_read_register(MAX77650_CNFG_CHG_I_ADDR) & 0b11110000) | ((target_val & 0b00001111) << 0)));
  if(MAX77650_getMUX_SEL()==target_val)
   return true;
  else
   return false;
}

//SIMO Buck-Boost Global Configuration Register 0x28
bool MAX77650_setMRT_OTP(bool target_val){ //Returns Manual Reset Time Configuration
  MAX77650_write_register(MAX77650_CNFG_SBB_TOP_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB_TOP_ADDR) & 0b10111111) | ((target_val & 0b00000001) << 6)));
  if(MAX77650_getMRT_OTP()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setSBIA_LPM_DEF(bool target_val){ //Returns Default voltage of the SBIA_LPM bit
  MAX77650_write_register(MAX77650_CNFG_SBB_TOP_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB_TOP_ADDR) & 0b11011111) | ((target_val & 0b00000001) << 5)));
  if(MAX77650_getSBIA_LPM_DEF()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setDBNC_nEN_DEF(bool target_val){ //Returns Default Value of the DBNC_nEN bit
  MAX77650_write_register(MAX77650_CNFG_SBB_TOP_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB_TOP_ADDR) & 0b11101111) | ((target_val & 0b00000001) << 4)));
  if(MAX77650_getDBNC_nEN_DEF()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setDRV_SBB(uint8_t target_val){ //Returns the SIMO Buck-Boost (all channels) Drive Strength Trim.
  MAX77650_write_register(MAX77650_CNFG_SBB_TOP_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB_TOP_ADDR) & 0b11111100) | ((target_val & 0b00000011) << 0)));
  if(MAX77650_getDRV_SBB()==target_val)
   return true;
  else
   return false;
}

//SIMO Buck-Boost 0 Configuration Register A 0x29
bool MAX77650_setIP_SBB0(uint8_t target_val){ //Returns the SIMO Buck-Boost Channel 0 Peak Current Limit
  MAX77650_write_register(MAX77650_CNFG_SBB0_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB0_A_ADDR) & 0b00111111) | ((target_val & 0b00000011) << 6)));
  if(MAX77650_getIP_SBB0()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setTV_SBB0(uint8_t target_val){ //Returns the SIMO Buck-Boost Channel 0 Target Output Voltage
  MAX77650_write_register(MAX77650_CNFG_SBB0_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB0_A_ADDR) & 0b11000000) | ((target_val & 0b00111111) << 0)));
  if(MAX77650_getTV_SBB0()==target_val)
   return true;
  else
   return false;
}

//SIMO Buck-Boost 0 Configuration Register B 0x2A
bool MAX77650_setADE_SBB0(bool target_val){ //Returns SIMO Buck-Boost Channel 0 Active-Discharge Enable
  MAX77650_write_register(MAX77650_CNFG_SBB0_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB0_B_ADDR) & 0b11110111) | ((target_val & 0b00000001) << 3)));
  if(MAX77650_getADE_SBB0()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setEN_SBB0(uint8_t target_val){ //Returns the Enable Control for SIMO Buck-Boost Channel 0.
  MAX77650_write_register(MAX77650_CNFG_SBB0_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB0_B_ADDR) & 0b11111000) | ((target_val & 0b00000111) << 0)));
  if(MAX77650_getEN_SBB0()==target_val)
   return true;
  else
   return false;
}

//SIMO Buck-Boost 1 Configuration Register A 0x2B
bool MAX77650_setIP_SBB1(uint8_t target_val){ //Returns the SIMO Buck-Boost Channel 1 Peak Current Limit
  MAX77650_write_register(MAX77650_CNFG_SBB1_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB1_A_ADDR) & 0b00111111) | ((target_val & 0b00000011) << 6)));
  if(MAX77650_getIP_SBB1()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setTV_SBB1(uint8_t target_val){ //Returns the SIMO Buck-Boost Channel 1 Target Output Voltage
  MAX77650_write_register(MAX77650_CNFG_SBB1_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB1_A_ADDR) & 0b11000000) | ((target_val & 0b00111111) << 0)));
  if(MAX77650_getTV_SBB1()==target_val)
   return true;
  else
   return false;
}

//SIMO Buck-Boost 1 Configuration Register B 0x2C
bool MAX77650_setADE_SBB1(bool target_val){ //Returns SIMO Buck-Boost Channel 1 Active-Discharge Enable
  MAX77650_write_register(MAX77650_CNFG_SBB1_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB1_B_ADDR) & 0b11110111) | ((target_val & 0b00000001) << 3)));
  if(MAX77650_getADE_SBB1()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setEN_SBB1(uint8_t target_val){ //Returns the Enable Control for SIMO Buck-Boost Channel 1.
  MAX77650_write_register(MAX77650_CNFG_SBB1_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB1_B_ADDR) & 0b11111000) | ((target_val & 0b00000111) << 0)));
  if(MAX77650_getEN_SBB1()==target_val)
   return true;
  else
   return false;
}

//SIMO Buck-Boost 2 Configuration Register A 0x2D
bool MAX77650_setIP_SBB2(uint8_t target_val){ //Returns the SIMO Buck-Boost Channel 2 Peak Current Limit
  MAX77650_write_register(MAX77650_CNFG_SBB2_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB2_A_ADDR) & 0b00111111) | ((target_val & 0b00000011) << 6)));
  if(MAX77650_getIP_SBB2()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setTV_SBB2(uint8_t target_val){ //Returns the SIMO Buck-Boost Channel 2 Target Output Voltage
  MAX77650_write_register(MAX77650_CNFG_SBB2_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB2_A_ADDR) & 0b11000000) | ((target_val & 0b00111111) << 0)));
  if(MAX77650_getTV_SBB2()==target_val)
   return true;
  else
   return false;
}

//SIMO Buck-Boost 2 Configuration Register B 0x2E
bool MAX77650_setADE_SBB2(bool target_val){ //Returns SIMO Buck-Boost Channel 2 Active-Discharge Enable
  MAX77650_write_register(MAX77650_CNFG_SBB2_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB2_B_ADDR) & 0b11110111) | ((target_val & 0b00000001) << 3)));
  if(MAX77650_getADE_SBB2()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setEN_SBB2(uint8_t target_val){ //Returns the Enable Control for SIMO Buck-Boost Channel 2.
  MAX77650_write_register(MAX77650_CNFG_SBB2_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_SBB2_B_ADDR) & 0b11111000) | ((target_val & 0b00000111) << 0)));
  if(MAX77650_getEN_SBB2()==target_val)
   return true;
  else
   return false;
}

//LED related registers
bool MAX77650_setEN_LED_MSTR(bool target_val){ //Returns Master LED Enable Bit
  MAX77650_write_register(MAX77650_CNFG_LED_TOP_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED_TOP_ADDR) & 0b11111110) | ((target_val & 0b00000001) << 0)));
  if(MAX77650_getEN_LED_MSTR()==target_val)
   return true;
  else
   return false;
}

//Register LED0
bool MAX77650_setLED_FS0(uint8_t target_val){ //Returns LED0 Full Scale Range
  MAX77650_write_register(MAX77650_CNFG_LED0_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED0_A_ADDR) & 0b00111111) | ((target_val & 0b00000011) << 6)));
  if(MAX77650_getLED_FS0()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setINV_LED0(bool target_val){ //Returns LED0 Invert
  MAX77650_write_register(MAX77650_CNFG_LED0_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED0_A_ADDR) & 0b11011111) | ((target_val & 0b00000001) << 5)));
  if(MAX77650_getINV_LED0()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setBRT_LED0(uint8_t target_val){ //Returns LED0 Brightness Control
  MAX77650_write_register(MAX77650_CNFG_LED0_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED0_A_ADDR) & 0b11100000) | ((target_val & 0b00011111) << 0)));
  if(MAX77650_getBRT_LED0()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setP_LED0(uint8_t target_val){ //Returns LED0 Period Settings
  MAX77650_write_register(MAX77650_CNFG_LED0_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED0_B_ADDR) & 0b00001111) | ((target_val & 0b00001111) << 4)));
  if(MAX77650_getP_LED0()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setD_LED0(uint8_t target_val){ //Returns LED0 On Duty-Cycle Settings
  MAX77650_write_register(MAX77650_CNFG_LED0_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED0_B_ADDR) & 0b11110000) | ((target_val & 0b00001111) << 0)));
  if(MAX77650_getD_LED0()==target_val)
   return true;
  else
   return false;
}

//Register LED1
bool MAX77650_setLED_FS1(uint8_t target_val){ //Returns LED1 Full Scale Range
  MAX77650_write_register(MAX77650_CNFG_LED1_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED1_A_ADDR) & 0b00111111) | ((target_val & 0b00000011) << 6)));
  if(MAX77650_getLED_FS1()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setINV_LED1(bool target_val){ //Returns LED1 Invert
  MAX77650_write_register(MAX77650_CNFG_LED1_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED1_A_ADDR) & 0b11011111) | ((target_val & 0b00000001) << 5)));
  if(MAX77650_getINV_LED1()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setBRT_LED1(uint8_t target_val){ //Returns LED1 Brightness Control
  MAX77650_write_register(MAX77650_CNFG_LED1_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED1_A_ADDR) & 0b11100000) | ((target_val & 0b00011111) << 0)));
  if(MAX77650_getBRT_LED1()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setP_LED1(uint8_t target_val){ //Returns LED1 Period Settings
  MAX77650_write_register(MAX77650_CNFG_LED1_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED1_B_ADDR) & 0b00001111) | ((target_val & 0b00001111) << 4)));
  if(MAX77650_getP_LED1()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setD_LED1(uint8_t target_val){ //Returns LED1 On Duty-Cycle Settings
  MAX77650_write_register(MAX77650_CNFG_LED1_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED1_B_ADDR) & 0b11110000) | ((target_val & 0b00001111) << 0)));
  if(MAX77650_getD_LED1()==target_val)
   return true;
  else
   return false;
}

//Register LED2
bool MAX77650_setLED_FS2(uint8_t target_val){ //Returns LED2 Full Scale Range
  MAX77650_write_register(MAX77650_CNFG_LED2_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED2_A_ADDR) & 0b00111111) | ((target_val & 0b00000011) << 6)));
  if(MAX77650_getLED_FS2()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setINV_LED2(bool target_val){ //Returns LED2 Invert
  MAX77650_write_register(MAX77650_CNFG_LED2_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED2_A_ADDR) & 0b11011111) | ((target_val & 0b00000001) << 5)));
  if(MAX77650_getINV_LED2()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setBRT_LED2(uint8_t target_val){ //Returns LED2 Brightness Control
  MAX77650_write_register(MAX77650_CNFG_LED2_A_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED2_A_ADDR) & 0b11100000) | ((target_val & 0b00011111) << 0)));
  if(MAX77650_getBRT_LED2()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setP_LED2(uint8_t target_val){ //Returns LED2 Period Settings
  MAX77650_write_register(MAX77650_CNFG_LED2_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED2_B_ADDR) & 0b00001111) | ((target_val & 0b00001111) << 4)));
  if(MAX77650_getP_LED2()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setD_LED2(uint8_t target_val){ //Returns LED2 On Duty-Cycle Settings
  MAX77650_write_register(MAX77650_CNFG_LED2_B_ADDR,((MAX77650_read_register(MAX77650_CNFG_LED2_B_ADDR) & 0b11110000) | ((target_val & 0b00001111) << 0)));
  if(MAX77650_getD_LED2()==target_val)
   return true;
  else
   return false;
}

//Global Configuration register 0x10
bool MAX77650_setBOK(bool target_val){ //Returns Main Bias Okay Status Bit
  MAX77650_write_register(MAX77650_CNFG_GLBL_ADDR,((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) & 0b10111111) | ((target_val & 0b00000001) << 6)));
  if(MAX77650_getBOK()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setSBIA_LPM(bool target_val){ //Returns Main Bias Low-Power Mode software request
  MAX77650_write_register(MAX77650_CNFG_GLBL_ADDR,((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) & 0b11011111) | ((target_val & 0b00000001) << 5)));
  if(MAX77650_getSBIA_LPM()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setSBIA_EN(bool target_val){ //Returns Main Bias Enable Software Request
  MAX77650_write_register(MAX77650_CNFG_GLBL_ADDR,((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) & 0b11101111) | ((target_val & 0b00000001) << 4)));
  if(MAX77650_getSBIA_EN()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setnEN_MODE(bool target_val){ //Returns nEN Input (ONKEY) Default Configuration Mode
  MAX77650_write_register(MAX77650_CNFG_GLBL_ADDR,((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) & 0b11110111) | ((target_val & 0b00000001) << 3)));
  if(MAX77650_getnEN_MODE()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setDBEN_nEN(bool target_val){ //Returns Debounce Timer Enable for the nEN Pin
  MAX77650_write_register(MAX77650_CNFG_GLBL_ADDR,((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) & 0b11111011) | ((target_val & 0b00000001) << 2)));
  if(MAX77650_getDBEN_nEN()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setSFT_RSTt(uint8_t target_val){ //Returns Software Reset Functions.
  MAX77650_write_register(MAX77650_CNFG_GLBL_ADDR,((MAX77650_read_register(MAX77650_CNFG_GLBL_ADDR) & 0b11111100) | ((target_val & 0b00000011) << 0)));
  if(MAX77650_getSFT_RST()==target_val)
   return true;
  else
   return false;
}

//Global Interrupt Mask Register 0x06
bool MAX77650_setINT_M_GLBL(uint8_t target_val){ //Sets Global Interrupt Mask Register
  MAX77650_write_register(MAX77650_INTM_GLBL_ADDR,(target_val & 0b01111111));
  if(MAX77650_getINT_M_GLBL()==(target_val & 0b01111111))
   return true;
  else
   return false;
}

//GPIO Configuration Register
bool MAX77650_setDBEN_GPI(bool target_val){ //Returns General Purpose Input Debounce Timer Enable
  MAX77650_write_register(MAX77650_CNFG_GPIO_ADDR,((MAX77650_read_register(MAX77650_CNFG_GPIO_ADDR) & 0b11101111) | ((target_val & 0b00000001) << 4)));
  if(MAX77650_getDBEN_GPI()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setDO(bool target_val){ //Returns General Purpose Output Data Output
  MAX77650_write_register(MAX77650_CNFG_GPIO_ADDR,((MAX77650_read_register(MAX77650_CNFG_GPIO_ADDR) & 0b11110111) | ((target_val & 0b00000001) << 3)));
  if(MAX77650_getDO()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setDRV(bool target_val){ //Returns General Purpose Output Driver Type
  MAX77650_write_register(MAX77650_CNFG_GPIO_ADDR,((MAX77650_read_register(MAX77650_CNFG_GPIO_ADDR) & 0b11111011) | ((target_val & 0b00000001) << 2)));
  if(MAX77650_getDRV()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setDI(bool target_val){ //Returns GPIO Digital Input Value
  MAX77650_write_register(MAX77650_CNFG_GPIO_ADDR,((MAX77650_read_register(MAX77650_CNFG_GPIO_ADDR) & 0b11111101) | ((target_val & 0b00000001) << 1)));
  if(MAX77650_getDI()==target_val)
   return true;
  else
   return false;
}
bool MAX77650_setDIR(bool target_val){ //Returns GPIO Direction
  MAX77650_write_register(MAX77650_CNFG_GPIO_ADDR,((MAX77650_read_register(MAX77650_CNFG_GPIO_ADDR) & 0b11111110) | ((target_val & 0b00000001) << 0)));
  if(MAX77650_getDIR()==target_val)
   return true;
  else
   return false;
}

//Charger Interrupt Mask Register 0x07
bool MAX77650_setINT_M_CHG(uint8_t target_val){ //Sets Global Interrupt Mask Register
  MAX77650_write_register(MAX77650_INT_M_CHG_ADDR,(target_val & 0b01111111));
  if(MAX77650_getINT_M_CHG()==(target_val & 0b01111111))
   return true;
  else
   return false;
}

static void pmu_print_error(bool rslt){
  if (rslt){
    NRF_LOG_RAW_INFO("okay\n");
  }else{
    NRF_LOG_RAW_INFO("failed\n");
  }
}

bool pmu_init(void)
{
  if (!i2cScanForPmu()) {
    NRF_LOG_RAW_INFO("%08d [pmu] not found\n", systemTimeGetMs());
    return false;
  }
  //Baseline Initialization following rules printed in MAX77650 Programmres Guide Chapter 4 Page 5

  // Set Main Bias to normal Mode
  if(MAX77650_debug){
    NRF_LOG_RAW_INFO("Set Main Bias to normal Mode: ");
    pmu_print_error(MAX77650_setSBIA_LPM(false));
  }else{
    MAX77650_setSBIA_LPM(false);
  }

  // Set On/Off-Button to push-button-mode
  if(MAX77650_debug){
    NRF_LOG_RAW_INFO("Set On/Off-Button to push-button-mode: ");
    pmu_print_error(MAX77650_setnEN_MODE(false));
  }else{
    MAX77650_setnEN_MODE(false);
  }

  // Set nEN input debounce time to 30ms
  if(MAX77650_debug){
    NRF_LOG_RAW_INFO("Set nEN input debounce time to 30ms: ");
    pmu_print_error(MAX77650_setDBEN_nEN(true));
    }else{
    MAX77650_setDBEN_nEN(true);
  }

  // Compare part-numbers
  NRF_LOG_RAW_INFO("Comparing part-numbers: ");
  pmu_print_error(MAX77650_getDIDM() == PMIC_partnumber);

  // Check OTP Options
  NRF_LOG_RAW_INFO("Checking OTP options: ");
  pmu_print_error(MAX77650_getCID() != MAX77650_CID);

  // Set CHGIN Regulation Voltage
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set CHGIN regulation voltage to 4.0V: ");
    pmu_print_error(MAX77650_setVCHGIN_MIN(0b000));
  }else{
    MAX77650_setVCHGIN_MIN(0b000);
  }

  // Set CHGIN Input Current Limit
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set CHGIN Input Current Limit to 475 mA: ");
    pmu_print_error(MAX77650_setICHGIN_LIM(CHGIN_LIM));
  }else{
    MAX77650_setICHGIN_LIM(CHGIN_LIM);
  }

  // Set prequalification charge current to 10%
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set the prequalification charge current to 10%: ");
    pmu_print_error(MAX77650_setI_PQ(false));
  }else{
    MAX77650_setI_PQ(false);
  }

  // Set prequalification charge voltage to 2.3V
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set Battery prequalification voltage threshold to 2.3V: ");
    pmu_print_error(MAX77650_setCHG_PQ(0b000));
  }else{
    MAX77650_setCHG_PQ(0b000);
  }

  // Set charger termination current to 15% of fast charge current
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set Charger Termination Current to 15% of of fast charge current: ");
    pmu_print_error(MAX77650_setI_TERM(0b11));
  }else{
    MAX77650_setI_TERM(0b11);
  }

  // Set topoff timer value to 0 minutes
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set Topoff timer value to 0 minutes: ");
    pmu_print_error(MAX77650_setT_TOPOFF(0b000));
  }else{
    MAX77650_setT_TOPOFF(0b000);
  }

  // Set die junction temperature regulation point to 60°C
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set the die junction temperature regulation point to 60°C: ");
    pmu_print_error(MAX77650_setTJ_REG(0));
  }else{
    MAX77650_setTJ_REG(0);
  }

  /********** Set System Voltage Regulation **********/
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set System voltage regulation to 4.50V: ");
    pmu_print_error(MAX77650_setVSYS_REG(VSYS_REG));
  }else{
    MAX77650_setVSYS_REG(VSYS_REG);
  }

  /********* Set Fast Charge constant current **************/
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set the fast-charge constant current value to 7.5mA: ");
    pmu_print_error(MAX77650_setCHG_CC(CHG_CC));
  }else{
    MAX77650_setCHG_CC(CHG_CC);
  }

  // set fast charge safety timer
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set the fast-charge safety timer to 3h: ");
    pmu_print_error(MAX77650_setT_FAST_CHG(0x01));
  }else{
    MAX77650_setT_FAST_CHG(0x01);
  }

  // Disable Temperature monitoring
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Disable Temperature monitoring: ");
    pmu_print_error(MAX77650_setTHM_EN(false));
  }else{
    MAX77650_setTHM_EN(false);
  }


  /******** Set fast-charge constant voltage ************/
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set fast-charge battery regulation voltage to");
    pmu_print_error(MAX77650_setCHG_CV(CHG_CV));
  }else{
    MAX77650_setCHG_CV(CHG_CV);
  }

  // Set USB not in power down (CHGIN not suspended and can draw current from adapter)
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set USB not in power down: ");
    pmu_print_error(MAX77650_setUSBS(false));
  }else{
    MAX77650_setUSBS(false);
  }

  // Selects the battery discharge current full-scale current value to 300mA
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Selects the battery discharge current full-scale current value to 300mA: ");
    pmu_print_error(MAX77650_setIMON_DISCHG_SCALE(0x0A));
  }else{
    MAX77650_setIMON_DISCHG_SCALE(0x0A);
  }

  // Set the charger to Enable
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set the Charger to Enable: ");
    // pmu_print_error(MAX77650_setCHG_EN(true));
  }else{
    NRF_LOG_RAW_INFO("Charger Disabled");
    MAX77650_setCHG_EN(false);
  }

  MAX77650_setCHG_EN(false);

  MAX77650_setSBIA_EN(true);

  bool activeDischarge = false;

  // Disable SIMO Buck-Boost Channel 0 Active-Discharge
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Disable SIMO Buck-Boost Channel 0 Active-Discharge: ");
    pmu_print_error(MAX77650_setADE_SBB0(activeDischarge));
  }else{
    MAX77650_setADE_SBB0(activeDischarge);
  }

  // Disable SIMO Buck-Boost Channel 1 Active-Discharge
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Disable SIMO Buck-Boost Channel 1 Active-Discharge: ");
    pmu_print_error(MAX77650_setADE_SBB1(activeDischarge));
  }else{
    MAX77650_setADE_SBB1(activeDischarge);
  }

  // Disable SIMO Buck-Boost Channel 2 Active-Discharge
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Disable SIMO Buck-Boost Channel 2 Active-Discharge: ");
    pmu_print_error(MAX77650_setADE_SBB2(activeDischarge));
  }else{
    MAX77650_setADE_SBB2(activeDischarge);
  }

  // Set SIMO Buck-Boost Channel to maximum drive strength
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set SIMO Buck-Boost to maximum drive strength: ");
    pmu_print_error(MAX77650_setDRV_SBB(0b00));
  }else{
    MAX77650_setDRV_SBB(0b00);
  }

  // Set SIMO Buck-Boost Channel 0 Peak Current Limit to 500mA
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set SIMO Buck-Boost Channel 0 Peak Current Limit to 500mA: ");
    pmu_print_error(MAX77650_setIP_SBB0(0b00));
  }else{
    MAX77650_setIP_SBB0(0b00);
  }

  // Set SIMO Buck-Boost Channel 1 Peak Current Limit to 500mA
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set SIMO Buck-Boost Channel 1 Peak Current Limit to 500mA: ");
    pmu_print_error(MAX77650_setIP_SBB1(0b00));
  }else{
    MAX77650_setIP_SBB1(0b00);
  }

  // Set SIMO Buck-Boost Channel 2 Peak Current Limit to 500mA
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Set SIMO Buck-Boost Channel 2 Peak Current Limit to 500mA: ");
    pmu_print_error(MAX77650_setIP_SBB2(0b00));
  }else{
    MAX77650_setIP_SBB2(0b00);
  }

  /******* Set and Enable Output Voltages ********/
  // SBB0
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Enable SBB0 Output to 1.5V: ");
    pmu_print_error(MAX77650_setTV_SBB0(TV_SBB0)); //Set output Voltage of SBB0 to 1.5V
    pmu_print_error(MAX77650_setEN_SBB0(0b110));  // Enable
  }else{
    MAX77650_setTV_SBB0(TV_SBB0);
    MAX77650_setEN_SBB0(0b110);
  }

  // SBBB1
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Enable SBB1 Output to 2.8V: ");
    pmu_print_error(MAX77650_setTV_SBB1(TV_SBB1)); //Set output Voltage of SBB1 to 1.5V
    pmu_print_error(MAX77650_setEN_SBB1(0b110));
  }else{
    MAX77650_setTV_SBB1(TV_SBB1); //Set output Voltage of SBB1 to 1.5V
    MAX77650_setEN_SBB1(0b110); // Enable
  }

  // SBB2
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Enable SBB2 Output to 2.8V: ");
    pmu_print_error(MAX77650_setTV_SBB2(TV_SBB2)); //Set output Voltage of SBB2 to 1.8V
    pmu_print_error(MAX77650_setEN_SBB2(0b110)); //Enable
  }else{
    MAX77650_setTV_SBB2(TV_SBB2); //Set output Voltage of SBB2 to 1.8V
    MAX77650_setEN_SBB2(0b110); //Enable SBB2
  }

  // LDO
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Enable LDO Output to 2.8V: ");
    pmu_print_error(MAX77650_setTV_LDO(TV_LDO)); //Set output Voltage of LDO to 2.8V
    pmu_print_error(MAX77650_setEN_LDO(true)); //Enable LDO
  }else{
    MAX77650_setTV_LDO(TV_LDO); //Set output Voltage of LDO to 2.8V
    MAX77650_setEN_LDO(true); //Enable LDO
  }

  NRF_LOG_RAW_INFO("SIMO Buck-Boost Voltage Channel 0: ");
  NRF_LOG_RAW_INFO(NRF_LOG_FLOAT_MARKER " V\n", NRF_LOG_FLOAT(MAX77650_getTV_SBB0() * 0.025 + 0.8));

  NRF_LOG_RAW_INFO("SIMO Buck-Boost Voltage Channel 2: ");
  if (!(MAX77650_getDIDM())){
    NRF_LOG_RAW_INFO(NRF_LOG_FLOAT_MARKER " V\n", NRF_LOG_FLOAT(MAX77650_getTV_SBB2() * 0.05 + 0.8));
  }
  else{
    NRF_LOG_RAW_INFO(NRF_LOG_FLOAT_MARKER " V\n", NRF_LOG_FLOAT(MAX77650_getTV_SBB2() * 0.05 + 2.4));
  }

  NRF_LOG_RAW_INFO("LDO Voltage: ");
  NRF_LOG_RAW_INFO(NRF_LOG_FLOAT_MARKER " V\n", NRF_LOG_FLOAT(MAX77650_getTV_LDO() * 0.0125 + 1.35));

  // Disable LED Master
  MAX77650_setEN_LED_MSTR(0);


  /*********** Global Interrupts ************/

  // Initialize Global Interrupt Mask Register
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Initialize Global Interrupt Mask Register: ");
    pmu_print_error(MAX77650_setINT_M_GLBL(0x0));
  }else{
    MAX77650_setINT_M_GLBL(0x0);
  }

  // Initialize Charger Interrupt Mask Register
  if (MAX77650_debug){
    NRF_LOG_RAW_INFO("Initialize Charger Interrupt Mask Register: ");
    pmu_print_error(MAX77650_setINT_M_CHG(0x0));
  }else{
    MAX77650_setINT_M_CHG(0x0);
  }

  //Read and clear Interrupt Registers
  MAX77650_getINT_GLBL();
  MAX77650_getINT_CHG();
  MAX77650_getERCFLAG();

  return true;
}
