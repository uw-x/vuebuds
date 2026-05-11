#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "gpio.h"
#include "i2c.h"

#ifndef PMU_H

#define MAX77651_I2C_ADDRESS 0x48


// Project specific definitions *** adapt to your requirements! ***
#define PMIC_partnumber 0x00  //placed PMIC part-numer on the board: MAX77650 or MAX77651
#define MAX77650_CID 0x73     //OTP option
#define MAX77650_ADDR 0x48    //alternate ADDR is 0x40

//***** Begin MAX77650 Register Definitions *****
#define MAX77650_CNFG_GLBL_ADDR 0x10     //Global Configuration Register; Reset Value OTP
#define MAX77650_INT_GLBL_ADDR 0x00      //Interrupt; Reset Value 0x00
#define MAX77650_INTM_GLBL_ADDR 0x06     //Interrupt Mask; Reset Value 0xFF
#define MAX77650_STAT_GLBL_ADDR 0x05     //Global Status Register; Reset Value OTP
#define MAX77650_ERCFLAG_ADDR 0x04       //Flags; Reset Value 0x00
#define MAX77650_CNFG_GPIO_ADDR 0x12     //GPIO Configuration Register; Reset Value 0x01
#define MAX77650_CID_ADDR 0x11           //Chip ID Register; Reset Value OTP; Read only
#define MAX77650_INT_CHG_ADDR 0x01       //Charger Interrupt Register; Reset Value 0x00; RC
#define MAX77650_INT_M_CHG_ADDR 0x07     //Charger Interrupt Mask Register; Reset Value 0xFF; Read/Write
#define MAX77650_STAT_CHG_A_ADDR 0x02    //Charger Status Register A; Reset Value 0x00; Read only
#define MAX77650_STAT_CHG_B_ADDR 0x03    //Charger Status Register B; Reset Value 0x00; Read only
#define MAX77650_CNFG_CHG_A_ADDR 0x018   //Charger Configuration Register A; Reset Value 0x0F; Read/Write
#define MAX77650_CNFG_CHG_B_ADDR 0x019   //Charger Configuration Register B; Reset Value OTP; Read/Write
#define MAX77650_CNFG_CHG_C_ADDR 0x01A   //Charger Configuration Register C; Reset Value 0xF8; Read/Write
#define MAX77650_CNFG_CHG_D_ADDR 0x01B   //Charger Configuration Register D; Reset Value 0x10; Read/Write
#define MAX77650_CNFG_CHG_E_ADDR 0x01C   //Charger Configuration Register E; Reset Value 0x05; Read/Write
#define MAX77650_CNFG_CHG_F_ADDR 0x01D   //Charger Configuration Register F; Reset Value 0x04; Read/Write
#define MAX77650_CNFG_CHG_G_ADDR 0x01E   //Charger Configuration Register G; Reset Value 0x00; Read/Write
#define MAX77650_CNFG_CHG_H_ADDR 0x01F   //Charger Configuration Register H; Reset Value 0x00; Read/Write
#define MAX77650_CNFG_CHG_I_ADDR 0x020   //Charger Configuration Register I; Reset Value 0xF0; Read/Write
#define MAX77650_CNFG_LDO_A_ADDR 0x38    //LDO Configuration Register A; Reset Value OTP; Read/Write
#define MAX77650_CNFG_LDO_B_ADDR 0x39    //LDO Configuration Register B; Reset Value OTP; Read/Write
#define MAX77650_CNFG_SBB_TOP_ADDR 0x28  //SIMO Buck-Boost Configuration Register; Reset Value OTP; Read/Write
#define MAX77650_CNFG_SBB0_A_ADDR 0x29   //SIMO Buck-Boost 0 Configuration Register A; Reset Value OTP; Read/Write
#define MAX77650_CNFG_SBB0_B_ADDR 0x2A   //SIMO Buck-Boost 0 Configuration Register B; Reset Value OTP; Read/Write
#define MAX77650_CNFG_SBB1_A_ADDR 0x2B   //SIMO Buck-Boost 1 Configuration Register A; Reset Value OTP; Read/Write
#define MAX77650_CNFG_SBB1_B_ADDR 0x2C   //SIMO Buck-Boost 1 Configuration Register B; Reset Value OTP; Read/Write
#define MAX77650_CNFG_SBB2_A_ADDR 0x2D   //SIMO Buck-Boost 2 Configuration Register A; Reset Value OTP; Read/Write
#define MAX77650_CNFG_SBB2_B_ADDR 0x2E   //SIMO Buck-Boost 2 Configuration Register B; Reset Value OTP; Read/Write
#define MAX77650_CNFG_LED0_A_ADDR 0x40   //LED 0 Configuration Register A; Reset Value 0x04; Read/Write
#define MAX77650_CNFG_LED0_B_ADDR 0x43   //LED 0 Configuration Register B; Reset Value 0x0F; Read/Write
#define MAX77650_CNFG_LED1_A_ADDR 0x41   //LED 1 Configuration Register A; Reset Value 0x04; Read/Write
#define MAX77650_CNFG_LED1_B_ADDR 0x44   //LED 1 Configuration Register B; Reset Value 0x0F; Read/Write
#define MAX77650_CNFG_LED2_A_ADDR 0x42   //LED 2 Configuration Register A; Reset Value 0x04; Read/Write
#define MAX77650_CNFG_LED2_B_ADDR 0x45   //LED 2 Configuration Register A; Reset Value 0x0F; Read/Write
#define MAX77650_CNFG_LED_TOP_ADDR 0x46  //LED Configuration Register; Reset Value 0x0D; Read/Write
//***** End MAX77650 Register Definitions *****

