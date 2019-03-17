
// This file contains methods used to interact with the IR sensor.

#include "PVision.h"

PVision ir;

int leftBlob = -1;
int rightBlob = -1;

float rotatedX[4];
float rotatedY[4];
int blobSize[4];
bool blobGood[4];

unsigned long lastReading = 0;

#define DIST_FILTER 0.1f
float omegaFilter = DIST_FILTER;

bool initIRTracking()
{
  return ir.begin();
}

void readIRTracking()
{
  int i = 0;
  unsigned long now = micros();
  
  irConfidence = 0.0f;

  if ((now - lastReading) < 10000)
  {
    return;
  }

  ledPower = 0;
  
  ir.read();

  lastReading = now;


  // Pre compute last known roll sin/cos in preparation for rotation
  float sin_roll = sinf(roll);
  float cos_roll = cosf(roll);

  //Process blobs
  for (i = 0; i < 4; i++)
  {
    blobGood[i] = ir.Blob[i].Found;
    if (blobGood[i])
    {
      // Rotate blobs based on the dcm roll angle
      rotatedX[i] = (ir.Blob[i].Scaled_X * cos_roll) + (ir.Blob[i].Scaled_Y * sin_roll);
      rotatedY[i] = (ir.Blob[i].Scaled_X * -sin_roll) + (ir.Blob[i].Scaled_Y * cos_roll);
      blobSize[i] = ir.Blob[i].Size;
    }
  }

  // Test the Left blob
  if (leftBlob < 0 || !blobGood[leftBlob])
  {
    //Left blob missing
    leftBlob = -1;
    for (i = 0; i < 4; i++)
    {
      if (i != rightBlob && blobGood[i])
      {
        leftBlob = i;
        break;
      }
    }
  }

  // Test the Right blob
  if (rightBlob < 0 || !blobGood[rightBlob])
  {
    //Right blob missing
    rightBlob = -1;
    for (i = 0; i < 4; i++)
    {
      if (i != leftBlob && blobGood[i])
      {
        rightBlob = i;
        break;
      }
    }
  }

  // Check if we can compute orientation
  if (leftBlob >= 0 && rightBlob >= 0)
  {
    // We have 2 blobs. Swap them if needed
    if (rotatedX[leftBlob] > rotatedX[rightBlob])
    {
      // swap
      int tmp = leftBlob;
      leftBlob = rightBlob;
      rightBlob = tmp;
    }

    // Both blobs, compute distance
    float deltaX = abs(rotatedX[leftBlob] - rotatedX[rightBlob]);
    float deltaY = abs(rotatedY[leftBlob] - rotatedY[rightBlob]);

    // Check if they are at the right distance
    if (deltaX > 0.075f && deltaX < 0.500f && deltaY < 0.075f)
    {
      irHeading = (rotatedX[leftBlob] + rotatedX[rightBlob]) * 0.5f;
      irPitch = (rotatedY[leftBlob] + rotatedY[rightBlob]) * 0.5f;

      irConfidence = constrain(1.0f - max(abs(irHeading * 4.5f), abs(irPitch * 3.8f)), 0.0f , 1.0f);

      omegaFilter = irConfidence * DIST_FILTER;

      // Note that since the two blobs are likely level, no need to account for DeltaY - we save a fer usec
      omegaBlobs = (omegaBlobs * (1.0f - omegaFilter)) + (deltaX * omegaFilter);
    }
    else
    {     
      // both blobs to their rooms for the evening
      leftBlob = -1;
      rightBlob = -1;
    }
  }
}
