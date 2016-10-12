
// This file contains methods used to read the buttons states.

// This is where you need to connect your buttons on the Teensy
// (connect the other side of the buttons to any Teensy ground pin)
#define TRIGGER_PIN     0
#define BUTTON1_PIN     1
#define BUTTON2_PIN     2

// If you want more buttons, follow the logic of button1 & 2 as a template.
// The Teensy Joystick template supports up to 16 buttons natively

#define HOLD_TIMEOUT 7000
// If you need to increase the trigger hold down time (to avoid entering calibration
//  while spraying down cyborgs in T2, for instance), increase the value to 30000

unsigned long lastButtonRelease = 0;

bool trigger = false;
bool button1 = false;
bool button2 = false;


float top = 0.0f;
float bottom = 0.0f;
float left = 0.0f;
float right = 0.0f;
float omegaCalib = 0.0f;


bool initButtons()
{
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  return true; //This one never fails...
}


void checkButtons()
{
  // This will check actual pins & generate events on change.
  // It will also do screen range calibration when the trigger is held down for some time
  // For those wondering, no need to debounce - most MAME games already implement some form of control debouncing anyways.
  //  it would only introduce more lag

  bool triggerPressed = !digitalRead(TRIGGER_PIN);
  bool button1Pressed = !digitalRead(BUTTON1_PIN);
  bool button2Pressed = !digitalRead(BUTTON2_PIN);

  if (triggerPressed != trigger)
  {
    setButtonState(1, triggerPressed);
    trigger = triggerPressed;
  }

  if (button1Pressed != button1)
  {
    setButtonState(2, button1Pressed);
    button1 = button1Pressed;
  }

  if (button2Pressed != button2)
  {
    setButtonState(3, button2Pressed);
    button2 = button2Pressed;
  }

  if (triggerPressed)
  {
    if (!calibrating && (millis() - lastButtonRelease) > HOLD_TIMEOUT)
    {
      if (button1Pressed)
      {
        //Switch mode
        useMouse = !useMouse;

        initOutputDevice();

        lastButtonRelease = millis(); //So that is doesn't switch in a continuous loop
      }
      else
      {
        //calibrate screen area et Upper left corner
        calibrating = true;

        top = pitch;
        bottom = pitch;
        left = heading;
        right = heading;
        omegaCalib = omegaBlobs;
      }
    }

    if (calibrating)
    {
      //Still calibrating, find screen boundaries
      if (pitch < top) top = pitch;
      if (pitch > bottom) bottom = pitch;
      if (heading > left) left = heading;
      if (heading < right) right = heading;
    }
  }
  else
  {
    if (calibrating)
    {
      //Trigger released after calibration
      omegaCalib = (omegaCalib + omegaBlobs) / 2.0f;
      centerHeading = (left + right) / 2.0f;
      centerPitch = (top + bottom) / 2.0f;

      rangeHeading = (left - right) / omegaCalib;
      rangePitch = (bottom - top) / omegaCalib;

      saveScreenCalib();

      calibrating = false;
    }

    lastButtonRelease = millis();
  }
}