bool MAX77650_init(void);
uint8_t MAX77650_read_register(uint8_t);
void MAX77650_write_register(uint8_t, uint8_t);

//Status Register (STAT_GLBL) related reads
bool MAX77650_getDIDM(void); //Returns Part Number; Return Value: 1=MAX77650; 2=MAX77651; -1=error reading DeviceID
bool MAX77650_getLDO_DropoutDetector(void); //Returns whether the LDO is in Dropout or not; Return Value: 0=LDO is not in dropout; 1=LDO is in dropout
bool MAX77650_getThermalAlarm1(void); //Returns Thermal Alarm (TJA1 limit); Return Value: 0=Tj<TJA1; 1=Tj>TJA1
bool MAX77650_getThermalAlarm2(void); //Returns Therma2 Alarm (TJA2 limit); Return Value: 0=Tj<TJA2; 1=Tj>TJA2
bool MAX77650_getDebounceStatusnEN0(void); //Returns Debounced Status of nEN input; Return Value: 0=nEN0 is not active; 1=nEN0 is active
bool MAX77650_getDebounceStatusPWR_HLD(void); //Returns Debounced Status of PWR_HLD input; Return Value: 0=logic low; 1=logic high

//Flag Register (ERCFLAG) related reads
uint8_t MAX77650_getERCFLAG(void); //Returns the ERCFLAG Register 0x04

//Register reads related to ChipID
uint8_t MAX77650_getChipID(void); // Return Value: 1=MAX77650; 2=MAX77651; -1=error reading ChipID

//CID Register
uint8_t MAX77650_getCID(void); // Returns the OTP programmed Chip Identification Code

