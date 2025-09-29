#!/usr/bin/env python3
"""Stack all gameover_* images vertically into one sprite sheet."""

import argparse
import glob
import os
from typing import List

from PIL import Image


def load_images(file_paths: List[str]) -> List[Image.Image]:
    images: List[Image.Image] = []
    for path in file_paths:
        img = Image.open(path).convert("RGBA")
        images.append(img)
    return images


def stack_vertically(images: List[Image.Image]) -> Image.Image:
    target_width = max(img.width for img in images)
    target_height = max(img.height for img in images)
    total_height = target_height * len(images)
    stacked = Image.new("RGBA", (target_width, total_height), (0, 0, 0, 0))

    y_offset = 0
    for img in images:
        if img.width != target_width or img.height != target_height:
            img = img.resize((target_width, target_height), Image.BICUBIC)
        stacked.paste(img, (0, y_offset), img)
        y_offset += target_height

    return stacked


def main() -> None:
    parser = argparse.ArgumentParser(description="Stack gameover_* images vertically.")
    parser.add_argument(
        "--pattern",
        default="gameover_*.png",
        help="Glob pattern for the source images (relative to this script).",
    )
    parser.add_argument(
        "--output",
        default="gameover_stacked.png",
        help="Output filename (relative to this script).",
    )
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    search_pattern = os.path.join(script_dir, args.pattern)
    output_path = os.path.join(script_dir, args.output)

    file_paths = sorted(glob.glob(search_pattern))
    if not file_paths:
        raise SystemExit(f"No images found matching pattern: {args.pattern}")

    images = load_images(file_paths)
    stacked = stack_vertically(images)
    stacked.save(output_path)

    for img in images:
        img.close()

    print(f"Stacked {len(file_paths)} images into {output_path}")


if __name__ == "__main__":
    main()
