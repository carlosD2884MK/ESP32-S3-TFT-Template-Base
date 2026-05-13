#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SD_MMC.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_NeoPixel.h>
#include <HardwareSerial.h>

#if defined(CONFIG_IDF_TARGET_ESP32S3) && defined(ARDUINO_USB_MODE) && !ARDUINO_USB_CDC_ON_BOOT
#include <HWCDC.h>
#define HAS_USB_SERIAL_JTAG 1
#else
#define HAS_USB_SERIAL_JTAG 0
#endif

#include "configuration.h"

namespace {

#if TOUCH_DEBUG_ENABLED
	constexpr uint8_t CST816_REG_FINGER_NUM = 0x02;
	constexpr uint8_t CST816_REG_XPOS_H = 0x03;
	constexpr uint8_t CST816_REG_XPOS_L = 0x04;
	constexpr uint8_t CST816_REG_YPOS_H = 0x05;
	constexpr uint8_t CST816_REG_YPOS_L = 0x06;
	constexpr int16_t TOUCH_BOX_WIDTH = 104;
	constexpr int16_t TOUCH_BOX_HEIGHT = 34;
	constexpr int16_t TOUCH_BOX_X = 320 - TOUCH_BOX_WIDTH - 8;
	constexpr int16_t TOUCH_BOX_Y = 240 - TOUCH_BOX_HEIGHT - 8;
#endif

Adafruit_ILI9341 tft(&SPI, TFT_PIN_DC, TFT_PIN_CS, TFT_PIN_RST);
Adafruit_NeoPixel statusLed(1, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

constexpr uint16_t COLOR_BG = ILI9341_BLACK;
constexpr uint16_t COLOR_TEXT = ILI9341_WHITE;
bool touchReadyState = false;
bool sdReadyState = false;
bool sdReadWriteOkState = false;

#if TOUCH_DEBUG_ENABLED
	constexpr uint16_t COLOR_CURSOR = ILI9341_YELLOW;

	bool touchAvailable = false;

	uint8_t touchAddress = TOUCH_ADDR_PRIMARY;

	struct TouchPoint {
	bool pressed = false;
	uint16_t rawX = 0;
	uint16_t rawY = 0;
	int16_t screenX = 0;
	int16_t screenY = 0;
	};
#endif

Print *debugPort = &Serial;

void setStatusLed(uint8_t red, uint8_t green, uint8_t blue) {
	statusLed.setPixelColor(0, statusLed.Color(red, green, blue));
	statusLed.show();
}

void initStatusLed() {
	statusLed.begin();
	statusLed.clear();
	statusLed.show();
}

void initDebugPort() {
#if HAS_USB_SERIAL_JTAG
  USBSerial.begin(115200);
  USBSerial.setTxTimeoutMs(20);
  debugPort = &USBSerial;
#else
  Serial.begin(115200);
  debugPort = &Serial;
#endif
}

void logBoot(const char *message) {
  debugPort->println(message);
}

void drawCenteredText(const char *text, int16_t topY, uint8_t textSize, uint16_t color) {
  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t width = 0;
  uint16_t height = 0;

  tft.setTextSize(textSize);
  tft.getTextBounds(text, 0, topY, &x1, &y1, &width, &height);

  int16_t x = (tft.width() - static_cast<int16_t>(width)) / 2;
  if (x < 0) {
    x = 0;
  }

  tft.setCursor(x, topY);
  tft.setTextColor(color, COLOR_BG);
  tft.print(text);
}

bool probeI2cAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

#if TOUCH_DEBUG_ENABLED
	bool readTouchRegisters(uint8_t startRegister, uint8_t *buffer, size_t length) {
	Wire.beginTransmission(touchAddress);
	Wire.write(startRegister);
	if (Wire.endTransmission(false) != 0) {
		return false;
	}

	size_t received = Wire.requestFrom(static_cast<int>(touchAddress), static_cast<int>(length));
	if (received != length) {
		while (Wire.available()) {
		Wire.read();
		}
		return false;
	}

	for (size_t index = 0; index < length; ++index) {
		buffer[index] = Wire.read();
	}

	return true;
	}

	int16_t clampCoord(int32_t value, int16_t maxValue) {
	if (value < 0) {
		return 0;
	}
	if (value > maxValue) {
		return maxValue;
	}
	return static_cast<int16_t>(value);
	}

	bool readTouchPoint(TouchPoint &point) {
	uint8_t data[5] = {};
	if (!readTouchRegisters(CST816_REG_FINGER_NUM, data, sizeof(data))) {
		return false;
	}

	uint8_t fingers = data[0] & 0x0F;
	if (fingers == 0) {
		point.pressed = false;
		return true;
	}

	point.pressed = true;
	point.rawX = static_cast<uint16_t>(((data[1] & 0x0F) << 8) | data[2]);
	point.rawY = static_cast<uint16_t>(((data[3] & 0x0F) << 8) | data[4]);

	point.screenX = clampCoord(map(point.rawY, 0, 319, 0, tft.width() - 1), tft.width() - 1);
	point.screenY = clampCoord(map(point.rawX, 0, 239, tft.height() - 1, 0), tft.height() - 1);
	return true;
	}
#endif

bool initTouch() {
  Wire.begin(TOUCH_I2C_SDA, TOUCH_I2C_SCL);
  Wire.setClock(100000);

  pinMode(TOUCH_RST_PIN, OUTPUT);
  digitalWrite(TOUCH_RST_PIN, LOW);
  delay(10);
  digitalWrite(TOUCH_RST_PIN, HIGH);
  delay(50);

  pinMode(TOUCH_INT_PIN, INPUT_PULLUP);

  if (probeI2cAddress(TOUCH_ADDR_PRIMARY)) {
	#if TOUCH_DEBUG_ENABLED
		touchAddress = TOUCH_ADDR_PRIMARY;
		touchAvailable = true;
	#endif
    return true;
  }

  if (probeI2cAddress(TOUCH_ADDR_FALLBACK)) {
	#if TOUCH_DEBUG_ENABLED
		touchAddress = TOUCH_ADDR_FALLBACK;
		touchAvailable = true;
	#endif
    return true;
  }

	#if TOUCH_DEBUG_ENABLED
	touchAvailable = false;
	#endif
  return false;
}

bool initSdCard() {
	if (!SD_MMC.setPins(SDMMC_CLK_PIN, SDMMC_CMD_PIN, SDMMC_D0_PIN)) {
		debugPort->println("[SD] No se pudieron asignar los pines SD_MMC");
    return false;
  }

  static constexpr uint8_t kMaxSdAttempts = 3;
  for (uint8_t attempt = 1; attempt <= kMaxSdAttempts; ++attempt) {
    if (SD_MMC.begin("/sdcard", true, false, SDMMC_FREQ_DEFAULT)) {
      if (attempt > 1) {
				debugPort->printf("[SD] Inicializacion exitosa en intento %u\n", attempt);
      }
      return true;
    }

		debugPort->printf("[SD] Fallo al inicializar la tarjeta, intento %u/%u\n", attempt, kMaxSdAttempts);
    SD_MMC.end();
    delay(150);
  }

	debugPort->println("[SD] No fue posible montar la SD en modo 1-bit");
  return false;
}

bool testSdCardReadWrite() {
	static constexpr const char *kTestPath = "/test.txt";
	static constexpr const char *kExpectedContent = "MKJoules SD test";

	File writeFile = SD_MMC.open(kTestPath, FILE_WRITE);
	if (!writeFile) {
		debugPort->println("[SD] No se pudo abrir test.txt para escritura");
		return false;
	}

	writeFile.seek(0);
	writeFile.print(kExpectedContent);
	writeFile.flush();
	writeFile.close();

	File readFile = SD_MMC.open(kTestPath, FILE_READ);
	if (!readFile) {
		debugPort->println("[SD] No se pudo abrir test.txt para lectura");
		return false;
	}

	String actualContent = readFile.readString();
	readFile.close();

	actualContent.trim();
	bool matches = actualContent == kExpectedContent;
	debugPort->printf("[SD] test.txt => %s\n", actualContent.c_str());
	debugPort->println(matches ? "[SD] Prueba escritura/lectura OK" : "[SD] Prueba escritura/lectura ERROR");
	return matches;
}

void initDisplay() {
  pinMode(TFT_PIN_BL, OUTPUT);
  digitalWrite(TFT_PIN_BL, HIGH);

  SPI.begin(TFT_PIN_SCLK, TFT_PIN_MISO, TFT_PIN_MOSI, TFT_PIN_CS);
  tft.begin(40000000);
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);
  tft.setTextWrap(false);
}

void drawSplashImage() {
  tft.fillScreen(COLOR_BG);

  drawCenteredText("MKJoules USA Corp.", 88, 3, ILI9341_CYAN);
  drawCenteredText("Pantalla Wiegand", 138, 2, COLOR_TEXT);
  drawCenteredText("Inicializando hardware...", 172, 2, ILI9341_CYAN);
}

void drawStatus(bool touchReady, bool sdReady, bool sdReadWriteOk) {
  tft.fillScreen(COLOR_BG);

  tft.setTextSize(3);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setCursor(12, 12);
  tft.print("Pantalla Wiegand");

  tft.setTextSize(2);
  tft.setCursor(12, 46);
  tft.print("Base de hardware");

  tft.setTextColor(touchReady ? ILI9341_GREEN : ILI9341_RED, COLOR_BG);
  tft.setCursor(12, 90);
  tft.print(touchReady ? "Touch I2C: OK" : "Touch I2C: no detectado");

  tft.setTextColor(sdReady ? ILI9341_GREEN : ILI9341_RED, COLOR_BG);
  tft.setCursor(12, 118);
  tft.print(sdReady ? "SD_MMC: OK" : "SD_MMC: error");

	tft.setTextColor(sdReadWriteOk ? ILI9341_GREEN : ILI9341_RED, COLOR_BG);
	tft.setCursor(12, 146);
	tft.print(sdReadWriteOk ? "SD test.txt: OK" : "SD test.txt: error");

  tft.setTextColor(ILI9341_CYAN, COLOR_BG);
	tft.setCursor(12, 174);
  tft.print("Backlight y TFT inicializados");
	tft.setCursor(12, 202);
  tft.print("Monitor: 115200 baudios");
}

#if TOUCH_DEBUG_ENABLED
	void drawTouchCursorMarker(const TouchPoint &point) {
	if (!point.pressed) {
		return;
	}

	tft.drawCircle(point.screenX, point.screenY, 10, COLOR_CURSOR);
	tft.drawLine(point.screenX - 8, point.screenY - 8, point.screenX + 8, point.screenY + 8, COLOR_CURSOR);
	tft.drawLine(point.screenX - 8, point.screenY + 8, point.screenX + 8, point.screenY - 8, COLOR_CURSOR);
	}

