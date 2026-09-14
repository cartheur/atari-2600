# Watcher: robot-camera input on the Linux host

`watcher.py` converts the robot's UVC camera feed into the only visual input
available to the Atari-playing policy.  It captures a frame, applies the
versioned screen crop or perspective transform, converts it to grayscale,
resizes it, and supplies a fixed history of recent frames.  It never reads
emulator memory or console state.

## Setup

On the Linux host, confirm the UVC device and its supported modes first:

```bash
v4l2-ctl --list-devices
v4l2-ctl --device=/dev/video0 --list-formats-ext
python3 -m pip install -r watcher/requirements.txt
```

Lock the camera's focus, exposure, gain and white balance using its V4L2
controls before each run.  The exact control names depend on the camera;
inspect them with `v4l2-ctl --device=/dev/video0 --list-ctrls`.

## Run

Use a normalized crop after physically framing the display.  This example
keeps the central 80% of the camera image, creates four-frame observations,
opens a preview, and stores an event log plus the processed display video:

```bash
python3 watcher/watcher.py --device /dev/video0 --capture-size 1280x720 \
  --crop 0.10,0.10,0.80,0.80 --history 4 --run-dir runs/2026-09-14-test \
  --record-video
```

Press `q` in the preview to finish.  For a headless host, add `--no-preview`.
`events.jsonl` gives every source frame a `time.monotonic_ns()` timestamp, so
camera observations can be correlated with requested and Nano-applied actions.

For a non-rectangular screen view, provide `--corners screen-corners.json`:

```json
[[151, 73], [1111, 80], [1120, 674], [144, 667]]
```

Points are pixel coordinates in the raw image, ordered top-left clockwise.
The resulting perspective-corrected screen is written at the configured policy
size.  Store the command and calibration file with the episode configuration.

## Connect a policy safely

Import `CameraWatcher` and provide an `on_observation` callback.  Its
`Observation.frames` is a `uint8` array shaped `(history, height, width)`,
oldest frame first.  Call `record_action()` in the host's serial bridge when
an action is requested and when the Nano confirms it.  The watcher does not
control relays itself; the Nano remains responsible for timeout-to-neutral.

For the production integration in the `arthur-bipedal` repository, see
[ARTHUR_BIPEDAL_INTEGRATION.md](ARTHUR_BIPEDAL_INTEGRATION.md).  Arthur's
**Real Atari 2600** window owns the live camera session; do not run two camera
capture processes against the same UVC device.
