# Arduino Nano Atari joystick relay bridge: assembly summary

This bridge lets the Linux host issue joystick actions to an original Atari
2600 through five normally-open relay contacts.  It does not modify the
console or cartridge.

```text
USB camera -> Linux box -> USB data cable -> Arduino Nano -> relay driver -> relays
                                                                    |
                                                           dry contacts only
                                                                    |
                                                         DE-9 male -> Atari port 1
```

The Nano is the fail-safe controller; it opens all relay contacts when it
boots, receives bad input, receives `STOP`, or lets an action command expire.
The Linux box runs camera capture and the agent, but must not be the only
safety mechanism.

## Parts

- Classic 5 V Arduino Nano (ATmega328P), with USB data cable.
- ULN2803A driver board, or a ULN2803A IC on perfboard with its flyback-diode
  common connection accessible.
- Five 5 V SPST-NO or SPDT relays.  A compatible 5 V relay module may replace
  individual relays and the driver.
- **Available substitution:** three of the two-channel 5 V relay modules on
  hand ([AliExpress listing](https://nl.aliexpress.com/item/32824662430.html))
  provide six channels.  Use five channels in the order below and leave the
  sixth unconnected.  These replace both the ULN2803A and the individual
  relays.
- Regulated 5 V supply sized for all relay coils (start with 1 A only after
  checking the coils' documented current), plus a suitable cable/terminal.
- DE-9 male solder-cup or screw-terminal connector and backshell.
- Perfboard/terminal blocks, 24--28 AWG stranded wire, heat-shrink, enclosure,
  labels, and a continuity meter.

Do **not** use Arduino GPIO, USB power, ULN2803A outputs, or their grounds as
Atari controller signals.  Only relay contacts connect to the DE-9.

## Wiring

### 1. Linux box to Nano

1. Connect the Nano to the Linux box with a USB **data** cable.
2. The Nano appears commonly as `/dev/ttyUSB0` or `/dev/ttyACM0`; identify the
   actual device with `dmesg --follow` or `ls /dev/ttyUSB* /dev/ttyACM*` after
   connecting it.
3. The sketch uses 115200 baud, 8 data bits, no parity, one stop bit.  It
   accepts newline-terminated commands such as `SET 17 100`.

The Linux USB connection powers Nano logic in the normal configuration.  Do
not use it to power relay coils.

### 2. Nano to relay driver and relay-coil supply

Use the following channel order consistently in the wiring, labels, and Linux
software:

| Action | Nano pin | ULN2803A input | ULN2803A output | Relay coil |
| --- | --- | --- | --- | --- |
| Up | D2 | IN1 | OUT1 | Up coil low side |
| Down | D3 | IN2 | OUT2 | Down coil low side |
| Left | D4 | IN3 | OUT3 | Left coil low side |
| Right | D5 | IN4 | OUT4 | Right coil low side |
| Fire | D6 | IN5 | OUT5 | Fire coil low side |

Make these low-voltage control connections:

```text
Nano D2..D6  -> ULN2803A IN1..IN5
Nano GND     -> ULN2803A logic/power ground -> relay-supply negative
Relay + side -> regulated +5 V relay supply
Relay - side -> corresponding ULN2803A OUT1..OUT5
ULN2803A COM -> regulated +5 V relay supply (enables internal flyback diodes)
```

For an individual bare ULN2803A, follow its package pinout/datasheet: the
input/output channel order and COM pin must be verified before soldering.  For
a prebuilt board, follow the board's labelled IN/OUT/COM/GND terminals.  Do
not connect the Nano's 5 V pin to a separate relay supply unless its power
path has been explicitly designed to prevent USB back-feeding.

#### Using the three in-stock two-channel relay modules

Connect Nano D2--D6 to five module inputs in the same Up, Down, Left, Right,
Fire order.  Power the modules' relay-coil side from the regulated 5 V supply
and make the required Nano/module control-ground connection according to the
module labelling.  Do not add a ULN2803A: the modules already contain their
relay drivers and coil flyback protection.

Many such modules are **active-low**: a Nano `LOW` energizes a relay and a
`HIGH` releases it.  Before connecting the Atari, test one channel with the
commissioning continuity check.  If it is active-low, invert the five
`digitalWrite` values in `setRelays()` in
`atari_joystick_bridge.ino`, then repeat the boot, `STOP`, timeout, and reset
tests to prove that all contacts are open in every neutral/fault state.

### 3. Relay contacts to the Atari DE-9 plug

Use the **common (COM)** and **normally open (NO)** contact of each relay;
leave normally closed (NC) unconnected.  Join the COM side of all five relay
contacts, then wire that shared contact common to DE-9 pin 8.

| Relay | NO contact goes to DE-9 pin | Atari function |
| --- | ---: | --- |
| Up | 1 | Up |
| Down | 2 | Down |
| Left | 3 | Left |
| Right | 4 | Right |
| Fire | 6 | Fire |
| Shared relay-contact COM | 8 | Controller common |

Leave DE-9 pins 5 and 9 (paddle inputs) and pin 7 (+5 V) unconnected.  Verify
the physical DE-9 plug's pin numbering before soldering; do not infer it from
an RS-232 cable or connector gender.

## Firmware and Linux smoke test

1. Open `atari_joystick_bridge.ino` in Arduino IDE or use:

   ```sh
   /home/cartheur/programs/bin/arduino-cli compile \
     --fqbn arduino:avr:nano:cpu=atmega328old \
     player/atari_joystick_bridge
   ```

2. Upload the sketch to the Nano.  It was compile-checked for that target;
   use the Nano's actual serial port and bootloader setting when uploading.
3. With no Atari connected, use a serial terminal at 115200 baud and send:

   ```text
   PING
   SET 1 100
   STOP
   SET 3 100
   ```

   Expected replies include `PONG`, `OK`, and `ERR MASK`.  `SET 1 100` clicks
   only the Up relay for 100 ms; `SET 3 100` is rejected because it requests
   Up and Down together.
4. The mask bit assignments are: Up=1, Down=2, Left=4, Right=8, Fire=16.
   Send a renewed `SET` command at a fixed cadence (for example 20--50 Hz)
   while holding an action.  Each command may request at most 250 ms.

## Commissioning checklist

Do every check below with the Atari **disconnected**.

1. Inspect for exposed strands, reversed relay-coil polarity, and a shared
   connection between relay contacts and the coil/USB circuit.
2. Power the Nano and relay supply.  At boot, all five DE-9 action pins must
   be open relative to pin 8.
3. For each legal single action, confirm continuity only between its intended
   DE-9 signal pin and pin 8 while the relay is active; it must return open at
   the requested timeout.
4. Confirm that `STOP`, a malformed serial line, unplugging the Linux USB
   cable, and resetting/powering the Nano leave all five contacts open no
   later than the 250 ms maximum command duration.
5. Confirm no continuity from any Atari signal pin to DE-9 pin 7, USB 5 V,
   USB ground, the Nano, or the relay supply in either relay state.
6. Fit the DE-9 backshell and enclosure/strain relief.  Only then connect the
   bridge to the **powered-off** Atari.  Power on and test one action at a
   time, with the original joystick available to recover control.

## Before the first agent run

Keep a monitor connected, use a physical USB disconnect or relay-supply power
switch as an emergency stop, and log Linux command timestamps plus Nano relay
responses.  Measure visible command-to-camera latency before training or
evaluating a policy on the console.