	void drawTouchCoordinatesBox(const TouchPoint &point) {
		tft.fillRect(TOUCH_BOX_X, TOUCH_BOX_Y, TOUCH_BOX_WIDTH, TOUCH_BOX_HEIGHT, COLOR_BG);
		tft.drawRect(TOUCH_BOX_X, TOUCH_BOX_Y, TOUCH_BOX_WIDTH, TOUCH_BOX_HEIGHT, ILI9341_DARKCYAN);
		tft.setTextSize(1);
		tft.setTextColor(COLOR_TEXT, COLOR_BG);
		tft.setCursor(TOUCH_BOX_X + 6, TOUCH_BOX_Y + 7);
		if (point.pressed) {
			tft.printf("X:%3d Y:%3d", point.screenX, point.screenY);
		} else {
			tft.print("X:--- Y:---");
		}
		tft.setCursor(TOUCH_BOX_X + 6, TOUCH_BOX_Y + 20);
		tft.print(point.pressed ? "touch activo" : "sin toque");
	}

	void drawTouchOverlay(const TouchPoint &point) {
		drawStatus(touchReadyState, sdReadyState, sdReadWriteOkState);
		drawTouchCursorMarker(point);
		drawTouchCoordinatesBox(point);
	}

	void runTouchDebugViewer() {
	static uint32_t lastRefreshMs = 0;
	static bool hadLastPoint = false;
	static TouchPoint lastPoint;
	uint32_t now = millis();
	if (now - lastRefreshMs < 25) {
		return;
	}
	lastRefreshMs = now;

	TouchPoint point;
	if (!readTouchPoint(point)) {
		debugPort->println("[TOUCH] lectura I2C fallida");
		return;
	}

	if (hadLastPoint && point.pressed == lastPoint.pressed && point.screenX == lastPoint.screenX && point.screenY == lastPoint.screenY) {
		return;
	}

	drawTouchOverlay(point);
	lastPoint = point;
	hadLastPoint = true;
	}
#endif

}  // namespace

