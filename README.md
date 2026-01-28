# OptaButton

OptaButton is a small, focused Arduino library for handling physical pushbuttons predictably across:

- Classic Arduino boards (Uno, Mega, etc.)
- Arduino Opta controllers (AFX00001 / AFX00002 / AFX00003)
- Opta Digital Expansion modules (AFX00005 / AFX00006)

It provides a clean, event-based API for buttons that's intentionally simple and heavily commented. 

---

## Do we really need another button library?

I love that Opta exists, and the analog expansion is especially coming in handy, but if you're coming from a purely Arduino IDE/Uno/Mega type environment you're in for some surprises. The Arduino Opta Blueprint library ([Arduino_Opta_Blueprint](https://github.com/arduino-libraries/Arduino_Opta_Blueprint)) has been updated recently—looking forward to good things—but when I was getting started with Opta, some of the examples on the official Opta tutorial weren't even working. I'm a noob, so some of that is likely on me, but my point is I had to spend a long time digging through the library folders for not-totally-clear-to-me comments and code in order to put this library together so someone in my shoes wouldn't have to next time.

Plus I found most button libraries weren't speaking my learning language when I first started out, so I'm hoping this one fills a hole.

## Maybe this is your first button library

So... why are there button libraries?

Because buttons:

- bounce
- get held down
- get tapped real fast
- need long-press behavior
- need hold-and-accelerated-repeat
- behave differently across hardware platforms

Learning how to handle those things in simple code is a great way to learn, but you don't want to have to do it every time. If you install the library, instead of writing debounce code in loop(), you ask questions like:

```cpp
thisButton.isShortPressed();
thatButton.isLongPressed();
thisOtherButton.isRepeating();
```

And those questions mean the same thing on both AVR and Opta.

---

## Features

- Edge-based button handling
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

OptaButton supports multiple hardware input sources:

| Input mode | Description |
|-----------|-------------|
| GPIO | Classic Arduino digital pins (Uno, Mega, etc.) |
| OPTA_CTL | Opta controller onboard digital inputs |
| EXP_DIG | Opta digital expansion modules (AFX00005 / AFX00006) |

There used to be an EXP_ANA mode, but the Opta Analog Expansion (AFX00007) is not recommended for pushbutton input. It's too slow and doesn't feel anything like the other three, so I got rid of it. The Analog Expansion is meant for other things—like outputting a 4-20mA current loop—which it does well.

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

## OptaButton constructor parameters

### Required parameters

When you create an OptaButton, you have to include the first three parameter arguments:

```cpp
DefLab::ButtonInputMode mode,  // which input hardware to use
uint8_t inputPin,              // pin number or channel index
const char* label,             // human-readable name for debugging
```

### Optional parameters

The remaining six default to values I've found useful in my system, which uses a variety of momentary pushbuttons to both AVR and Opta.
You must define these parameters in order up to the one you want to modify/override, but if you want to use the defaults after that you don't need to pass them.

For example, if you just wanted to modify the debounce time to 35ms, you'd create an OptaButton with four parameters (everything up to and including debounceMs) and the compiler would complete the button by taking the defaults after that. 

However, if you were only interested in modifying the long press time, you'd still have to enter the first six parameter arguments—including debounceMS and inverted, even if you liked the defaults. You can't just skip to the one you want, because the compiler won't know which one you're talking about. 

Here are the remaining parameters and their defaults:

```cpp
uint16_t debounceMs = 20,      // ms to ignore bounce after edge
bool inverted = false,         // true if LOW=pressed instead of HIGH
uint16_t longPressMs = 800,    // ms to hold before long press fires
uint16_t repeatStartMs = 100,  // initial delay between repeats
uint16_t repeatMinMs = 8,      // fastest delay when accelerating
uint8_t accelRate = 100        // how much to speed up per second
```

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
