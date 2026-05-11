/*
 * HM01B0_I2c.c
 *
 *  Created on: Sept 15, 2018
 *      Author: Ali Najafi
 */

#include "i2c.h"
#include "HM01B0_FUNC.h"
#include "HM01B0_BLE_DEFINES.h"

void hm01b0_init_fixed_rom_qvga_fixed(void)
{
  i2cWrite16(CAMERA_I2C_ADDR, REG_MODE_SELECT, 0x00);                           // go to stand by mode
  i2cWrite16(CAMERA_I2C_ADDR, REG_ANA_REGISTER_17, 0x00);                       // clk source(osc:1 mclk:0)
  i2cWrite16(CAMERA_I2C_ADDR, REG_TEST_PATTERN_MODE, 0x0);
  // i2cWrite16(CAMERA_I2C_ADDR, REG_TEST_PATTERN_MODE, 0x11);
  i2cWrite16(CAMERA_I2C_ADDR, REG_IO_DRIVE_STR, 0xFF);

#ifdef QQVGA
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_MODE, 0x3);
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_RDOUT_X, 0x03); // Horizontal Binning enable
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_RDOUT_Y, 0x03); // vertical Binning enable
  i2cWrite16(CAMERA_I2C_ADDR, REG_FRAME_LENGTH_LINES_H, 0x0);
  i2cWrite16(CAMERA_I2C_ADDR, REG_FRAME_LENGTH_LINES_L, 0x80);
  i2cWrite16(CAMERA_I2C_ADDR, REG_LINE_LENGTH_PCLK_H, 0x0);
  i2cWrite16(CAMERA_I2C_ADDR, REG_LINE_LENGTH_PCLK_L, 0xD7);

  i2cWrite16(CAMERA_I2C_ADDR, REG_QVGA_WIN_EN, 0x01); // Set line length LSB to QQVGA => enabled: makes the image 160(row)*240(col)

  i2cWrite16(CAMERA_I2C_ADDR, REG_GRP_PARAM_HOLD, 0x1);
#else
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_MODE, 0x0);
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_RDOUT_X, 0x01);          // Horizontal Binning enable
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_RDOUT_Y, 0x01);          // vertical Binning enable
  i2cWrite16(CAMERA_I2C_ADDR, REG_FRAME_LENGTH_LINES_H, 0x01); // changed by Ali
  i2cWrite16(CAMERA_I2C_ADDR, REG_FRAME_LENGTH_LINES_L, 0x04); // changed by Ali
  i2cWrite16(CAMERA_I2C_ADDR, REG_LINE_LENGTH_PCLK_H, 0x01);
  i2cWrite16(CAMERA_I2C_ADDR, REG_LINE_LENGTH_PCLK_L, 0x78); // changed by Ali
  i2cWrite16(CAMERA_I2C_ADDR, REG_QVGA_WIN_EN, 0x01); // Set line length LSB to QQVGA => enabled: makes the image 160(row)*240(col)