//Charger Status Register (STAT_CHG_A and B) related reads
bool MAX77650_getVCHGIN_MIN_STAT(void); //Returns Minimum Input Voltage Regulation Loop Status; Return Value: 0=no event; 1=The minimum CHGIN voltage regulation loop has engaged to regulate VCHGIN >= VCHGIN-MIN
bool MAX77650_getICHGIN_LIM_STAT(void); //Returns Input Current Limit Loop Status; Return Value: 0=no event; 1=The CHGIN current limit loop has engaged to regulate ICHGIN <= ICHGIN-LIM
bool MAX77650_getVSYS_MIN_STAT(void); //Returns Minimum System Voltage Regulation Loop Status; Return Value: 0=no event; 1=The minimum system voltage regulation loop is engaged to regulate VSYS >= VSYS-MIN
bool MAX77650_getTJ_REG_STAT(void); //Returns Maximum Junction Temperature Regulation Loop Status; Return Value: 0=no event; 1=The maximum junction temperature regulation loop has engaged to regulate the junction temperature less than TJ-REG
uint8_t MAX77650_getTHM_DTLS(void); //Returns Battery Temperature Details; Return Value: Battery Temperature Details
uint8_t MAX77650_getCHG_DTLS(void); //Returns Charger Details
uint8_t MAX77650_getCHGIN_DTLS(void); //Returns Charger Input Status Details
bool MAX77650_getCHG(void); //Returns Quick Charger Status
bool MAX77650_getTIME_SUS(void); //Returns Time Suspend Indicator

//LED related registers
bool MAX77650_getCLK_64_S(void); //Returns 64Hz Clock Status
bool MAX77650_getEN_LED_MSTR(void); //Returns Master LED Enable Bit
uint8_t MAX77650_getLED_FS0(void); //Returns LED0 Full Scale Range
bool MAX77650_getINV_LED0(void); //Returns LED0 Invert
uint8_t MAX77650_getBRT_LED0(void); //Returns LED0 Brightness Control
uint8_t MAX77650_getP_LED0(void); //Returns LED0 Period Settings
uint8_t MAX77650_getD_LED0(void); //Returns LED0 On Duty-Cycle Settings
uint8_t MAX77650_getLED_FS1(void); //Returns LED1 Full Scale Range
bool MAX77650_getINV_LED1(void); //Returns LED1 Invert
uint8_t MAX77650_getBRT_LED1(void); //Returns LED1 Brightness Control
uint8_t MAX77650_getP_LED1(void); //Returns LED1 Period Settings
uint8_t MAX77650_getD_LED1(void); //Returns LED1 On Duty-Cycle Settings
uint8_t MAX77650_getLED_FS2(void); //Returns LED2 Full Scale Range
bool MAX77650_getINV_LED2(void); //Returns LED2 Invert
uint8_t MAX77650_getBRT_LED2(void); //Returns LED2 Brightness Control
uint8_t MAX77650_getP_LED2(void); //Returns LED2 Period Settings
uint8_t MAX77650_getD_LED2(void); //Returns LED2 On Duty-Cycle Settings

//Global Configuration register 0x10
bool MAX77650_getBOK(void); //Returns Main Bias Okay Status Bit
bool MAX77650_getSBIA_LPM(void); //Returns Main Bias Low-Power Mode software request
bool MAX77650_getSBIA_EN(void); //Returns Main Bias Enable Software Request
bool MAX77650_getnEN_MODE(void); //Returns nEN Input (ONKEY) Default Configuration Mode
bool MAX77650_getDBEN_nEN(void); //Returns Debounce Timer Enable for the nEN Pin
uint8_t MAX77650_getSFT_RST(void); //Returns Software Reset Functions.

//Global Interrupt Status Register 0x00
uint8_t MAX77650_getINT_GLBL(void); //Returns Interrupt Status Register 0x00

//Global Interrupt Mask Register 0x06
uint8_t MAX77650_getINT_M_GLBL(void); //Returns Global Interrupt Mask Register

//GPIO Configuration Register
bool MAX77650_getDBEN_GPI(void); //Returns General Purpose Input Debounce Timer Enable
bool MAX77650_getDO(void); //Returns General Purpose Output Data Output
bool MAX77650_getDRV(void); //Returns General Purpose Output Driver Type
bool MAX77650_getDI(void); //Returns GPIO Digital Input Value
bool MAX77650_getDIR(void); //Returns GPIO Direction

//Charger Interrupt Status Register 0x01
uint8_t MAX77650_getINT_CHG(void); //Returns Charger Interrupt Status Register 0x01

