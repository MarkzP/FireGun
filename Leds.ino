
// This file contains methods used to turn leds on or off

// Up to 2 leds are supported:
//  -The Onboard led (comment to turn off)
#define INT_LED LED_BUILTIN

//  -An external led driven by a 20ma capable pin via a ~120ohm resistor
//    (uncomment one or none)
#define EXT_LED 5
//#define EXT_LED 16
//#define EXT_LED 17
//#define EXT_LED 21

void initLeds()
{
#ifdef INT_LED
  pinMode(INT_LED, OUTPUT);
#endif
#ifdef EXT_LED
  pinMode(EXT_LED, OUTPUT);
#endif
}

// Helper function to simplify turning all leds on or off
void led(uint8_t state)
{
#ifdef INT_LED
  digitalWrite(INT_LED, state);
#endif
#ifdef EXT_LED
  digitalWrite(EXT_LED, state);
#endif
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
  led(calibrating ? HIGH : LOW);
}
