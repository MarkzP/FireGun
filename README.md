# FireGun
a MAME compatible lightgun that uses a Wiimote IR tracking camera fused with an overkill DCM filter just so you can shoot gangsta style

By Marc Paquette  marc @^ dacsystemes.com

   Uses highly customized DCM code from Razor AHRS  https://github.com/ptrbrtz/razor-9dof-ahrs
   And more customized IR sensor code from http://www.kako.com/neta/2007-001/2007-001.html, www.stephenhobley.com,
   as well as http://procrastineering.blogspot.ca/2008/09/working-with-pixart-camera-directly.html
   Runs on a Teensy LC from PJRC http://www.pjrc.com/store/teensylc.html
   Relies on Sparkfun LSM9DS1 library https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library
   You may need to hack one of these to get the sensor: https://www.amazon.ca/PDP-Rock-Candy-Gesture-Controller/dp/B008U2XED0
   Compiled with Arduino 1.6.9 & Teensyduino 1.30  http://www.pjrc.com/teensy/teensyduino.html

   Form performance reasons, some trig equations have been simplified to their small angle approximation where suitable.
   I stopped optimizing when I figured out CPU load was less than 50% at ~100Hz.
   Also, the DCM / IR fusion stuff has been tuned experimentally. It may not work in all situations.

   If you build one (or two), let me know!

   USB Type should be "Keyboard + Mouse + Joystick"
   Alternatively, you can add a new Keyboard-less USB type by following instructions from tab "usb_desc"
    (My pc wouldn't boot with 2 identical Teensy USB Keyboards)

   To build:
     -I stole a Nerf gun from my kids (don't be a jerk, get your own!)
     -Everything is held together nicely with hot melt glue
     -Look at the Buttons file to figure out how to connect the buttons
     -Look at the AccelGyro file to get the best way to connect to the imu
     -Finally look at the PVision.h file for additional details on the Ir sensor configuration

   To calibrate the screen area:
     -Point to the upper left corner
     -Hold the trigger until the led comes on (default is 7 seconds)
     -While holding the trigger, move to the lower right corner
     -Release the trigger to save calibration

   To switch between joystick & mouse mode:
     -Hold both the trigger & button 1 until led flashes once (quickly)

   Troubleshooting:
     -Make sure to aim squarely at the sensor bar once in a while (this will re-aling the DCM heading)
     -If it starts to drift or becomed somewhat unuseable, just put the gun down for ~10 sec. This will recalibrate the gyros.
     -Make sure everything is connected correctly
     -Pullups are required on I2C lines, but don't overdo it
     -If initialization fails, leds will display error code (number of flashes):
        3: Accel/Gyro error
        4: IR Sensor error


   This code is released under GPL v3.0: https://gnu.org/licenses/gpl-3.0.txt
