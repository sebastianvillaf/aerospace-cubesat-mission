#include "sd_logger.h"
 
SD_Logger::SD_Logger() : initialized(false), flushInterval(1), rowCounter(0) {
    currentFilename[0] = '\0';
}
 
bool SD_Logger::begin(uint8_t chipSelectPin) {
    Serial.print("[SD] Initializing SD card on CS pin ");
    Serial.print(chipSelectPin);
    Serial.print("... ");
 
    if (!SD.begin(chipSelectPin)) {
        Serial.println("FAILED!");
        initialized = false;
        return false;
    }
 
    initialized = true;
    Serial.println("OK");
    return true;
}
 
bool SD_Logger::createFile(const char* filename, const char* header, uint16_t flushEveryNRows) {
    if (!initialized) {
        Serial.println("[SD] ERROR: SD card not initialized");
        return false;
    }
 
    //Start clean: remove any previous file with the same name
    if (SD.exists(filename)) {
        SD.remove(filename);
    }
 
    logFile = SD.open(filename, FILE_WRITE);
    if (!logFile) {
        Serial.print("[SD] ERROR: could not create file ");
        Serial.println(filename);
        return false;
    }
 
    logFile.println(header);
    logFile.flush();
 
    strncpy(currentFilename, filename, sizeof(currentFilename) - 1);
    currentFilename[sizeof(currentFilename) - 1] = '\0';
 
    flushInterval = (flushEveryNRows > 0) ? flushEveryNRows : 1;
    rowCounter = 0;
 
    Serial.print("[SD] File created: ");
    Serial.print(filename);
    Serial.print(" (flush every ");
    Serial.print(flushInterval);
    Serial.println(" rows)");
    return true;
}
 
bool SD_Logger::logRow(float timestamp_s, const float* values, uint8_t numValues) {
    if (!initialized || !logFile) {
        return false;
    }
 
    logFile.print(timestamp_s, 3);
    for (uint8_t i = 0; i < numValues; i++) {
        logFile.print(",");
        logFile.print(values[i], 3);
    }
    logFile.println();
 
    //Flush according to the cadence set in createFile(): every row for slow
    //tests (thermal), every N rows for high sample-rate tests (vibration) to
    //avoid the write latency desyncing the sampling loop.
    rowCounter++;
    if (rowCounter >= flushInterval) {
        logFile.flush();
        rowCounter = 0;
    }
 
    return true;
}
 
void SD_Logger::close() {
    if (logFile) {
        logFile.flush();
        logFile.close();
    }
}
 
bool SD_Logger::isConnected() {
    return initialized;
}