#endif

  // Unknown Regs
  i2cWrite16(CAMERA_I2C_ADDR, 0x0350, 0x7F);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3044, 0x0A);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3045, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3047, 0x0A);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3050, 0xC0);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3051, 0x42);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3052,0x50);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3053, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3054, 0x03);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3055, 0xF7);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3056, 0xF8);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3057, 0x29);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3058, 0x1F);
  i2cWrite16(CAMERA_I2C_ADDR, REG_ANA_REGISTER_14, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1008, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1009, 0xA0);
  i2cWrite16(CAMERA_I2C_ADDR, 0x100A, 0x60);
  i2cWrite16(CAMERA_I2C_ADDR, 0x100B, 0x90);
  i2cWrite16(CAMERA_I2C_ADDR, 0x100C, 0x40);

  // black level control
  i2cWrite16(CAMERA_I2C_ADDR, 0x1000, 0x43);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1001, 0x40);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1002, 0x32);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1003, 0x08); // default from lattice 0x08
  i2cWrite16(CAMERA_I2C_ADDR, 0x1006, 0x01);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1007, 0x08); // default from lattice 0x08

  // Vsync, hsync and pixel shift register
  i2cWrite16(CAMERA_I2C_ADDR, 0x1012, 0x00); // lattice value

  // Statistic control and read only
  i2cWrite16(CAMERA_I2C_ADDR, 0x2000, 0x07);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2003, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2004, 0x1C);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2007, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2008, 0x58);
  i2cWrite16(CAMERA_I2C_ADDR, 0x200B, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x200C, 0x7A);
  i2cWrite16(CAMERA_I2C_ADDR, 0x200F, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2010, 0xB8);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2013, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2014, 0x58);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2017, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2018, 0x9B);

  // Automatic exposure gain control - TODO: try changing these
  i2cWrite16(CAMERA_I2C_ADDR, 0x2100, 0x01); // Auto Exposure: 1-On 0-Off
  i2cWrite16(CAMERA_I2C_ADDR, 0x2101, 0x70); // 0x70);//lattice 0xA0 Brightness control!
  i2cWrite16(CAMERA_I2C_ADDR, 0x2102, 0x06); // lattice 0x06
  i2cWrite16(CAMERA_I2C_ADDR, 0x2104, 0x07);
  i2cWrite16(CAMERA_I2C_ADDR, REG_MAX_INTG_H, 0x03);
  i2cWrite16(CAMERA_I2C_ADDR, REG_MAX_INTG_L, 0xA4);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2108, 0x33);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2109, 0x33);
  i2cWrite16(CAMERA_I2C_ADDR, 0x210A, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x210B, 0x80);
  // i2cWrite16(CAMERA_I2C_ADDR, 0x210C, 0x04);
  i2cWrite16(CAMERA_I2C_ADDR, 0x210F, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2110, 0xE9);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2111, 0x01);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2112, 0x17);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2150, 0x03);

  // Sensor exposure gain
  i2cWrite16(CAMERA_I2C_ADDR, 0x0202, 0x01); // Vikram
  i2cWrite16(CAMERA_I2C_ADDR, 0x0203, 0x08); // Vikram
  i2cWrite16(CAMERA_I2C_ADDR, 0x0205, 0x05); // Vikram - Analog Gain
  i2cWrite16(CAMERA_I2C_ADDR, 0x020E, 0x01); // Vikram - Digital Gain
  i2cWrite16(CAMERA_I2C_ADDR, 0x020F, 0x00); // Vikram - Digital Gain

  i2cWrite16(CAMERA_I2C_ADDR, REG_OSC_CLK_DIV, 0x30);               // This is effective when we use external clk, Use the camera in the gated clock mode to make the clock zero when there is no data
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIT_CONTROL, 0x20);               // Set the output to send 1 bit serial
  i2cWrite16(CAMERA_I2C_ADDR, REG_PMU_PROGRAMMABLE_FRAMECNT, 0x01); // set the number of frames to be sent out, it sends N frames
  // i2cWrite16(CAMERA_I2C_ADDR, REG_TEST_PATTERN_MODE, 0x01); // Test mode enable
}

void hm01b0_init_optimized_vlm(void)
{
  i2cWrite16(CAMERA_I2C_ADDR, REG_MODE_SELECT, 0x00);     // go to stand by mode
  i2cWrite16(CAMERA_I2C_ADDR, REG_ANA_REGISTER_17, 0x00); // clk source(osc:1 mclk:0)
  i2cWrite16(CAMERA_I2C_ADDR, REG_TEST_PATTERN_MODE, 0x0);
  i2cWrite16(CAMERA_I2C_ADDR, REG_IO_DRIVE_STR, 0xFF);

#ifdef QQVGA
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_MODE, 0x3);
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_RDOUT_X, 0x03); // Horizontal Binning enable
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_RDOUT_Y, 0x03); // vertical Binning enable
  i2cWrite16(CAMERA_I2C_ADDR, REG_FRAME_LENGTH_LINES_H, 0x0);
  i2cWrite16(CAMERA_I2C_ADDR, REG_FRAME_LENGTH_LINES_L, 0x80);
  i2cWrite16(CAMERA_I2C_ADDR, REG_LINE_LENGTH_PCLK_H, 0x0);
  i2cWrite16(CAMERA_I2C_ADDR, REG_LINE_LENGTH_PCLK_L, 0xD7);
  i2cWrite16(CAMERA_I2C_ADDR, REG_QVGA_WIN_EN, 0x01);
  i2cWrite16(CAMERA_I2C_ADDR, REG_GRP_PARAM_HOLD, 0x1);
#else
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_MODE, 0x0);
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_RDOUT_X, 0x01);
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIN_RDOUT_Y, 0x01);
  i2cWrite16(CAMERA_I2C_ADDR, REG_FRAME_LENGTH_LINES_H, 0x01);
  i2cWrite16(CAMERA_I2C_ADDR, REG_FRAME_LENGTH_LINES_L, 0x04);
  i2cWrite16(CAMERA_I2C_ADDR, REG_LINE_LENGTH_PCLK_H, 0x01);
  i2cWrite16(CAMERA_I2C_ADDR, REG_LINE_LENGTH_PCLK_L, 0x78);
  i2cWrite16(CAMERA_I2C_ADDR, REG_QVGA_WIN_EN, 0x01);
