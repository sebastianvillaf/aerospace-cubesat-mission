#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class SD_Logger {
public:
    SD_Logger();

    //Initializes SD card on the given chip-select pin
    bool begin(uint8_t chipSelectPin);

    //Creates (overwriting if it exists) a CSV file and writes the header line
    bool createFile(const char* filename, const char* header);

    //Appends one row: timestamp followed by N comma-separated float values
    bool logRow(float timestamp_s, const float* values, uint8_t numValues);

    //Flushes and closes the currently open file
    void close();

    //return true if SD card initialized correctly
    bool isConnected();

private:
    File logFile;
    bool initialized;
    char currentFilename[32];
};

#endif
