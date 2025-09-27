#!/usr/bin/env python3
"""Split an RGBA sprite sheet into tightly cropped, normalised components.

Features:
  * Detects connected opaque regions via flood fill on the alpha channel.
  * Sorts components by row (top to bottom) and column (left to right).
  * Saves each crop with configurable names, optional padding, and minimum area.
  * Normalises output sprites to a fixed canvas size (default 72x72).
"""
from __future__ import annotations

import argparse
import math
from collections import deque
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, List, Sequence, Tuple

try:
    from PIL import Image
except ImportError as exc:  # pragma: no cover - guidance for manual install
    raise SystemExit(
        "Pillow is required. Install with `python3 -m pip install Pillow`."
    ) from exc


@dataclass
class Component:
    left: int
    top: int
    right: int
    bottom: int
    area: int

    @property
    def width(self) -> int:
        return self.right - self.left

    @property
    def height(self) -> int:
        return self.bottom - self.top

    @property
    def center(self) -> Tuple[float, float]:
        return (self.left + self.width / 2, self.top + self.height / 2)


@dataclass
class Config:
    input_path: Path
    output_dir: Path
    min_area: int
    padding: int
    labels: Sequence[str]
    canvas_size: Tuple[int, int]


NEIGHBOURS = ((1, 0), (-1, 0), (0, 1), (0, -1))


def parse_size(value: str) -> Tuple[int, int]:
    parts = value.lower().split("x")
    if len(parts) != 2:
        raise argparse.ArgumentTypeError("Size must be WIDTHxHEIGHT, e.g. 72x72")
    try:
        width, height = (int(p) for p in parts)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("Size dimensions must be integers") from exc
    if width <= 0 or height <= 0:
        raise argparse.ArgumentTypeError("Size dimensions must be positive")
    return width, height


def parse_labels(value: str | None) -> Sequence[str]:
    if value is None:
        return list("1234567890")
    items = [item.strip() for item in value.split(",") if item.strip()]
    if not items:
        raise SystemExit("At least one label must be provided")
    return items


def parse_args(argv: Iterable[str]) -> Config:
    parser = argparse.ArgumentParser(
        description="Split an image into tightly cropped RGBA components",
    )
    parser.add_argument("input", type=Path, help="Path to the transparent source image")
    parser.add_argument(
        "output_dir", type=Path, help="Directory to store the individual PNG files"
    )
    parser.add_argument(
        "--min-area",
        type=int,
        default=100,
        help="Ignore components with fewer opaque pixels than this (default: 100)",
    )
    parser.add_argument(
        "--padding",
        type=int,
        default=0,
        help="Inset added evenly around each crop (default: 0)",
    )
    parser.add_argument(
        "--labels",
        type=str,
        default=None,
        help="Comma-separated names applied per sprite. Defaults to 1-9,0.",
    )
    parser.add_argument(
        "--size",
        type=parse_size,
        default=(72, 72),
        help="Output canvas size WIDTHxHEIGHT (default: 72x72)",
    )

    args = parser.parse_args(list(argv))
    if not args.input.is_file():
        raise SystemExit(f"Input file not found: {args.input}")

    if args.min_area < 1:
        raise SystemExit("min-area must be >= 1")
    if args.padding < 0:
        raise SystemExit("padding must be >= 0")

    labels = parse_labels(args.labels)

    return Config(
        input_path=args.input,
        output_dir=args.output_dir,
        min_area=args.min_area,
        padding=args.padding,
        labels=labels,
        canvas_size=args.size,
    )


def find_components(image: Image.Image, min_area: int) -> List[Component]:
    rgba = image.convert("RGBA")
    width, height = rgba.size
    pixels = rgba.load()

    visited = bytearray(width * height)
    components: List[Component] = []

    for y in range(height):
        for x in range(width):
            idx = y * width + x
            if visited[idx]:
                continue
            visited[idx] = 1
            if pixels[x, y][3] == 0:
                continue

            queue = deque([(x, y)])
            left = right = x
            top = bottom = y
            area = 0

            while queue:
                cx, cy = queue.popleft()
                cidx = cy * width + cx
                r, g, b, a = pixels[cx, cy]
                if a == 0:
                    continue

                area += 1
                if cx < left:
                    left = cx
                if cx > right:
                    right = cx
                if cy < top:
                    top = cy
                if cy > bottom:
                    bottom = cy

                for dx, dy in NEIGHBOURS:
                    nx, ny = cx + dx, cy + dy
                    if 0 <= nx < width and 0 <= ny < height:
                        nidx = ny * width + nx
                        if not visited[nidx]:
                            visited[nidx] = 1
                            queue.append((nx, ny))

            if area >= min_area:
                components.append(Component(left, top, right + 1, bottom + 1, area))

    return components


def sort_components(components: List[Component]) -> List[Component]:
    if not components:
        return []

    # Sort primarily by top, then left to preserve reading order.
    return sorted(components, key=lambda c: (c.top, c.left))


def clamp(value: int, lower: int, upper: int) -> int:
    return max(lower, min(upper, value))


def normalise_sprite(sprite: Image.Image, size: Tuple[int, int]) -> Image.Image:
    target_w, target_h = size
    sprite = sprite.convert("RGBA")

    if sprite.width > target_w or sprite.height > target_h:
        scale = min(target_w / sprite.width, target_h / sprite.height)
        new_size = (
            max(1, int(math.floor(sprite.width * scale))),
            max(1, int(math.floor(sprite.height * scale))),
        )
        sprite = sprite.resize(new_size, Image.LANCZOS)

    canvas = Image.new("RGBA", size, (0, 0, 0, 0))
    offset = (
        (target_w - sprite.width) // 2,
        (target_h - sprite.height) // 2,
    )
    canvas.paste(sprite, offset)
    return canvas


def export_components(image: Image.Image, components: List[Component], cfg: Config) -> None:
    cfg.output_dir.mkdir(parents=True, exist_ok=True)
    width, height = image.size
    count = 0

    for index, comp in enumerate(components):
        left = clamp(comp.left - cfg.padding, 0, width)
        top = clamp(comp.top - cfg.padding, 0, height)
        right = clamp(comp.right + cfg.padding, 0, width)
        bottom = clamp(comp.bottom + cfg.padding, 0, height)
        crop = image.crop((left, top, right, bottom))
        sprite = normalise_sprite(crop, cfg.canvas_size)

        if index < len(cfg.labels):
            filename = cfg.labels[index]
        else:
            filename = f"sprite_{index}"  # fallback if more components than labels

        path = cfg.output_dir / f"{filename}.png"
        sprite.save(path)
        count += 1

    print(f"Wrote {count} sprites to {cfg.output_dir}")


def main(argv: Iterable[str]) -> int:
    cfg = parse_args(argv)
    with Image.open(cfg.input_path) as img:
        components = find_components(img, cfg.min_area)
        if not components:
            raise SystemExit("No components found above the min-area threshold")
        ordered = sort_components(components)
        if len(cfg.labels) != len(ordered):
            print(
                f"Warning: found {len(ordered)} components but {len(cfg.labels)} labels; "
                "extra items will use fallback names."
            )
        export_components(img, ordered, cfg)
    return 0


if __name__ == "__main__":
    import sys

    raise SystemExit(main(sys.argv[1:]))
