
// This file contains methods used to turn leds on or off

//  -An external led driven by a 20ma capable pin via a ~120ohm resistor (pins 5, 16, 17 & 21)
//    (uncomment one or none)
#define EXT_LED     16
#define EXT_LED_2   5

uint8_t flashState = HIGH;

void initLeds()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(EXT_LED, OUTPUT);
  pinMode(EXT_LED_2, OUTPUT);
}

// Helper function to simplify turning all leds on or off
void led(uint8_t state)
{
  digitalWriteFast(EXT_LED, state);
  digitalWriteFast(EXT_LED_2, state);
}

// Flash forever if something goes wrong
void flashOfDeath(int quickFlashes)
{
  for (;;)
  {
    for (int i = 0; i < quickFlashes; i++)
    {
      led(HIGH);
      delay(200);
      led(LOW);
      delay(200);      
    }
    delay(1000);
  }
}

void refreshLeds()
{
  // For now, only turn on led when calibrating
  if (calibrating)
  {
    if (irConfidence > 0.5f) flashState = !flashState;
    else flashState = HIGH;
    digitalWriteFast(EXT_LED, flashState);
    digitalWriteFast(EXT_LED_2, flashState);
  }
  else
  {
    digitalWriteFast(EXT_LED, LOW);
    digitalWriteFast(EXT_LED_2, LOW);
  }  
}
