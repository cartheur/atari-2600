# Four-switch Atari 2600 composite-video modification

This procedure is for **two known-working four-switch Atari 2600 consoles**
and the pictured Vintage Gaming and More **Atari 2600/7800 Composite Video Mod
2.0** board ([`images/video-mod.jpg`](../images/video-mod.jpg)). It permanently
removes RF output and adds composite video plus mono audio on RCA sockets. It
does not apply to six-switch, Jr., 7800, or PAL boards.

The [four-switch installation guide](https://vintagegamingandmore.com/installation-guide-4switch/)
is the authority for physical board locations. Its 19 installation images are
copied locally in [`images/video-mod-4switch`](../images/video-mod-4switch/)
for offline reference and remain attributed to Vintage Gaming and More.

Complete and test one console before modifying the second. Photograph each
console before disassembly.

![Four-switch Atari 2600 reference](../images/video-mod-4switch/IMG_3767.jpg)

![Source-guide assembled-board reference](../images/video-mod-4switch/IMG_3812.jpg)

## Mod-board connections

Follow the board silkscreen and the cable colours shown in
[`images/video-mod.jpg`](../images/video-mod.jpg). Colours identify a function;
the printed terminal label is authoritative.

| Board side | Terminal | Cable colour | Connection |
| --- | --- | --- | --- |
| `In` | `Gnd` | Black | Atari ground: cleared RF-modulator pin 1 |
| `In` | `Vid` | Blue | Atari video: cleared RF-modulator pin 4 |
| `In` | `+5V` | Red | Atari +5 V: cleared RF-modulator pin 3 |
| `In` | `Aud` | Yellow | Base of R206, or the connected C208 hole |
| `Out` | `Gnd` | Black | All three RCA jack ground rings |
| `Out` | `Vid` | Blue | Yellow RCA jack centre pin (composite video) |
| `Out` | `Aud` | Red | Both red and white RCA jack centre pins (mono audio) |

The two transistors follow the board's silkscreen orientation. The resistors
and ceramic capacitor are non-polarized. Leave the brightness trimmer at its
initial setting until the first powered picture test.

## Ordered installation procedure

1. With power disconnected, remove the four case screws. Disconnect the RF
   cable, lift out the main board, remove the black switch foam covers, and
   remove the foil tape from the switches.

   ![Four-switch console underside and screw locations](../images/video-mod-4switch/IMG_3769.jpg)

   ![RF cable and main-board removal](../images/video-mod-4switch/IMG_3775.jpg)

2. Bend up the four RF-shield tabs and remove both shield sections so the main
   board is accessible.

   ![RF shield removal](../images/video-mod-4switch/IMG_3774.jpg)

   ![Main board ready for modification](../images/video-mod-4switch/IMG_3777.jpg)

3. Remove the video transistor marked **Q201** or **Q202** (the marking varies
   by board) by cutting its three leads. Ensure remaining lead stubs cannot
   touch. Disconnect the four RF-modulator pins: either cut and bend them
   safely away, or remove the unused RF daughterboard.

   ![Q201/Q202 and RF-modulator removal](../images/video-mod-4switch/IMG_4864-2.jpg)

   ![Modified RF-modulator area](../images/video-mod-4switch/IMG_3780-1.jpg)

4. Assemble the 2.0 mod board. Follow its silkscreen for the transistor
   orientations and resistor locations. The source guide specifies 2.2 kΩ
   (red-red-red) and 3.3 kΩ (orange-orange-red) resistors.

   ![Assembled mod board](../images/video-mod-4switch/IMG_41152.jpg)

5. Prepare two four-conductor cables. Discard one yellow conductor, leaving a
   three-wire output harness (black/red/blue) and a four-wire input harness
   (black/red/blue/yellow). Solder both harnesses to the mod board according to
   the connection table.

   ![Input cable wiring](../images/video-mod-4switch/IMG_41252.jpg)

   ![Output cable wiring](../images/video-mod-4switch/IMG_41232.jpg)

6. Desolder and clear the RF-modulator holes. From the board underside, wire
   black to pin 1, red to pin 3, and blue to pin 4. Wire yellow to the base of
   R206, or to the connected C208 hole.

   ![RF-modulator-hole and audio-wire connections](../images/video-mod-4switch/IMG_1627.jpg)

   ![Four-switch main-board wiring](../images/video-mod-4switch/IMG_1630.jpg)

7. Mark the RCA-jack location on the lower shell, checking internal clearance.
   Drill three 1/8-inch pilot holes, then three 1/4-inch holes at 1/2-inch
   spacing. Mount the RCA jacks with their ground rings and nuts inside the
   shell.

   ![RCA pilot-hole drilling](../images/video-mod-4switch/IMG_3789.jpg)

   ![RCA holes](../images/video-mod-4switch/IMG_3792.jpg)

8. Solder the output harness: black joins all three RCA ground rings; red joins
   the red and white audio-jack centre pins; blue joins the yellow composite
   video-jack centre pin.

   ![RCA jack installation](../images/video-mod-4switch/IMG_3795.jpg)

   ![RCA ground wiring](../images/video-mod-4switch/IMG_3800.jpg)

9. Inspect all solder joints and ensure removed RF parts cannot short. Return
   the main board to the case, connect power and RCA leads, and test with a
   familiar colourful game. Adjust the Atari colour potentiometer first, then
   make only small brightness changes with the 2.0 board trimmer. Power off,
   secure the mod board to the right side of the lower case without stressing
   wires, restore the switch covers and case, then re-test video and mono
   audio.

   ![Final internal placement](../images/video-mod-4switch/IMG_38092.jpg)

   ![Completed four-switch console](../images/video-mod-4switch/IMG_3811.jpg)

This is an irreversible RF-removal modification. Do not power the console while
soldering, and do not use the relay bridge or controller wiring as a video-mod
power source.
