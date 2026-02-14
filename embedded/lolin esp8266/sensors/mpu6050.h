#ifndef MPU6050_DRIVER_H
#define MPU6050_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

class MPU6050_Driver {
public:
    MPU6050_Driver();
    
    bool begin(uint8_t address = 0x68);
    
    
    //Read all sensor axis x y and z acceleration (m/s²)
    bool read(float& accel_x, float& accel_y, float& accel_z,
              float& gyro_x, float& gyro_y, float& gyro_z);
    

    bool readAccel(float& x, float& y, float& z);
    
    //Read gyroscope axis x y and z (rad/s)
    bool readGyro(float& x, float& y, float& z);
    
    //Read temperature from MPU6050
    float readTemperature();
    

    //return true if sensor responds
    bool isConnected();

private:
    Adafruit_MPU6050 mpu;
    bool initialized;
};

#endif