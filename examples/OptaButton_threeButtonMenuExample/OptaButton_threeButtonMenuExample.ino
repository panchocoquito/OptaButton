/*
  Example Sketch: Menu Interface with Three-Button Control

  Code Prompt:
    Build a simple user interface using three buttons {Program, Up, Down}
    that works on an Arduino AVR (Uno, Mega) or micro PLC (Opta)
    The interface follows a common pattern for small, embedded systems:
      • PROGRAM: long-press to enter or exit "program mode"
      • PROGRAM: short-press to switch between adjustable settings
      • UP / DOWN: adjust values with auto-repeat and acceleration
      • Serial print is suppressed once min or max value has been reached

  Settings include:
    0. Volume      – range 0–100
    1. Brightness  – range 0–255
    2. Contrast    – range 0–100

  OptaButton object parameter list:
    Required parameters
    – inputMode = Controller type (GPIO, OPTA_CTL, EXP_DIG)
        • GPIO: Anything compiling with Arduino AVR Board manager (Uno, Mega, etc)
        • OPTA_CTL: Opta Controllers AFX00001/02/03
        • EXP_DIG: Opta Digital Expansion modules AFX00005/06
    – inputPin = Arduino pin name (or channel ID if Opta exp) the button is wired to
    – label = human-readable name of your button for HMI or serial monitor display
    Optional parameters
    – debounceMs = Time for input settling after edge detection; defaults to 20 ms
    – inverted = True if wiring backward (e.g., HIGH=pressed on an Uno; see Wiring Notes); default false
    – longPressMs = How long to hold until long press is triggered; defaults to 800 ms
    – repeatStartMs = Initial delay between repeats for accelerated hold counting; defaults to 100 ms
    – repeatMinMs = Fastest delay between repeats after full acceleration; defaults to 8 ms
    – accelRate = How much to speed up during acceleration; defaults to 10 ms/s

  Implementation Overview:
    1. Include OptaButton library
    2. Declare three buttons, using your board's input mode (GPIO, OPTA_CTL, EXP_DIG)
    3. Use enum + name arrays to manage multiple settings
    4. Define min/max ranges and default values for each setting
    5. In setup(): initialize serial, expansions (Opta), and button inputs
    6. In loop():
         • Call update() on each button
         • Long press PROGRAM toggles programMode flag
         • Within programMode:
             – PROGRAM short-press moves through settings
             – UP/DOWN hold-repeat adjusts values within limits

  How to Use:
    • Long-hold the “Program” button to enable adjustment mode
    • Tap “Program” to choose setting (Volume, Brightness, Contrast)
    • Hold “Up” or “Down” to change values; repeat rate accelerates as the button remains held
    • Exit program mode by holding “Program” again
  Wiring Notes by Input Mode:
    • GPIO: classic INPUT_PULLUP, button to GND, LOW=pressed
    • OPTA_CTL: onboard input, button to +24 V, HIGH=pressed
    • EXP_DIG: expansion module, button to +24 V, HIGH=pressed
    • Opta Analog Expansion AFX00007 not recommended for reliable pushbutton input
*/

#include "OptaButton.h"         // include the OptaButton library header (brings in Arduino core too)
using DefLab::ButtonInputMode;  // alias the shared enum so we can write ButtonInputMode::GPIO, etc

// ---------- Menu Settings Model ----------

// Define a small ID list for our menu items (these become array indexes below)
enum Settings {        // a plain enumumeration for settings (used like an int index)
  SETTING_VOLUME,      // 0 = first setting
  SETTING_BRIGHTNESS,  // 1 = second setting
  SETTING_CONTRAST,    // 2 = third setting
  NUM_SETTINGS         // 3 = total count (keep this last; it sizes the arrays)
};

// Write a name table for the settings (index with currentSetting)
const char* settingNames[NUM_SETTINGS] = {
  // array length matches NUM_SETTINGS
  "Volume",      // name for SETTING_VOLUME (index 0)
  "Brightness",  // name for SETTING_BRIGHTNESS (index 1)
  "Contrast"     // name for SETTING_CONTRAST (index 2)
};

// Minimum allowed value for each setting (same index scheme as settingNames)
const int settingMin[NUM_SETTINGS] = {
  // min constraints for each setting
  0,  // Volume minimum
  0,  // Brightness minimum
  0   // Contrast minimum
};

