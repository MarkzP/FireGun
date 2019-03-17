// Example of using the PVision library for interaction with the Pixart sensor on a WiiMote
// This work was derived from Kako's excellent Japanese website
// http://www.kako.com/neta/2007-001/2007-001.html

// This work is also derived from Steve Hobley furter hacking - www.stephenhobley.com
/******************************************************************************
  Includes
******************************************************************************/
#include "PVision.h"
#include <Wire.h>

/******************************************************************************
  Private methods
******************************************************************************/

void PVision::resetBlobs(void)
{
  Blob[0] = { 1023, 1023, 15, 1, 0.0f, 0.0f, false };
  Blob[1] = { 1023, 1023, 15, 2, 0.0f, 0.0f, false };
  Blob[2] = { 1023, 1023, 15, 3, 0.0f, 0.0f, false };
  Blob[3] = { 1023, 1023, 15, 4, 0.0f, 0.0f, false };
}

void PVision::writeByte(uint8_t address, uint8_t data)
{
  Wire.beginTransmission(slaveAddress);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

void PVision::writeBytes(uint8_t address, uint8_t * data, uint8_t len)
{
  Wire.beginTransmission(slaveAddress);
  Wire.write(address);
  Wire.write(data, len);
  Wire.endTransmission();
}

/******************************************************************************
  Constructor
******************************************************************************/
PVision::PVision()
{
  resetBlobs();
}

/******************************************************************************
  Public methods
******************************************************************************/
// init the PVision sensor
bool PVision::begin(void)
{
  Wire.begin();
  Wire.setClock(800000);

  delay(100);
  writeByte(0x30, 0x01);
  delay(10);
  writeBytes(0x00, block1, sizeof(block1)); //Sensitivity Block 1
  delay(10);
  writeBytes(0x1A, block2, sizeof(block2)); //Sensitivity Block 2
  delay(10);
  writeByte(0x33, 0x03); //Extended Mode
  delay(10);
  writeByte(0x30, 0x08);
  delay(10);

  return read() >= 0;
}

int PVision::read(void)
{
  //IR sensor read
  int i;
  int offset;
  int timeout = 16;
  
  blobcount = -1;
  memset(data, 0, count);

  //IR sensor read
  Wire.beginTransmission(slaveAddress);
  Wire.write(0x37);
  Wire.endTransmission();

  Wire.requestFrom(slaveAddress, count);

  while ((Wire.available() < count) && (timeout-- > 0))
  {
    delay(1);
  }
  if (timeout <= 0)
  {
    resetBlobs();
    return -1;
  }

  for (i = 0; i < count; i++)
  {
    if (!Wire.available())
    {
      resetBlobs();
      return -1;
    }
    data[i] = Wire.read();
  }

  blobcount = 0;

  for (i = 0; i < 4; i++)
  {
    offset = i * 3;
    Blob[i].X = data[offset + 0] + ((data[offset + 2] & 0x30) << 4);
    Blob[i].Y = data[offset + 1] + ((data[offset + 2] & 0xC0) << 2);
    Blob[i].Size = data[offset + 2] & 0x0F;
    Blob[i].Found = Blob[i].Size < 15;
    if (Blob[i].Found)
    {
      Blob[i].Scaled_X = (float)(Blob[i].X - HALF_H_RES) * (float)RAD_PER_PIXEL;
      Blob[i].Scaled_Y = (float)(Blob[i].Y - HALF_V_RES) * (float)RAD_PER_PIXEL;
      blobcount++;
    }
  }

  return blobcount;
}
