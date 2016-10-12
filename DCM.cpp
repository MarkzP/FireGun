/***************************************************************************************************************
  Razor AHRS Firmware v1.4.2 (heavily modified for this project!)
  9 Degree of Measurement Attitude and Heading Reference System
  for Sparkfun "9DOF Razor IMU" (SEN-10125 and SEN-10736)
  and "9DOF Sensor Stick" (SEN-10183, 10321 and SEN-10724)

  Released under GNU GPL (General Public License) v3.0
  Copyright (C) 2013 Peter Bartz [http://ptrbrtz.net]
  Copyright (C) 2011-2012 Quality & Usability Lab, Deutsche Telekom Laboratories, TU Berlin

  Infos, updates, bug reports, contributions and feedback:
      https://github.com/ptrbrtz/razor-9dof-ahrs


  History:
*   * Original code (http://code.google.com/p/sf9domahrs/) by Doug Weibel and Jose Julio,
      based on ArduIMU v1.5 by Jordi Munoz and William Premerlani, Jose Julio

      And then Marc Paquette screwed it up completely, then tried to fix it the best he could
*/

#include "DCM.h"
#include <Arduino.h>

void DCM::update(float gx, float gy, float gz, float ax, float ay, float az, float irh, float irp, float irc)
{
  unsigned long timestamp_old = timestamp;
  timestamp = micros();

  G_Dt = (timestamp - timestamp_old) / 1000000.0f; // Real time of loop run. We use this on the DCM algorithm (gyro integration time)

  gyro[0] = gx;
  gyro[1] = gy;
  gyro[2] = gz;

  accel[0] = ax;
  accel[1] = ay;
  accel[2] = az;

  ir_heading = irh;
  ir_pitch = irp;
  ir_confidence = irc;

  detectMotion();

  if (motion)
  {
    gyro_sum[0] = 0;
    gyro_sum[1] = 0;
    gyro_sum[2] = 0;
    gyroSampleCount = 0;
  }
  else
  {
    if (gyroSampleCount < GYRO_SAMPLE_COUNT)
    {
      gyro_sum[0] += gx;
      gyro_sum[1] += gy;
      gyro_sum[2] += gz;
    }

    gyroSampleCount++;

    if (gyroSampleCount == GYRO_SAMPLE_COUNT)
    {
      computeGyroBias();
      reset_sensor_fusion();
      return;
    }
  }

  if (!gyroCalibrated)
  {
    return;
  }

  if (!ir_lock)
  {
    if (ir_confidence > 0.6f)
    {
      yaw = ir_heading;
      init_rotation_matrix();
      ir_lock = true;
    }
  }

  //Apply bias
  gyro[0] -= gyro_bias[0];
  gyro[1] -= gyro_bias[1];
  gyro[2] -= gyro_bias[2];

  matrixUpdate();
  normalize();
  driftCorrection();
  computeAngles();
}


float DCM::getAccelPitch(void)
{
  return -atan2(accel[0], sqrt(accel[1] * accel[1] + accel[2] * accel[2]));
}


float DCM::getAccelRoll(void)
{
  float temp1[3];
  float temp2[3];
  float xAxis[] = {1.0f, 0.0f, 0.0f};

  // Compensate pitch of gravity vector
  Vector_Cross_Product(temp1, accel, xAxis);
  Vector_Cross_Product(temp2, xAxis, temp1);

  // Normally using x-z-plane-component/y-component of compensated gravity vector
  // roll = atan2(temp2[1], sqrt(temp2[0] * temp2[0] + temp2[2] * temp2[2]));
  // Since we compensated for pitch, x-z-plane-component equals z-component:
  return atan2(temp2[1], temp2[2]);
}

void DCM::detectMotion(void)
{
  //This is a crude bandpass filter used to detect motion
  float gyroSum = abs(gyro[0]) + abs(gyro[1]) + abs(gyro[2]);
  gyroAvg = (gyroAvg * (1.0f - MOTION_FILTER)) + gyroSum * MOTION_FILTER;
  motionAvg = (motionAvg * (1.0f - MOTION_FILTER)) + abs(gyroAvg - gyroSum) * MOTION_FILTER;

  //Serial.println(motionAvg, 5);

  motion = motionAvg > MOTION_THRESHOLD + (motion ? 0.0f : 0.2f);
}

void DCM::computeGyroBias(void)
{
  float magnitude = 0;

  for (int j = 0; j < 3; j++)
  {
    gyro_sum[j] /= (float)GYRO_SAMPLE_COUNT; //Average over samples
    magnitude += (gyro_sum[j] * gyro_sum[j]); //Compute magnitude
  }
  magnitude = sqrt(magnitude);

  if (magnitude < 0.5f)
  {
    //Apply new offsets if they are reasonable
    for (int j = 0; j < 3; j++)
    {
      gyro_bias[j] = gyro_sum[j];
    }

    gyroCalibrated = true;
  }
}


