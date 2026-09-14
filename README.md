## The Atari 2600

You'd be surprised who likes to play games.

_Background_

This console is the most direct and simplistic way to not just have an agent (aeon) play live games but to also understand and chart _intelligence_. We also include the idea that we can create our own programs and/or burn any of the available games stored in the `ROMs` folder [here](/ROMs/README.md).

![2600](/images/Atari-2600.png)

The work herein will work to explore this aspect.

_Implementation_

See the [proposal for an efficient Atari-playing agent](AGENT_PLAY_PROPOSAL.md), including an emulator-first training plan and a safe path to play through a
real Atari 2600's video and joystick port. We _will not emulate_ the solution.

The purchase list, wiring boundary, and commissioning sequence for the first
*Adventure* hardware experiment are in the [dedicated hardware BoM](HARDWARE_BOM.md).

### Controller-port pinout configuration

The Atari controller interface uses a DE-9/DB-9 connector with the following
pin assignments.  Verify the physical pin numbering from the mating side of the
specific connector before soldering.

| Pin number | Signal | Description |
| ---: | --- | --- |
| 1 | Up | Digital Up input |
| 2 | Down | Digital Down input |
| 3 | Left | Digital Left input |
| 4 | Right | Digital Right input |
| 5 | POT X | Analog input for paddle X |
| 6 | Fire 1 | Primary Fire button |
| 7 | +5 V | Power supply |
| 8 | GND | Ground |
| 9 | Fire 2 | Secondary Fire button |

Additional information on the ecosystem can be found [here](https://www.8bitclassics.com/product/atari-2600-2k4k-pc-board/),

### Agent 57

A [benchmark](https://deepmind.google/blog/agent57-outperforming-the-human-atari-benchmark/)

In order to address these claims, we will need to construct our own cartridges expressing our own ROMs.

![cartridge](/images/cartridge-board.jpg)
