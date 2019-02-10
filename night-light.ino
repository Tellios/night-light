#include <FastLED.h>

// Neopixel strip data pin
#define LED_DATA_PIN 6
#define NUM_LEDS 60

// PIR motion sensor pin
#define MOTION_DATA_PIN 4

// In milliseconds, how long motion should be disabled when
// the potentiometer input changes (so the motion sensor does
// not interfer when changing brightness levels)
#define DISABLE_MOTION_THRESHOLD 4000

// Potentiometer input pin
#define BRIGHT_ANALOG_PIN A0

// How much of an increment the potentiometer must differentiate
// from the old value for the new value to be accepted
#define BRIGHTNESS_CHANGE_THRESHOLD 3


// Define the array of leds
CRGB leds[NUM_LEDS];
int currentBrightness;
int currentMotion;
int currentMotionEnabled = true;
unsigned long disableMotionSnapshot = 0;

void setup() {
  Serial.begin(9600);

  Serial.println("Setup started");

  pinMode(MOTION_DATA_PIN, INPUT);
  initializeLeds();

  Serial.println("Setup finished");
}

void initializeLeds() {
  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS);

  for (uint16_t i = 0; i < NUM_LEDS; i += 3) {
    leds[i].setRGB(255, 255, 40);
  }

  FastLED.setBrightness(0);
  FastLED.show();
}

void loop() {
  int motion = digitalRead(MOTION_DATA_PIN);
  int brightness = analogRead(BRIGHT_ANALOG_PIN) / 4;
  int brightnessDifference = currentBrightness - brightness;
  int motionEnabled = isMotionEnabled();

  if (motionEnabled != currentMotionEnabled) {
    currentMotionEnabled = motionEnabled;

    if (currentMotionEnabled) {
      /*
         When the potentiomater haven't been changed after a few
         seconds we turn off the leds and resets the
         disableMotionSnapshot variable
      */
      disableMotionSnapshot = 0;
      turnOffLeds();
    }
  }

  /*
     Since the potentiometer can give small changes between
     readings even though it haven't been changed we use a
     small threshold
  */
  if (abs(brightnessDifference) > BRIGHTNESS_CHANGE_THRESHOLD) {
    /*
       The potentiometer reading has changed and we want
       to display the new brightness level
    */
    disableMotionSnapshot = millis();
    currentBrightness = brightness;
    FastLED.setBrightness(currentBrightness);
    FastLED.show();
  } else if (motion != currentMotion && currentMotionEnabled) {
    currentMotion = motion;

    if (currentMotion == HIGH) {
      Serial.println("Motion detected");
      brightenLeds();
    } else {
      Serial.println("Motion no longer detected");
      dimLeds();
    }
  }
}

/**
   Returns true if motion events should be handled
*/
int isMotionEnabled() {
  if (disableMotionSnapshot > 0) {
    int motionDisabledTime = millis() - disableMotionSnapshot;

    if (motionDisabledTime > DISABLE_MOTION_THRESHOLD) {
      return true;
    } else {
      return false;
    }
  }

  return true;
}

/**
   Animates the leds brightness to the current brightness level
*/
void brightenLeds() {
  for (uint16_t i = 0; i <= currentBrightness; i++) {
    FastLED.setBrightness(i);
    FastLED.show();
    delay(20);
  }
}

/**
   Animates the leds from the current brightness level to 0
*/
void dimLeds() {
  for (int i = currentBrightness; i >= 0; i--) {
    FastLED.setBrightness(i);
    FastLED.show();
    delay(20);
  }

  turnOffLeds();
}

/**
   Turns off all leds
*/
void turnOffLeds() {
  FastLED.setBrightness(0);
  FastLED.show();
}
