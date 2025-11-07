#!/usr/bin/env python3
"""Process the witch sprite variants into atlases and metadata."""

from __future__ import annotations

import json
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Sequence, Tuple

import numpy as np
from PIL import Image

BASE_DIR = Path(__file__).resolve().parent
WITCH_ROOT = BASE_DIR / "assets" / "witch"

# Name overrides so metadata stays readable.
ANIMATION_NAME_OVERRIDES = {
    "atack": "attack",
    "idle_atack": "idle_attack",
}

# Preferred ordering for rows in the atlas. Any additional animations are added
# alphabetically after these.
ANIMATION_ORDER: Sequence[str] = (
    "attack",
    "back_start_run",
    "death",
    "fall",
    "hit",
    "idle_attack",
    "idle_back",
    "idle",
    "jump",
    "run",
    "start_run",
    "stop_run",
    "turn",
)


@dataclass(frozen=True)
class VariantPaths:
    name: str
    root: Path
    portrait_dir: Path
    portrait_input: Path
    sprites_dir: Path
    full_body_output: Path
    animate_output: Path
    metadata_output: Path


@dataclass
class FrameSlice:
    image: Image.Image
    offset: Tuple[int, int]
    original_size: Tuple[int, int]


@dataclass
class AnimationEntry:
    name: str
    frames: List[FrameSlice]
    source: Path
    min_offset_x: int
    max_right: int
    min_offset_y: int
    max_bottom: int


def load_rgba_image(path: Path) -> Image.Image:
    """Read an image file and convert it to RGBA."""
    if not path.exists():
        raise FileNotFoundError(f"Missing image: {path}")
    return Image.open(path).convert("RGBA")


def run_length_encode(columns: np.ndarray) -> List[Tuple[bool, int, int]]:
    """Return [(value, start, end)] runs for the boolean column array."""
    width = len(columns)
    if width == 0:
        return []

    runs: List[Tuple[bool, int, int]] = []
    start = 0
    current = bool(columns[0])

    for idx in range(1, width):
        value = bool(columns[idx])
        if value != current:
            runs.append((current, start, idx))
            start = idx
            current = value

    runs.append((current, start, width))
    return runs


def compute_max_hole_width(columns: np.ndarray) -> int:
    """Determine how many consecutive transparent columns can be treated as noise."""
    runs = run_length_encode(columns)
    separator_widths = []

    for idx, (value, start, end) in enumerate(runs):
        if not value:
            continue

        left_is_frame = idx > 0 and not runs[idx - 1][0]
        right_is_frame = idx < len(runs) - 1 and not runs[idx + 1][0]
        if left_is_frame and right_is_frame:
            separator_widths.append(end - start)

    if not separator_widths:
        return 0

    # Frames are separated by gaps at least as wide as the narrowest separator.
    # Anything smaller is treated as an internal transparent gap (noise).
    min_separator = min(separator_widths)
    return max(0, min_separator - 1)


def remove_small_transparent_gaps(columns: np.ndarray, max_hole: int) -> np.ndarray:
    """Fill transparent gaps that are too small to be real separators."""
    if max_hole <= 0:
        return columns

    cleaned = columns.copy()
    width = len(columns)
    idx = 0

    while idx < width:
        if not columns[idx]:
            idx += 1
            continue

        start = idx
        while idx < width and columns[idx]:
            idx += 1
        end = idx

        left_is_frame = start > 0 and not columns[start - 1]
        right_is_frame = end < width and not columns[end]
        if left_is_frame and right_is_frame and (end - start) <= max_hole:
            cleaned[start:end] = False

    return cleaned


def detect_frame_columns(img: Image.Image) -> List[Tuple[int, int]]:
    """Locate horizontal frame slices by checking for fully transparent columns."""
    rgba = img if img.mode == "RGBA" else img.convert("RGBA")
    alpha = np.asarray(rgba)[:, :, 3]
    transparent_columns = np.all(alpha == 0, axis=0)

    if transparent_columns.size == 0:
        return []

    max_hole = compute_max_hole_width(transparent_columns)
    cleaned_columns = remove_small_transparent_gaps(transparent_columns, max_hole)

    frames: List[Tuple[int, int]] = []
    width = cleaned_columns.shape[0]
    idx = 0

    while idx < width:
        while idx < width and cleaned_columns[idx]:
            idx += 1
        if idx >= width:
            break

        start = idx
        while idx < width and not cleaned_columns[idx]:
            idx += 1
        end = idx
        frames.append((start, end))

    return frames


