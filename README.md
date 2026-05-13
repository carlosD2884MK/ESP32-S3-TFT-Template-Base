# Template Base ESP32-S3 TFT

Esta rama sirve como plantilla base para proyectos futuros sobre la misma tarjeta ESP32-S3 con pantalla TFT, touch I2C, SD_MMC y LED RGB WS2812B.

## Hardware integrado

### Pantalla TFT ILI9341

- MISO: GPIO 13
- MOSI: GPIO 11
- SCLK: GPIO 12
- CS: GPIO 10
- DC: GPIO 46
- RST: -1
- Backlight: GPIO 45

Notas:

- El reset de la TFT queda en `-1`, lo que indica que no hay pin dedicado y el controlador maneja la inicializacion sin reset fisico externo.
- La pantalla se inicializa por SPI usando `Adafruit_ILI9341`.

### Touch capacitivo CST816S/D

- SDA: GPIO 16
- SCL: GPIO 15
- INT: GPIO 17
- RST: GPIO 18
- Direccion primaria I2C: `0x15`
- Direccion alternativa I2C: `0x38`

Notas:

- El firmware prueba ambas direcciones I2C para detectar el touch.
- Si se habilita `TOUCH_DEBUG_ENABLED`, se superpone un marcador tactil y un recuadro con coordenadas sobre la pantalla principal.

### SD_MMC en modo 1-bit

- CLK: GPIO 38
- CMD: GPIO 40
- D0: GPIO 39

Notas:

- La SD trabaja por `SD_MMC`, no por SPI.
- Durante el arranque se monta la tarjeta, se crea `test.txt`, se escribe contenido de prueba y se vuelve a leer para validar funcionamiento real.

### LED RGB WS2812B

- DATA: GPIO 42

Notas:

- El LED usa `Adafruit_NeoPixel`.
- Se emplea como indicador de estado del arranque.

## Estructura base

- `src/main.cpp`: inicializacion principal de display, touch, SD y LED RGB.
- `include/configuration.h`: concentrador de pines del hardware.
- `include/tft_espi_setup.h`: configuracion heredada para `TFT_eSPI`, mantenida para compatibilidad aunque el proyecto actual usa `Adafruit_ILI9341`.
- `platformio.ini`: configuracion del entorno PlatformIO.

## platformio.ini explicado

### Entorno

- `platform = espressif32`: plataforma ESP32 en PlatformIO.
- `board = esp32-s3-devkitc-1`: objetivo de compilacion para ESP32-S3.
- `framework = arduino`: uso del core Arduino sobre ESP-IDF.
- `monitor_speed = 115200`: velocidad del monitor serie.

### USB nativo

- `board_build.arduino.usb_mode = 1`: selecciona el modo USB nativo del ESP32-S3.
- `board_build.arduino.usb_cdc_on_boot = 0`: evita usar CDC al arranque y deja disponible el canal USB Serial/JTAG, que en esta placa fue el que funciono correctamente por USB-C.

### build_flags

- `-D ARDUINO_USB_MODE=1`: fija en compilacion el modo USB.
- `-D ARDUINO_USB_CDC_ON_BOOT=0`: alinea las macros del core con el modo USB Serial/JTAG usado.
- `-D USER_SETUP_LOADED`: evita que librerias como `TFT_eSPI` busquen configuracion por defecto externa.
- `-include include/tft_espi_setup.h`: inyeccion de la configuracion de pantalla para proyectos que aun reutilicen `TFT_eSPI`.

Opcional:

- `-D TOUCH_DEBUG_ENABLED`: activa el overlay de depuracion tactil. En esta rama queda desactivado por defecto para que la plantilla arranque en modo normal.

### Librerias

- `Adafruit GFX Library`: primitivas graficas base.
- `Adafruit ILI9341`: driver de la pantalla TFT.
- `Adafruit NeoPixel`: control del LED RGB WS2812B.

## Flujo de arranque actual

1. Inicializa el LED RGB y el canal serie.
2. Inicializa la pantalla y muestra el splash.
3. Detecta el touch por I2C.
4. Monta la SD_MMC.
5. Ejecuta una prueba real de escritura y lectura en `test.txt`.
6. Muestra el estado del hardware en pantalla y por monitor serie.

## Recomendaciones para nuevos proyectos

1. Mantener `include/configuration.h` como unica fuente de verdad para pines.
2. Activar `TOUCH_DEBUG_ENABLED` solo cuando se este calibrando o verificando el tactil.
3. Reutilizar el test de SD si el proyecto depende de almacenamiento local.
4. Usar el LED RGB como diagnostico rapido del estado de arranque.

## Como clonar y arrancar

### Clonar el repositorio

```bash
git clone https://github.com/carlosD2884MK/Teclado-Wiegand-con-pantalla-TFT-ESP32.git
cd Teclado-Wiegand-con-pantalla-TFT-ESP32
git checkout Template_base
```

### Abrir el proyecto

1. Abrir la carpeta del repositorio en VS Code.
2. Tener instalada la extension de PlatformIO.
3. Esperar a que PlatformIO resuelva e instale las dependencias del entorno.

### Compilar

Desde VS Code:

1. Abrir la paleta de comandos de PlatformIO o usar la tarea de build.

Desde terminal:

```bash
pio run -e esp32-s3-devkitc-1
```

### Subir al ESP32-S3

Conectar la tarjeta por USB-C y ejecutar:

```bash
pio run -e esp32-s3-devkitc-1 --target upload
```

### Abrir el monitor serie

```bash
pio device monitor -b 115200
```

Notas:

- Esta plantilla usa USB Serial/JTAG en el ESP32-S3.
- Si el puerto no abre, cerrar otros monitores serie que lo esten usando.

### Activar depuracion tactil

En `platformio.ini`, descomentar esta linea dentro de `build_flags`:

```ini
;-D TOUCH_DEBUG_ENABLED
```

y dejarla asi:

```ini
-D TOUCH_DEBUG_ENABLED
```

Con eso se habilita el overlay de touch sobre la pantalla principal.