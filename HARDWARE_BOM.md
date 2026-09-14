# Hardware bill of materials: Atari 2600 *Adventure* agent

## Scope and design choice

This is the purchasing and construction reference for **Experiment 1** in
[the agent proposal](AGENT_PLAY_PROPOSAL.md): an external agent plays an
original Atari 2600 cartridge of *Adventure* through one controller port and
watches the console's own video output.

The minimum system does **not** alter the console, cartridge, or original
joysticks.  It emulates the five switch closures of a joystick with isolated
relay contacts.  For a human comparison, unplug the bridge and plug the
original joystick into the same port.  Paddles, two-player input, physical
controller actuators, and console modifications belong to Experiment 2.

## System boundary

```text
                       USB                 dry-contact closures
Linux host -> USB serial -> Arduino Nano -> relay-driver + relay board -> Atari port 1
   ^                                                                  (up/down/left/right/fire)
   |
   | UVC frames (choose one video path)
   |
USB camera <----------------- display <------------------------------- Atari video output
  or
USB video capture <---------- composite video <----------------------- Atari composite output
```

The agent receives pixels and sends joystick commands only.  It must not read
Atari RAM, score, room identifiers, or cartridge signals during an evaluation.

## Compatibility check before purchase

Do these three checks first; they determine the only items that vary by
console.

1. **Video output:** identify whether the console already has a yellow RCA
   composite-video output or only an RF/coaxial lead.  A composite capture
   device cannot accept RF directly.  A USB camera pointed at a working display
   works with either output type and needs no video conversion.
2. **Video standard:** record PAL or NTSC.  Buy a capture device that
   explicitly supports that standard at composite input.
3. **Controller-port orientation:** photograph the port and use a continuity
   meter to identify the intended plug/pin numbering.  Do not rely on an
   RS-232 cable's wiring or gender.

For the Atari controller port, the bridge needs only these connections:

| Atari port pin | *Adventure* function | Bridge action |
| ---: | --- | --- |
| 1 | Up | Close to pin 8 while active |
| 2 | Down | Close to pin 8 while active |
| 3 | Left | Close to pin 8 while active |
| 4 | Right | Close to pin 8 while active |
| 6 | Fire | Close to pin 8 while active |
| 8 | Controller ground/common | Common side of all five relay contacts |

Leave pins 5 and 9 (paddle inputs) and pin 7 (+5 V) unconnected in Experiment
1.  The pin mapping is documented in the Atari controller-port reference and
should be checked again against the actual plug before power is applied.

## Minimum build: ordered bill of materials

Prices are approximate EUR planning ranges, excluding the console, cartridge,
host computer, tools, shipping, and VAT where applicable.  They intentionally
specify electrical characteristics rather than a single vendor SKU, so parts
can be sourced locally.

| Item | Qty. | Target specification | Estimate | Why it is needed |
| --- | ---: | --- | ---: | --- |
| Controller board: **classic 5 V Arduino Nano (ATmega328P)** | 1 | USB serial; at least five digital outputs; watchdog support | EUR 5–15 | Enforces legal actions and operates the watchdog. |
| 5 V miniature relay | 5 | SPST-NO or SPDT; dry contacts; coil current documented | EUR 2–5 each | One isolated switch closure for each joystick input. |
| ULN2803A or equivalent relay-driver board | 1 | Eight low-side channels with flyback diodes | EUR 2–8 | Lets Nano GPIO switch relay coils safely. |
| 5 V, 1 A regulated USB supply | 1 | Certified supply plus cable | EUR 8–15 | Powers controller and relay coils; it does not power the Atari. |
| DE-9 male plug and backshell | 1 | Solder-cup or screw-terminal type; clear pin labels | EUR 3–8 | Mates with one Atari controller port. |
| Small terminal block or perfboard | 1 | Six low-voltage signal positions minimum | EUR 3–10 | Cleanly distributes the five contacts and common. |
| Hook-up wire and heat-shrink | 1 lot | 24–28 AWG stranded, labelled | EUR 5–15 | Console-port wiring and strain relief. |
| Non-conductive project enclosure | 1 | Space for board, relays, cable exit and labels | EUR 8–20 | Prevents accidental shorts and protects the bridge. |
| USB cable | 1 | Data-capable, appropriate to board | EUR 3–8 | Host-to-controller link. |
| Multimeter/continuity tester | 1 | Audible continuity mode | EUR 15–30 | Mandatory commissioning tool. |

