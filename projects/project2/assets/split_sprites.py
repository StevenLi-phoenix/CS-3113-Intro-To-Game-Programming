#!/usr/bin/env python3
"""Split a grid image into individual sprite PNG files.

Example:
    python split_sprites.py digits.png output_dir --rows 2 --cols 5

The script assumes each cell in the grid carries the same size and trims the
image accordingly. Output files are named with zero-padded indices unless a
name pattern is provided.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from typing import Iterable

try:
    from PIL import Image
except ImportError as exc:  # pragma: no cover - guidance for manual install
    raise SystemExit(
        "Pillow is required. Install with `python3 -m pip install Pillow`."
    ) from exc


def parse_args(argv: Iterable[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Split a sprite sheet into PNG files.")
    parser.add_argument("input", type=Path, help="Path to the source sheet")
    parser.add_argument(
        "output_dir",
        type=Path,
        help="Directory where split images will be stored (created if missing)",
    )
    parser.add_argument("--rows", type=int, default=2, help="Number of rows in the sheet")
    parser.add_argument("--cols", type=int, default=5, help="Number of columns in the sheet")
    parser.add_argument(
        "--name",
        type=str,
        default="sprite_{index:02d}.png",
        help="Filename pattern using {index}, {row}, {col}. Default: sprite_{index:02d}.png",
    )
    return parser.parse_args(list(argv))


def split_sheet(args: argparse.Namespace) -> None:
    if not args.input.is_file():
        raise SystemExit(f"Input file not found: {args.input}")
    if args.rows <= 0 or args.cols <= 0:
        raise SystemExit("rows and cols must be positive integers")

    args.output_dir.mkdir(parents=True, exist_ok=True)

    with Image.open(args.input) as sheet:
        width, height = sheet.size
        cell_width = width // args.cols
        cell_height = height // args.rows

        if cell_width * args.cols != width or cell_height * args.rows != height:
            print(
                "Warning: image dimensions are not evenly divisible by the grid; "
                "the rightmost/bottom pixels will be ignored."
            )

        index = 0
        for row in range(args.rows):
            for col in range(args.cols):
                left = col * cell_width
                upper = row * cell_height
                box = (left, upper, left + cell_width, upper + cell_height)
                crop = sheet.crop(box)
                filename = args.name.format(index=index, row=row, col=col)
                crop.save(args.output_dir / filename)
                index += 1

        print(f"Wrote {index} sprites to {args.output_dir}")


def main(argv: Iterable[str]) -> int:
    args = parse_args(argv)
    split_sheet(args)
    return 0


if __name__ == "__main__":
    import sys

    raise SystemExit(main(sys.argv[1:]))
