#!/usr/bin/env python3
"""Low-latency UVC camera capture and observation logging.

The policy-facing image is always derived solely from camera pixels.  This
module deliberately has no console or emulator state inputs.
"""

from __future__ import annotations

import argparse
import json
import logging
import signal
import sys
import time
from collections import deque
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Deque, Optional, Sequence, Tuple, Union

import cv2
import numpy as np


LOG = logging.getLogger(__name__)
Size = Tuple[int, int]  # width, height (OpenCV convention)


@dataclass(frozen=True)
class Observation:
    """A policy observation with a host-monotonic capture timestamp.

    ``frames`` is ordered oldest to newest and has shape
    ``(history, height, width)``.  Values are uint8 grayscale pixels.
    """

    frame_id: int
    captured_at_ns: int
    frames: np.ndarray


class CameraWatcher:
    """Capture a UVC camera, stabilize its screen view, and publish frames.

    The caller may supply ``on_observation`` to run a policy.  That callback
    must return quickly: blocking inference increases capture latency.  For a
    larger agent, hand observations to a bounded latest-frame queue instead.
    """

    def __init__(
        self,
        device: Union[int, str] = 0,
        *,
        capture_size: Optional[Size] = None,
        output_size: Size = (160, 210),
        history: int = 4,
        crop: Optional[Tuple[float, float, float, float]] = None,
        corners: Optional[Sequence[Sequence[float]]] = None,
        run_dir: Optional[Path] = None,
        record_video: bool = False,
        on_observation: Optional[Callable[[Observation], None]] = None,
    ) -> None:
        if history < 1:
            raise ValueError("history must be at least 1")
        if any(value <= 0 for value in output_size):
            raise ValueError("output size must be positive")
        if crop is not None and not self._valid_crop(crop):
            raise ValueError("crop must be x,y,width,height normalized to 0..1")
        if corners is not None and len(corners) != 4:
            raise ValueError("corners must contain four points: top-left clockwise")

        self.device = device
        self.capture_size = capture_size
        self.output_size = output_size
        self.history = history
        self.crop = crop
        self.corners = None if corners is None else np.float32(corners)
        self.run_dir = run_dir
        self.record_video = record_video
        self.on_observation = on_observation
        self._frames: Deque[np.ndarray] = deque(maxlen=history)
        self._capture: Optional[cv2.VideoCapture] = None
        self._video_writer: Optional[cv2.VideoWriter] = None
        self._events = None
        self._frame_id = 0

    @staticmethod
    def _valid_crop(crop: Tuple[float, float, float, float]) -> bool:
        x, y, width, height = crop
        return x >= 0 and y >= 0 and width > 0 and height > 0 and x + width <= 1 and y + height <= 1

    def start(self) -> None:
        """Open the camera and create the run log. Raises RuntimeError on failure."""
        api = cv2.CAP_V4L2 if sys.platform.startswith("linux") else cv2.CAP_ANY
        self._capture = cv2.VideoCapture(self.device, api)
        if not self._capture.isOpened():
            raise RuntimeError(f"could not open camera {self.device!r}")
        # Ask V4L2 to retain as little stale video as the driver permits.
        self._capture.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        if self.capture_size:
            self._capture.set(cv2.CAP_PROP_FRAME_WIDTH, self.capture_size[0])
            self._capture.set(cv2.CAP_PROP_FRAME_HEIGHT, self.capture_size[1])
        if self.run_dir:
            self.run_dir.mkdir(parents=True, exist_ok=True)
            self._events = (self.run_dir / "events.jsonl").open("a", encoding="utf-8", buffering=1)
            self._write_event({"type": "run_started", "captured_at_ns": time.monotonic_ns(),
                               "device": str(self.device), "output_size": self.output_size,
                               "history": self.history, "crop": self.crop})

    def stop(self) -> None:
        """Release local resources; this never sends a joystick command."""
        if self._events:
            self._write_event({"type": "run_stopped", "captured_at_ns": time.monotonic_ns()})
            self._events.close()
            self._events = None
        if self._video_writer:
            self._video_writer.release()
            self._video_writer = None
        if self._capture:
            self._capture.release()
            self._capture = None

    def _write_event(self, event: dict) -> None:
        if self._events:
            self._events.write(json.dumps(event, separators=(",", ":")) + "\n")

    def record_action(self, requested_mask: int, duration_ms: int, *, applied_mask: Optional[int] = None) -> None:
        """Log an action from the serial bridge using the same monotonic clock.

        Call this when the host requests an action, and again with
        ``applied_mask`` once a Nano response is received.
        """
        self._write_event({"type": "action", "captured_at_ns": time.monotonic_ns(),
                           "requested_mask": requested_mask, "duration_ms": duration_ms,
                           "applied_mask": applied_mask})

    def _screen_image(self, image: np.ndarray) -> np.ndarray:
        height, width = image.shape[:2]
        if self.corners is not None:
            destination = np.float32([[0, 0], [self.output_size[0] - 1, 0],
                                      [self.output_size[0] - 1, self.output_size[1] - 1],
                                      [0, self.output_size[1] - 1]])
            transform = cv2.getPerspectiveTransform(self.corners, destination)
            return cv2.warpPerspective(image, transform, self.output_size)
        if self.crop:
            x, y, crop_width, crop_height = self.crop
            left, top = round(x * width), round(y * height)
            right, bottom = round((x + crop_width) * width), round((y + crop_height) * height)
            image = image[top:bottom, left:right]
        return cv2.resize(image, self.output_size, interpolation=cv2.INTER_AREA)

    def read(self) -> Optional[Observation]:
        """Read and preprocess one frame; returns None for a transient miss."""
        if not self._capture:
            raise RuntimeError("call start() before read()")
        ok, raw = self._capture.read()
        captured_at_ns = time.monotonic_ns()
        if not ok or raw is None:
            self._write_event({"type": "frame_drop", "captured_at_ns": captured_at_ns})
            return None
        screen = self._screen_image(raw)
        gray = cv2.cvtColor(screen, cv2.COLOR_BGR2GRAY)
        self._frames.append(gray)
        while len(self._frames) < self.history:
            self._frames.appendleft(gray)
        self._frame_id += 1
        observation = Observation(self._frame_id, captured_at_ns, np.stack(self._frames, axis=0))
        self._write_event({"type": "frame", "frame_id": observation.frame_id,
                           "captured_at_ns": captured_at_ns, "source_width": int(raw.shape[1]),
                           "source_height": int(raw.shape[0])})
        if self.record_video:
            self._record(screen)
        if self.on_observation:
            self.on_observation(observation)
        return observation

    def _record(self, screen: np.ndarray) -> None:
        if not self.run_dir:
            raise RuntimeError("--record-video requires --run-dir")
        if not self._video_writer:
            path = str(self.run_dir / "screen.mp4")
            writer = cv2.VideoWriter(path, cv2.VideoWriter_fourcc(*"mp4v"), 30.0, self.output_size)
            if not writer.isOpened():
                raise RuntimeError(f"could not create {path}")
            self._video_writer = writer
        self._video_writer.write(screen)


