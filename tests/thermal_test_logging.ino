/**
 * Prueba de Calificacion Casera - Ciclado Termico
 *
 * Registra temperatura cada SAMPLE_INTERVAL_MS y la guarda en microSD
 * en el formato CSV exacto pedido por la rubrica CNPS: Tiempo,T1,T2
 *
 *   T1 = BMP280  (temperatura de referencia / estructura)
 *   T2 = MPU6050 (temperatura interna del IMU)
 *
 * Hardware: ESP32 + BMP280 (I2C 0x76) + MPU6050 (I2C 0x68) + modulo microSD (SPI)
 */

#include "bmp280.h"
#include "mpu6050.h"
#include "sd_logger.h"

#define SD_CS_PIN 5              //Ajustar segun el wiring del modulo SD
#define SAMPLE_INTERVAL_MS 2000  //1 muestra cada 2s (ajustable; cada fase del ciclo dura >= 15 min)
#define LOG_FILENAME "/TEMP_LOG.csv"

BMP280_Driver bmp;
MPU6050_Driver mpu;
SD_Logger sdLogger;

unsigned long lastSampleTime = 0;
unsigned long testStartTime = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Prueba Termica - Logger CSV ===");

    Wire.begin();

    bool bmpOk = bmp.begin(0x76);
    bool mpuOk = mpu.begin(0x68);
    bool sdOk  = sdLogger.begin(SD_CS_PIN);

    if (!bmpOk || !mpuOk || !sdOk) {
        Serial.println("[FATAL] Algun sensor no inicializo. Revisa el wiring antes de continuar.");
        while (true) { delay(1000); }  //Detener ejecucion para no generar datos incompletos
    }

    //Header EXACTO pedido por la rubrica (solo nombres de columna, sin unidades)
    sdLogger.createFile(LOG_FILENAME, "Tiempo,T1,T2");

    testStartTime = millis();
    lastSampleTime = testStartTime;

    Serial.println("[OK] Logger listo. Iniciando ciclado termico...");
}

void loop() {
    unsigned long now = millis();

    if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
        lastSampleTime = now;

        float elapsed_s = (now - testStartTime) / 1000.0;
        float t1 = bmp.readTemperature();
        float t2 = mpu.readTemperature();

        float values[2] = { t1, t2 };
        sdLogger.logRow(elapsed_s, values, 2);

        Serial.print("t=");
        Serial.print(elapsed_s, 1);
        Serial.print("s  T1=");
        Serial.print(t1, 2);
        Serial.print("C  T2=");
        Serial.print(t2, 2);
        Serial.println("C");
    }
}
