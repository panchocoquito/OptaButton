# OptaButton

OptaButton is a small, focused Arduino library for handling physical pushbuttons predictably across:

- Classic Arduino boards (Uno, Mega, etc.)
- Arduino Opta controllers (AFX00001 / AFX00002 / AFX00003)
- Opta Digital Expansion modules (AFX00005 / AFX00006)

It provides a clean, event-based API for buttons that's intentionally simple and heavily commented. 

---

## Do we really need another button library?

I love that Opta exists, and the analog expansion is especially coming in handy, but if you're coming from a purely Arduino IDE/Uno/Mega type environment you're in for some surprises. The Arduino Opta Blueprint library ([Arduino_Opta_Blueprint](https://github.com/arduino-libraries/Arduino_Opta_Blueprint)) has been updated recently, so hopefully it's better than it was, but when I was getting started with Opta, some of the examples on the official Opta tutorial weren't even working. I'm a noob myself, so some of that is likely on me, but my main point is I had to spend a long time digging through the library src folders for not-totally-clear-to-me comments and code in order to put this library together so someone in my shoes wouldn't have to next time.

Also I found most button libraries weren't speaking my learning language when I first started out, so I'm hoping this one at least fills a hole.

Maybe this is your first button library... so... why are there button libraries? 

Because buttons:

- bounce
- get held down
- need long-press behavior
- often need accelerated repeat
- behave differently across hardware platforms

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

If your wiring causes the pressed state to be inverted relative to the default, set the invertedLogic constructor parameter to true.

---

## Basic usage

```cpp
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
```

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
over-polling inputs while remaining compatible with earlier Opta core behavior, which
required updating input states every time you polled the pins. As the Opta core library 
evolves, we'll update here if necessary/helpful.

---

## Examples

This repository includes example sketches demonstrating common embedded UI patterns:

- Three-button menu  
  (Program / Up / Down)

- Four-button menu  
  (Program / Select / Up / Down)

The examples are written as teaching tools:

- verbose comments
- no hidden magic
- designed to be read line-by-line

They intentionally favor clarity over brevity.

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