// Init rotation matrix using euler angles
void DCM::init_rotation_matrix(void)
{
  float c1 = cos(roll);
  float s1 = sin(roll);
  float c2 = cos(pitch);
  float s2 = sin(pitch);
  float c3 = cos(yaw);
  float s3 = sin(yaw);

  // Euler angles, right-handed, intrinsic, XYZ convention
  // (which means: rotate around body axes Z, Y', X'')
  DCM_Matrix[0][0] = c2 * c3;
  DCM_Matrix[0][1] = c3 * s1 * s2 - c1 * s3;
  DCM_Matrix[0][2] = s1 * s3 + c1 * c3 * s2;

  DCM_Matrix[1][0] = c2 * s3;
  DCM_Matrix[1][1] = c1 * c3 + s1 * s2 * s3;
  DCM_Matrix[1][2] = c1 * s2 * s3 - c3 * s1;

  DCM_Matrix[2][0] = -s2;
  DCM_Matrix[2][1] = c2 * s1;
  DCM_Matrix[2][2] = c1 * c2;
}

void DCM::reset_sensor_fusion(void) {
  float Accel_magnitude;

  Accel_magnitude = sqrt(accel[0] * accel[0] + accel[1] * accel[1] + accel[2] * accel[2]);
  if (Accel_magnitude > 0)
  {
    pitch = getAccelPitch();
    roll = getAccelRoll();
  }

  if (ir_confidence > 0.5f)
  {
    yaw = ir_heading;
  }

  // Init rotation matrix
  init_rotation_matrix();

  // Init variables
  for (int j = 0; j < 3; j++)
  {
    Omega_Vector[j] = 0;
    Omega_P[j] = 0;
    Omega_I[j] = 0;
    Omega[j] = 0;
    errorRollPitch[j] = 0;
    errorYaw[j] = 0;
  }
}


void DCM::normalize(void)
{
  float error = 0;
  float temporary[3][3];
  float renorm = 0;

  error = -Vector_Dot_Product(&DCM_Matrix[0][0], &DCM_Matrix[1][0]) * .5; //eq.19

  Vector_Scale(&temporary[0][0], &DCM_Matrix[1][0], error); //eq.19
  Vector_Scale(&temporary[1][0], &DCM_Matrix[0][0], error); //eq.19

  Vector_Add(&temporary[0][0], &temporary[0][0], &DCM_Matrix[0][0]);//eq.19
  Vector_Add(&temporary[1][0], &temporary[1][0], &DCM_Matrix[1][0]);//eq.19

  Vector_Cross_Product(&temporary[2][0], &temporary[0][0], &temporary[1][0]); // c= a x b //eq.20

  renorm = .5 * (3 - Vector_Dot_Product(&temporary[0][0], &temporary[0][0])); //eq.21
  Vector_Scale(&DCM_Matrix[0][0], &temporary[0][0], renorm);

  renorm = .5 * (3 - Vector_Dot_Product(&temporary[1][0], &temporary[1][0])); //eq.21
  Vector_Scale(&DCM_Matrix[1][0], &temporary[1][0], renorm);

  renorm = .5 * (3 - Vector_Dot_Product(&temporary[2][0], &temporary[2][0])); //eq.21
  Vector_Scale(&DCM_Matrix[2][0], &temporary[2][0], renorm);
}


