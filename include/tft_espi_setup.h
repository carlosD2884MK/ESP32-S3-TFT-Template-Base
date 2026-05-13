#pragma once

#include "configuration.h"

#define USER_SETUP_INFO "pantalla_wiegand"

#define ILI9341_DRIVER

#define TFT_MISO TFT_PIN_MISO
#define TFT_MOSI TFT_PIN_MOSI
#define TFT_SCLK TFT_PIN_SCLK

#define TFT_CS TFT_PIN_CS
#define TFT_DC TFT_PIN_DC
#define TFT_RST TFT_PIN_RST

// El touch de esta placa es I2C; TOUCH_CS queda sin uso practico aqui.
#define TOUCH_CS 3

#define USE_HSPI_PORT

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 20000000
#define SPI_TOUCH_FREQUENCY 2500000