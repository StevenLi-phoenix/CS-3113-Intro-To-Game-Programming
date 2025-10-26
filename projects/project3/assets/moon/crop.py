import cv2
import numpy as np
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

fp = "as11-40-5891-apollo-11-apollo-11-mission-image-shallow-craters-on-lunar-surface-01aa4a-1024.jpg"
# Load the image
img = cv2.imread(fp)

# Get image dimensions
height, width = img.shape[:2]

# Find the crop point by detecting where the black space ends
# We'll scan from top to bottom and find where the image content starts
crop_start = 0
threshold = 30  # Brightness threshold to distinguish black from content

for y in range(height):
    # Get the average brightness of the row
    row_mean = np.mean(img[y, :, :])
    
    # If the row is bright enough, we've found the start of content
    if row_mean > threshold:
        crop_start = y
        break

# Add a small margin to ensure we don't cut into the content
crop_start = max(0, crop_start - 5)

# Crop the image to remove the top black space
cropped_img = img[crop_start:height, 0:width]

# Save the cropped image
output_fp = fp.replace('.jpg', '_cropped.jpg')
cv2.imwrite(output_fp, cropped_img)

print(f"Cropped image saved as: {output_fp}")
print(f"Original size: {width}x{height}")
print(f"Cropped size: {cropped_img.shape[1]}x{cropped_img.shape[0]}")
print(f"Detected crop start at row: {crop_start} ({crop_start/height*100:.1f}% from top)")

# Now split the cropped image into 128x128 tiles
tile_size = 128
img_height, img_width = cropped_img.shape[:2]

# Calculate number of tiles horizontally (should be 8 for 1024 width)
num_tiles_x = img_width // tile_size

# Calculate number of tiles vertically
num_tiles_y = (img_height + tile_size - 1) // tile_size  # Round up

print(f"\nTiling information:")
print(f"Tile size: {tile_size}x{tile_size}")
print(f"Number of tiles horizontally: {num_tiles_x}")
print(f"Number of tiles vertically: {num_tiles_y}")
print(f"Total tiles: {num_tiles_x * num_tiles_y}")

# Create output directory for tiles
tiles_dir = "tiles"
os.makedirs(tiles_dir, exist_ok=True)
# Extract and save tiles
tile_count = 0
for y in range(num_tiles_y):
    for x in range(num_tiles_x):
        # Calculate tile boundaries
        x_start = x * tile_size
        y_start = y * tile_size
        x_end = min(x_start + tile_size, img_width)
        y_end = min(y_start + tile_size, img_height)
        
        # Extract tile
        tile = cropped_img[y_start:y_end, x_start:x_end]
        
        # If tile is smaller than tile_size (edge case), pad it from the bottom row
        if tile.shape[0] < tile_size or tile.shape[1] < tile_size:
            padded_tile = np.zeros((tile_size, tile_size, 3), dtype=np.uint8)
            # For the last row, we want to take the last 128 pixels instead of padding
            if y == num_tiles_y - 1 and tile.shape[0] < tile_size:
                # Extract from the bottom of the image
                y_start_adjusted = img_height - tile_size
                tile = cropped_img[y_start_adjusted:img_height, x_start:x_end]
            
            # If still need padding (for width), pad from the right
            if tile.shape[1] < tile_size:
                x_offset = 0
                padded_tile[:tile.shape[0], x_offset:x_offset+tile.shape[1]] = tile
                tile = padded_tile
        
        # Save tile
        tile_filename = f"tile_{y:02d}_{x:02d}.png"
        tile_path = os.path.join(tiles_dir, tile_filename)
        cv2.imwrite(tile_path, tile)
        tile_count += 1

print(f"\nSuccessfully saved {tile_count} tiles to '{tiles_dir}/' directory")

# Create a composite image showing all tiles in an 8xN grid
composite_height = num_tiles_y * tile_size
composite_width = num_tiles_x * tile_size
composite_img = np.zeros((composite_height, composite_width, 3), dtype=np.uint8)

# Place each tile in the composite image
for y in range(num_tiles_y):
    for x in range(num_tiles_x):
        tile_filename = f"tile_{y:02d}_{x:02d}.png"
        tile_path = os.path.join(tiles_dir, tile_filename)
        tile = cv2.imread(tile_path)
        
        y_start = y * tile_size
        x_start = x * tile_size
        composite_img[y_start:y_start+tile_size, x_start:x_start+tile_size] = tile

# Save the composite image
composite_filename = f"composite_8x{num_tiles_y}.png"
cv2.imwrite(composite_filename, composite_img)
print(f"\nCreated composite image '{composite_filename}' with dimensions {composite_width}x{composite_height}")