/**************************************************/
void DCM::driftCorrection(void)
{
  //Compensation the Roll and Pitch drift.
  static float Scaled_Omega_I[3];
  static float Scaled_Omega_P[3];

  float Accel_magnitude;
  float Accel_weight;

  float heading_x;
  float heading_y;
  float errorCourse;

  //*****Roll and Pitch***************
  // Calculate the magnitude of the accelerometer vector

  Accel_magnitude = sqrt(accel[0] * accel[0] + accel[1] * accel[1] + accel[2] * accel[2]);
  if (Accel_magnitude > 0)
  {
    // Dynamic weighting of accelerometer info (reliability filter)
    // Weight for accelerometer info (<0.5G = 0.0, 1G = 1.0 , >1.5G = 0.0)
    //Accel_weight = constrain(1 - 2*abs(1 - Accel_magnitude),0,1);  //

    // Modified to account for off-center sensors; accels are trusted only between 0.75 & 1.25g
    Accel_weight = constrain(1 - 4 * abs(1 - Accel_magnitude), 0, 1);

    Vector_Cross_Product(&errorRollPitch[0], &accel[0], &DCM_Matrix[2][0]); //adjust the ground of reference
    Vector_Scale(&Omega_P[0], &errorRollPitch[0], Kp_ROLLPITCH * Accel_weight);

    Vector_Scale(&Scaled_Omega_I[0], &errorRollPitch[0], Ki_ROLLPITCH * Accel_weight);
    Vector_Add(Omega_I, Omega_I, Scaled_Omega_I);
  }

  //*****YAW***************
  // We make the gyro YAW drift correction based on ir heading
  heading_x = cos(ir_heading);
  heading_y = sin(ir_heading);
  errorCourse = (DCM_Matrix[0][0] * heading_y) - (DCM_Matrix[1][0] * heading_x); //Calculating YAW error
  Vector_Scale(errorYaw, &DCM_Matrix[2][0], errorCourse); //Applys the yaw correction to the XYZ rotation of the aircraft, depeding the position.

  Vector_Scale(&Scaled_Omega_P[0], &errorYaw[0], Kp_YAW * ir_confidence);
  Vector_Add(Omega_P, Omega_P, Scaled_Omega_P);//Adding  Proportional.

  Vector_Scale(&Scaled_Omega_I[0], &errorYaw[0], Ki_YAW * ir_confidence);
  Vector_Add(Omega_I, Omega_I, Scaled_Omega_I);//adding integrator to the Omega_I
}

void DCM::computeAngles(void)
{
  pitch = -asin(DCM_Matrix[2][0]);
  roll = atan2(DCM_Matrix[2][1], DCM_Matrix[2][2]);
  yaw = atan2(DCM_Matrix[1][0], DCM_Matrix[0][0]);
}

void DCM::matrixUpdate(void)
{
  Vector_Add(&Omega[0], &gyro[0], &Omega_I[0]);  //adding proportional term
  Vector_Add(&Omega_Vector[0], &Omega[0], &Omega_P[0]); //adding Integrator term

  Update_Matrix[0][0] = 0;
  Update_Matrix[0][1] = -G_Dt * Omega_Vector[2]; //-z
  Update_Matrix[0][2] = G_Dt * Omega_Vector[1]; //y
  Update_Matrix[1][0] = G_Dt * Omega_Vector[2]; //z
  Update_Matrix[1][1] = 0;
  Update_Matrix[1][2] = -G_Dt * Omega_Vector[0]; //-x
  Update_Matrix[2][0] = -G_Dt * Omega_Vector[1]; //-y
  Update_Matrix[2][1] = G_Dt * Omega_Vector[0]; //x
  Update_Matrix[2][2] = 0;

  Matrix_Multiply(DCM_Matrix, Update_Matrix, Temporary_Matrix); //a*b=c

  for (int x = 0; x < 3; x++) //Matrix Addition (update)
  {
    for (int y = 0; y < 3; y++)
    {
      DCM_Matrix[x][y] += Temporary_Matrix[x][y];
    }
  }
}


// Computes the dot product of two vectors
float DCM::Vector_Dot_Product(float *v1, float *v2)
{
  float result = 0;

  for (int c = 0; c < 3; c++)
  {
    result += v1[c] * v2[c];
  }

  return result;
}

// Computes the cross product of two vectors
// out has to different from v1 and v2 (no in-place)!
void DCM::Vector_Cross_Product(float *out,  float *v1,  float *v2)
{
  out[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
  out[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
  out[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

// Multiply the vector by a scalar
void DCM::Vector_Scale(float out[3],  float v[3], float scale)
{
  for (int c = 0; c < 3; c++)
  {
    out[c] = v[c] * scale;
  }
}

// Adds two vectors
void DCM::Vector_Add(float out[3],  float v1[3],  float v2[3])
{
  for (int c = 0; c < 3; c++)
  {
    out[c] = v1[c] + v2[c];
  }
}

// Multiply two 3x3 matrices: out = a * b
// out has to different from a and b (no in-place)!
void DCM::Matrix_Multiply( float a[3][3], float b[3][3], float out[3][3])
{
  for (int x = 0; x < 3; x++) // rows
  {
    for (int y = 0; y < 3; y++) // columns
    {
      out[x][y] = a[x][0] * b[0][y] + a[x][1] * b[1][y] + a[x][2] * b[2][y];
    }
  }
}

// Multiply 3x3 matrix with vector: out = a * b
// out has to different from b (no in-place)!
void DCM::Matrix_Vector_Multiply( float a[3][3], float b[3], float out[3])
{
  for (int x = 0; x < 3; x++)
  {
    out[x] = a[x][0] * b[0] + a[x][1] * b[1] + a[x][2] * b[2];
  }
}