def extract_frames(sheet: Image.Image, *, trim: bool) -> List[FrameSlice]:
    """Slice a sprite sheet into frames, optionally trimming transparent borders."""
    frames: List[FrameSlice] = []
    frame_columns = detect_frame_columns(sheet)

    for left, right in frame_columns:
        full_frame = sheet.crop((left, 0, right, sheet.height))
        if trim:
            alpha = full_frame.split()[3]
            bbox = alpha.getbbox()
            if bbox:
                offset_x, offset_y = bbox[0], bbox[1]
                trimmed = full_frame.crop(bbox)
                frames.append(
                    FrameSlice(
                        image=trimmed,
                        offset=(offset_x, offset_y),
                        original_size=full_frame.size,
                    )
                )
                continue
        # No trimming or empty bbox: keep original slice
        frames.append(
            FrameSlice(
                image=full_frame,
                offset=(0, 0),
                original_size=full_frame.size,
            )
        )

    return frames


def discover_animation_sheets(sprites_dir: Path) -> List[Tuple[str, Path]]:
    """Gather sprite sheets and map them to canonical animation names."""
    if not sprites_dir.exists():
        raise FileNotFoundError(f"Missing sprite directory: {sprites_dir}")

    discovered: Dict[str, Path] = {}
    for path in sorted(sprites_dir.glob("*.png")):
        if not path.is_file():
            continue
        canonical_name = ANIMATION_NAME_OVERRIDES.get(path.stem, path.stem)
        discovered[canonical_name] = path

    if not discovered:
        raise RuntimeError(f"No sprite sheets found in {sprites_dir}")

    ordered_names: List[str] = [name for name in ANIMATION_ORDER if name in discovered]
    extras = sorted(name for name in discovered if name not in ANIMATION_ORDER)
    ordered_names.extend(extras)

    return [(name, discovered[name]) for name in ordered_names]


def create_full_body_atlas(portrait_path: Path) -> Image.Image:
    """Rotate the character select portrait sheet into a vertical stack."""
    portrait = load_rgba_image(portrait_path)
    frame_columns = detect_frame_columns(portrait)
    if not frame_columns:
        raise RuntimeError(f"No frames detected in portrait {portrait_path}")

    frames = [
        portrait.crop((left, 0, right, portrait.height))
        for left, right in frame_columns
    ]

    atlas_width = max(frame.width for frame in frames)
    atlas_height = portrait.height * len(frames)
    atlas = Image.new("RGBA", (atlas_width, atlas_height), (0, 0, 0, 0))

    for row, frame in enumerate(frames):
        x_offset = (atlas_width - frame.width) // 2
        y_offset = row * portrait.height
        atlas.paste(frame, (x_offset, y_offset))

    return atlas


def load_animation_entries(sprites_dir: Path) -> List[AnimationEntry]:
    """Split every animation sheet into trimmed frames with positional offsets."""
    entries: List[AnimationEntry] = []

    for name, path in discover_animation_sheets(sprites_dir):
        sheet = load_rgba_image(path)
        frames = extract_frames(sheet, trim=True)
        if not frames:
            raise RuntimeError(f"No frames detected in animation {path}")
        min_offset_x = min(frame.offset[0] for frame in frames)
        max_right = max(frame.offset[0] + frame.image.width for frame in frames)
        min_offset_y = min(frame.offset[1] for frame in frames)
        max_bottom = max(frame.offset[1] + frame.image.height for frame in frames)

        entries.append(
            AnimationEntry(
                name=name,
                frames=frames,
                source=path,
                min_offset_x=min_offset_x,
                max_right=max_right,
                min_offset_y=min_offset_y,
                max_bottom=max_bottom,
            )
        )

    return entries