// Maximum allowed value for each setting (same index scheme as settingNames)
const int settingMax[NUM_SETTINGS] = {
  // max constraints for each setting
  100,  // Volume maximum
  255,  // Brightness maximum (common 8-bit range)
  100   // Contrast maximum
};

// Current live values for each setting (these are the numbers we edit in program mode)
int settingValues[NUM_SETTINGS] = {
  // current values (we set the starting values/defaults here)
  50,   // Volume starts at 50
  128,  // Brightness starts at mid-range
  75    // Contrast starts at 75
};

// One-shot print suppression flags (per-setting) so we don't spam Serial at the limits
bool printedAtMin[NUM_SETTINGS] = {
  // "we already printed the min message once"
  false,  // for Volume
  false,  // for Brightness
  false   // for Contrast
};

bool printedAtMax[NUM_SETTINGS] = {
  // "we already printed the max message once"
  false,  // for Volume
  false,  // for Brightness
  false   // for Contrast
};

// ---------- Button Declarations ----------

// Create an OptaButton for the PROGRAM button (this is our mode/selection button)
OptaButton btnProgram(    // create an object of type OptaButton named btnProgram
  ButtonInputMode::GPIO,  // inputMode: GPIO means classic Arduino digital pin (INPUT_PULLUP)
  3,                      // inputPin: Arduino digital pin number (D3 on Uno/Mega)
  "Program"               // label: human-friendly name (useful for debugging / HMI text)
);                        // constructor ends here (other params use defaults)

// Create an OptaButton for the UP button (increments the selected setting)
OptaButton btnUp(         // create an object named btnUp
  ButtonInputMode::GPIO,  // inputMode: same GPIO wiring scheme
  4,                      // inputPin: Arduino digital pin number (D4)
  "Up"                    // label: name for prints / UI
);                        // end btnUp

// Create an OptaButton for the DOWN button (decrements the selected setting)
OptaButton btnDown(       // create an object named btnDown
  ButtonInputMode::GPIO,  // inputMode: same GPIO wiring scheme
  5,                      // inputPin: Arduino digital pin number (D5)
  "Down"                  // label: name for prints / UI
);                        // end btnDown

// ---------- Menu Runtime State ----------

// Program mode flag: false = read-only mode, true = edit mode
bool programMode = false;  // start with editing disabled

// Currently selected menu item (this value is used as an index into the arrays)
Settings currentSetting = SETTING_VOLUME;  // start with Volume selected

// ---------- SETUP ----------

void setup() {
  // Indicate the board is powered on (handy on Opta because it doesn't always show power status clearly)
  pinMode(LED_BUILTIN, OUTPUT);     // configure built-in LED pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on so we know the sketch started

  // Start Serial so we can see prompts and value changes
  Serial.begin(115200);  // open serial port at 115200 baud
  delay(500);            // small delay so the Serial Monitor can attach after reset

  // Start the Opta core (safe no-op on AVR, required for Opta expansion support)
  OPTA_BEGIN();  // expands to OptaController.begin() on Opta; compiles out elsewhere

  // Initialize the hardware side of each button (pinMode, etc, based on selected input mode)
  btnProgram.begin();  // sets up the PROGRAM input pin/channel
  btnUp.begin();       // sets up the UP input pin/channel
  btnDown.begin();     // sets up the DOWN input pin/channel

  // Print a starting message so the user knows what to do next
  Serial.println("Ready. Press and hold PROGRAM to start adjusting settings.");  // beginner prompt
}

// ---------- LOOP ----------

