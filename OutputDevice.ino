
// This file contains methods used to interact with the joystick / mouse objects.

#define SERIAL_PORT Serial3

//#define ALT_AXIS


#define OUTPUT_MIN 0
#define OUTPUT_MAX 1023 //Per Teensy specs
#define OUTPUT_MID 512 //Per Teensy specs



// Should restart Teensy 3, will also disconnect USB during restart

// From http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337e/Cihcbadd.html
// Search for "0xE000ED0C"
// Original question http://forum.pjrc.com/threads/24304-_reboot_Teensyduino%28%29-vs-_restart_Teensyduino%28%29?p=35981#post35981

#define RESTART_ADDR       0xE000ED0C
#define READ_RESTART()     (*(volatile uint32_t *)RESTART_ADDR)
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))


int outputX = 0;
int outputY = 0;

bool buttonState[7];

#ifdef SERIAL_PORT
uint8_t serialData[4];
#endif

bool initOutputDevice()
{
#ifdef SERIAL_PORT
  SERIAL_PORT.begin(115200);
#endif

#ifdef JOYSTICK_INTERFACE
  Joystick.useManualSend(true);

  Joystick.X(OUTPUT_MID);
  Joystick.Y(OUTPUT_MID);
  Joystick.Z(OUTPUT_MID);
  Joystick.sliderLeft(OUTPUT_MID);
  Joystick.sliderRight(OUTPUT_MID);
  Joystick.Zrotate(OUTPUT_MID);
  Joystick.hat(-1);

  Joystick.button(1, 0);
  Joystick.button(2, 0);
  Joystick.button(3, 0);
  Joystick.button(4, 0);
  Joystick.button(5, 0);
  Joystick.button(6, 0);

  Joystick.send_now();
#endif

#ifdef MOUSE_INTERFACE

  if (Mouse.isPressed(MOUSE_LEFT)) Mouse.release(MOUSE_LEFT);
  if (Mouse.isPressed(MOUSE_RIGHT)) Mouse.release(MOUSE_RIGHT);
  if (Mouse.isPressed(MOUSE_MIDDLE)) Mouse.release(MOUSE_MIDDLE);

  Mouse.screenSize(OUTPUT_MAX, OUTPUT_MAX);
#endif
  
  return true;
}

void setButtonState(uint8_t button, bool val)
{
  if (button >= 7 || button < 1)
  {
    return;
  }
  
  buttonState[button] = val;

  if (useMouse)
  {
#ifdef MOUSE_INTERFACE
    if (val)
    {
      switch (button)
      {
        case 1: if (!Mouse.isPressed(MOUSE_LEFT)) Mouse.press(MOUSE_LEFT);
        case 2: if (!Mouse.isPressed(MOUSE_RIGHT)) Mouse.press(MOUSE_RIGHT);
        case 3: if (!Mouse.isPressed(MOUSE_MIDDLE)) Mouse.press(MOUSE_MIDDLE);
      }
    }
    else
    {
      switch (button)
      {
        case 1: if (Mouse.isPressed(MOUSE_LEFT)) Mouse.release(MOUSE_LEFT);
        case 2: if (Mouse.isPressed(MOUSE_RIGHT)) Mouse.release(MOUSE_RIGHT);
        case 3: if (Mouse.isPressed(MOUSE_MIDDLE)) Mouse.release(MOUSE_MIDDLE);
      }
    }
#endif
  }
  else
  {
#ifdef JOYSTICK_INTERFACE
    Joystick.button(button, val);
#endif
  }
}

void sendPosition()
{
  
#ifdef SERIAL_PORT
  int8_t c;
  while ((c = SERIAL_PORT.read()) >= 0)
  {
    if (c == 'r') WRITE_RESTART(0x5FA0004);
  }
#endif
  
  if (calibrating || !active)
  {
    outputX = OUTPUT_MID;
    outputY = OUTPUT_MID;
  }
  else {

    //Compute screen position based on orientation angles
    outputX = (((heading - centerHeading) / (-rangeHeading * omegaBlobs)) * OUTPUT_MAX) + OUTPUT_MID;
    outputY = (((pitch - centerPitch) / (rangePitch * omegaBlobs)) * OUTPUT_MAX) + OUTPUT_MID;
  
    //Determine if we are offscreen
    if (outputX >= OUTPUT_MAX || outputX < OUTPUT_MIN || outputY >= OUTPUT_MAX || outputY < OUTPUT_MIN) {
        outputX = OUTPUT_MAX;
        outputY = OUTPUT_MAX;
    }
  }

#ifdef SERIAL_PORT
  //Send to Serial
  serialData[0] = outputX & 0x7F;
  serialData[1] = outputY & 0x7F;
  serialData[2] = ((outputX & 0x0380) >> 7) | ((outputY & 0x0380) >> 3);
  serialData[3] = 
      (buttonState[1] ? 0x01 : 0x00)
    | (buttonState[2] ? 0x02 : 0x00)
    | (buttonState[3] ? 0x04 : 0x00)
    | (buttonState[4] ? 0x08 : 0x00)
    | (buttonState[5] ? 0x10 : 0x00)
    | (buttonState[6] ? 0x20 : 0x00)
    |         (active ? 0x40 : 0x00)
    |                   0x80;
  
  SERIAL_PORT.write(serialData, sizeof(serialData));
#endif
  
  if (useMouse)
  {
#ifdef MOUSE_INTERFACE
    if (active)
    {
      Mouse.moveTo(outputX, outputY);
    }
#endif
  }
  else
  {
#ifdef JOYSTICK_INTERFACE
    #ifdef ALT_AXIS
      Joystick.sliderRight(outputX);
      Joystick.sliderLeft(outputY);
    #else
      Joystick.X(outputX);
      Joystick.Y(outputY);
    #endif

    Joystick.send_now();
#endif
  }
}

