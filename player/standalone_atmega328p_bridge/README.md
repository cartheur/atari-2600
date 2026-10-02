# Standalone ATmega328P-PU Atari joystick bridge

This is the replacement for the Arduino Nano version of the Atari 2600
joystick bridge.  It uses a bare, 5 V **ATmega328P-PU** (DIP-28), a USB-to-TTL
serial adapter, a ULN2803A relay driver, and five 5 V relays.

See [BOM.md](BOM.md) for the inventory cross-reference, purchase list, and an
all-inventory discrete 2N2222A relay-driver alternative.

The Atari connection remains five isolated, normally-open relay contacts.  The
microcontroller, its ground, USB serial adapter, and relay-coil power must
never connect directly to an Atari joystick signal pin.

```text
Linux host -- USB -- USB-to-TTL adapter -- ATmega328P-PU -- ULN2803A -- relays
                                                                         |
                                                                  dry contacts
                                                                         |
                                                               Atari DE-9 port
```

## Parts

| Quantity | Part | Notes |
| ---: | --- | --- |
| 1 | ATmega328P-PU | DIP-28, programmed for 16 MHz / 5 V Arduino-compatible operation |
| 1 | 16 MHz crystal | With two 22 pF capacitors |
| 3 | 100 nF ceramic capacitors | Decoupling: VCC, AVCC, and supply rail |
| 1 | 10 uF electrolytic capacitor | Across the local 5 V supply |
| 1 | 10 kOhm resistor | RESET pull-up |
| 5 | 10 kOhm resistors | ULN2803A input pull-downs; ensure relays are off at reset |
| 1 | ULN2803A | Eight-channel low-side driver with flyback diodes |
| 5 | 5 V SPST-NO or SPDT relays | Use COM and NO contacts only |
| 1 | USB-to-TTL serial adapter | **5 V TTL logic**, not RS-232 voltage levels |
| 1 | 6-pin ISP header | For initial programming and recovery |
| 1 | DE-9 female connector | Connects to the Atari controller port |
| 1 | Regulated 5 V supply | Allow at least 0.5 A for five relay coils plus logic |

Do not power the circuit from Atari DE-9 pin 7.  A USB 5 V supply is suitable
if it can supply the relay current.  If a higher-voltage input is used, add an
appropriate regulated 5 V supply stage and its recommended input/output
capacitors.

## ATmega328P-PU core circuit

```text
ATmega pin  7  VCC   ---- +5 V
ATmega pin 20  AVCC  ---- +5 V
ATmega pins 8, 22 GND ---- 0 V

100 nF capacitor: pin 7  to GND, close to IC
100 nF capacitor: pin 20 to GND, close to IC
10 uF + 100 nF: +5 V to GND at the board power entry

pin  9 XTAL1 --+-- 16 MHz crystal --+-- pin 10 XTAL2
               |                    |
             22 pF                22 pF
               |                    |
              GND                  GND

pin 1 RESET ---- 10 kOhm ---- +5 V
pin 1 RESET ---- optional pushbutton ---- GND
```

Place the decoupling capacitors close to the ATmega pins.  Keep the crystal
and its capacitors close to pins 9 and 10, with short traces.

## Programming and host serial

Fit a 2x3 ISP header as follows:

| ISP signal | ATmega DIP pin |
| --- | ---: |
| RESET | 1 |
| MOSI | 17 (PB3) |
| MISO | 18 (PB4) |
| SCK | 19 (PB5) |
| +5 V | 7 / 20 |
| GND | 8 / 22 |

Use an ISP programmer to burn the 16 MHz Arduino-compatible bootloader and
upload the existing bridge sketch.  The USB-to-TTL adapter then supplies the
normal 115200-baud host connection:

```text
USB-to-TTL TX  -> ATmega pin 2  (PD0 / RX)
USB-to-TTL RX  -> ATmega pin 3  (PD1 / TX)
USB-to-TTL GND -> circuit GND
```

Cross TX and RX.  If automatic serial bootloader reset is wanted, connect the
adapter's DTR or RTS through a 100 nF capacitor to RESET.  This is optional:
the reset pushbutton can be used during uploads instead.

## Relay driver and output mapping

Use the same logical pins as the Nano sketch.  With a bare relay and
ULN2803A, an ATmega output **HIGH** energises a relay and **LOW** releases it.

| Action | Arduino pin | ATmega DIP pin | ULN2803 input | ULN2803 output |
| --- | ---: | ---: | ---: | ---: |
| Up | D2 | 4 | 1 | 18 |
| Down | D3 | 5 | 2 | 17 |
| Left | D4 | 6 | 3 | 16 |
| Right | D5 | 11 | 4 | 15 |
| Fire | D6 | 12 | 5 | 14 |

For every row, connect the ATmega pin to the specified ULN input and connect a
10 kOhm resistor from that ULN input to GND.  Connect its ULN output to the
low side of the corresponding relay coil.  Connect the high side of every
relay coil to +5 V.

```text
ATmega D2 ----+---- ULN2803 input 1       ULN2803 output 18 ---- relay coil ---- +5 V
              |
            10 kOhm
              |
             GND

ULN2803 pin 9  (GND)    -> GND
ULN2803 pin 10 (COM)    -> +5 V   (enables internal relay-coil flyback diodes)
```

Repeat this circuit for Down, Left, Right, and Fire.  LEDs may be fitted with a
series resistor as indicators, but they must not replace the ULN2803A driver.
The ATmega must not drive relay coils directly.

## Atari DE-9 dry-contact wiring

Use each relay's COM and NO terminals only; leave NC unconnected.  Join all
five COM terminals and connect that common exclusively to Atari DE-9 pin 8.

| Relay | NO terminal to Atari DE-9 pin | Function |
| --- | ---: | --- |
| Up | 1 | Up |
| Down | 2 | Down |
| Left | 3 | Left |
| Right | 4 | Right |
| Fire | 6 | Fire |
| Shared COM | 8 | Ground / switch return |

Leave Atari DE-9 pins 5, 7, and 9 unconnected.  Confirm the connector pin
numbering from the female connector's solder side before soldering.

## Firmware change

[`../atari_joystick_bridge/atari_joystick_bridge.ino`](../atari_joystick_bridge/atari_joystick_bridge.ino)
currently targets active-low relay modules.  For the ULN2803A circuit above,
change these two lines:

```cpp
const uint8_t RELAY_ACTIVE_LEVEL = HIGH;
const uint8_t RELAY_INACTIVE_LEVEL = LOW;
```

The logical assignments, serial protocol, action timeout, and safety checks
otherwise remain unchanged.

## Commission safely

1. Before fitting the Atari cable, power the board and check that every relay
   is released at reset, while programming, and after a power cycle.
2. Send each command and confirm only the intended relay clicks.  Confirm the
   250 ms timeout releases it.
3. With a multimeter, check continuity only from the appropriate DE-9 signal
   pin to pin 8 when its relay is active.
4. Confirm there is no continuity from any Atari signal pin to +5 V, circuit
   GND, USB, or DE-9 pin 7 in either relay state.
5. Enclose the board, provide strain relief, connect it only to a powered-off
   Atari, and test one action at a time.
