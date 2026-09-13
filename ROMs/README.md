# Atari 2600 ROMs and cartridges

This directory contains ROM images for use with an Atari 2600 emulator or with
compatible hardware.  Only program, distribute, or manufacture copies of ROMs
when you have the rights to do so; use your own homebrew builds for the
simplest and safest physical-cartridge workflow.

## Make a cartridge board

The easiest first cartridge is a **2 KiB or 4 KiB, non-bankswitched** design.
It connects a suitably sized, 5 V-compatible parallel EEPROM or flash chip to
the 2600 cartridge edge connector: address lines, data lines, read-enable,
power, and ground.  Use a proven open-hardware PCB or a cartridge kit rather
than inventing the pinout, and install the board in a shell with a clean,
keyed cartridge edge.  Observe electrostatic precautions and verify all
continuity before inserting it into the console.

Larger games require a mapper/bankswitching board.  The mapper must match the
ROM image's bankswitch scheme (for example, an F8-style 8 KiB image is not a
drop-in replacement for a plain 4 KiB ROM).  Start small, then choose a board
whose documentation explicitly supports the image size and mapper you build
for.

## Program a ROM image

1. Choose a `.a26` or `.bin` image you are licensed to use, and identify its
   byte size and bankswitching scheme.  The extension alone is not enough.
2. Select a blank device and cartridge board compatible with that image; do
   not exceed the board's addressable ROM size.
3. With an EEPROM/flash programmer that supports the exact chip, erase it if
   needed, write the image, and run the programmer's byte-for-byte verify.
4. Test the image in an emulator first, then power off the 2600 before fitting
   or removing the cartridge.  Test on hardware with an appropriate display
   setup for the ROM's NTSC/PAL variant.

Keep the source image and a checksum alongside every programmed cartridge so
you can reproduce it later.  A flash multicart/cartridge-development board is
often more convenient for iteration than repeatedly swapping bare chips.

## Create games for Aeon to play

Write new games in 6502 assembly and begin with
[`../code/basic-example.asm`](../code/basic-example.asm).  Assemble a raw ROM
with a 2600-aware assembler (the example follows the DASM style), keeping the
reset and interrupt vectors at `$FFFC` and targeting a 2 KiB or 4 KiB ROM
until you deliberately add a mapper.

Design an agent-friendly play loop: render a stable, readable state every
frame; use standard joystick/fire inputs; make reset and game-over states
unambiguous; and make scoring, lives, and terminal outcomes visible.  Test
determinism and input timing in an emulator before burning hardware.  Aeon can
then receive the video state and send joystick actions through the emulator or
the real-console capture/input path described in the repository's main
documentation.

For each homebrew game, commit the source, built ROM, mapper/TV-standard
metadata, controls, and a short description of the objective and win/loss
conditions.  That makes the game repeatable for both people and Aeon.