//Charger Interrupt Mask Register 0x07
uint8_t MAX77650_getINT_M_CHG(void); //Returns Global Interrupt Mask Register

//Charger Configuration Register A 0x18
uint8_t MAX77650_getTHM_HOT(void); //Returns the VHOT JEITA Temperature Threshold
uint8_t MAX77650_getTHM_WARM(void); //Returns the VWARM JEITA Temperature Threshold
uint8_t MAX77650_getTHM_COOL(void); //Returns the VCOOL JEITA Temperature Threshold
uint8_t MAX77650_getTHM_COLD(void); //Returns the VCOLD JEITA Temperature Threshold

//Charger Configuration Register B 0x19
uint8_t MAX77650_getVCHGIN_MIN(void); //Returns the Minimum CHGIN regulation voltage (VCHGIN-MIN)
uint8_t MAX77650_getICHGIN_LIM(void); //Returns the CHGIN Input Current Limit (ICHGIN-LIM)
bool MAX77650_getI_PQ(void); //Returns the prequalification charge current (IPQ) as a percentage of IFAST-CHG
bool MAX77650_getCHG_EN(void); //Returns Charger Enable

//Charger Configuration Register C 0x1A
uint8_t MAX77650_getCHG_PQ(void); //Returns the Battery prequalification voltage threshold (VPQ)
uint8_t MAX77650_getI_TERM(void); //Returns the Charger Termination Current (ITERM). I_TERM[1:0] sets the charger termination current as a percentage of the fast charge current IFAST-CHG.
uint8_t MAX77650_getT_TOPOFF(void); //Returns the Topoff timer value (tTO)

//Charger Configuration Register D 0x1B
uint8_t MAX77650_getTJ_REG(void); //Returns the die junction temperature regulation point, TJ-REG
uint8_t MAX77650_getVSYS_REG(void); //Returns the System voltage regulation (VSYS-REG)

//Charger Configuration Register E 0x1C
uint8_t MAX77650_getCHG_CC(void); //Returns the fast-charge constant current value, IFAST-CHG.
uint8_t MAX77650_getT_FAST_CHG(void); //Returns the fast-charge safety timer, tFC.

//Charger Configuration Register F 0x1D
uint8_t MAX77650_getCHG_CC_JEITA(void); //Returns the IFAST-CHG_JEITA for when the battery is either cool or warm as defined by the TCOOL and TWARM temperature thresholds. This register is a don't care if the battery temperature is normal.
uint8_t MAX77650_getTHM_EN(void); //Returns the Thermistor enable bit

//Charger Configuration Register G 0x1E
uint8_t MAX77650_getCHG_CV(void); //Returns the fast-charge battery regulation voltage, VFAST-CHG.
bool MAX77650_getUSBS(void); //this bit places CHGIN in USB suspend mode

//Charger Configuration Register H 0x1F
uint8_t MAX77650_getCHG_CV_JEITA(void); //Returns the modified VFAST-CHG for when the battery is either cool or warm as defined by the TCOOL and TWARM temperature thresholds. This register is a don't care if the battery temperature is normal.

//Charger Configuration Register I 0x20
uint8_t MAX77650_getIMON_DISCHG_SCALE(void); //Returns the Selects the battery discharge current full-scale current value This 4-bit configuration starts at 7.5mA and ends at 300mA.
uint8_t MAX77650_getMUX_SEL(void); //Returns the analog channel to connect to AMUX configuration

//LDO Configuration Register A 0x38
uint8_t MAX77650_getTV_LDO(void); //Returns the LDO Target Output Voltage

//LDO Configuration Register B 0x39
bool MAX77650_getADE_LDO(void); //Returns LDO active-Discharge Enable
uint8_t MAX77650_getEN_LDO(void); //Enable Control for LDO.

