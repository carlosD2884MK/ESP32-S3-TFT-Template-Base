# ESP32-S3 TFT Template Base

Plantilla pública para arrancar proyectos con una placa ESP32-S3 que integra:

- pantalla TFT ILI9341 por SPI
- touch capacitivo CST816 por I2C
- SD_MMC en modo 1-bit
- LED RGB WS2812B en GPIO 42
- salida serie por USB Serial/JTAG

La idea de este repositorio es evitar volver a resolver lo mismo en cada proyecto nuevo: pines, arranque de pantalla, touch, validación de SD y estado visual del hardware ya quedan listos como base.

Licencia:

- Esta plantilla se publica bajo licencia MIT. Ver [LICENSE](LICENSE).

## Qué incluye

- inicialización funcional de TFT, touch, SD_MMC y LED RGB
- prueba real de escritura y lectura sobre la SD con `test.txt`
- monitor serie listo para USB-C en ESP32-S3
- modo opcional de depuración táctil con `TOUCH_DEBUG_ENABLED`
- estructura de proyecto compatible con PlatformIO

## Arranque rápido

### Usar como plantilla en GitHub

Este repositorio está marcado como template repository, así que puedes crear uno nuevo con `Use this template` desde GitHub y empezar desde una copia limpia.

### Clonar directamente

```bash
git clone https://github.com/carlosD2884MK/ESP32-S3-TFT-Template-Base.git
cd ESP32-S3-TFT-Template-Base
```

### Abrir en VS Code

1. Abre la carpeta del repositorio en VS Code.
2. Asegúrate de tener instalada la extensión de PlatformIO.
3. Espera a que PlatformIO resuelva e instale dependencias.

### Compilar

```bash
pio run -e esp32-s3-devkitc-1
```

### Subir al ESP32-S3

```bash
pio run -e esp32-s3-devkitc-1 --target upload
```

### Abrir monitor serie

```bash
pio device monitor -b 115200
```

Notas:

- Esta base usa USB Serial/JTAG por USB-C.
- Si el puerto no abre, cierra otros monitores serie que puedan estar usándolo.

## Activar depuración táctil

En `platformio.ini`, descomenta esta línea dentro de `build_flags`:

```ini
;-D TOUCH_DEBUG_ENABLED
```

Déjala así:

```ini
-D TOUCH_DEBUG_ENABLED
```

Con eso se activa un overlay sobre la pantalla principal que dibuja el punto tocado y muestra coordenadas X/Y.

## Hardware soportado

### Pantalla TFT ILI9341

- MISO: GPIO 13
- MOSI: GPIO 11
- SCLK: GPIO 12
- CS: GPIO 10
- DC: GPIO 46
- RST: -1
- Backlight: GPIO 45

Notas:

- `RST = -1` indica que no hay pin de reset dedicado.
- El proyecto actual usa `Adafruit_ILI9341`.

### Touch capacitivo CST816S/D

- SDA: GPIO 16
- SCL: GPIO 15
- INT: GPIO 17
- RST: GPIO 18
- Dirección I2C primaria: `0x15`
- Dirección I2C alternativa: `0x38`

Notas:

- El firmware prueba ambas direcciones I2C.
- Si `TOUCH_DEBUG_ENABLED` está activo, se dibuja el punto de toque sobre la pantalla base.

### SD_MMC en modo 1-bit

- CLK: GPIO 38
- CMD: GPIO 40
- D0: GPIO 39

Notas:

- No usa SPI para la tarjeta SD.
- En el arranque se crea `test.txt`, se escribe, se lee y se compara para validar la SD de verdad.

### LED RGB WS2812B

- DATA: GPIO 42

Notas:

- El LED usa `Adafruit_NeoPixel`.
- Refleja el estado del arranque con colores simples.

## Estructura del proyecto

- `src/main.cpp`: inicialización principal y lógica base de hardware.
- `include/configuration.h`: definición centralizada de pines.
- `include/tft_espi_setup.h`: configuración heredada para `TFT_eSPI`, mantenida por compatibilidad.
- `platformio.ini`: entorno PlatformIO, dependencias y flags de compilación.

## platformio.ini explicado

### Entorno

- `platform = espressif32`: plataforma ESP32 de PlatformIO.
- `board = esp32-s3-devkitc-1`: objetivo base de compilación.
- `framework = arduino`: uso del core Arduino sobre ESP-IDF.
- `monitor_speed = 115200`: velocidad del monitor serie.

### USB nativo

- `board_build.arduino.usb_mode = 1`: selecciona USB nativo en ESP32-S3.
- `board_build.arduino.usb_cdc_on_boot = 0`: deja operativo el canal USB Serial/JTAG, que fue el modo funcional probado en esta placa.

### build_flags

- `-D ARDUINO_USB_MODE=1`: fija el modo USB en compilación.
- `-D ARDUINO_USB_CDC_ON_BOOT=0`: alinea las macros del core con el modo USB utilizado.
- `-D USER_SETUP_LOADED`: evita configuraciones por defecto externas en librerías heredadas.
- `-include include/tft_espi_setup.h`: inyecta la configuración de pantalla para compatibilidad con `TFT_eSPI`.

Opcional:

- `-D TOUCH_DEBUG_ENABLED`: activa el overlay de depuración táctil.

### Librerías

- `Adafruit GFX Library`: primitivas gráficas base.
- `Adafruit ILI9341`: driver de la pantalla TFT.
- `Adafruit NeoPixel`: control del LED RGB WS2812B.

## Flujo de arranque

1. Inicializa LED RGB y canal serie.
2. Inicializa la pantalla y muestra splash.
3. Detecta el touch por I2C.
4. Monta la SD_MMC.
5. Ejecuta la prueba real de `test.txt`.
6. Muestra estado de hardware en pantalla y monitor serie.

## Recomendaciones para reutilizar esta base

1. Mantén `include/configuration.h` como fuente única de pines.
2. Activa `TOUCH_DEBUG_ENABLED` solo durante pruebas del touch.
3. Conserva la prueba de SD si el proyecto depende de almacenamiento.
4. Usa el LED RGB como diagnóstico rápido del estado del sistema.