#ifndef DCM_H
#define DCM_H

/*
  Derived from  https://github.com/ptrbrtz/razor-9dof-ahrs

  Razor AHRS
  9 Degree of Measurement Attitude and Heading Reference System

  Copyright (C) 2013 Peter Bartz (http://ptrbrtz.net)
  Copyright (C) 2011-2012 Quality & Usability Lab, Deutsche Telekom Laboratories, TU Berlin

  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses/>.
*/


/*
  Added gyro self-calibration when sensor is not moving

  Sensors Units of Measures:
    accel is in G
    gyro is in rad/sed

  Sensors Orientation:
  Right-hand system with Z axis pointing up:


  Output is in radians

  Right-hand system with Z axis pointing up (note: this is different from Razor AHRS original orientations)
    Positive yaw   : counter clockwise
    Positive roll  : right wing down
    Positive pitch : nose down

  Transformation order: first yaw then pitch then roll.
*/



class DCM
{
  public:
    DCM() {}

    // DCM parameters
#define Kp_ROLLPITCH 1.5f
#define Ki_ROLLPITCH 0.00002f
#define Kp_YAW 30.0f
#define Ki_YAW 0.00000f


#define MOTION_FILTER 0.01f
#define MOTION_THRESHOLD 0.004f
#define GYRO_SAMPLE_COUNT 128

#define TO_DEG(x) ((x) * 180.0f / PI)


    // Euler angles (in radians)
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;

    bool motion = false;
    bool ir_lock = false;

    void update(float gx, float gy, float gz, float ax, float ay, float az, float irh, float irp, float irc);

    float getAccelPitch(void);
    float getAccelRoll(void);

  private:
    // Sensor variables
    float accel[3] = {0, 0, 0};
    float gyro[3] = {0, 0, 0};
    float ir_heading = 0.0f;
    float ir_pitch = 0.0f;
    float ir_confidence = 0.0f;

    // DCM timing in the main loop
    unsigned long timestamp = 0;
    float G_Dt = 0.0f; // Integration time for DCM algorithm

    // DCM variables
    float Omega_Vector[3] = {0, 0, 0}; // Corrected Gyro_Vector data
    float Omega_P[3] = {0, 0, 0}; // Omega Proportional correction
    float Omega_I[3] = {0, 0, 0}; // Omega Integrator
    float Omega[3] = {0, 0, 0};
    float errorRollPitch[3] = {0, 0, 0};
    float errorYaw[3] = {0, 0, 0};
    float DCM_Matrix[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    float Update_Matrix[3][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
    float Temporary_Matrix[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    

    float gyroAvg = 0;
    float motionAvg = 0;

    float gyro_sum[3] = {0, 0, 0};
    float gyro_bias[3] = {0, 0, 0};
    int gyro_index = 0;
    int gyroSampleCount = 0;
    
    bool gyroCalibrated = false;

    void reset_sensor_fusion(void);
    void init_rotation_matrix(void);
    void detectMotion(void);
    void computeGyroBias(void);
    void compassHeading(void);
    void matrixUpdate(void);
    void driftCorrection(void);
    void normalize(void);
    void computeAngles(void);

    void Matrix_Vector_Multiply( float a[3][3],  float b[3], float out[3]);
    void Matrix_Multiply( float a[3][3],  float b[3][3], float out[3][3]);
    void Vector_Add(float out[3],  float v1[3],  float v2[3]);
    void Vector_Scale(float out[3],  float v[3], float scale);
    void Vector_Cross_Product(float *out,  float *v1,  float *v2);
    float Vector_Dot_Product(float *v1, float *v2);
};


#endif
