import cv2
import numpy as np
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

fp = "flames.jpeg"
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

# Find bounding box of non-transparent pixels
alpha_channel = image[:, :, 3]
non_transparent = np.where(alpha_channel > 0)

if len(non_transparent[0]) > 0:
    min_y, max_y = non_transparent[0].min(), non_transparent[0].max()
    min_x, max_x = non_transparent[1].min(), non_transparent[1].max()
    
    # Calculate center of the bounding box
    center_crop_y = (min_y + max_y) // 2
    center_crop_x = (min_x + max_x) // 2
    
    # Calculate new bounds for 256x256 crop
    half_size = 256 // 2
    new_min_y = max(0, center_crop_y - half_size)
    new_max_y = min(height, center_crop_y + half_size)
    new_min_x = max(0, center_crop_x - half_size)
    new_max_x = min(width, center_crop_x + half_size)
    
    # Crop to 256x256
    image = image[new_min_y:new_max_y, new_min_x:new_max_x]
    
    # Ensure exact 256x256 size by padding if necessary
    current_height, current_width = image.shape[:2]
    if current_height < 256 or current_width < 256:
        # Create a transparent 256x256 canvas
        canvas = np.zeros((256, 256, 4), dtype=np.uint8)
        # Calculate position to paste the cropped image
        paste_y = (256 - current_height) // 2
        paste_x = (256 - current_width) // 2
        canvas[paste_y:paste_y+current_height, paste_x:paste_x+current_width] = image
        image = canvas

# Save the result
output_fp = fp.replace('.jpeg', '_transparent.png')
cv2.imwrite(output_fp, image)

print(f"Saved transparent image to {output_fp}")
print(image.shape)


# Stack 10 images vertically
stacked_image = np.vstack([image] * 10)

# Save the stacked result
stacked_output_fp = fp.replace('.jpeg', '_stacked.png')
cv2.imwrite(stacked_output_fp, stacked_image)

print(f"Saved stacked image to {stacked_output_fp}")
print(f"Stacked image shape: {stacked_image.shape}")
