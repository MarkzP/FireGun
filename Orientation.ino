
// This file contains methods used to compute screen coordinates from sensor input.

#include <EEPROM.h>
#include "DCM.h"

#define EADR 0x00
#define eepr_magic 0xB00B

DCM dcm;

uint32_t noMotionCount;

// This is used to somewhat compensate for the IR sensor not pivoting exactly on it's axis.
// Normally, you hold the gun in your hand, so the IR sensor will be at about an arm's length
// from the pivot point.
float irHeadingRatio = 1.0f; 


void loadScreenCalib()
{
  uint32_t eepr_test;

  EEPROM.get(EADR + 0, eepr_test);
  if (eepr_test == eepr_magic)
  {
    EEPROM.get(EADR + 4, centerHeading);
    EEPROM.get(EADR + 8, centerPitch);
    EEPROM.get(EADR + 12, rangeHeading);
    EEPROM.get(EADR + 16, rangePitch);
  }
}

void saveScreenCalib()
{
  EEPROM.put(EADR + 4, centerHeading);
  EEPROM.put(EADR + 8, centerPitch);
  EEPROM.put(EADR + 12, rangeHeading);
  EEPROM.put(EADR + 16, rangePitch);

  EEPROM.put(EADR + 0, eepr_magic);
}

bool initOrientation()
{
  loadScreenCalib();

  return true; //that's what I call a robust initializer
}

void updateOrientation()
{
  dcm.update(gx, gy, gz, ax, ay, az, irHeading * irHeadingRatio, irPitch, irConfidence);
  heading = dcm.yaw * irHeadingRatio;
  pitch = dcm.pitch;
  roll = dcm.roll;

  if (!active && irConfidence > 0.75f) active = true;
  else {
    if (!dcm.motion) {
      
      noMotionCount++;

      if (noMotionCount >= 128) active = false;
    }
    else {
      noMotionCount = 0;
    }
  }
}