void loop() {
  // 1] Update the Opta controller core (safe no-op on AVR; important on Opta for expansions)
  
  OPTA_UPDATE();  // expands to OptaController.update() on Opta; compiles out elsewhere

  // Update button state machines (debounce, edge detection, long press, repeat timing)
  btnProgram.update();  // refresh PROGRAM events for this loop
  btnUp.update();       // refresh UP events for this loop
  btnDown.update();     // refresh DOWN events for this loop

  // 2] Check for PROGRAM long-press to toggle program mode

  if (btnProgram.isLongPressed()) {                        // true only on the moment we cross the long-press threshold
    programMode = !programMode;                            // flip edit mode on/off
    Serial.print("Program mode ");                         // print a label
    Serial.println(programMode ? "ENABLED" : "DISABLED");  // print the new state (ternary operator)
  }

  // If we are not in program mode, stop here (we still updated buttons above, so mode can be entered anytime)
  if (!programMode) return;  // exit loop early; nothing else should change when not editing

  // 3] Check for PROGRAM short-press to select which setting to edit

  if (btnProgram.isShortPressed()) {                                 // true only on the press edge (short press event)
    currentSetting = Settings((currentSetting + 1) % NUM_SETTINGS);  // wrap to 0 after last setting
    Serial.print("Selected: ");                                      // label
    Serial.println(settingNames[currentSetting]);                    // print the name by indexing the name array

    // Reset boundary print suppression for the newly selected setting
    printedAtMin[currentSetting] = false;  // allow printing again if this setting is at MIN
    printedAtMax[currentSetting] = false;  // allow printing again if this setting is at MAX
  }

  // 4] Check for UP button to increase the selected value with repeat

  if (btnUp.isShortPressed() || btnUp.isRepeating()) {  // either a tap OR an auto-repeat tick while holding
    settingValues[currentSetting]++;                    // increase the current setting by 1

    // Clamp the value so it never goes below min or above max for that setting
    settingValues[currentSetting] = constrain(  // constrain(x, min, max) returns a clamped value
      settingValues[currentSetting],            // x: the value we are clamping
      settingMin[currentSetting],               // min: look up the correct min for this setting
      settingMax[currentSetting]                // max: look up the correct max for this setting
    );

    // Since we moved up, we are definitely not "stuck at min" anymore (so re-arm min printing)
    printedAtMin[currentSetting] = false;  // re-enable the "min" print if we ever come back down later

    // If we are at the maximum value, print it only once (avoid spamming while holding UP at the limit)
    if (settingValues[currentSetting] == settingMax[currentSetting]) {  // check if we're at max
      if (!printedAtMax[currentSetting]) {                              // only print if we haven't yet
        Serial.print(settingNames[currentSetting]);                     // print setting name
        Serial.print(" = ");                                            // print separator
        Serial.println(settingValues[currentSetting]);                  // print numeric value
        printedAtMax[currentSetting] = true;                            // remember we printed at max
      }
    } else {
      // Normal range: always print the current value (and clear max suppression so it can trigger again later)
      Serial.print(settingNames[currentSetting]);     // print setting name
      Serial.print(" = ");                            // print separator
      Serial.println(settingValues[currentSetting]);  // print numeric value
      printedAtMax[currentSetting] = false;           // re-arm "max" printing
    }
  }

  // 5] Check for DOWN button to decrease the selected value with repeat

  if (btnDown.isShortPressed() || btnDown.isRepeating()) {  // either a tap OR an auto-repeat tick while holding
    settingValues[currentSetting]--;                        // decrease the current setting by 1

    // Clamp the value so it stays inside its allowed range
    settingValues[currentSetting] = constrain(  // clamp again after decrement
      settingValues[currentSetting],            // make sure this current value
      settingMin[currentSetting],               // is above min for this setting
      settingMax[currentSetting]                // and below max for this setting
    );

    // Since we moved down, we are definitely not "stuck at max" anymore (so re-arm max printing)
    printedAtMax[currentSetting] = false;  // re-enable the "max" print if we ever come back up later

    // If we are at the minimum value, print it only once (avoid spamming while holding DOWN at the limit)
    if (settingValues[currentSetting] == settingMin[currentSetting]) {  // check if we're at min
      if (!printedAtMin[currentSetting]) {                              // only print if we haven't yet
        Serial.print(settingNames[currentSetting]);                     // print setting name
        Serial.print(" = ");                                            // print separator
        Serial.println(settingValues[currentSetting]);                  // print numeric value
        printedAtMin[currentSetting] = true;                            // remember we printed at min
      }
    } else {
      // Normal range: always print the current value (and clear min suppression so it can trigger again later)
      Serial.print(settingNames[currentSetting]);     // print setting name
      Serial.print(" = ");                            // print separator
      Serial.println(settingValues[currentSetting]);  // print numeric value
      printedAtMin[currentSetting] = false;           // re-arm "min" printing
    }
  }
}

// OptaButton_threeButtonMenuExample.ino