#endif

  // Unknown Regs (keeping your existing values)
  i2cWrite16(CAMERA_I2C_ADDR, 0x0350, 0x7F);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3044, 0x0A);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3045, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3047, 0x0A);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3050, 0xC0);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3051, 0x42);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3052, 0x50);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3053, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3054, 0x03);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3055, 0xF7);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3056, 0xF8);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3057, 0x29);
  i2cWrite16(CAMERA_I2C_ADDR, 0x3058, 0x1F);
  i2cWrite16(CAMERA_I2C_ADDR, REG_ANA_REGISTER_14, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1008, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1009, 0xA0);
  i2cWrite16(CAMERA_I2C_ADDR, 0x100A, 0x60);
  i2cWrite16(CAMERA_I2C_ADDR, 0x100B, 0x90);
  i2cWrite16(CAMERA_I2C_ADDR, 0x100C, 0x40);

  // black level control
  i2cWrite16(CAMERA_I2C_ADDR, 0x1000, 0x43);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1001, 0x40);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1002, 0x32);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1003, 0x08);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1006, 0x01);
  i2cWrite16(CAMERA_I2C_ADDR, 0x1007, 0x08);

  // Vsync, hsync and pixel shift register
  i2cWrite16(CAMERA_I2C_ADDR, 0x1012, 0x00);

  // Statistic control and read only
  i2cWrite16(CAMERA_I2C_ADDR, 0x2000, 0x07);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2003, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2004, 0x1C);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2007, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2008, 0x58);
  i2cWrite16(CAMERA_I2C_ADDR, 0x200B, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x200C, 0x7A);
  i2cWrite16(CAMERA_I2C_ADDR, 0x200F, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2010, 0xB8);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2013, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2014, 0x58);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2017, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2018, 0x9B);

  // OPTIMIZED Automatic exposure gain control for VLM applications
  i2cWrite16(CAMERA_I2C_ADDR, 0x2100, 0x01); // Auto Exposure: 1-On 0-Off
  i2cWrite16(CAMERA_I2C_ADDR, 0x2101, 0x60); // Target brightness (was 0x70) - lower for better contrast
  i2cWrite16(CAMERA_I2C_ADDR, 0x2102, 0x06); // Min mean

  // More aggressive convergence for faster response
  i2cWrite16(CAMERA_I2C_ADDR, 0x2103, 0x05); // CONVERGE_IN_TH (was 0x03)
  i2cWrite16(CAMERA_I2C_ADDR, 0x2104, 0x08); // CONVERGE_OUT_TH (was 0x05)

  // Reduced max integration time for motion blur reduction
  i2cWrite16(CAMERA_I2C_ADDR, REG_MAX_INTG_H, 0x01); // was 0x03
  i2cWrite16(CAMERA_I2C_ADDR, REG_MAX_INTG_L, 0xA0); // was 0xA4

  // Minimum integration time - fastest possible
  i2cWrite16(CAMERA_I2C_ADDR, 0x2107, 0x01); // MIN_INTG (was 0x02)

  // Higher gain limits to compensate for faster shutter
  i2cWrite16(CAMERA_I2C_ADDR, 0x2108, 0x30); // MAX_AGAIN_FULL - 8x gain
  i2cWrite16(CAMERA_I2C_ADDR, 0x2109, 0x30); // MAX_AGAIN_BIN2 - 8x gain
  i2cWrite16(CAMERA_I2C_ADDR, 0x210A, 0x00); // MIN_AGAIN
  i2cWrite16(CAMERA_I2C_ADDR, 0x210B, 0xFF); // MAX_DGAIN - increased for low light
  i2cWrite16(CAMERA_I2C_ADDR, 0x210C, 0x40); // MIN_DGAIN

  // More aggressive damping for faster AE response
  i2cWrite16(CAMERA_I2C_ADDR, 0x210D, 0x10); // DAMPING_FACTOR (was 0x20)

  i2cWrite16(CAMERA_I2C_ADDR, 0x210F, 0x00);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2110, 0xE9);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2111, 0x01);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2112, 0x17);
  i2cWrite16(CAMERA_I2C_ADDR, 0x2150, 0x03);

  // Sensor exposure gain - optimized starting values
  i2cWrite16(CAMERA_I2C_ADDR, 0x0202, 0x00); // Start with faster integration
  i2cWrite16(CAMERA_I2C_ADDR, 0x0203, 0x20); // 32 rows integration time
  i2cWrite16(CAMERA_I2C_ADDR, 0x0205, 0x10); // 2x Analog Gain (was 0x05)
  i2cWrite16(CAMERA_I2C_ADDR, 0x020E, 0x01); // Digital Gain
  i2cWrite16(CAMERA_I2C_ADDR, 0x020F, 0x40); // Digital Gain (increased from 0x00)

  // OPTIMIZATION: Enable early gain application for faster AE response
  i2cWrite16(CAMERA_I2C_ADDR, 0x3035, 0x80); // Enable early gain (N+1 instead of N+2)

  i2cWrite16(CAMERA_I2C_ADDR, REG_OSC_CLK_DIV, 0x30);
  i2cWrite16(CAMERA_I2C_ADDR, REG_BIT_CONTROL, 0x20);
  i2cWrite16(CAMERA_I2C_ADDR, REG_PMU_PROGRAMMABLE_FRAMECNT, 0x01);
}

////////////////////////////////////////////////
