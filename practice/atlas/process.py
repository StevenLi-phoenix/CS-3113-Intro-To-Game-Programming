import json
from collections import deque
from pathlib import Path
from typing import Dict, List, Tuple

from PIL import Image, ImageDraw, ImageFont


ASSET_PATH = Path("ElderAsset.png")
ATLAS_PATH = Path("atlas.json")
OUTPUT_JSON = Path("atlas_refined.json")
OUTPUT_OVERLAY = Path("atlas_overlay.png")
OVERLAY_SCALE = 4


def load_data() -> Tuple[Image.Image, Dict]:
    image = Image.open(ASSET_PATH).convert("RGBA")
    atlas = json.loads(ATLAS_PATH.read_text())
    return image, atlas


def build_alpha_mask(image: Image.Image):
    pixels = image.load()
    width, height = image.size
    mask = [[False] * width for _ in range(height)]
    for y in range(height):
        for x in range(width):
            mask[y][x] = pixels[x, y][3] > 0
    return mask


def refine_region(
    mask: List[List[bool]],
    seed_box: Dict[str, int],
) -> Dict[str, int]:
    width = len(mask[0])
    height = len(mask)
    x0 = max(0, seed_box["x"])
    y0 = max(0, seed_box["y"])
    x1 = min(width - 1, x0 + seed_box["w"] - 1)
    y1 = min(height - 1, y0 + seed_box["h"] - 1)

    seeds = []
    for yy in range(y0, y1 + 1):
        for xx in range(x0, x1 + 1):
            if mask[yy][xx]:
                seeds.append((xx, yy))

    if not seeds:
        return seed_box

    q = deque(seeds)
    visited = set(seeds)
    min_x = min(xx for xx, _ in seeds)
    max_x = max(xx for xx, _ in seeds)
    min_y = min(yy for _, yy in seeds)
    max_y = max(yy for _, yy in seeds)
    dirs = [(-1, 0), (1, 0), (0, -1), (0, 1)]

    while q:
        px, py = q.popleft()
        for dx, dy in dirs:
            nx, ny = px + dx, py + dy
            if (
                0 <= nx < width
                and 0 <= ny < height
                and (nx, ny) not in visited
                and mask[ny][nx]
            ):
                visited.add((nx, ny))
                q.append((nx, ny))
                if nx < min_x:
                    min_x = nx
                if nx > max_x:
                    max_x = nx
                if ny < min_y:
                    min_y = ny
                if ny > max_y:
                    max_y = ny

    return {
        "tag": seed_box["tag"],
        "x": min_x,
        "y": min_y,
        "w": max_x - min_x + 1,
        "h": max_y - min_y + 1,
    }


def load_font(size: int) -> ImageFont.ImageFont:
    try:
        return ImageFont.truetype("DejaVuSans.ttf", size)
    except OSError:
        base = ImageFont.load_default()
        if hasattr(base, "font_variant"):
            return base.font_variant(size=size)
        return base


def draw_overlay(image: Image.Image, regions: List[Dict[str, int]]):
    scale = OVERLAY_SCALE
    overlay = image.resize(
        (image.width * scale, image.height * scale), Image.NEAREST
    )
    draw = ImageDraw.Draw(overlay)
    font = load_font(8 * scale)
    for region in regions:
        x0 = region["x"] * scale
        y0 = region["y"] * scale
        x1 = (region["x"] + region["w"]) * scale
        y1 = (region["y"] + region["h"]) * scale
        draw.rectangle((x0, y0, x1, y1), outline="red", width=max(1, scale))
        cx = x0 + (region["w"] * scale) // 2
        cy = y0 + (region["h"] * scale) // 2
        tag = region["tag"]
        bbox = font.getbbox(tag)
        text_w = bbox[2] - bbox[0]
        text_h = bbox[3] - bbox[1]
        draw.text(
            (cx - text_w // 2, cy - text_h // 2),
            tag,
            fill="yellow",
            font=font,
        )
    overlay.save(OUTPUT_OVERLAY)


def main():
    image, atlas = load_data()
    mask = build_alpha_mask(image)
    refined = []

    for sprite in atlas.get("sprites", []):
        refined.append(refine_region(mask, sprite))

    OUTPUT_JSON.write_text(json.dumps({"sprites": refined}, indent=2))
    draw_overlay(image, refined)
    print(f"Saved {OUTPUT_JSON} and {OUTPUT_OVERLAY}")


if __name__ == "__main__":
    main()