def create_animation_atlas(
    entries: Sequence[AnimationEntry]
) -> Tuple[Image.Image, Dict]:
    """Produce the animation atlas and its metadata."""
    max_dimension = 0
    max_columns = 0

    for entry in entries:
        max_columns = max(max_columns, len(entry.frames))
        width_span = entry.max_right - entry.min_offset_x
        height_span = entry.max_bottom - entry.min_offset_y
        for frame_slice in entry.frames:
            max_dimension = max(max_dimension, frame_slice.image.width, frame_slice.image.height)
        max_dimension = max(max_dimension, width_span, height_span)

    if max_dimension == 0 or max_columns == 0:
        raise RuntimeError("Unable to determine atlas dimensions from provided frames.")

    rows = len(entries)
    atlas = Image.new(
        "RGBA",
        (max_columns * max_dimension, rows * max_dimension),
        (0, 0, 0, 0),
    )

    metadata = {
        "square_size": max_dimension,
        "grid": {"columns": max_columns, "rows": rows},
        "animations": {},
    }

    for row_idx, entry in enumerate(entries):
        metadata["animations"][entry.name] = {
            "row": row_idx,
            "frame_count": len(entry.frames),
            "source": entry.source.name,
        }

        width_span = entry.max_right - entry.min_offset_x
        horizontal_padding = max_dimension - width_span
        row_x_offset = horizontal_padding // 2 if horizontal_padding > 0 else 0

        for col_idx, frame_slice in enumerate(entry.frames):
            offset_x, offset_y = frame_slice.offset

            tile_origin_x = col_idx * max_dimension
            tile_origin_y = row_idx * max_dimension

            paste_x = tile_origin_x + row_x_offset + (offset_x - entry.min_offset_x)
            paste_y = tile_origin_y + (offset_y - entry.min_offset_y)

            atlas.paste(frame_slice.image, (paste_x, paste_y))

    return atlas, metadata


def save_image(image: Image.Image, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path)


def save_metadata(metadata: Dict, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as stream:
        json.dump(metadata, stream, indent=2)


def cleanup_variant_sources(variant: VariantPaths) -> None:
    """Remove portrait/sprite sources and associated metadata files."""
    targets: Iterable[Path] = (
        variant.portrait_dir,
        variant.portrait_dir.parent / f"{variant.portrait_dir.name}.meta",
        variant.sprites_dir,
        variant.sprites_dir.parent / f"{variant.sprites_dir.name}.meta",
    )

    for path in targets:
        if path.is_dir():
            shutil.rmtree(path)
        elif path.is_file():
            path.unlink()


def enumerate_variants() -> List[VariantPaths]:
    """Discover each witch variant that has source folders."""
    if not WITCH_ROOT.exists():
        raise FileNotFoundError(f"Missing assets directory: {WITCH_ROOT}")

    variants: List[VariantPaths] = []
    for entry in sorted(WITCH_ROOT.iterdir(), key=lambda p: p.name.lower()):
        if not entry.is_dir():
            continue
        if entry.name == "processed":
            continue

        portrait_dir = entry / "portrait"
        sprites_dir = entry / "sprites"
        portrait_input = portrait_dir / "full_body.png"

        variants.append(
            VariantPaths(
                name=entry.name,
                root=entry,
                portrait_dir=portrait_dir,
                portrait_input=portrait_input,
                sprites_dir=sprites_dir,
                full_body_output=entry / "full_body.png",
                animate_output=entry / "animate.png",
                metadata_output=entry / "witch_animations.json",
            )
        )

    return variants


def process_variant(variant: VariantPaths, *, cleanup: bool) -> None:
    print(f"\nVariant: {variant.name}")

    if not variant.portrait_input.exists():
        print("  ! Skipping – portrait not found.")
        return
    if not variant.sprites_dir.exists():
        print("  ! Skipping – sprites directory not found.")
        return

    full_body = create_full_body_atlas(variant.portrait_input)
    save_image(full_body, variant.full_body_output)
    print(f"  • Saved vertical portrait to {variant.full_body_output.relative_to(BASE_DIR)}")

    try:
        animation_entries = load_animation_entries(variant.sprites_dir)
    except RuntimeError as error:
        print(f"  ! Skipping animations – {error}")
        return

    atlas, metadata = create_animation_atlas(animation_entries)
    save_image(atlas, variant.animate_output)
    print(f"  • Saved animation atlas to {variant.animate_output.relative_to(BASE_DIR)}")

    save_metadata(metadata, variant.metadata_output)
    print(f"  • Saved metadata to {variant.metadata_output.relative_to(BASE_DIR)}")

    if cleanup:
        cleanup_variant_sources(variant)
        print("  • Cleaned up portrait/ and sprites/ sources")

    print(
        f"  Summary → full_body: {full_body.size[0]}×{full_body.size[1]}, "
        f"animate: {atlas.size[0]}×{atlas.size[1]}, "
        f"square_size: {metadata['square_size']}"
    )


def main() -> None:
    print("Processing witch sprite variants…")
    variants = enumerate_variants()

    if not variants:
        print("No variants discovered.")
        return

    for variant in variants:
        process_variant(variant, cleanup=True)


if __name__ == "__main__":
    main()
