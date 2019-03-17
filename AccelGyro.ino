
// This file contains methods used to interact with the Accel & Gyro sensors.

// Here I'm using Sparkfun SEN-13284 9DoF IMU Breakout - LSM9DS1 https://www.sparkfun.com/products/13284
//
// However the code could be quickly ported to any 6dof platform - no need for a mag here!
// For instance, a cheap MPU-6050 board might be perfectly suitable


#define FAST_MODE //Sensors runs at 230 Hz as opposed to 115Hz

#include <SparkFunLSM9DS1.h>
#include <Wire.h>

#define INT1_PIN        20    //Gyro Data Ready Interrupt (optional, improves stability)

#define I2C_SDA         18    //For reference only
#define I2C_SCL         19    //These are the standard Teensy LC I2C pins

#define A_RES   0.000244f     //0.244 mG/LSB
#define G_RES   0.000320f     //Got to this value experimentally, about 5% higher than the datasheet...

LSM9DS1 imu;

inline bool gyroDataAvailable()
{
#ifdef INT1_PIN
  return !digitalReadFast(INT1_PIN);
#else
  return imu.gyroAvailable();
#endif
}


bool initAccelGyro()
{
  Wire.setClock(800000);
  delay(100);

  imu.settings.device.commInterface = IMU_MODE_I2C;

  imu.settings.accel.scale = 8;
  imu.settings.gyro.scale = 500;

#ifdef FAST_MODE
  imu.settings.accel.sampleRate = 5; // ~230Hz
  imu.settings.gyro.sampleRate = 5; // ~230Hz
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

  Wire.setClock(800000);
  
  return true;
}


void readGyroAccel()
{
  imu.readGyro();
  imu.readAccel();

  ax = (float)imu.ax * A_RES;
  ay = (float)-imu.ay * A_RES;
  az = (float)imu.az * A_RES;
  gx = (float)imu.gx * G_RES;
  gy = (float)-imu.gy * G_RES;
  gz = (float)imu.gz * G_RES;
}
