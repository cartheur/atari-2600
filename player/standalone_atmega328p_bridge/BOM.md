# Bill of materials and inventory cross-reference

Source checked: `/home/cartheur/ame/aiventure/aiventure-github/cartheur/parts-is-parts/inventory/immediate.csv`, 2 October 2026.  Quantities below are for one five-action Atari joystick bridge.

## Required core circuit

| Qty. | Requirement | Inventory match | In stock | Acquire / note |
| ---: | --- | --- | --- | --- |
| 1 | 5 V DIP-28 MCU | Microchip `ATMEGA328P-PU` | 10 | Yes |
| 1 | 28-pin DIP socket | Mill-Max `123-47-628-41-001000` | 8 | Yes; recommended |
| 1 | 16 MHz crystal | CTS `ATS16B-E`, 16 MHz, 18 pF load | 5 | Yes |
| 2 | Crystal capacitors, nominal 18–22 pF, C0G/NP0 | No match | — | Buy; select according to crystal/load and board stray capacitance |
| 2 | 100 nF ceramic decoupling capacitors | No match | — | Buy; one at VCC and one at AVCC |
| 1 | 10 uF bulk capacitor, >= 10 V | No match | — | Buy; at 5 V entry |
| 1 | RESET pull-up, 10 kOhm | KOA `MF1/4DCT52R1002F` | 100 | Yes |
| 1 | Reset pushbutton (optional) | E-Switch `PS1024ALBLK` | 10 | Yes |
| 1 | 2x3 0.1 in ISP header | No match | — | Buy or make from suitable pin header |
| 1 | 5 V USB-to-TTL serial adapter | No match | — | Buy; FTDI/CP2102/CH340 type, 5 V logic levels |
| 1 | Regulated 5 V supply, >= 0.5 A | No match | — | Buy or use a verified existing USB supply |

The inventory's `LM1117T` is a **2.5 V** regulator, so it is not suitable for
this 5 V circuit.  The `251-05000` crystal is 5 MHz and is not an alternative
for a 16 MHz bootloader configuration.

## Output driver: preferred purchase version

This is the circuit documented in [`README.md`](README.md).  It is compact and
uses its internal clamp diodes.

| Qty. | Requirement | Inventory match | In stock | Acquire / note |
| ---: | --- | --- | --- | --- |
| 1 | ULN2803A eight-channel low-side relay driver | No match | — | Buy |
| 5 | 10 kOhm input pull-down resistors | `MF1/4DCT52R1002F` | 100 | Yes |
| 5 | 5 V SPST-NO or SPDT relays | No match | — | Buy; coil current determines supply sizing |

## Output driver: all-inventory alternative

If obtaining a ULN2803A delays the build, use five discrete low-side NPN
drivers.  This replaces only the ULN2803A stage; the ATmega core and isolated
relay contacts are unchanged.

```text
ATmega output -- 4.7 kOhm -- base  2N2222A
                              emitter -- GND
                              collector -- relay coil -- +5 V

1N5819 across each coil: cathode to +5 V, anode to collector
10 kOhm from base to GND
```

| Qty. | Requirement | Inventory match | In stock | Result |
| ---: | --- | --- | --- | --- |
| 5 | NPN low-side transistor | `2N2222A`, TO-92, 600 mA | 100 | Available |
| 5 | Base resistor, 4.7 kOhm | Yageo `MFR25SFRF52-4K7` | 100 | Available |
| 5 | Base pull-down, 10 kOhm | KOA `MF1/4DCT52R1002F` | 100 | Available |
| 5 | Flyback diode | Diotec `1N5819`, 1 A Schottky | 100 | Available |
| 5 | 5 V relay | No match | — | Still must acquire |

The ATmega output is HIGH to energise each NPN-driven relay.  Therefore use:

```cpp
const uint8_t RELAY_ACTIVE_LEVEL = HIGH;
const uint8_t RELAY_INACTIVE_LEVEL = LOW;
```

Do not substitute a 74LS/74HCT logic buffer for the relay driver: it cannot
switch relay-coil current.  Do not use an individual ATmega output to drive a
relay coil directly.

## Atari interface, mechanical, and optional indicators

| Qty. | Requirement | Inventory match | In stock | Acquire / note |
| ---: | --- | --- | --- | --- |
| 1 | DE-9 female connector with backshell | No match | — | Buy; verify solder-side pin numbering |
| 1 | Enclosure / strain relief | No match | — | Buy or provide from existing materials |
| Wire | Low-voltage hook-up wire | 24 AWG black `3050/1 BK005`; 26 AWG red `9976 002100`; 28 AWG Kynar `801-R28` | Yes | Use heavier/short wiring for relay power where practical |
| 5 | Relay-active LEDs (optional) | Kingbright `WP57IID` red, `WP7113ED` orange, or `WP1034GDT` green | 50 / 25 / 20 | Available |
| 5 | LED resistors, 330 Ohm (optional) | Vishay `RN60D3300FB14` | 10 | Available |

## Purchase summary

Purchase the following minimum items for either driver option:

- Five 5 V relays with suitable contact ratings (SPST-NO or SPDT).
- One DE-9 female connector and backshell.
- One USB-to-TTL adapter with 5 V logic levels.
- One 2x3 ISP header and an ISP programmer, if one is not already available.
- Two 18–22 pF C0G/NP0 capacitors, two 100 nF ceramic capacitors, and one 10 uF capacitor.
- A regulated 5 V, at least 0.5 A power source.

Additionally purchase one ULN2803A for the preferred design, or use the
listed 2N2222A/4.7 kOhm/10 kOhm/1N5819 parts for the discrete-driver version.

Before connecting an Atari, prove with a meter that relay contacts are the
only electrical path to DE-9 pins 1, 2, 3, 4, 6, and 8.  DE-9 pin 7 must stay
unconnected.