Minimum controller-bridge subtotal: **EUR 57–139**.  A preassembled,
opto-isolated relay board with five or more dry-contact channels may replace
the five relays and driver, provided its input logic works at 5 V and its
outputs are genuinely isolated, normally open contacts.

### Controller-board choice

A classic 5 V Arduino Nano (ATmega328P) is the specified controller bridge for
this build.  It sets the five output pins and independently opens all relays
when its command timeout expires.  Its 5 V logic also suits many common 5 V
relay/ULN2803A modules.  Do not substitute a Nano 33 variant without checking
its 3.3 V GPIO levels.  Do not power relay coils from the Atari port, and use
only relay contacts on the console-facing side.

No Raspberry Pi is part of this design.  For phase one, the seated
Arthur-bipedal robot is the gameplay platform: its application hosts UVC
camera capture, policy inference, preview, episode logging, and the validated
serial command link to the Nano.  Arthur stays seated with a fixed camera pose;
locomotion and arm control are not part of this relay-bridge baseline.

## Video path A: reuse a USB robot camera (preferred first option)

A UVC-compatible USB camera pointed at the same display a human would use is
sufficient for *Adventure*.  It avoids purchasing a composite capture device,
works whether the console emits RF or composite, and is a particularly fair
evaluation because the agent sees the rendered screen rather than an internal
video signal.

Use the camera only if it can deliver stable frames to the host.  Configure
fixed focus, exposure, gain and white balance; autofocus or automatic exposure
will otherwise make the visual input drift during a run.  Mount the camera
rigidly, frame only the active screen, and crop/rectify the image in software.
For this slower first game, 30 fps is an acceptable starting point; prefer 60
fps if the existing camera and host support it.

| Item | Qty. | Target specification | Estimate | Why it is needed |
| --- | ---: | --- | ---: | --- |
| Existing USB robot camera | 1 | UVC-compatible; fixed exposure/focus controls available | EUR 0 | Agent observation source. |
| Existing display/TV | 1 | Accepts the console's current RF or composite output | EUR 0 | The physical screen observed by camera and human. |
| Rigid camera mount | 1 | Tripod, clamp or 3D-printed bracket | EUR 0–30 | Stops framing changes between episodes. |
| Matte shroud / black card | 0–1 | Non-reflective | EUR 0–10 | Optional: blocks room reflections and improves contrast. |
| Calibration target | 1 | Printed high-contrast rectangle/checkerboard | EUR 0–5 | Establishes crop, perspective correction and screen alignment. |

Camera-path subtotal: **EUR 0–45** when the camera and display already exist.
This path does add display-plus-camera latency, rolling-shutter artefacts and
possible CRT flicker/scan-line effects.  Those are acceptable for Experiment
1 only after they are measured and represented in emulator training.  If the
camera cannot keep a stable crop/exposure or adds too much delay, use Video
Path B instead.

### Using the existing two-axis Robotis camera head

The Robotis pan/tilt head can replace the camera mount at zero additional
cost.  Use it during setup to centre the display and fill the camera frame,
then record its two motor positions and keep them fixed for every episode.
Prefer a mechanical clamp or a stable holding torque so that the view does not
creep between runs.  The initial *Adventure* policy must not command pan or
tilt: camera motion would turn a controlled screen crop into an extra action
space and confound fair comparison with a human.

Active pan/tilt is a later, separate perception experiment.  It would need
motor-position logging, an expanded observation/action interface, a motion
settling delay, and new latency measurements before its results could be
compared with the fixed-camera baseline.

### Camera-path acceptance test

1. Record the camera's advertised frame rate and the actual delivered frame
   timestamps for a five-minute idle capture; flag dropped frames.
2. Lock camera controls, capture a title screen, and save the crop and
   perspective transform as versioned configuration.
3. Issue a visible joystick action 20 times.  Log host-command, relay-action,
   and first camera frame showing the response.  Report median, 90th-percentile
   and worst-case end-to-end delay.
4. Repeat after turning room lights on/off.  If reflections, exposure changes,
   or scan artefacts change the observation materially, improve the shroud or
   use the direct composite path.

## Video path B: direct composite USB capture

| Item | Qty. | Target specification | Estimate | Why it is needed |
| --- | ---: | --- | ---: | --- |
| USB video capture device | 1 | UVC-compatible; **composite CVBS input**; PAL/NTSC support stated | EUR 25–80 | Converts console video into frames readable by the host. |
| Composite RCA lead/adapters | 1 lot | Correct gender; 75-ohm video cable | EUR 5–15 | Connects console output to capture input. |
| Powered composite distribution amplifier | 0–1 | One input, two terminated outputs | EUR 20–50 | Optional: lets a monitor and capture device share the signal without an unreliable passive Y-splitter. |
| RF demodulator with composite output | 0–1 | Correct regional RF channel/standard | EUR 30–100 | Required only when the console has RF output and must remain unmodified. |

