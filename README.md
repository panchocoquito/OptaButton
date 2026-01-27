# OptaButton

OptaButton is a small, focused Arduino library for handling physical pushbuttons
correctly and predictably across:

- Classic Arduino boards (Uno, Mega, etc.)
- Arduino Opta controllers (AFX00001 / AFX00002 / AFX00003)
- Opta Digital Expansion modules (AFX00005 / AFX00006)

It provides a clean, event-based API for one of the most fundamental (and most commonly
mishandled) problems in embedded systems: buttons.

This library is intentionally simple, explicit, and heavily commented.
It is designed to be readable by beginners and reliable enough for industrial projects.

---

## Why OptaButton exists

Reading a button is easy.

Reading a button correctly is not.

Real buttons:

- bounce
- get held down
- need long-press behavior
- often need accelerated repeat
- behave differently across hardware platforms

OptaButton solves these problems once, cleanly, so your sketches can focus on behavior
instead of timing logic.

Instead of writing debounce code in loop(), you ask questions like:

    btn.isShortPressed();
    btn.isLongPressed();
    btn.isRepeating();

Those questions mean the same thing on AVR and on Opta.

---

## Features

- Edge-based button handling (no level polling in user code)
- Debounce handled internally
- Short press detection
- Long press detection
- Long-press release detection
- Hold-repeat with optional acceleration
- Works with:
  - GPIO (INPUT_PULLUP)
  - Opta controller digital inputs
  - Opta digital expansion inputs
- No blocking delays
- No interrupts required
- Clear, beginner-readable implementation

---

## Supported input modes

OptaButton supports multiple hardware input sources through a single API.

| Input mode | Description |
|-----------|-------------|
| GPIO | Classic Arduino digital pins (Uno, Mega, etc.) |
| OPTA_CTL | Opta controller onboard digital inputs |
| EXP_DIG | Opta digital expansion modules (AFX00005 / AFX00006) |

The Opta Analog Expansion (AFX00007) is not recommended for pushbutton input.

---

## Wiring overview

### GPIO (Uno / Mega / AVR)

- Configure pin as INPUT_PULLUP
- Wire button between the pin and GND
- LOW = pressed (default behavior)

### Opta controller digital inputs

- Wire button between the input and +24 V
- HIGH = pressed

### Opta digital expansion inputs

- Wire button between the DI channel and +24 V
- HIGH = pressed

If your wiring causes the pressed state to be inverted relative to the default,
set the invertedLogic constructor parameter to true.

---

## Basic usage

    #include <OptaButton.h>
    using DefLab::ButtonInputMode;

    OptaButton myButton(
      ButtonInputMode::GPIO,
      2,
      "My Button"
    );

    void setup() {
      OPTA_BEGIN();      // required on Opta, safe no-op on AVR
      myButton.begin();  // configure the input hardware
    }

    void loop() {
      OPTA_UPDATE();     // required on Opta, safe no-op on AVR
      myButton.update(); // update button state machine

      if (myButton.isShortPressed()) {
        // handle short press
      }

      if (myButton.isRepeating()) {
        // handle accelerated hold-repeat
      }
    }

---

## Opta-specific notes

OptaButton provides two convenience macros:

    OPTA_BEGIN();
    OPTA_UPDATE();

On Opta boards, these expand to:

- OptaController.begin()
- OptaController.update()

On non-Opta boards, they compile out to no-ops.

This allows example sketches to remain portable without cluttering code with
platform-specific preprocessor logic.

For Opta digital expansion modules, OptaButton includes internal safeguards to avoid
over-polling inputs while remaining compatible with earlier Opta core behavior.

---

## Examples

This repository includes example sketches demonstrating common embedded UI patterns:

- Three-button menu  
  (Program / Up / Down)

- Four-button menu  
  (Program / Back / Up / Down)

The examples are written as teaching tools:

- verbose comments
- no hidden magic
- designed to be read line-by-line

They intentionally favor clarity over brevity.

---

## Design philosophy

OptaButton follows a few simple rules:

- prefer clarity over cleverness
- prefer events over polling
- prefer explicit state over hidden behavior
- write code that can be understood months later

Buttons are a primitive.
They should be boring, predictable, and trustworthy.

---

## Dependencies

OptaButton depends on DefLab_Common for shared enums and utilities.

You do not need to understand DefLab_Common to use OptaButton,
but it must be installed for the library to compile.

---

## Author

panchocoquito  
Def Lab

---

## License

MIT
