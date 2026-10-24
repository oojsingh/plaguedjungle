import os
from PIL import Image
import numpy as np
from scipy.ndimage import label, binary_dilation

src_path = 'assets/WhatsApp Image 2026-10-13 at 10.59.42 AM.jpeg'
if not os.path.exists(src_path):
    print("Source image not found:", src_path)
    exit(1)

img = Image.open(src_path).convert('RGBA')
arr = np.array(img)
rgb = arr[:, :, :3].astype(float)
bg = np.array([39.0, 39.0, 39.0])
dist = np.max(np.abs(rgb - bg), axis=-1)

CELL_W = 128
CELL_H = 192
COLS = 7
ROWS = 6

out_sheet = Image.new('RGBA', (COLS * CELL_W, ROWS * CELL_H), (0, 0, 0, 0))

# Exact bounding column slices for each row
ROW_CONFIGS = [
    {
        'name': 'IDLE',
        'row_idx': 0,
        'y_range': (6, 144),
        'cols': [(125, 235), (260, 360), (395, 495), (550, 650), (690, 790)],
        'align': 'ground',
        'ref_y': 130 # Ground feet reference y in source image
    },
    {
        'name': 'RUN',
        'row_idx': 1,
        'y_range': (148, 287),
        'cols': [(115, 235), (255, 375), (395, 490), (510, 610), (625, 735), (750, 870), (885, 990)],
        'align': 'ground',
        'ref_y': 271 # Ground feet reference y in source image
    },
    {
        'name': 'JUMP',
        'row_idx': 2,
        'y_range': (291, 439),
        'cols': [(125, 205), (230, 320), (340, 465), (485, 600), (620, 750), (785, 875), (920, 1000)],
        'align': 'ground',
        'ref_y': 425 # Ground feet reference y in source image
    },
    {
        'name': 'SWING',
        'row_idx': 3,
        'y_range': (443, 630),
        'cols': [(120, 200), (245, 335), (385, 485), (515, 635), (650, 755), (775, 850), (875, 995)],
        'align': 'top',
        'ref_y': 443 # Overhead anchor
    },
    {
        'name': 'TAIL_HOOK',
        'row_idx': 4,
        'y_range': (633, 817),
        'cols': [(115, 235), (250, 365), (380, 495), (510, 625), (640, 755), (765, 875), (885, 1000)],
        'align': 'top',
        'ref_y': 644 # Overhead branch anchor
    },
    {
        'name': 'WALL_CLING',
        'row_idx': 5,
        'y_range': (821, 1017),
        'cols': [(120, 215), (260, 355), (400, 490), (535, 625), (675, 765), (810, 925)],
        'align': 'fill',
        'ref_y': 825
    }
]

GROUND_Y = 176 # Feet y inside 192px tall cell
TOP_Y = 10     # Top anchor y inside 192px tall cell

for config in ROW_CONFIGS:
    r_idx = config['row_idx']
    y1, y2 = config['y_range']
    ref_y = config['ref_y']
    align = config['align']
    name = config['name']
    frames = []

    for c_idx, (x1, x2) in enumerate(config['cols']):
        sub_dist = dist[y1:y2, x1:x2]
        sub_rgb = rgb[y1:y2, x1:x2]
        sub_arr = arr[y1:y2, x1:x2].copy()

        # Solid foreground mask
        solid_mask = sub_dist > 16
        labeled, num_features = label(solid_mask)
        if num_features == 0:
            continue

        # Keep large components (remove jpeg ringing noise)
        clean_mask = np.zeros_like(solid_mask)
        for i in range(1, num_features + 1):
            if (labeled == i).sum() >= 15:
                clean_mask |= (labeled == i)

        edge_zone = binary_dilation(clean_mask, iterations=2)
        alpha = np.zeros_like(sub_dist)
        alpha[clean_mask] = 255.0
        border = edge_zone & (~clean_mask)
        alpha[border] = np.clip((sub_dist[border] - 7.0) / 9.0, 0.0, 1.0) * 255.0

        sub_arr[:, :, 3] = alpha.astype(np.uint8)

        # Decontaminate border colors
        for by, bx in zip(*np.where(border)):
            a = alpha[by, bx] / 255.0
            if a > 0.05:
                sub_arr[by, bx, :3] = np.clip((sub_rgb[by, bx] - bg * (1.0 - a)) / a, 0, 255).astype(np.uint8)

        # Find visible bounding box
        ys, xs = np.where(sub_arr[:, :, 3] > 10)
        if len(ys) == 0:
            continue

        abs_y_min = ys.min() + y1
        abs_y_max = ys.max() + y1
        abs_x_min = xs.min() + x1
        abs_x_max = xs.max() + x1

        crop = Image.fromarray(sub_arr[ys.min():ys.max()+1, xs.min():xs.max()+1])
        w, h = crop.size

        # Compute placement inside (c_idx * CELL_W, r_idx * CELL_H)
        cell_ox = c_idx * CELL_W
        cell_oy = r_idx * CELL_H

        # Center horizontally in 128px cell
        dest_x = cell_ox + (CELL_W - w) // 2

        if align == 'ground':
            # Align relative to the ground reference y
            foot_offset = ref_y - abs_y_max
            dest_y = cell_oy + (GROUND_Y - h) + foot_offset
            # Clamp if needed
            dest_y = min(dest_y, cell_oy + CELL_H - h)
            dest_y = max(dest_y, cell_oy)
        elif align == 'top':
            top_offset = abs_y_min - ref_y
            dest_y = cell_oy + TOP_Y + top_offset
        else: # 'fill'
            dest_y = cell_oy + (CELL_H - h) // 2

        out_sheet.paste(crop, (dest_x, dest_y), crop)

        # Also store for animated GIF preview
        frame_canvas = Image.new('RGBA', (CELL_W, CELL_H), (0,0,0,0))
        frame_canvas.paste(crop, ((CELL_W - w) // 2, dest_y - cell_oy), crop)
        frames.append(frame_canvas)

    if frames:
        gif_path = f'assets/{name.lower()}_anim.gif'
        dur = 120 if name == 'RUN' else 150
        frames[0].save(gif_path, save_all=True, append_images=frames[1:], duration=dur, loop=0, disposal=2)
        print(f"Saved {name} ({len(frames)} frames) to {gif_path}")

out_path = 'assets/textures/monkey_sheet.png'
out_sheet.save(out_path)
print(f"Successfully generated transparent master sprite sheet at {out_path} ({out_sheet.size[0]}x{out_sheet.size[1]})")

# Also copy/overwrite player.png so existing texture loaders pick it up immediately
out_sheet.save('assets/textures/player.png')
print("Updated assets/textures/player.png")
