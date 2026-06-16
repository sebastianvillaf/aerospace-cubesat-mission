/**
 * Prueba de Calificacion Casera - Vibracion (aleatoria + impacto)
 *
 * Usa DOS MPU6050 en el mismo bus I2C con direcciones distintas:
 *   - 0x68 (pin AD0 a GND)   -> montado DENTRO del CubeSat   (respuesta)
 *   - 0x69 (pin AD0 a 3.3V)  -> montado en la PLATAFORMA del shaker (fuente)
 *
 * Registra en microSD:
 *   Tiempo,Ax_fuente,Ay_fuente,Az_fuente,Ax_cubesat,Ay_cubesat,Az_cubesat
 *
 * El mismo sketch sirve para la prueba de vibracion aleatoria (2.3/2.4) y
 * para la prueba de impacto (2.5): selecciona el modo por Serial al
 * arrancar, asi no hay que reflashear entre sub-pruebas.
 *
 * Notas de configuracion:
 * - Filtro a 260Hz (el mas ancho del MPU6050) y rango +-16g: se prioriza no
 *   perder contenido de alta frecuencia ni saturar con el pico del impacto.
 *   El filtrado/analisis fino (PSD, deteccion de pico) se hace despues en
 *   Python a partir del CSV, no aqui.
 * - El timestamp se calcula con micros() real en cada muestra, no asumiendo
 *   un intervalo nominal, asi que sigue siendo correcto aunque el loop
 *   tenga jitter ocasional por la escritura a SD.
 *
 * Hardware: ESP32 + 2x MPU6050 (I2C 0x68 y 0x69) + modulo microSD (SPI)
 */

#include "mpu6050.h"
#include "sd_logger.h"

#define SD_CS_PIN 5                 //Ajustar segun el wiring del modulo SD
#define MPU_CUBESAT_ADDR 0x68
#define MPU_SOURCE_ADDR  0x69

#define SAMPLE_INTERVAL_US 5000     //200 Hz objetivo (ajustar si el I2C/SD no lo sostienen)
#define FLUSH_EVERY_N_ROWS 50       //evita flush por fila a alta tasa de muestreo
#define MODE_SELECT_TIMEOUT_MS 5000

MPU6050_Driver mpuSource;
MPU6050_Driver mpuCubesat;
SD_Logger sdLogger;

unsigned long testStartTime_us = 0;
unsigned long lastSampleTime_us = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Wire.begin();
    Wire.setClock(400000);  //I2C Fast Mode: necesario para sostener ~200Hz leyendo 2 sensores

    Serial.println("=== Prueba de Vibracion - Logger CSV ===");

    bool sourceOk  = mpuSource.begin(MPU_SOURCE_ADDR, MPU6050_RANGE_16_G, MPU6050_BAND_260_HZ);
    bool cubesatOk = mpuCubesat.begin(MPU_CUBESAT_ADDR, MPU6050_RANGE_16_G, MPU6050_BAND_260_HZ);
    bool sdOk      = sdLogger.begin(SD_CS_PIN);

    if (!sourceOk || !cubesatOk || !sdOk) {
        Serial.println("[FATAL] Algun sensor no inicializo. Revisa direcciones I2C (AD0) y wiring.");
        while (true) { delay(1000); }
    }

    //Seleccion de sub-prueba sin reflashear
    Serial.println("Escribe 'V' (vibracion aleatoria) o 'I' (impacto). 5s, default V:");
    unsigned long waitStart = millis();
    char mode = 'V';
    while (millis() - waitStart < MODE_SELECT_TIMEOUT_MS) {
        if (Serial.available()) {
            mode = toupper(Serial.read());
            break;
        }
    }

    const char* filename = (mode == 'I') ? "/VIB_IMPACT.csv" : "/VIB_RANDOM.csv";
    Serial.print("[OK] Modo seleccionado -> ");
    Serial.println(filename);

    sdLogger.createFile(filename,
        "Tiempo,Ax_fuente,Ay_fuente,Az_fuente,Ax_cubesat,Ay_cubesat,Az_cubesat",
        FLUSH_EVERY_N_ROWS);

    testStartTime_us = micros();
    lastSampleTime_us = testStartTime_us;

    Serial.println("[OK] Logger listo. Iniciando captura...");
}

void loop() {
    unsigned long now_us = micros();

    if (now_us - lastSampleTime_us >= SAMPLE_INTERVAL_US) {
        lastSampleTime_us = now_us;

        float sx, sy, sz, cx, cy, cz;
        mpuSource.readAccel(sx, sy, sz);
        mpuCubesat.readAccel(cx, cy, cz);

        float elapsed_s = (now_us - testStartTime_us) / 1000000.0;
        float values[6] = { sx, sy, sz, cx, cy, cz };

        sdLogger.logRow(elapsed_s, values, 6);
    }
}
