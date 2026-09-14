# Arthur-bipedal integration note

The production camera watcher for a real Atari session lives in the
`arthur-bipedal` repository, inside its **Real Atari 2600** window:

```text
Robot UVC camera -> AtariAdventureWindow -> AtariAdventureSession
    -> policy frame history / calibrated preview / episode log
    -> Nano serial bridge -> relay contacts -> Atari controller port
```

The watcher in this repository is a standalone Python reference and
commissioning tool.  It is useful for validating a UVC device, a screen crop,
perspective correction, frame delivery, and timestamp logging independently of
the robot application.  It is not a second production camera process to run
beside Arthur: one camera device should have one capture owner per session.

## No code-level repository link

Do not add a Git submodule, symbolic link, or direct cross-repository import.
The reference watcher is Python and Arthur is a .NET/GTK application; linking
their source trees would add deployment coupling without sharing executable
code.  Arthur owns the live camera, policy observation history, UI, Nano
connection, and episode lifecycle.

Instead, keep these interfaces compatible and documented in both repositories:

| Interface | Contract |
| --- | --- |
| Calibration | `atari-camera-calibration.json` records normalized screen corners and crop.  Version the exact file with each run. |
| Observation | The policy sees only rectified camera pixels.  Its frame history is oldest-first, with a monotonic host capture timestamp for each processed frame. |
| Episode events | Log camera-frame, requested-action, Nano confirmation/applied mask, and stop/failure events with the same monotonic-clock convention. |
| Nano protocol | Newline-terminated `SET <mask> <duration_ms>`, `STOP`, and `PING`; treat `OK` replies as confirmations.  The Nano remains fail-neutral on error or timeout. |

If a future need arises for shared executable logic, extract only the stable
calibration and event-log schemas into a small, language-neutral package or
specification.  Do not couple either application repository until that shared
artifact has a real consumer in both languages.
