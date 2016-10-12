// PVision library for interaction with the Pixart sensor on a WiiMote
//
// Once again, this has been heavilly hacked for this project.
// I stole the sensor from a Rock Candy Wiimote clone
//    You must use the pink one https://www.amazon.ca/PDP-Rock-Candy-Gesture-Controller/dp/B008U2XED0
//
// In my gun, I've installed the sensor with the connections facing up, just like I found it inside
// of the cute pink controller (I'm color blind; mine's probably blue but pink sounds cooler)
//
// Also, with this I didn't have to jerry rig a 24Mhz crystal, which is great!
// If you are lucky and you end up with the exact same sensor as mine, here's the pinout:
//
//          [Top view, front end (the side you normally point at the TV)
//
//                 (N.C)   (N.C)    3.3v    GND
//                   8       6       4       2
//               ----------------------------------
//                   7       5       3       1
//                 (N.C)    SDA     SCL     RST (4.7k to GND)
//                           |       |
//                          18      19
//                        Teensy I2C Pins
//                       (Shared with IMU)
//
// Note: SDA & SCL both requires 2.2k pullup resistors to 3.3v.
//       IMUs connected via I2C may already have those pullups.
//       RST must be pulled low for sensor to work.
//
// This work was derived from Kako's excellent Japanese website
// http://www.kako.com/neta/2007-001/2007-001.html
//
// This work is also derived from Steve Hobley furter hacking - www.stephenhobley.com - thanks!
// Also found some answers on http://procrastineering.blogspot.ca/2008/09/working-with-pixart-camera-directly.html

#ifndef PVision_h
#define PVision_h

#include <Wire.h>

#define H_RES 1024
#define V_RES 768

#define HALF_H_RES  (H_RES / 2)
#define HALF_V_RES  (V_RES / 2)

#define H_FOV_DEG 40.0f  //The internets says FOV is closer to 33, but my sensor definately has an FOV of 40 degrees
#define H_FOV_RAD (H_FOV_DEG * PI / 180.0f)

#define DEG_PER_PIXEL (H_FOV_DEG / H_RES)
#define RAD_PER_PIXEL (H_FOV_RAD / H_RES)

// Returned structure from a call to readSample()
struct blob_t
{
  int X;
  int Y;
  int Size;
  byte number;
  float Scaled_X; //Scaled X axis position in Rads from sensor center
  float Scaled_Y; //Scaled Y axis position in Rads from sensor center
  bool Found;
};


class PVision
{

  public:
    PVision();

    bool init();   // returns true if the connection to the sensor established correctly
    int read();   // updated the blobs, and returns the number of blobs detected

    blob_t Blob[4];
  private:

    uint8_t block1[9] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x41 }; //0x00
    uint8_t block2[2] = { 0x40, 0x00 };  //0x1A
    uint8_t slaveAddress = 0x58;
    uint8_t count = 12;
    uint8_t data[12];

    int blobcount; // returns the number of blobs found - reads the sensor

    void resetBlobs(void);
    void writeByte(uint8_t address, uint8_t data);
    void writeBytes(uint8_t address, uint8_t * data, uint8_t len);
};

#endif
