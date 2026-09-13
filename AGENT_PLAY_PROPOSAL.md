# Proposal: an efficient Atari-playing agent

## Purpose

Build an agent that learns Atari 2600 games with much less interaction than
Agent57, first in an emulator and then through the controls and video output
of a real console.  The real console is the evaluation target, not the first
training environment: it is slow to reset, has video/control latency, and
cannot expose the game score as a convenient training reward.

The initial success criterion is deliberately modest: given a selected ROM and
the standard joystick action set, the agent reliably starts a game, survives
and improves its score on a small held-out set of games.  We should report
frames used, wall-clock time, energy/compute, score distribution, and
performance after transfer to hardware--not only a mean score.

## First target: *Adventure*

Start with Atari 2600 *Adventure* using a standard joystick and fire button.
It is a good first real-hardware target because the controls are discrete yet
the task needs room-to-room memory, object interaction, risk management and
delayed outcomes.  Paddles stay in the rig plan but are out of scope for this
first game.

For a controlled first milestone, the operator manually loads the cartridge,
selects the documented game variation, and presses reset.  The agent then has
only composite-video observations and joystick/fire actions.  Record each
episode's variation, console region, start/reset procedure, elapsed frames,
completion state and a human-verified result.  Do not give the policy RAM,
room IDs, object coordinates or an emulator score during evaluation.

Initial milestones, in order:

1. Learn reliable movement, wall collision avoidance and fire-button timing
   from emulator video.
2. Identify and carry a visible object across a room transition.
3. Build enough recurrent/world-model memory to return to a previously seen
   room and pursue a known objective.
4. Transfer the frozen emulator policy to the original console, measure the
   video/action latency gap, and fine-tune only with documented hardware
   interactions if transfer is inadequate.

## Design

The agent has four cooperating parts:

1. **Visual encoder and memory.**  A small recurrent visual model turns a
   stack of video frames into a compact state and remembers relevant recent
   events.  Pretrain it on gameplay from several games so it does not have to
   rediscover motion, sprites and controllable objects for every ROM.
2. **Latent world model.**  Given state and joystick action, predict the next
   compact state, reward and terminal probability.  Planning can then evaluate
   short action sequences in this model instead of requiring an equivalent
   number of console/emulator frames.
3. **Policy and value heads.**  A lightweight reactive policy is the default.
   It asks the planner for help only when the world model is uncertain, a life
   is at risk, or a meaningful choice has a delayed consequence.  This keeps
   inference and planning cost proportional to need.
4. **Exploration and replay manager.**  Prefer actions expected to reduce model
   uncertainty or discover controllable new states.  Store rare transitions,
   prediction failures, deaths and score-changing events at higher priority.
   Do not reward incidental pixel flicker as novelty.

This replaces a large fixed population of exploration/time-horizon settings
with a policy conditioned on uncertainty, remaining lives and inferred game
phase.  The system must regularly validate imagined plans with actual steps;
world-model errors must be added to replay rather than trusted.

## Emulator training interface

Use an Atari emulator environment as the first adapter.  It should expose:

```
reset(rom, seed) -> observation
step(action, repeat_frames) -> observation, score_delta, done, diagnostics
```

`action` is limited to the physical joystick affordances: neutral, four
directions, fire, and legal direction/fire combinations.  `repeat_frames`
must be explicit so experiments can distinguish a decision rate from an
emulation rate.  Record raw frames, action, frame number, score delta, lives
(when available), terminal state, and the configuration/version of the run.

Training can use emulator score deltas.  Evaluation should additionally use
pixels only, so the same trained policy can later operate from composite video
where score and internal RAM are unavailable.  Include randomized action hold
times, skipped frames, video noise, colour variation and 1--5-frame input/
capture delays during training to make the hardware transition less brittle.

Start with a simple baseline--frame-stack encoder plus recurrent policy and
prioritized replay--before adding the world model.  A baseline establishes the
data and control path and makes any claimed efficiency gain measurable.

## Playing a real Atari 2600

The console-facing system is an external closed loop; it does not modify the
console or ROM:

```
Atari composite video -> low-latency USB capture -> frame normalizer -> agent
agent action -> USB serial -> joystick-interface microcontroller -> controller port
```

### Video

Capture the console's composite output with a known-low-latency capture device.
The host timestamps each captured frame and normalizes it to the encoder's
input size/colour format.  Calibrate the capture pipeline by showing a
recognisable scene and measuring its capture delay.  PAL/NTSC detection and
frame cadence must be logged per run.

### Controller interface

Use an RP2040, Arduino-class microcontroller, or equivalent as a USB-serial
joystick bridge.  Its hardware must emulate **momentary switch closures** on
the Atari joystick port: each requested direction/fire line is connected to
the controller common/ground only while active.  Use opto-isolators or
open-drain switching designed for this purpose; never drive a voltage into a
console controller pin.  Build and test it first with a spare controller,
continuity meter and logic analyser, then with a console while a human can
unplug it immediately.

The firmware should accept a small, fail-safe command such as
`SET <bitmask> <duration_ms>`.  It must default to neutral on boot, USB loss,
bad packets and watchdog expiry; bound a hold duration; and report its applied
actions with timestamps.  Keep console power, reset and difficulty switches
manual for the first version.  An operator selects the cartridge and starts a
run; the bridge supplies only joystick input.

### Original-controller actuation (implementation step)