//SIMO Buck-Boost Global Configuration Register 0x28
bool MAX77650_getMRT_OTP(void); //Returns Manual Reset Time Configuration
bool MAX77650_getSBIA_LPM_DEF(void); //Returns Default voltage of the SBIA_LPM bit
bool MAX77650_getDBNC_nEN_DEF(void); //Returns Default Value of the DBNC_nEN bit
uint8_t MAX77650_getDRV_SBB(void); //Returns the SIMO Buck-Boost (all channels) Drive Strength Trim.

//SIMO Buck-Boost 0 Configuration Register A 0x29
uint8_t MAX77650_getIP_SBB0(void); //Returns the SIMO Buck-Boost Channel 0 Peak Current Limit
uint8_t MAX77650_getTV_SBB0(void); //Returns the SIMO Buck-Boost Channel 0 Target Output Voltage

//SIMO Buck-Boost 0 Configuration Register B 0x2A
bool MAX77650_getADE_SBB0(void); //Returns SIMO Buck-Boost Channel 0 Active-Discharge Enable
uint8_t MAX77650_getEN_SBB0(void); //Returns the Enable Control for SIMO Buck-Boost Channel 0.

//SIMO Buck-Boost 1 Configuration Register A 0x2B
uint8_t MAX77650_getIP_SBB1(void); //Returns the SIMO Buck-Boost Channel 1 Peak Current Limit
uint8_t MAX77650_getTV_SBB1(void); //Returns the SIMO Buck-Boost Channel 1 Target Output Voltage

//SIMO Buck-Boost 1 Configuration Register B 0x2C
bool MAX77650_getADE_SBB1(void); //Returns SIMO Buck-Boost Channel 1 Active-Discharge Enable
uint8_t MAX77650_getEN_SBB1(void); //Returns the Enable Control for SIMO Buck-Boost Channel 1.

//SIMO Buck-Boost 2 Configuration Register A 0x2D
uint8_t MAX77650_getIP_SBB2(void); //Returns the SIMO Buck-Boost Channel 2 Peak Current Limit
uint8_t MAX77650_getTV_SBB2(void); //Returns the SIMO Buck-Boost Channel 2 Target Output Voltage

//SIMO Buck-Boost 2 Configuration Register B 0x2E
bool MAX77650_getADE_SBB2(void); //Returns SIMO Buck-Boost Channel 2 Active-Discharge Enable
uint8_t MAX77650_getEN_SBB2(void); //Returns the Enable Control for SIMO Buck-Boost Channel 2.


//***** Register writes *****

//LDO Configuration Register A 0x38
bool MAX77650_setTV_LDO(uint8_t val); //Sets the LDO Target Output Voltage

//LDO Configuration Register B 0x39
bool MAX77650_setADE_LDO(uint8_t val); //Sets LDO active-Discharge Enable
bool MAX77650_setEN_LDO(uint8_t val); //Enable LDO

//Charger Configuration Register A 0x18
bool MAX77650_setTHM_HOT(uint8_t val); //Sets the VHOT JEITA Temperature Threshold
bool MAX77650_setTHM_WARM(uint8_t val); //Sets the VWARM JEITA Temperature Threshold
bool MAX77650_setTHM_COOL(uint8_t val); //Sets the VCOOL JEITA Temperature Threshold
bool MAX77650_setTHM_COLD(uint8_t val); //Sets the VCOLD JEITA Temperature Threshold

//Charger Configuration Register B 0x19
bool MAX77650_setVCHGIN_MIN(uint8_t val); //Sets the Minimum CHGIN regulation voltage (VCHGIN-MIN)
bool MAX77650_setICHGIN_LIM(uint8_t val); //Sets the CHGIN Input Current Limit (ICHGIN-LIM)
bool MAX77650_setI_PQ(bool); //Sets the prequalification charge current (IPQ) as a percentage of IFAST-CHG
bool MAX77650_setCHG_EN(bool); //Sets Charger Enable

