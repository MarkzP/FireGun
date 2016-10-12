
// This file contains methods used to interact with the Accel & Gyro sensors.

// Here I'm using Sparkfun SEN-13284 9DoF IMU Breakout - LSM9DS1 https://www.sparkfun.com/products/13284
//
// However the code could be quickly ported to any 6dof platform - no need for a mag here!
// For instance, a cheap MPU-6050 board might be perfectly suitable


//#define FAST_MODE //Sensors runs at 230 Hz as opposed to 115Hz

#include <SparkFunLSM9DS1.h>
#include <Wire.h>
#include <SPI.h>

#define INT1_PIN        20    //Gyro Data Ready Interrupt (optional, improves stability)

#define I2C_SDA         18    //For reference only
#define I2C_SCL         19    //These are the standard Teensy LC I2C pins

#define A_RES   0.000244  //0.244 mG/LSB
#define G_RES   0.000320; //Got to this value experimentally, about 5% higher than the datasheet...

LSM9DS1 imu;

bool gyroDataAvailable()
{
#ifdef INT1_PIN
  return digitalRead(INT1_PIN) == LOW;
#else
  return imu.gyroAvailable();
#endif
}


bool initAccelGyro()
{
  Wire.setClock(400000);
  imu.settings.device.commInterface = IMU_MODE_I2C;

  imu.settings.accel.scale = 8;
  imu.settings.gyro.scale = 500;

#ifdef FAST_MODE
  imu.settings.accel.sampleRate = 4; // ~230Hz
  imu.settings.gyro.sampleRate = 4; // ~230Hz
#else
  imu.settings.accel.sampleRate = 3; // ~115Hz
  imu.settings.gyro.sampleRate = 3; // ~115Hz
#endif

  imu.settings.mag.enabled = false; // No need for that ever drifting mess called "mag" here

  if (!imu.begin())
  {
    return false;
  }

#ifdef INT1_PIN
  pinMode(INT1_PIN, INPUT_PULLUP);
  imu.configInt(XG_INT1, INT_DRDY_G, INT_ACTIVE_LOW, INT_PUSH_PULL);
#endif

  return true;
}


void readGyroAccel()
{
  imu.readGyro();
  imu.readAccel();

  ax = imu.ax * A_RES;
  ay = -imu.ay * A_RES;
  az = imu.az * A_RES;
  gx = imu.gx * G_RES;
  gy = -imu.gy * G_RES;
  gz = imu.gz * G_RES;
}

