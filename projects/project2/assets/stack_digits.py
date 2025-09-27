#!/usr/bin/env python3
"""Stack labelled sprites vertically and emit a UV mapping JSON file.

Example:
    python stack_digits.py digits_norm stacked_digits.png --json uv.json --labels 1,2,3,4,5,6,7,8,9,0
"""
from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, List, Sequence, Tuple

try:
    from PIL import Image
except ImportError as exc:  # pragma: no cover - guidance for manual install
    raise SystemExit(
        "Pillow is required. Install with `python3 -m pip install Pillow`."
    ) from exc


@dataclass(frozen=True)
class Config:
    input_dir: Path
    output_image: Path
    output_json: Path
    labels: Sequence[str]
    tile_size: Tuple[int, int]


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
    labels = [item.strip() for item in value.split(",") if item.strip()]
    if not labels:
        raise SystemExit("At least one label must be provided")
    return labels


def parse_args(argv: Iterable[str]) -> Config:
    parser = argparse.ArgumentParser(description="Stack sprites vertically and write UV data")
    parser.add_argument("input_dir", type=Path, help="Directory containing labelled PNG sprites")
    parser.add_argument(
        "output_image",
        type=Path,
        help="Path for the stacked spritesheet (PNG recommended)",
    )
    parser.add_argument(
        "--json",
        dest="output_json",
        type=Path,
        help="Path for the UV JSON output. Defaults to <output_image>.json",
    )
    parser.add_argument(
        "--labels",
        type=str,
        default=None,
        help="Comma-separated list of sprite basenames in stack order (default: 1-9,0)",
    )
    parser.add_argument(
        "--size",
        type=parse_size,
        default=(72, 72),
        help="Expected sprite size WIDTHxHEIGHT (default: 72x72)",
    )

    args = parser.parse_args(list(argv))

    input_dir: Path = args.input_dir
    if not input_dir.is_dir():
        raise SystemExit(f"Input directory not found: {input_dir}")

    labels = parse_labels(args.labels)

    output_image: Path = args.output_image
    output_json: Path
    if args.output_json:
        output_json = args.output_json
    else:
        output_json = output_image.with_suffix(".json")

    return Config(
        input_dir=input_dir,
        output_image=output_image,
        output_json=output_json,
        labels=labels,
        tile_size=args.size,
    )


def load_sprite(path: Path, size: Tuple[int, int]) -> Image.Image:
    sprite = Image.open(path).convert("RGBA")
    if sprite.size != size:
        raise SystemExit(
            f"Sprite {path} has size {sprite.size}, expected {size}. "
            "Resize the input sheet first or adjust --size."
        )
    return sprite


def stack_sprites(cfg: Config) -> Tuple[Image.Image, List[dict]]:
    width, height = cfg.tile_size
    canvas_height = height * len(cfg.labels)
    sheet = Image.new("RGBA", (width, canvas_height), (0, 0, 0, 0))
    frames: List[dict] = []

    for index, label in enumerate(cfg.labels):
        sprite_path = cfg.input_dir / f"{label}.png"
        if not sprite_path.is_file():
            raise SystemExit(f"Sprite not found: {sprite_path}")

        sprite = load_sprite(sprite_path, cfg.tile_size)
        y_offset = index * height
        sheet.paste(sprite, (0, y_offset))

        frames.append(
            {
                "name": label,
                "x": 0,
                "y": y_offset,
                "w": width,
                "h": height,
            }
        )

    return sheet, frames


def write_outputs(sheet: Image.Image, frames: List[dict], cfg: Config) -> None:
    cfg.output_image.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(cfg.output_image)

    data = {
        "image": cfg.output_image.name,
        "size": {"w": sheet.width, "h": sheet.height},
        "frames": frames,
    }
    cfg.output_json.parent.mkdir(parents=True, exist_ok=True)
    with cfg.output_json.open("w", encoding="utf-8") as fh:
        json.dump(data, fh, indent=2)

    print(
        f"Created {cfg.output_image} ({sheet.width}x{sheet.height}) and "
        f"UV JSON {cfg.output_json}"
    )


def main(argv: Iterable[str]) -> int:
    cfg = parse_args(argv)
    sheet, frames = stack_sprites(cfg)
    write_outputs(sheet, frames, cfg)
    return 0


if __name__ == "__main__":
    import sys

    raise SystemExit(main(sys.argv[1:]))
