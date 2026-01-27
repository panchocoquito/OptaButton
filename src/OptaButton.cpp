/*
 * OptaButton.cpp
 * Button handler implementation with comments on each line
 */

#include "OptaButton.h"  // include our header

// Shared gate so expansion inputs are refreshed at most once per loop pass
static uint32_t g_optaExpRefreshToken = 0;  // increments once per button update call chain

// Constructor implementation
OptaButton::OptaButton(
  DefLab::ButtonInputMode mode,  // hardware mode
  uint8_t inputPin,              // pin or channel
  const char* label,             // button name
  uint16_t debounceMs,
  bool inverted,
  uint16_t longPressMs,
  uint16_t repeatStartMs,
  uint16_t repeatMinMs,
  uint8_t accelRate)

  // Now that you've named the public parameters, assign them
  : inputMode(mode),                       // save mode
    inputID(inputPin),                     // save pin
    name(label),                           // save label
    debounceTime(debounceMs),              // save debounce time
    invertedLogic(inverted),               // save inversion flag
    longPressThreshold(longPressMs),       // save long-press threshold
    repeatIntervalStart(repeatStartMs),    // save initial repeat interval
    repeatIntervalMin(repeatMinMs),        // save minimum repeat interval
    currentRepeatInterval(repeatStartMs),  // start repeats at initial interval
    acceleration(accelRate),               // save acceleration speed

    // And initialize these runtime variables
    lastUpdateTime(0),           // no updates yet
    edgeTime(0),                 // no edge yet
    lastRepeatTime(0),           // no repeats yet
    lastAccelUpdate(0),          // no accel steps yet
    rawState(false),             // assume not pressed
    debouncing(false),           // not in debounce initially
    currentPressed(false),       // debounced state false
    longPressActive(false),      // long-press not active
    shortPressDetected(false),   // clear all event flags
    releaseDetected(false),      //
    longPressDetected(false),    //
    longReleaseDetected(false),  //
    repeatTriggered(false),      //
    longPressReported(false)     // initialize the guard
{
  // Constructor body empty: all initialization done above
}

// ---------- begin() ----------
void OptaButton::begin() {
  switch (inputMode) {
    case DefLab::ButtonInputMode::GPIO:
      pinMode(inputID, INPUT_PULLUP);  // use internal pullup resistor
      break;
    case DefLab::ButtonInputMode::OPTA_CTL:
      pinMode(inputID, INPUT);  // simple digital input
      break;
    case DefLab::ButtonInputMode::EXP_DIG:
      // no setup needed for digital expansion
      break;
  }
}

