# OptaButton – Four Button Menu Example

This example demonstrates a simple but very common embedded UI pattern using **four physical buttons**:

- **Program**
- **Cycle**
- **Up**
- **Down**

It is designed to work the same way on:

- Classic Arduino boards (Uno, Mega, etc.)
- Arduino Opta controllers
- Opta Digital Expansion modules

The goal of this sketch is not to be clever or minimal, but to be **readable and teachable**.  
It shows how OptaButton can be used to build a predictable menu-style interface without writing debounce logic or timing code in `loop()`.

---

## What this example does

This sketch implements a small settings menu with three adjustable values:

| Setting     | Range   |
|-------------|---------|
| Volume      | 0–100   |
| Brightness  | 0–255   |
| Contrast    | 0–100   |

The interface behaves as follows:

- **PROGRAM (long press)**  
  Enter or exit *program mode*

- **CYCLE (short press)**  
  Choose which setting is currently selected

- **UP / DOWN (short press or hold)**  
  Increase or decrease the selected value  
  (holding the button accelerates the repeat rate)

Serial output is intentionally suppressed when a value is already at its minimum or maximum, to avoid spamming the Serial Monitor while a button is held.

---

## Button roles (at a glance)

| Button  | Action |
|--------|--------|
| Program | Long-press toggles program mode |
| Cycle  | Short-press cycles through settings |
| Up     | Increases the selected value |
| Down   | Decreases the selected value |

This separation of responsibilities is typical in small machines and control panels:
one button for *mode*, one for *selection*, and two for *adjustment*.

---

## Wiring overview

### GPIO (Arduino Uno / Mega / AVR)

- Configure pin as `INPUT_PULLUP`
- Wire each button between the pin and **GND**
- **LOW = pressed** (default behavior)

### Opta controller digital inputs

- Wire button between the input and **+24 V**
- **HIGH = pressed**

### Opta digital expansion inputs

- Wire button between the DI channel and **+24 V**
- **HIGH = pressed**

> The Opta Analog Expansion (AFX00007) is **not recommended** for pushbutton input.
> It is much slower and behaves differently from digital inputs.

If your wiring produces the opposite logic level from the default, you can set the
`invertedLogic` constructor parameter to `true`.

---

## How the sketch is structured

### 1. A small “settings model”

The sketch uses:

- an `enum` to identify each setting
- parallel arrays for:
  - names
  - minimum values
  - maximum values
  - current values

This is a common embedded pattern that avoids dynamic memory and keeps everything explicit.

---

### 2. One OptaButton per physical button

Each button is declared once, with:

- the input mode (GPIO / OPTA_CTL / EXP_DIG)
- the pin or channel number
- a human-readable label

All debounce, edge detection, long-press timing, and repeat behavior is handled internally by the library.

---

### 3. A simple state machine in `loop()`

Each pass through `loop()` does four things, in order:

1. Update the Opta core (required on Opta, no-op on AVR)
2. Update each button’s internal state machine
3. Check for **PROGRAM long-press** to toggle edit mode
4. If in program mode:
   - check **CYCLE** to select a setting
   - check **UP / DOWN** to adjust the value

The sketch exits early when not in program mode, which keeps the logic easy to follow.

---

## What to try

Once the sketch is running and the Serial Monitor is open:

1. **Hold PROGRAM**  
   → Program mode becomes enabled

2. **Tap CYCLE**  
   → Watch the selected setting name change

3. **Tap UP or DOWN**  
   → Adjust the value one step at a time

4. **Hold UP or DOWN**  
   → Watch the repeat accelerate

5. **Hold PROGRAM again**  
   → Exit program mode

---

## Why this example exists

This sketch is meant to show a **realistic pattern**, not a toy demo.

Four-button menus like this show up everywhere:
small machines, lab equipment, industrial controllers, and DIY tools.

The comments are intentionally verbose, and the code favors clarity over compactness.
It is meant to be read line-by-line by someone who is still getting comfortable with:

- enums
- arrays
- state flags
- non-blocking input handling

---

## Related examples

- **Three Button Menu Example**  
  (Program / Up / Down)  
  Demonstrates the same idea with one fewer button and slightly different ergonomics.

---

## License

MIT