//Charger Configuration Register C 0x1A
bool MAX77650_setCHG_PQ(uint8_t val); //Sets the Battery prequalification voltage threshold (VPQ)
bool MAX77650_setI_TERM(uint8_t val); //Sets the Charger Termination Current (ITERM). I_TERM[1:0] sets the charger termination current as a percentage of the fast charge current IFAST-CHG.
bool MAX77650_setT_TOPOFF(uint8_t val); //Sets the Topoff timer value (tTO)

//Charger Configuration Register D 0x1B
bool MAX77650_setTJ_REG(uint8_t val); //Sets the die junction temperature regulation point, TJ-REG
bool MAX77650_setVSYS_REG(uint8_t val); //Sets the System voltage regulation (VSYS-REG)

//Charger Configuration Register E 0x1C
bool MAX77650_setCHG_CC(uint8_t val); //Sets the fast-charge constant current value, IFAST-CHG.
bool MAX77650_setT_FAST_CHG(uint8_t val); //Sets the fast-charge safety timer, tFC.

//Charger Configuration Register F 0x1D
bool MAX77650_setCHG_CC_JEITA(uint8_t val); //Sets the IFAST-CHG_JEITA for when the battery is either cool or warm as defined by the TCOOL and TWARM temperature thresholds. This register is a don't care if the battery temperature is normal.
bool MAX77650_setTHM_EN(bool); //Sets the Thermistor enable bit

//Charger Configuration Register G 0x1E
bool MAX77650_setCHG_CV(uint8_t val); //Sets the fast-charge battery regulation voltage, VFAST-CHG.
bool MAX77650_setUSBS(bool); //this bit places CHGIN in USB suspend mode

//Charger Configuration Register H 0x1F
bool MAX77650_setCHG_CV_JEITA(uint8_t val); //Sets the modified VFAST-CHG for when the battery is either cool or warm as defined by the TCOOL and TWARM temperature thresholds. This register is a don't care if the battery temperature is normal.

//Charger Configuration Register I 0x20
bool MAX77650_setIMON_DISCHG_SCALE(uint8_t val); //Sets the Selects the battery discharge current full-scale current value This 4-bit configuration starts at 7.5mA and ends at 300mA.
bool MAX77650_setMUX_SEL(uint8_t val); //Sets the analog channel to connect to AMUX configuration

//SIMO Buck-Boost Global Configuration Register 0x28
bool MAX77650_setMRT_OTP(bool); //Sets Manual Reset Time Configuration
bool MAX77650_setSBIA_LPM_DEF(bool); //Sets Default voltage of the SBIA_LPM bit
bool MAX77650_setDBNC_nEN_DEF(bool); //Sets Default Value of the DBNC_nEN bit
bool MAX77650_setDRV_SBB(uint8_t val); //Sets the SIMO Buck-Boost (all channels) Drive Strength Trim.

//SIMO Buck-Boost 0 Configuration Register A 0x29
bool MAX77650_setIP_SBB0(uint8_t val); //Sets the SIMO Buck-Boost Channel 0 Peak Current Limit
bool MAX77650_setTV_SBB0(uint8_t val); //Sets the SIMO Buck-Boost Channel 0 Target Output Voltage

//SIMO Buck-Boost 0 Configuration Register B 0x2A
bool MAX77650_setADE_SBB0(bool); //Sets SIMO Buck-Boost Channel 0 Active-Discharge Enable
bool MAX77650_setEN_SBB0(uint8_t val); //Sets the Enable Control for SIMO Buck-Boost Channel 0.

//SIMO Buck-Boost 1 Configuration Register A 0x2B
bool MAX77650_setIP_SBB1(uint8_t val); //Sets the SIMO Buck-Boost Channel 1 Peak Current Limit
bool MAX77650_setTV_SBB1(uint8_t val); //Sets the SIMO Buck-Boost Channel 1 Target Output Voltage

