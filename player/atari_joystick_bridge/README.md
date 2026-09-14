# Atari joystick bridge sketch

Flash `atari_joystick_bridge.ino` to a **classic 5 V Arduino Nano**.  The
Linux host sends newline-terminated commands at 115200 baud:

```text
SET <mask> <duration_ms>
STOP
PING
```

Bits in `mask` are up=1, down=2, left=4, right=8, fire=16.  Example:
`SET 17 100` requests up and fire for 100 ms.  A command may last at most
250 ms and must be renewed by the host; on expiry, reset, malformed input, or
`STOP`, all five relay outputs are opened.  Opposing directions are rejected.

Wire Nano D2--D6 to the five ULN2803A inputs in the order Up, Down, Left,
Right, Fire.  The ULN2803A drives relay coils only.  Wire the relay **dry
contacts** to Atari DE-9 pins as documented in
[HARDWARE_BOM.md](../../HARDWARE_BOM.md); do not connect Nano, USB, or driver
grounds to the Atari controller signals.

Commission with a continuity meter and no Atari connected: verify that boot,
`STOP`, a malformed command, and timeout all leave every contact open.