// ---------- update() ----------
void OptaButton::update() {
  // Clear all the event flags first
  shortPressDetected = false;
  releaseDetected = false;
  longPressDetected = false;
  longReleaseDetected = false;
  repeatTriggered = false;

  // Then check the loop timer
  uint32_t now = millis();                              // read current time
  if (now - lastUpdateTime < LOOP_INTERVAL_MS) return;  // too soon, skip
  lastUpdateTime = now;                                 // mark this update time

  // Gate token: advance at most once per millisecond tick
  static uint32_t lastTokenMs = 0;  // last millis() tick used to advance the token
  if (now != lastTokenMs) {         // only advance once per tick
    g_optaExpRefreshToken++;        // token changes, enabling one refresh
    lastTokenMs = now;              // remember this tick
  }

  // Then poll the pins
  bool pressed = readInput();  // return true if the hardware reads “pressed”

  // If state just changed AND we’re not already waiting out a debounce, treat it as a real edge
  if (pressed != rawState && !debouncing) {
    rawState = pressed;  // remember this new raw input so we can detect future changes
    debouncing = true;   // starting next loop, pass this logic gate until debounceTime
    edgeTime = now;      // stamp the exact millisecond of this transition for timing

    if (pressed) {
      shortPressDetected = true;                    // fire a one‑time “button down” event right now
      currentPressed = true;                        // immediately update our logical state to “down”
      longPressActive = false;                      // clear any leftover long‑press status
      currentRepeatInterval = repeatIntervalStart;  // reset the repeat delay back to its initial value
      lastRepeatTime = now;                         // schedule the first repeat after that start delay
      lastAccelUpdate = now;                        // start counting from now toward the next speed‑up
    } else {
      releaseDetected = true;                 // fire a one‑time “button up” event right now
      longReleaseDetected = longPressActive;  // if we were in a long‑press, report its end
      currentPressed = false;                 // immediately update our logical state to “up”
      longPressActive = false;                // turn off the long‑press flag so repeats stop
      longPressReported = false;              // allow the next press to be reported as a long‑press again
    }
  }

  // After the debounce window has passed, allow new edges to be detected
  if (debouncing && (now - edgeTime >= debounceTime)) {
    debouncing = false;  // exit debounce, so (pressed != rawState) can fire again
  }

  // Once we’re out of debounce AND the button is held down, handle long‑press timing and repeats
  if (!debouncing && currentPressed) {
    // Only fire the long‑press event once, when the hold time crosses the threshold
    if (!longPressReported && (now - edgeTime >= longPressThreshold)) {
      longPressDetected = true;  // report “you’ve held it long enough” this one time
      longPressActive = true;    // enter the long‑press state so repeats can happen
      longPressReported = true;  // block any further long‑press events until release
      lastRepeatTime = now;      // reset repeat timer so the first repeat waits the full interval
      lastAccelUpdate = now;     // reset accel timer so we don’t speed up immediately
    }

    // If we’re in long‑press mode and the repeat interval has elapsed, fire another repeat
    if (longPressActive && (now - lastRepeatTime >= currentRepeatInterval)) {
      repeatTriggered = true;                   // report a repeat event now
      lastRepeatTime += currentRepeatInterval;  // schedule the next one at the same interval
    }

    // Once per second during a long‑press, shorten the repeat interval until it hits the minimum
    if (longPressActive
        && (now - lastAccelUpdate >= 1000)
        && currentRepeatInterval > repeatIntervalMin) {
      // subtract our acceleration amount, but never go below the configured minimum
      currentRepeatInterval = max(
        int(currentRepeatInterval - acceleration),
        int(repeatIntervalMin));
      lastAccelUpdate = now;  // reset the 1 s accel timer for the next speed‑up
    }
  }
}

// Low-level raw input read with inversion applied
bool OptaButton::readInput() {
  bool raw = false;  // default to not pressed
  switch (inputMode) {
    case DefLab::ButtonInputMode::GPIO:
      raw = (digitalRead(inputID) == LOW);  // active-LOW wiring
      break;
    case DefLab::ButtonInputMode::OPTA_CTL:
      raw = (digitalRead(inputID) == HIGH);  // active-HIGH wiring
      break;
    case DefLab::ButtonInputMode::EXP_DIG:
      // Only compile this on Opta targets
#if OPTA == 1
      // Gate expansion refresh so it happens at most once per loop pass
      static uint32_t lastExpRefreshToken = 0;  // last token we refreshed on

      // Loop through mechanical and solid‑state expansion
      for (int i = 0; i < OPTA_CONTROLLER_MAX_EXPANSION_NUM; i++) {
        Opta::DigitalMechExpansion mechExp = Opta::OptaController.getExpansion(i);
        Opta::DigitalStSolidExpansion solidExp = Opta::OptaController.getExpansion(i);

        if (mechExp) {  // if mechanical expansion present
          if (g_optaExpRefreshToken != lastRefreshToken) {
            mechExp.updateDigitalInputs();  // refresh its input state once per loop
            lastExpRefreshToken = g_optaExpRefreshToken;
          }
          raw = mechExp.digitalRead(inputID);  // read channel state
          break;                               // stop after first valid expansion match
        }
        if (solidExp) {  // if solid-state expansion present
          if (g_optaExpRefreshToken != lastRefreshToken) {
            solidExp.updateDigitalInputs();  // refresh its input state once per loop
            lastExpRefreshToken = g_optaExpRefreshToken;
          }
          raw = solidExp.digitalRead(inputID);  // read channel state
          break;                                // stop after first valid expansion match
        }
      }
#endif
      break;
  }
  return invertedLogic ? !raw : raw;  // apply inversion if needed
}

// Query functions return the flags set in update()
bool OptaButton::isShortPressed() const {
  return shortPressDetected;
}
bool OptaButton::isReleased() const {
  return releaseDetected;
}
bool OptaButton::isLongPressed() const {
  return longPressDetected;
}
bool OptaButton::isLongReleased() const {
  return longReleaseDetected;
}
bool OptaButton::isRepeating() const {
  return repeatTriggered;
}
const char* OptaButton::getLabel() const {
  return name;
}

// OptaButton.cpp