For the fairest hardware evaluation, preserve the original console, two
joysticks and paddle pair as the input devices.  The recommended first build
is a **reversible actuator rig**, not a modification to the controllers:

- Mount a removable fixture around each original joystick.  Five small
  push-actuators press up, down, left, right and fire; limit switches or a
  conservative travel calibration prevent excessive force.  Only one of each
  opposing direction pair may be pressed at once.
- Couple a position-controlled actuator to each original paddle knob with a
  removable, slip-clutch coupler.  The original 1 M-ohm paddle potentiometer
  and button remain in the circuit, so the console reads exactly the type of
  analogue input it expects.  A separate push-actuator operates each paddle
  button.
- Drive the fixture from a microcontroller through an external 5 V actuator
  supply.  USB supplies logic only.  On watchdog timeout or emergency-stop,
  release all actuators to neutral; the console/controller wiring is never
  electrically driven by the host.

This setup is more faithful than synthesising a paddle resistance with a
digital potentiometer: Atari paddle games measure the original controller's
resistance/timing behaviour, including its real mechanical range.  It also
makes a human-versus-agent comparison straightforward: disconnect the USB
controller and use the same console, cartridge, video path and physical
controllers.

An electrical joystick bridge remains a useful low-cost development tool.  Do
not connect its output in parallel with an actuated original controller during
evaluation; choose one input path per port and document it in the run log.

### Approximate bill of materials: original-controller actuator rig

Prices are planning ranges in EUR for one two-port rig, excluding the console,
controllers, host computer and tools.  Verify dimensions, voltage and stock
before ordering; mechanical fit is the largest uncertainty.

| Item | Qty. | Estimate | Notes |
| --- | ---: | ---: | --- |
| RP2040 Pico-class USB microcontroller | 1 | EUR 5--12 | Command receiver, timing and watchdog. |
| PCA9685 16-channel PWM driver board | 1 | EUR 5--12 | Provides stable control channels for actuators. |
| Metal-gear micro servos or miniature linear push-actuators | 14 | EUR 4--10 each | Ten for two joystick controls; four for two paddle knobs/buttons.  Allow spares. |
| 5 V, 10--15 A regulated power supply | 1 | EUR 20--35 | Size after measuring actuator stall current; do not power actuators from USB or the Atari port. |
| Inline fuse holder and correctly rated fuse; emergency-stop switch | 1 each | EUR 10--20 | Size the fuse to measured wiring/load capacity; it cuts actuator supply and leaves all controls neutral. |
| Limit switches, cabling, connectors and strain relief | 1 lot | EUR 20--40 | Includes servo extensions and a USB cable. |
| Laser-cut/3D-printed fixture, fasteners and slip-clutch couplers | 1 lot | EUR 30--80 | Must be designed around the actual controller shells. |
| Breadboard/prototyping board, headers and test points | 1 lot | EUR 10--25 | Replace with a fused enclosure/PCB after proving the design. |
| Low-latency composite-to-USB capture device and video adapters | 1 lot | EUR 25--80 | Measure actual latency; many inexpensive devices buffer frames. |
| Optional logic analyser and USB current meter | 1 each | EUR 20--50 | Strongly recommended during commissioning. |

Expected project total: **about EUR 190--390**, or **EUR 150--270** if a
capture device and basic test equipment are already available.  Add 15--20%
contingency for spare actuators and fixture iterations.

Before building the full rig, prototype one joystick direction and one paddle
knob.  Characterise force, travel, repeatability and paddle end stops using
the original controller only.  A fixture must never force a control past its
normal human-operated range.

### Hardware evaluation protocol

1. Select one game and a documented console/video/capture configuration.
2. Measure end-to-end action latency using a visible, repeatable response.
3. Run a human-controller smoke test through the bridge.
4. Run the trained policy at a conservative decision rate, logging captured
   frames and applied actions.
5. Compare score, survival and action latency with emulator evaluation.  Feed
   observed timing/visual differences back into emulator randomization.

## Delivery plan

| Phase | Deliverable | Exit criterion |
| --- | --- | --- |
| 0. Reproducibility | ROM inventory, legal-use notes, experiment config and run log format | A fixed emulator episode can be replayed from its log. |
| 1. Baseline | Emulator adapter and recurrent joystick policy | Learns a simple game and produces comparable metrics. |
| 2. Efficient learner | World model, uncertainty exploration and prioritized replay | Beats the baseline at equal interaction frames on held-out seeds. |
| 3. Console bridge | Capture pipeline, safe joystick firmware and latency measurement | Human smoke test and neutral-on-failure behaviour pass. |
| 3a. Original-control rig | Reversible joystick/paddle actuator fixture, emergency stop and calibration log | Each control is repeatable, releases on failure, and does not alter the controller. |
| 4. Transfer | Hardware evaluation harness | Policy completes repeatable unattended runs on the chosen game. |

## Decisions to make before implementation

- The first game is *Adventure*.  Select a simple reaction game later as a
  complementary control/latency benchmark; do not treat one game's result as
  evidence of generality.
- Which emulator/API and host platform will be supported?
- Which real console region (NTSC or PAL), capture device and cartridge/ROM
  setup will be the reference hardware?
- Are only lawfully obtained ROMs used, and can their hashes be recorded
  without committing copyrighted binaries?

## Non-goals for the first iteration

- Modifying a console, cartridge or game ROM to expose RAM or score.
- Automating console power/reset hardware.
- Claiming human-level or general intelligence from a small game set.
