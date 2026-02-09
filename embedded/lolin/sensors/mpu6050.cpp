#include "mpu6050_driver.h"

MPU6050_Driver::MPU6050_Driver() : initialized(false) {
}

bool MPU6050_Driver::begin(uint8_t address) {
    Serial.print("[MPU6050] Initializing at address 0x");
    Serial.print(address, HEX);
    Serial.print("... ");
    
    if (!mpu.begin(address)) {
        Serial.println("FAILED!");
        initialized = false;
        return false;
    }
    
    //Configure sensor ranges
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);   //±8g (sufficient for freefall)
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);        //±500°/s
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);     //Low pass filter
    
    initialized = true;
    Serial.println("OK");
    
    //Print configuration
    Serial.print("[MPU6050] Accel range: ±");
    switch (mpu.getAccelerometerRange()) {
        case MPU6050_RANGE_2_G:  Serial.println("2G");  break;
        case MPU6050_RANGE_4_G:  Serial.println("4G");  break;
        case MPU6050_RANGE_8_G:  Serial.println("8G");  break;
        case MPU6050_RANGE_16_G: Serial.println("16G"); break;
    }
    
    Serial.print("[MPU6050] Gyro range: ±");
    switch (mpu.getGyroRange()) {
        case MPU6050_RANGE_250_DEG:  Serial.println("250°/s");  break;
        case MPU6050_RANGE_500_DEG:  Serial.println("500°/s");  break;
        case MPU6050_RANGE_1000_DEG: Serial.println("1000°/s"); break;
        case MPU6050_RANGE_2000_DEG: Serial.println("2000°/s"); break;
    }
    
    return true;
}

bool MPU6050_Driver::read(float& accel_x, float& accel_y, float& accel_z,
                         float& gyro_x, float& gyro_y, float& gyro_z) {
    if (!initialized) {
        return false;
    }
    
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    //Acceleration in m/s
    accel_x = a.acceleration.x;
    accel_y = a.acceleration.y;
    accel_z = a.acceleration.z;
    
    //Gyroscope in rad/s
    gyro_x = g.gyro.x;
    gyro_y = g.gyro.y;
    gyro_z = g.gyro.z;
    
    return true;
}

bool MPU6050_Driver::readAccel(float& x, float& y, float& z) {
    if (!initialized) {
        return false;
    }
    
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    x = a.acceleration.x;
    y = a.acceleration.y;
    z = a.acceleration.z;
    
    return true;
}

bool MPU6050_Driver::readGyro(float& x, float& y, float& z) {
    if (!initialized) {
        return false;
    }
    
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    x = g.gyro.x;
    y = g.gyro.y;
    z = g.gyro.z;
    
    return true;
}

float MPU6050_Driver::readTemperature() {
    if (!initialized) {
        return 0.0;
    }
    
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    return temp.temperature;
}

bool MPU6050_Driver::isConnected() {
    return initialized;
}