Video Path B subtotal: **EUR 30–95** with direct composite output; add **EUR 30–100**
for an RF-only console.  Before committing to a capture device, test that it
delivers frames to the selected OS and measure its latency; "USB capture" does
not by itself mean low latency.

Expected Experiment 1 total: **EUR 57–181** with the existing USB camera and
display, **EUR 87–231** with direct composite capture, or **EUR 117–331** if
external RF demodulation is necessary.  Existing tools, cables, or a suitable
5 V USB supply lower this cost.

### Recommended phase-one configuration: seated Arthur, Nano bridge, and USB camera

Use the hardware already available as follows:

```text
USB camera -> seated Arthur application (capture, policy, preview, episode log)
Arthur application -> USB serial -> Arduino Nano (command validation + watchdog)
Arduino Nano GPIO -> relay modules -> DE-9 dry contacts -> Atari
```

The Nano owns the safety-critical neutral-on-timeout action, even if Arthur's
camera process, application, or operating system stalls.  This configuration
does not use a Raspberry Pi.  It also avoids an initial composite-capture
purchase; point Arthur's existing robot camera at the normal game display and
use Video Path A's acceptance test.

Use Arthur's display (or a second monitor) for a live preview of the camera
feed.  The Arthur application should write synchronized video/frame timestamps, Nano
commands and applied relay masks to each episode log; a later playback view
can overlay those actions on the gameplay.  The preview and log are for human
observability only and must not add hidden console/emulator state to the
agent's input.

With the Nano and camera already on hand, the remaining purchases are
normally five relays, a ULN2803A driver, 5 V relay supply, DE-9 plug,
perfboard/terminal block, wire, enclosure and a continuity meter.  Budget
**about EUR 42–134**, or less if a suitable 5 V supply and meter are already
available.  Do not buy a Raspberry Pi, capture device, RF demodulator, second
controller bridge, paddle parts, or physical actuators until their later
experiments are needed.

### Amazon.de examples to evaluate

Amazon listings and sellers change too quickly to treat a particular listing
as the design specification.  These searches are good starting points for the
minimum build; purchase an item only when its current listing satisfies the
requirement in the BoM table above.

