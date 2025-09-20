import cv2
import numpy as np
import os
import math
import json

print(cv2.__version__)

os.chdir(os.path.dirname(os.path.abspath(__file__)))

print(os.listdir("assets"))
picture_list = os.listdir("assets")
picture_list = [file for file in picture_list if file.endswith(".png")]
print(picture_list)

def crop_by_alpha(img):
    if img.shape[2] < 4:
        return img
    alpha = img[:, :, 3]
    coords = np.argwhere(alpha > 0)
    if coords.size == 0:
        return img
    y0, x0 = coords.min(axis=0)
    y1, x1 = coords.max(axis=0) + 1
    return img[y0:y1, x0:x1]

def pad_to_square(img):
    h, w = img.shape[:2]
    size = max(h, w)
    top = (size - h) // 2
    bottom = size - h - top
    left = (size - w) // 2
    right = size - w - left
    return cv2.copyMakeBorder(
        img, top, bottom, left, right,
        cv2.BORDER_CONSTANT, value=(0,0,0,0)
    )

image_list = []
for picture in picture_list:
    image = cv2.imread(f"assets/{picture}", cv2.IMREAD_UNCHANGED)
    cropped = crop_by_alpha(image)
    squared = pad_to_square(cropped)
    image_list.append(squared)

min_width = min([img.shape[1] for img in image_list])
min_height = min([img.shape[0] for img in image_list])
target_size = min(min_width, min_height)

resized_images = [cv2.resize(img, (target_size, target_size), interpolation=cv2.INTER_AREA)
                  for img in image_list]

print(f"All images resized to {target_size}x{target_size}")

grid_size = math.ceil(math.sqrt(len(resized_images)))
atlas_size = target_size * grid_size
uv_atlas = np.zeros((atlas_size, atlas_size, 4), dtype=np.uint8)

coords = {}
for idx, (name, img) in enumerate(zip(picture_list, resized_images)):
    row = idx // grid_size
    col = idx % grid_size
    y, x = row * target_size, col * target_size
    uv_atlas[y:y+target_size, x:x+target_size] = img

    u0, v0 = col / grid_size, row / grid_size
    u1, v1 = (col+1) / grid_size, (row+1) / grid_size

    px_min = (x, y)
    px_max = (x + target_size, y + target_size)

    coords[name] = {
        "uv_min": [u0, v0],
        "uv_max": [u1, v1],
        "px_min": px_min,
        "px_max": px_max
    }


cv2.imwrite("merged_texture.png", uv_atlas)
with open("merged_texture.json", "w") as f:
    json.dump(coords, f, indent=4)

print("Merged texture saved as 'merged_texture.png'")
print("UV coordinates saved as 'merged_texture.json'")
