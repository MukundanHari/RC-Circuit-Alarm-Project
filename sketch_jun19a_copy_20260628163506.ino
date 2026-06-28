#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// ==========================
// OLED SETTINGS
// ==========================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// ==========================
// PIN DEFINITIONS
// ==========================

const int potPin   = A0;
const int startPin = 7;
const int compPin  = 4;

// ==========================
// VARIABLES
// ==========================

bool timerRunning = false;
bool timerFinished = false;

bool previousSwitchState = false;

unsigned long startTime = 0;
unsigned long observedTime = 0;

float liveExpectedTime = 0;
float capturedExpectedTime = 0;

// ==========================
// SETUP
// ==========================

void setup() {

  pinMode(startPin, INPUT);
  pinMode(compPin, INPUT);

  display.begin(
    SSD1306_SWITCHCAPVCC,
    0x3C
  );

  display.clearDisplay();
  display.display();
}

// ==========================
// LOOP
// ==========================

void loop() {

  // --------------------------
  // Read potentiometer
  // --------------------------

  int adc = analogRead(potPin);

  float voltage =
      adc * (5.0 / 1023.0);

  float resistance;

  if (voltage >= 4.99) {

    resistance = 97056;

  } else {

    resistance =
      (voltage * 94000.0) /
      (5.0 - voltage);
  }

  // --------------------------
  // Calculate live expected time
  // --------------------------

  liveExpectedTime =
    1601.49 *
    log(
      1.0 +
      resistance / 94000.0
    );

  // --------------------------
  // Read switch and comparator
  // --------------------------

  bool switchState =
      digitalRead(startPin);

  bool comparatorTriggered =
      digitalRead(compPin);

  // =====================================================
  // SWITCH JUST CLOSED
  // =====================================================

  if (
      switchState &&
      !previousSwitchState
     ) {

    capturedExpectedTime =
      liveExpectedTime;

    timerRunning = true;
    timerFinished = false;

    observedTime = 0;

    startTime = millis();
  }

  // =====================================================
  // TIMER RUNNING
  // =====================================================

  if (timerRunning) {

    observedTime =
      (millis() - startTime) / 1000;

    if (comparatorTriggered) {

      observedTime =
        (millis() - startTime) / 1000;

      timerRunning = false;
      timerFinished = true;
    }
  }

  // =====================================================
  // SWITCH OPENED AGAIN
  // RESET FOR NEXT RUN
  // =====================================================

  if (
      !switchState &&
      previousSwitchState
     ) {

    timerRunning = false;
    timerFinished = false;

    observedTime = 0;

    capturedExpectedTime = 0;
  }

  // =====================================================
  // OLED DISPLAY
  // =====================================================

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  // --------------------------
  // Expected Time
  // --------------------------

  display.setCursor(0, 0);

  display.print("Expected: ");

  if (switchState) {

    display.print(
      capturedExpectedTime,
      0
    );

  } else {

    display.print(
      liveExpectedTime,
      0
    );
  }

  display.print("s");

  // --------------------------
  // Observed Time
  // --------------------------

  display.setCursor(0, 16);

  display.print("Observed: ");
  display.print(observedTime);
  display.print("s");

  // --------------------------
  // Status
  // --------------------------

  display.setCursor(0, 32);

  if (timerRunning) {

    display.print("Status: TIMING");

  } else if (timerFinished) {

    display.print("Status: DONE");

  } else {

    display.print("Status: READY");
  }

  // --------------------------
  // Error
  // --------------------------

  display.setCursor(0, 48);

  if (timerFinished) {

    long error =
      (long)observedTime -
      (long)capturedExpectedTime;

    display.print("Err: ");
    display.print(error);
    display.print("s");

  } else {

    display.print("Err: ---");
  }

  display.display();

  // --------------------------
  // Save previous switch state
  // --------------------------

  previousSwitchState =
      switchState;

  delay(100);
}