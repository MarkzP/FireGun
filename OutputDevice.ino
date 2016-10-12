
// This file contains methods used to interact with the joystick / mouse objects.

#define OUTPUT_MIN 0
#define OUTPUT_MAX 1023 //Per Teensy specs
#define OUTPUT_MID 512 //Per Teensy specs

bool offscreen = false;
bool wasOnscreen = false;

#define OFFSCREEN_TIMEOUT 5000
unsigned long lastSeenOnScreen = 0;

int outputX = 0;
int outputY = 0;

bool initOutputDevice()
{
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

  if (Mouse.isPressed(MOUSE_LEFT)) Mouse.release(MOUSE_LEFT);
  if (Mouse.isPressed(MOUSE_RIGHT)) Mouse.release(MOUSE_RIGHT);
  if (Mouse.isPressed(MOUSE_MIDDLE)) Mouse.release(MOUSE_MIDDLE);

  Mouse.screenSize(OUTPUT_MAX, OUTPUT_MAX);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);

  Joystick.send_now();

  return true;
}

void setButtonState(uint8_t button, bool val)
{
  if (useMouse)
  {
    if (val)
    {
      switch (button)
      {
        case 1: Mouse.press(MOUSE_LEFT); break;
        case 2: Mouse.press(MOUSE_RIGHT); break;
        case 3: Mouse.press(MOUSE_MIDDLE); break;
      }
    }
    else
    {
      switch (button)
      {
        case 1: Mouse.release(MOUSE_LEFT); break;
        case 2: Mouse.release(MOUSE_RIGHT); break;
        case 3: Mouse.release(MOUSE_MIDDLE); break;
      }
    }
  }
  else
  {
    Joystick.button(button, val);
  }
}

void sendPosition()
{
  if (calibrating)
  {
    return;
  }

  //Compute screen position based on orientation angles
  outputX = (((heading - centerHeading) / (-rangeHeading * omegaBlobs)) * OUTPUT_MAX) + OUTPUT_MID;
  outputY = (((pitch - centerPitch) / (rangePitch * omegaBlobs)) * OUTPUT_MAX) + OUTPUT_MID;

  //Determine if we are offscreen
  offscreen = outputX >= OUTPUT_MAX || outputX < OUTPUT_MIN || outputY >= OUTPUT_MAX || outputY < OUTPUT_MIN;

  if (useMouse)
  {
    if (offscreen)
    {
      //Use the edge of screen coordinate
      outputX = constrain(outputX, 0, OUTPUT_MAX);
      outputY = constrain(outputY, 0, OUTPUT_MAX);
    }

    if (!offscreen || wasOnscreen)
    {
      //This will ensure the mouse moves to the edge of the screen, but it will then stop updating
      // so you can use your *other* mouse
      Mouse.moveTo(outputX, outputY);
    }
  }
  else
  {
    if (offscreen)
    {
      if ((millis() - lastSeenOnScreen) < OFFSCREEN_TIMEOUT)
      {
        //Go to the lower left corner to facilitate reload...
        Joystick.X(OUTPUT_MAX);
        Joystick.Y(OUTPUT_MAX);
      }
      else
      {
        //...but eventually, go back to center
        Joystick.X(OUTPUT_MID);
        Joystick.Y(OUTPUT_MID);
      }
    }
    else
    {
      //Joystick normally
      lastSeenOnScreen = millis();
      Joystick.X(outputX);
      Joystick.Y(outputY);
    }

    Joystick.send_now();
  }

  wasOnscreen = !offscreen;
}