| Need | Amazon.de starting point | Check before ordering |
| --- | --- | --- |
| Controller board | [Classic Arduino Nano ATmega328P](https://www.amazon.de/s?k=arduino+nano+atmega328p) | USB data connector; headers supplied or separately ordered; distinguish the 5 V classic Nano from 3.3 V Nano 33 variants. |
| Five closures (one spare) | [5 V 8-channel relay module with optocoupler](https://www.amazon.de/s?k=5v+8+channel+relay+module+optocoupler) | Normally-open relay contacts; a 5 V-compatible input or a separate ULN2803A driver; do not assume the word “optocoupler” makes a board suitable for Nano GPIO. |
| Discrete relay driver | [ULN2803A relay-driver module](https://www.amazon.de/s?k=ULN2803A+relay+driver+module) | Eight channels and flyback-diode/common connection documented. |
| Atari-port connector | [DE-9 male solder connector with backshell](https://www.amazon.de/s?k=DE-9+male+solder+connector+backshell) | Male DE-9, solder-cup/screw terminal, and the physical pin numbering visible. |
| Optional direct capture | [USB composite/CVBS video capture (PAL/NTSC)](https://www.amazon.de/s?k=usb+video+capture+composite+cvbs+pal+ntsc) | Yellow-RCA **composite input** (not HDMI-only); PAL/NTSC support; works on the intended OS; return option for latency testing. |
| RCA video lead | [75-ohm RCA composite-video cable](https://www.amazon.de/s?k=75+ohm+RCA+composite+video+cable) | Correct connector genders and sufficient length. |
| Commissioning meter | [Digital multimeter with continuity buzzer](https://www.amazon.de/s?k=digital+multimeter+continuity+buzzer) | Audible continuity mode and insulated probes. |

For the capture device especially, prefer a returnable listing and perform the
latency test in this document before committing to it.  Do not buy an
HDMI-only “capture card” for an unmodified composite/RF Atari output.

## Circuit construction

### Controller-contact side

Use five independent relay contacts.  Wire one side of every relay contact to
DE-9 pin 8.  Wire the other side of each contact to exactly one signal pin:
1, 2, 3, 4, or 6.  Nothing from the Arduino Nano, relay-coil supply, USB ground, or
ULN2803A output may be wired directly to a controller signal pin.

```text
DE-9 pin 8  --------+----[ relay: Up ]------ DE-9 pin 1
                    +----[ relay: Down ]---- DE-9 pin 2
                    +----[ relay: Left ]---- DE-9 pin 3
                    +----[ relay: Right ]--- DE-9 pin 4
                    +----[ relay: Fire ]---- DE-9 pin 6
```

The relay **contacts** are the only electrical connection between the bridge
and console.  The relay **coils** are a separate circuit: 5 V to coil, coil to
the ULN2803A output, with the driver's common/flyback connection made as
specified by its datasheet.  Connect Arduino Nano GPIO pins to five ULN2803A
inputs and share ground only between the Nano and the relay-driver/coil supply.

### Firmware safety rules

The draft Nano implementation and its serial protocol are in
[`player/atari_joystick_bridge`](player/atari_joystick_bridge/).
Its bench assembly and commissioning summary is in
[`player/atari_joystick_bridge/ASSEMBLY.md`](player/atari_joystick_bridge/ASSEMBLY.md).

- All five relays start and fail neutral (open).
- A watchdog opens every relay if the host command stream stops.
- Reject `up + down` and `left + right`; fire may be combined with one
  vertical and one horizontal direction.
- Impose a maximum hold interval, for example 250 ms, unless renewed by the
  host.
- Log the received command and actual relay mask with a monotonic timestamp.
- Provide a physical USB disconnect or power switch as the emergency stop.

Do not source relay-coil current from an Atari controller-port pin.  Do not
power the console from the bridge.  Never test a bridge by plugging it into
the console first.

## Construction and acceptance sequence

1. Assemble the Arduino Nano, relay driver and **one** relay on a breadboard or
   perfboard.  Test the relay from a host command and confirm it opens after a
   forced watchdog timeout.
2. With no Atari connected, test each relay contact using a multimeter:
   inactive means open circuit; active means continuity only between its
   intended signal wire and the common wire.
3. Verify no signal wire has continuity to pin 7, pin 5, pin 9, USB 5 V, or
   USB ground in either relay state.
4. Add the remaining four relays, label every wire, and repeat the test.
5. Connect the bridge to the powered-off console, then check that all controls
   are neutral on boot and host disconnect.
6. Use a simple joystick game or *Adventure* title screen to test each action
   one at a time.  Keep the original joystick immediately available.
7. Only after controls are verified, bring up the video capture path and
   timestamp a visible action-to-frame response to establish end-to-end delay.

## Video-capture acceptance test

For either video path, capture a short clip while repeatedly pressing one visible-control action.
Record the host command timestamp, relay timestamp, and first video frame in
which the game visibly responds.  Repeat at least 20 times and record median,
90th-percentile and worst-case latency.  Train the emulator version with a
randomized delay covering that observed range.

Keep a monitor in the loop during commissioning.  If a monitor is needed in
parallel with capture, use a powered distribution amplifier rather than
assuming an RCA Y-splitter preserves a valid 75-ohm video signal.

## Deferred to Experiment 2

Do not buy these for *Adventure* unless the first bridge is already working:

| Deferred item | Reason |
| --- | --- |
| 16-channel PWM board | Needed for a multi-actuator physical-controller rig, not relay contacts. |
| 14 servos/linear actuators | Needed to physically actuate two joysticks and a paddle pair. |
| 5 V, 10–15 A actuator supply | Needed for servo/actuator stall current, not the five-relay bridge. |
| Emergency-stop actuator circuit, limit switches, fixture and slip couplers | Needed to protect original controller mechanics during physical actuation. |
| Paddle resistance/position calibration equipment | *Adventure* uses only joystick/fire. |
| Second controller-port bridge | Needed only for two-player or later experiments. |

See [AGENT_PLAY_PROPOSAL.md](AGENT_PLAY_PROPOSAL.md) for the Experiment 2
actuator-rig outline and its separate estimate.

## References

- [Atari 2600 controller-port pinout](https://consolemods.org/wiki/Atari_2600%3AConnector_Pinouts)
- [Atari 2600 domestic field service manual](https://www.atarimania.com/documents/Atari_2600_2600_A_VCS_Domestic_Field_Service_Manual.pdf)
