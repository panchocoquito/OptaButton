/*
  NAME:
    OptaButton — Button handler with short press, long press, and hold-repeat behavior

  VERSION:
    2026-01-26 - v1.0
    • Added DefLab_Common
    • Added loop token to the exp update calls  
  
  Purpose
  A flexible button handling class designed for Opta that also works on older Arduino
  Gives a simple but powerful API for buttons:
    • Tap (short press) event
    • Hold (long press) event
    • Hold-and-repeat with gradual acceleration

  Interface scenarios
    • Long press to enter a special mode
    • Tap to confirm or cycle values
    • Hold to increment/decrement values quickly

  Input Modes & Recommended Wiring
    • GPIO (Arduino Uno/Mega and other AVR boards)
        – Use INPUT_PULLUP, wire SPST between pin and GND
        – LOW = pressed; set invertedLogic = false (default)

    • OPTA_CTL (Opta controller AFX00001/02/03)
        – Wire SPST between pin and +24 V (internal input handles isolation)
        – HIGH = pressed; set invertedLogic = false (default)

    • EXP_DIG (Opta digital expansion AFX00005/06)
        – Wire SPST between expansion DI channel and +24 V
        – HIGH = pressed; set invertedLogic = false (default)

  Choose invertedLogic only if your wiring makes the pressed state opposite of the default.

  How to Use the Library
    1. Include header
    2. Select your platform at the #define OPTA
    3. Instantiate DefLabButton objects
    4. Call begin() in setup()
    5. Call update() in loop()
    6. Use isShortPressed(), isLongPressed(), isRepeating()
*/

#pragma once  // guard against multiple inclusion

#include <Arduino.h>        // include Arduino core
#include <DefLab_Common.h>  // shared enums + debug helpers

// ---------- PLATFORM Control ----------
#ifndef OPTA
// Auto‑detect common cores
#if defined(ARDUINO_ARCH_MBED)
#define OPTA 1  // Opta family
#elif defined(ARDUINO_ARCH_AVR)
#define OPTA 0  // Uno/Mega/etc
#else
#define OPTA 0  // fallback to classic
#endif
#endif

#if OPTA == 1
#include <OptaBlue.h>  // Required for expansion support
#endif

#if OPTA == 1
#define OPTA_BEGIN() Opta::OptaController.begin()
#define OPTA_UPDATE() Opta::OptaController.update()
#else
// Stubs on non‑Opta builds
#define OPTA_BEGIN() ((void)0)
#define OPTA_UPDATE() ((void)0)
#endif

// ---------- OptaButton CLASS ----------

// Define timing variables
static constexpr uint16_t LOOP_INTERVAL_MS = 1;  // minimum ms between updates

class OptaButton {
public:
  // ---------- Constructor ----------
  OptaButton(
    DefLab::ButtonInputMode mode,  // which input hardware to use
    uint8_t inputPin,              // pin number or channel index
    const char* label,             // human-readable name for debugging
    uint16_t debounceMs = 20,      // ms to ignore bounce after edge
    bool inverted = false,         // true if LOW=pressed instead of HIGH
    uint16_t longPressMs = 800,    // ms to hold before long press fires
    uint16_t repeatStartMs = 100,  // initial delay between repeats
    uint16_t repeatMinMs = 8,      // fastest delay when accelerating
    uint8_t accelRate = 100        // how much to speed up per second
  );                               // end constructor

  void begin();   // call in setup() to configure hardware for the chosen mode
  void update();  // call in loop() to handle timing and events

  // ---------- Query Functions ----------
  bool isShortPressed() const;   // true if just pressed (first ms)
  bool isReleased() const;       // true if just released
  bool isLongPressed() const;    // true if long-press just started
  bool isLongReleased() const;   // true if long-press just ended
  bool isRepeating() const;      // true for each repeat during hold
  const char* getLabel() const;  // return the button name

private:
  // Configuration values stored once
  const DefLab::ButtonInputMode inputMode;  // which hardware mode
  const uint8_t inputID;                    // pin or channel
  const char* name;                         // label for prints
  const uint16_t debounceTime;              // how long to wait after edge
  const bool invertedLogic;                 // flip raw HIGH/LOW if needed
  const uint16_t longPressThreshold;        // how long to hold for long press
  const uint16_t repeatIntervalStart;       // starting interval for repeats
  const uint16_t repeatIntervalMin;         // fastest interval
  uint16_t currentRepeatInterval;           // running interval that shrinks
  uint8_t acceleration;                     // speed-up in ms per second

  // Runtime variables updated each loop
  uint32_t lastUpdateTime;   // last millis() when update() ran
  uint32_t edgeTime;         // millis() when last edge occurred
  uint32_t lastRepeatTime;   // millis() when last repeat event fired
  uint32_t lastAccelUpdate;  // millis() when last acceleration step happened

  bool rawState;         // last raw readInput() value
  bool debouncing;       // true until debounceTime has passed
  bool currentPressed;   // egde-triggerd state (true if pressed)
  bool longPressActive;  // true after longPressDetected until release

  // These flags are cleared each update() then set when events occur
  bool shortPressDetected;   // set true on press edge
  bool releaseDetected;      // set true on release edge
  bool longPressDetected;    // set true when entering long-press
  bool longReleaseDetected;  // set true when leaving long-press on release
  bool repeatTriggered;      // set true on each repeat interval

  // Guard so longPressDetected only fires once per physical press
  bool longPressReported;

  // ---------- Helper Methods ----------
  bool readInput();  // low-level read of the hardware, applies inversion
};

// OptaButton.h