void setup() {
	initStatusLed();
	setStatusLed(32, 18, 0);

  initDebugPort();
  delay(200);

  logBoot("[BOOT] setup start");

  initDisplay();
	setStatusLed(0, 0, 32);
  logBoot("[BOOT] display init done");
  drawSplashImage();
  delay(1200);

  bool touchReady = initTouch();
	touchReadyState = touchReady;
  logBoot(touchReady ? "[BOOT] touch init ok" : "[BOOT] touch init failed");
  bool sdReady = initSdCard();
	sdReadyState = sdReady;
  logBoot(sdReady ? "[BOOT] sd init ok" : "[BOOT] sd init failed");
  bool sdReadWriteOk = sdReady && testSdCardReadWrite();
	sdReadWriteOkState = sdReadWriteOk;

	#if TOUCH_DEBUG_ENABLED
	drawStatus(touchReady, sdReady, sdReadWriteOk);
	if (touchReady) {
		TouchPoint initialPoint;
		drawTouchOverlay(initialPoint);
		logBoot("[BOOT] debug_touch activo");
	}
	#else
	drawStatus(touchReady, sdReady, sdReadWriteOk);
	#endif

  debugPort->println();
  debugPort->println("[BOOT] Base de hardware inicializada");
  debugPort->printf("[BOOT] Touch I2C: %s\n", touchReady ? "OK" : "NO DETECTADO");
	debugPort->printf("[BOOT] SD_MMC: %s\n", sdReady ? "OK" : "ERROR");
	debugPort->printf("[BOOT] SD test.txt: %s\n", sdReadWriteOk ? "OK" : "ERROR");

	if (touchReady && sdReadWriteOk) {
		setStatusLed(0, 32, 0);
	} else if (touchReady || sdReady) {
		setStatusLed(32, 20, 0);
	} else {
		setStatusLed(32, 0, 0);
	}
}

void loop() {
	#if TOUCH_DEBUG_ENABLED
	if (touchAvailable) {
		runTouchDebugViewer();
	}
	#endif

  static uint32_t lastBeatMs = 0;
  uint32_t now = millis();

  if (now - lastBeatMs >= 5000) {
    lastBeatMs = now;
    //debugPort->println("[LOOP] heartbeat");
  }
}