//SIMO Buck-Boost 1 Configuration Register B 0x2C
bool MAX77650_setADE_SBB1(bool); //Sets SIMO Buck-Boost Channel 1 Active-Discharge Enable
bool MAX77650_setEN_SBB1(uint8_t val); //Sets the Enable Control for SIMO Buck-Boost Channel 1.

//SIMO Buck-Boost 2 Configuration Register A 0x2D
bool MAX77650_setIP_SBB2(uint8_t val); //Sets the SIMO Buck-Boost Channel 2 Peak Current Limit
bool MAX77650_setTV_SBB2(uint8_t val); //Sets the SIMO Buck-Boost Channel 2 Target Output Voltage

//SIMO Buck-Boost 2 Configuration Register B 0x2E
bool MAX77650_setADE_SBB2(bool); //Sets SIMO Buck-Boost Channel 2 Active-Discharge Enable
bool MAX77650_setEN_SBB2(uint8_t val); //Sets the Enable Control for SIMO Buck-Boost Channel 2.

//LED related registers
bool MAX77650_setEN_LED_MSTR(bool); //Sets Master LED Enable Bit
bool MAX77650_setLED_FS0(uint8_t val); //Sets LED0 Full Scale Range
bool MAX77650_setINV_LED0(bool); //Sets LED0 Invert
bool MAX77650_setBRT_LED0(uint8_t val); //Sets LED0 Brightness Control
bool MAX77650_setP_LED0(uint8_t val); //Sets LED0 Period Settings
bool MAX77650_setD_LED0(uint8_t val); //Sets LED0 On Duty-Cycle Settings
bool MAX77650_setLED_FS1(uint8_t val); //Sets LED1 Full Scale Range
bool MAX77650_setINV_LED1(bool); //Sets LED1 Invert
bool MAX77650_setBRT_LED1(uint8_t val); //Sets LED1 Brightness Control
bool MAX77650_setP_LED1(uint8_t val); //Sets LED1 Period Settings
bool MAX77650_setD_LED1(uint8_t val); //Sets LED1 On Duty-Cycle Settings
bool MAX77650_setLED_FS2(uint8_t val); //Sets LED2 Full Scale Range
bool MAX77650_setINV_LED2(bool); //Sets LED2 Invert
bool MAX77650_setBRT_LED2(uint8_t val); //Sets LED2 Brightness Control
bool MAX77650_setP_LED2(uint8_t val); //Sets LED2 Period Settings
bool MAX77650_setD_LED2(uint8_t val); //Sets LED2 On Duty-Cycle Settings

//Global Configuration register 0x10
bool MAX77650_setBOK(bool); //Sets Main Bias Okay Status Bit
bool MAX77650_setSBIA_LPM(bool); //Sets Main Bias Low-Power Mode software request
bool MAX77650_setSBIA_EN(bool); //Sets Main Bias Enable Software Request
bool MAX77650_setnEN_MODE(bool); //Sets nEN Input (ONKEY) Default Configuration Mode
bool MAX77650_setDBEN_nEN(bool); //Sets Debounce Timer Enable for the nEN Pin
bool MAX77650_setSFT_RST(uint8_t val); //Sets Software Reset Functions.

//Global Interrupt Mask Register 0x06
bool MAX77650_setINT_M_GLBL(uint8_t val); //Sets Global Interrupt Mask Register

//GPIO Configuration Register
bool MAX77650_setDBEN_GPI(bool); //Sets General Purpose Input Debounce Timer Enable
bool MAX77650_setDO(bool); //Sets General Purpose Output Data Output
bool MAX77650_setDRV(bool); //Sets General Purpose Output Driver Type
bool MAX77650_setDI(bool); //Sets GPIO Digital Input Value
bool MAX77650_setDIR(bool); //Sets GPIO Direction

//Charger Interrupt Mask Register 0x07
bool MAX77650_setINT_M_CHG(uint8_t val); //Sets Global Interrupt Mask Register


bool pmu_init(void);
int enableLDO(void);
int disableLDO(void);
int setSBB2Voltage(float tv_v);
bool pmuIsCharging(void);

#endif