def _parse_size(value: str) -> Size:
    try:
        width, height = (int(part) for part in value.lower().split("x", 1))
    except ValueError as error:
        raise argparse.ArgumentTypeError("size must be WIDTHxHEIGHT") from error
    if width <= 0 or height <= 0:
        raise argparse.ArgumentTypeError("size must be positive")
    return width, height


def _parse_crop(value: str) -> Tuple[float, float, float, float]:
    try:
        crop = tuple(float(part) for part in value.split(","))
    except ValueError as error:
        raise argparse.ArgumentTypeError("crop must be x,y,width,height") from error
    if len(crop) != 4 or not CameraWatcher._valid_crop(crop):
        raise argparse.ArgumentTypeError("crop must be normalized x,y,width,height within 0..1")
    return crop  # type: ignore[return-value]


def main() -> int:
    parser = argparse.ArgumentParser(description="Capture the robot UVC camera as Atari policy observations.")
    parser.add_argument("--device", default="0", help="UVC device index or path (default: 0)")
    parser.add_argument("--capture-size", type=_parse_size, help="request camera WIDTHxHEIGHT")
    parser.add_argument("--output-size", type=_parse_size, default=(160, 210), help="policy WIDTHxHEIGHT")
    parser.add_argument("--history", type=int, default=4, help="number of grayscale frames per observation")
    parser.add_argument("--crop", type=_parse_crop, help="normalized screen crop: x,y,width,height")
    parser.add_argument("--corners", type=Path, help="JSON file with four screen corners, top-left clockwise")
    parser.add_argument("--run-dir", type=Path, help="directory for timestamped events and optional video")
    parser.add_argument("--record-video", action="store_true", help="write rectified screen.mp4 in run directory")
    parser.add_argument("--no-preview", action="store_true", help="run without an OpenCV preview window")
    args = parser.parse_args()
    device: Union[int, str] = int(args.device) if args.device.isdigit() else args.device
    corners = json.loads(args.corners.read_text(encoding="utf-8")) if args.corners else None
    watcher = CameraWatcher(device, capture_size=args.capture_size, output_size=args.output_size,
                            history=args.history, crop=args.crop, corners=corners,
                            run_dir=args.run_dir, record_video=args.record_video)
    keep_running = True
    def stop_signal(_signum: int, _frame: object) -> None:
        nonlocal keep_running
        keep_running = False
    signal.signal(signal.SIGINT, stop_signal)
    signal.signal(signal.SIGTERM, stop_signal)
    try:
        watcher.start()
        while keep_running:
            observation = watcher.read()
            if observation is not None and not args.no_preview:
                cv2.imshow("Atari watcher — q to quit", observation.frames[-1])
                if cv2.waitKey(1) & 0xFF == ord("q"):
                    break
    finally:
        watcher.stop()
        cv2.destroyAllWindows()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
