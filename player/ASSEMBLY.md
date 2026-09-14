# Arduino Nano Atari joystick relay bridge: assembly summary

This bridge lets the Linux host issue joystick actions to an original Atari
2600 through five normally-open relay contacts.  It does not modify the
console or cartridge.

```text
USB camera -> Linux box -> USB data cable -> Arduino Nano -> relay driver -> relays
                                                                    |
                                                           dry contacts only
                                                                    |
                                                       DB-9 female -> Atari port 1
```

The Nano is the fail-safe controller; it opens all relay contacts when it
boots, receives bad input, receives `STOP`, or lets an action command expire.
The Linux box runs camera capture and the agent, but must not be the only
safety mechanism.

Electrical requirements and terminal names for the two-channel relay modules
are from the [TS0010D datasheet](../datasheets/TS0010D.pdf): 5 V operation,
`VCC`/`GND`/`IN1`/`IN2` input terminals, `JD-VCC` coil supply, active-low
triggering, and 15--20 mA input current per channel.

Do **not** use Arduino GPIO, USB power, relay-module logic wiring, or their
grounds as Atari controller signals.  Only the SRD05 relay contacts connect to
the DB-9 female connector.

## Wiring

### 1. Linux box to Nano

1. Connect the Nano to the Linux box with a USB **data** cable.
2. The Nano appears commonly as `/dev/ttyUSB0` or `/dev/ttyACM0`; identify the
   actual device with `dmesg --follow` or `ls /dev/ttyUSB* /dev/ttyACM*` after
   connecting it.
3. The sketch uses 115200 baud, 8 data bits, no parity, one stop bit.  It
   accepts newline-terminated commands such as `SET 17 100`.

The Linux/Arthur USB connection powers Nano logic in the normal configuration.
The separate 5 V USB power bank powers relay coils through `JD-VCC` (per the
[TS0010D datasheet](../datasheets/TS0010D.pdf)); do not use the Nano USB
connection for relay coils and do not join the two 5 V rails.

### 2. Nano to the SRD05 relay-module inputs

Use the following channel order consistently in the wiring, labels, and Linux
software:

| Action | Nano pin | Relay module | Module input | SRD05 relay channel |
| --- | --- | --- | --- | --- |
| Up | D2 | A | IN1 | A / CH1 |
| Down | D3 | A | IN2 | A / CH2 |
| Left | D4 | B | IN1 | B / CH1 |
| Right | D5 | B | IN2 | B / CH2 |
| Fire | D6 | C | IN1 | C / CH1 |

Make these low-voltage control connections:

```text
Nano D2       -> module A IN1 (Up)
Nano D3       -> module A IN2 (Down)
Nano D4       -> module B IN1 (Left)
Nano D5       -> module B IN2 (Right)
Nano D6       -> module C IN1 (Fire)
Remove VCC--JD-VCC jumper -> on each relay module
Nano 5V       -> each relay module VCC (input/optocoupler supply)
Nano GND      -> each relay module GND (input reference)
5 V power-bank + -> each relay module JD-VCC (relay-coil supply)
5 V power-bank - -> each relay module GND
```

This is the TS0010D split-supply configuration: `VCC` powers the input side and
`JD-VCC` powers the relay coils.  Do not connect the Nano's 5 V pin to the
power bank; the removed jumper prevents USB back-feeding between the two 5 V
supplies.  The [TS0010D datasheet](../datasheets/TS0010D.pdf) specifies
15--20 mA of driver current per input.  Five active channels draw 75--100 mA
from Nano GPIO in total, so keep the Nano wiring short and verify the module
input current on the actual boards before sustained five-button operation.

The [TS0010D datasheet](../datasheets/TS0010D.pdf) specifies **active-low**
triggering: a Nano `LOW` closes the relay's NO-to-COM contact and a `HIGH`
opens it.  The supplied sketch is configured accordingly.
Before connecting the Atari, still repeat the boot, `STOP`, timeout, and reset
continuity tests to prove that all contacts are open in every neutral/fault
state.

### 3. SRD05 relay contacts to the Atari DB-9 console inputs

Use the **common (COM)** and **normally open (NO)** contact of each relay;
leave normally closed (NC) unconnected.  Join the COM side of all five relay
contacts, then wire that shared contact common to DE-9 pin 8.

| Action | SRD05 relay channel | COM terminal | NO terminal connects to Atari DB-9 male console input |
| --- | --- | --- | --- |
| Up | A / CH1 | Shared relay-contact COM -> DB-9 pin 8 | DB-9 pin 1 (Up) |
| Down | A / CH2 | Shared relay-contact COM -> DB-9 pin 8 | DB-9 pin 2 (Down) |
| Left | B / CH1 | Shared relay-contact COM -> DB-9 pin 8 | DB-9 pin 3 (Left) |
| Right | B / CH2 | Shared relay-contact COM -> DB-9 pin 8 | DB-9 pin 4 (Right) |
| Fire | C / CH1 | Shared relay-contact COM -> DB-9 pin 8 | DB-9 pin 6 (Fire) |

Leave DB-9 pins 5 and 9 (paddle inputs) and pin 7 (+5 V) unconnected.  Verify
the physical pin numbering when viewing the female connector's solder side;
do not infer it from an RS-232 cable or connector gender.

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
