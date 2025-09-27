#!/usr/bin/env python3
"""Remove a solid background from an image by flood filling from the edges.

Example:
    python remove_background.py image.png output.png --tolerance 28 --expand 2

The script assumes the background colour is consistent (e.g. white) and that it
appears on the image border. It flood-fills from every border pixel using the
specified colour tolerance, marks the discovered background as transparent, and
optionally expands the background mask a few pixels to eliminate compression
artifacts.
"""
from __future__ import annotations

import argparse
import sys
from collections import deque
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Tuple

try:
    from PIL import Image
except ImportError as exc:  # pragma: no cover - guidance for manual install
    raise SystemExit(
        "Pillow is required. Install with `python3 -m pip install Pillow`."
    ) from exc

RGB = Tuple[int, int, int]


@dataclass(frozen=True)
class Config:
    input_path: Path
    output_path: Path
    tolerance: int
    expand: int
    background: RGB


def parse_args(argv: Iterable[str]) -> Config:
    parser = argparse.ArgumentParser(
        description="Flood-fill the border colour to make it transparent."
    )
    parser.add_argument("input", type=Path, help="Input image (any Pillow format)")
    parser.add_argument(
        "output",
        nargs="?",
        type=Path,
        help="Output file. Defaults to '<input>_transparent.<suffix>'.",
    )
    parser.add_argument(
        "--tolerance",
        type=int,
        default=28,
        help="Per-channel colour distance (0-255) to treat as background (default: 28).",
    )
    parser.add_argument(
        "--expand",
        type=int,
        default=1,
        help="Number of times to grow the background mask to remove halos (default: 1).",
    )
    parser.add_argument(
        "--bg-color",
        type=str,
        default=None,
        help="Override background colour as R,G,B or #RRGGBB. Defaults to averaging the four corners.",
    )

    args = parser.parse_args(list(argv))

    if not args.input.is_file():
        raise SystemExit(f"Input file not found: {args.input}")

    if args.output:
        output_path = args.output
    else:
        output_filename = f"{args.input.stem}_transparent{args.input.suffix}"
        output_path = args.input.parent / output_filename

    tolerance = max(0, min(255, args.tolerance))
    expand = max(0, args.expand)

    background = (
        parse_colour(args.bg_color)
        if args.bg_color
        else estimate_background_colour(args.input)
    )

    return Config(args.input, output_path, tolerance, expand, background)


def parse_colour(value: str) -> RGB:
    value = value.strip()
    if value.startswith("#"):
        hex_value = value[1:]
        if len(hex_value) != 6:
            raise SystemExit("Hex colours must be in #RRGGBB format")
        r = int(hex_value[0:2], 16)
        g = int(hex_value[2:4], 16)
        b = int(hex_value[4:6], 16)
        return r, g, b

    parts = value.split(",")
    if len(parts) != 3:
        raise SystemExit("Colour must be formatted as R,G,B or #RRGGBB")

    try:
        r, g, b = (int(p) for p in parts)
    except ValueError as exc:
        raise SystemExit("Colour components must be integers") from exc

    for channel in (r, g, b):
        if not 0 <= channel <= 255:
            raise SystemExit("Colour components must be between 0 and 255")

    return r, g, b


def estimate_background_colour(image_path: Path) -> RGB:
    with Image.open(image_path) as img:
        rgb = img.convert("RGB")
        w, h = rgb.size
        corners = [
            rgb.getpixel((0, 0)),
            rgb.getpixel((w - 1, 0)),
            rgb.getpixel((0, h - 1)),
            rgb.getpixel((w - 1, h - 1)),
        ]
    avg = tuple(sum(channel[i] for channel in corners) // len(corners) for i in range(3))
    return avg  # type: ignore[return-value]


def colour_within(colour: RGB, reference: RGB, tolerance: int) -> bool:
    return max(abs(channel - ref) for channel, ref in zip(colour, reference)) <= tolerance


def flood_fill_background(image: Image.Image, cfg: Config) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = rgba.load()
    width, height = rgba.size

    visited = bytearray(width * height)
    mask = bytearray(width * height)
    queue: deque[Tuple[int, int]] = deque()

    def enqueue(x: int, y: int) -> None:
        idx = y * width + x
        if visited[idx]:
            return
        visited[idx] = 1
        queue.append((x, y))

    # Seed the queue with every edge pixel.
    for x in range(width):
        enqueue(x, 0)
        enqueue(x, height - 1)
    for y in range(height):
        enqueue(0, y)
        enqueue(width - 1, y)

    neighbours = ((1, 0), (-1, 0), (0, 1), (0, -1))

    while queue:
        x, y = queue.popleft()
        idx = y * width + x
        r, g, b, a = pixels[x, y]

        if not colour_within((r, g, b), cfg.background, cfg.tolerance):
            continue

        mask[idx] = 1
        pixels[x, y] = (r, g, b, 0)

        for dx, dy in neighbours:
            nx, ny = x + dx, y + dy
            if 0 <= nx < width and 0 <= ny < height:
                n_idx = ny * width + nx
                if not visited[n_idx]:
                    visited[n_idx] = 1
                    queue.append((nx, ny))

    if cfg.expand:
        grow_mask(mask, width, height, cfg.expand)

    # Apply the final mask to ensure expanded pixels are transparent.
    for idx, flag in enumerate(mask):
        if not flag:
            continue
        x = idx % width
        y = idx // width
        r, g, b, _ = pixels[x, y]
        pixels[x, y] = (r, g, b, 0)

    return rgba


def grow_mask(mask: bytearray, width: int, height: int, steps: int) -> None:
    for _ in range(steps):
        new_mask = mask[:]
        for y in range(height):
            row_offset = y * width
            for x in range(width):
                idx = row_offset + x
                if mask[idx]:
                    continue
                if has_masked_neighbour(mask, width, height, x, y):
                    new_mask[idx] = 1
        mask[:] = new_mask


def has_masked_neighbour(mask: bytearray, width: int, height: int, x: int, y: int) -> bool:
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        nx, ny = x + dx, y + dy
        if 0 <= nx < width and 0 <= ny < height:
            if mask[ny * width + nx]:
                return True
    return False


def main(argv: Iterable[str]) -> int:
    cfg = parse_args(argv)

    with Image.open(cfg.input_path) as img:
        result = flood_fill_background(img, cfg)
        cfg.output_path.parent.mkdir(parents=True, exist_ok=True)
        result.save(cfg.output_path)

    print(
        f"Saved image with transparent background to {cfg.output_path} "
        f"(tolerance={cfg.tolerance}, expand={cfg.expand}, bg={cfg.background})"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
