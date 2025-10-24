import cv2
import numpy as np
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))


fp = "earth.png"
if not os.path.exists(fp):
    raise FileNotFoundError(f"{fp} not found")
image = cv2.imread(fp, cv2.IMREAD_UNCHANGED)

# Convert to BGRA if not already
if image.shape[2] == 3:
    image = cv2.cvtColor(image, cv2.COLOR_BGR2BGRA)

# Set all black pixels (0,0,0) to transparent
mask = np.all(image[:, :, :3] == [0, 0, 0], axis=2)
image[mask, 3] = 0

# Get image dimensions
height, width = image.shape[:2]
center_x, center_y = width // 2, height // 2

# Create a mask for the circle
y_coords, x_coords = np.ogrid[:height, :width]
circle_mask = (x_coords - center_x)**2 + (y_coords - center_y)**2 <= (1024 // 2)**2

# Set circle pixels to black where alpha is 0
transparent_mask = image[:, :, 3] == 0
combined_mask = circle_mask & transparent_mask
image[combined_mask] = [0, 0, 0, 255]

# Find bounding box of non-transparent pixels
alpha_channel = image[:, :, 3]
non_transparent = np.where(alpha_channel > 0)

if len(non_transparent[0]) > 0:
    min_y, max_y = non_transparent[0].min(), non_transparent[0].max()
    min_x, max_x = non_transparent[1].min(), non_transparent[1].max()
    
    # Calculate dimensions
    crop_height = max_y - min_y + 1
    crop_width = max_x - min_x + 1
    
    # Make it square by using the larger dimension
    square_size = max(crop_height, crop_width)
    
    # Calculate center of the bounding box
    center_crop_y = (min_y + max_y) // 2
    center_crop_x = (min_x + max_x) // 2
    
    # Calculate new bounds for square crop
    half_size = square_size // 2
    new_min_y = max(0, center_crop_y - half_size)
    new_max_y = min(height, center_crop_y + half_size)
    new_min_x = max(0, center_crop_x - half_size)
    new_max_x = min(width, center_crop_x + half_size)
    
    # Crop to square
    image = image[new_min_y:new_max_y, new_min_x:new_max_x]

# Save the result
output_fp = fp.replace('.png', '_transparent.png')
cv2.imwrite(output_fp, image)

print(f"Saved transparent image to {output_fp}")
print(image.shape)