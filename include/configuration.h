#pragma once

#include <stdint.h>

// Pantalla TFT ILI9341 integrada.
static const int TFT_PIN_MISO = 13;
static const int TFT_PIN_MOSI = 11;
static const int TFT_PIN_SCLK = 12;
static const int TFT_PIN_CS = 10;
static const int TFT_PIN_DC = 46;
static const int TFT_PIN_RST = -1;
static const int TFT_PIN_BL = 45;

// Touch capacitivo I2C CST816S/D.
static const int TOUCH_I2C_SDA = 16;
static const int TOUCH_I2C_SCL = 15;
static const int TOUCH_INT_PIN = 17;
static const int TOUCH_RST_PIN = 18;
static const uint8_t TOUCH_ADDR_PRIMARY = 0x15;
static const uint8_t TOUCH_ADDR_FALLBACK = 0x38;

// SD via SD_MMC en modo 1-bit, no usa CS SPI.
static const int SDMMC_CLK_PIN = 38;
static const int SDMMC_CMD_PIN = 40;
static const int SDMMC_D0_PIN = 39;

// LED RGB WS2812B integrado.
static const int RGB_LED_PIN = 42;
