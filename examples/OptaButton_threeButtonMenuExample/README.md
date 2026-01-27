# OptaButton — Three Button Menu Example

This example shows a basic “menu” user interface using three physical buttons:

- PROGRAM (mode + selection)
- UP (increase)
- DOWN (decrease)

The sketch prints everything to Serial so you can see what’s happening.

---

## What you should see

- Holding PROGRAM toggles “program mode” on/off.
- When program mode is ON:
  - Tapping PROGRAM cycles through settings (Volume, Brightness, Contrast).
  - Holding UP/DOWN changes the selected setting.
  - Hold-repeat accelerates over time.
- When you reach the min/max of a setting, Serial output is suppressed so it doesn’t spam.

---

## Wiring (choose one section)

### Option A — Classic Arduino (Uno / Mega) using GPIO

This example is configured for GPIO mode.

- Use INPUT_PULLUP wiring:
  - One side of each button goes to the Arduino pin
  - The other side goes to GND
- Default behavior:
  - LOW = pressed

Pins used by default:
- PROGRAM = D3
- UP      = D4
- DOWN    = D5

### Option B — Arduino Opta controller inputs (OPTA_CTL)

If you want to use Opta controller inputs instead of GPIO:

- Wire each button between the chosen Opta input and +24 V
- Default behavior:
  - HIGH = pressed
- Change the button constructors from ButtonInputMode::GPIO to ButtonInputMode::OPTA_CTL
- Change the input IDs to match the Opta input numbers you are using

### Option C — Opta digital expansion inputs (EXP_DIG)

If you want to use an Opta Digital Expansion (AFX00005/06):

- Wire each button between the chosen DI channel and +24 V
- Default behavior:
  - HIGH = pressed
- Change the button constructors from ButtonInputMode::GPIO to ButtonInputMode::EXP_DIG
- Change the input IDs to match the expansion channel numbers you are using

Note: This sketch calls OPTA_BEGIN() / OPTA_UPDATE(). On Opta these run the controller core.
On AVR boards they compile out and do nothing.

---

## How to run it

1) Open the sketch:
   examples/OptaButton_threeButtonMenuExample/OptaButton_threeButtonMenuExample.ino

2) Select your board and port in the Arduino IDE.

3) Upload the sketch.

4) Open Serial Monitor:
   - Baud rate: 115200

5) Try the buttons:
   - Hold PROGRAM to enter program mode
   - Tap PROGRAM to select setting
   - Hold UP or DOWN to change the value
   - Hold PROGRAM again to exit program mode

---

## What this sketch is teaching (high level)

1) Buttons as events (not raw pin reads)
   - btn.isShortPressed() happens once per press
   - btn.isLongPressed() happens once after you hold long enough
   - btn.isRepeating() happens repeatedly while held, and accelerates

2) Arrays + enum indexing
   - currentSetting is used as an index into:
     - settingNames[]
     - settingMin[]
     - settingMax[]
     - settingValues[]
     - printedAtMin[] / printedAtMax[]

This is a clean way to scale a menu from 3 settings to 10+ settings.

---

## Common changes

### Change which pins you use (GPIO)
Edit the button constructors:

    OptaButton btnProgram(ButtonInputMode::GPIO, 3, "Program");
    OptaButton btnUp(ButtonInputMode::GPIO, 4, "Up");
    OptaButton btnDown(ButtonInputMode::GPIO, 5, "Down");

### Change long-press timing, repeat speed, etc.
OptaButton has optional constructor parameters (debounce, long press ms, repeat start/min, acceleration).
See OptaButton.h for the full parameter list.

---

## Troubleshooting

- If nothing prints:
  - confirm Serial Monitor baud = 115200
  - confirm you uploaded to the right board/port

- If buttons read backwards:
  - your wiring is likely inverted for the selected mode
  - set the invertedLogic constructor parameter to true

- If Opta expansion buttons don’t respond:
  - confirm the expansion is detected and wired correctly
  - confirm you are using ButtonInputMode::EXP_DIG
  - confirm OPTA_BEGIN() is called in setup and OPTA_UPDATE() runs in loop
