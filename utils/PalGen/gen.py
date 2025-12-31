import struct
import numpy as np
from PIL import Image
import os
import math

def read_palette_dat(filepath="palette.dat"):
    """Read Duke3D PALETTE.DAT file"""
    with open(filepath, 'rb') as f:
        # Read base palette (256 colors, 3 bytes each, 6-bit VGA)
        palette = []
        for i in range(256):
            r, g, b = struct.unpack('BBB', f.read(3))
            # Convert 6-bit to 8-bit (multiply by 4)
            palette.append((r * 4, g * 4, b * 4))
        
        # Skip shade tables
        numpalookups = struct.unpack('<H', f.read(2))[0]
        f.seek(numpalookups * 256, 1)  # Skip shade tables
        
        # Skip translucency table
        f.seek(256 * 256, 1)  # Skip translucency table
    
    return palette

def read_lookup_dat(filepath="lookup.dat"):
    """Read Duke3D LOOKUP.DAT file including alternate palettes"""
    if not os.path.exists(filepath):
        return [], []
    
    with open(filepath, 'rb') as f:
        # Read number of sprite palettes
        spritepals = struct.unpack('B', f.read(1))[0]
        
        palette_swaps = []
        for i in range(spritepals):
            # Read palette swap index
            swap_index = struct.unpack('B', f.read(1))[0]
            # Read lookup table
            swap_table = list(struct.unpack('256B', f.read(256)))
            palette_swaps.append((swap_index, swap_table))
        
        # Read alternate palettes (animation palettes)
        alt_palettes = []
        palette_index = 0
        
        # Read remaining data as alternate palettes
        while True:
            try:
                palette = []
                for i in range(256):
                    r, g, b = struct.unpack('BBB', f.read(3))
                    # Convert 6-bit to 8-bit (multiply by 4)
                    palette.append((r * 4, g * 4, b * 4))
                alt_palettes.append((palette_index, palette))
                palette_index += 1
            except struct.error:
                break  # End of file
    
    return palette_swaps, alt_palettes

def apply_palette_swap(palette, swap_table):
    """Apply palette swap to base palette"""
    swapped_palette = []
    for i in range(256):
        mapped_index = swap_table[i]
        swapped_palette.append(palette[mapped_index])
    return swapped_palette

def euclidean_distance(color1, color2):
    """Calculate euclidean distance between two RGB colors"""
    r1, g1, b1 = color1
    r2, g2, b2 = color2
    return math.sqrt((r1 - r2)**2 + (g1 - g2)**2 + (b1 - b2)**2)

def find_nearest_palette_color(target_color, palette):
    """Find nearest color in palette using euclidean distance"""
    min_distance = float('inf')
    nearest_color = palette[0]
    
    for color in palette:
        distance = euclidean_distance(target_color, color)
        if distance < min_distance:
            min_distance = distance
            nearest_color = color
    
    return nearest_color

def generate_neutral_lut_256x16():
    """Generate neutral 256x16 true color LUT with 16x16 squares"""
    lut_width = 256
    lut_height = 16
    
    neutral_lut = []
    
    for y in range(lut_height):
        row = []
        for x in range(lut_width):
            # Determine which 16x16 square we're in
            square_x = x // 16  # 0-15 (16 squares horizontally)
            
            # Position within the current square
            local_x = x % 16    # 0-15
            local_y = y         # 0-15
            
            # Blue increases per square (square_x determines blue level)
            blue = (square_x * 255) // 15  # 0-255 across 16 squares
            
            # Within each square: RG gradient
            red = (local_x * 255) // 15    # Red increases left to right within square
            green = (local_y * 255) // 15  # Green increases top to bottom within square
            
            row.append((red, green, blue))
        neutral_lut.append(row)
    
    return neutral_lut

def convert_lut_to_palette(neutral_lut, target_palette):
    """Convert neutral LUT to palette-mapped LUT"""
    palette_lut = []
    
    for row in neutral_lut:
        palette_row = []
        for color in row:
            nearest_color = find_nearest_palette_color(color, target_palette)
            palette_row.append(nearest_color)
        palette_lut.append(palette_row)
    
    return palette_lut

def save_lut_image(lut_data, filename):
    """Save LUT data as PNG image with nearest neighbor sampling"""
    height = len(lut_data)
    width = len(lut_data[0])
    
    # Create image with nearest neighbor resampling
    image = Image.new('RGB', (width, height))
    pixels = []
    
    for row in lut_data:
        for color in row:
            pixels.append(color)
    
    image.putdata(pixels)
    
    # Save with no compression and nearest neighbor hint
    image.save(filename, optimize=False, compress_level=0)
    print(f"Saved {filename} ({width}x{height}) with nearest neighbor sampling")

def main():
    # Read palette data
    print("Reading PALETTE.DAT...")
    base_palette = read_palette_dat()
    
    # Read lookup data and alternate palettes
    palette_swaps = []
    alt_palettes = []
    if os.path.exists("lookup.dat"):
        print("Reading LOOKUP.DAT...")
        palette_swaps, alt_palettes = read_lookup_dat()
    
    # Generate neutral LUT
    print("Generating neutral 256x16 LUT...")
    neutral_lut = generate_neutral_lut_256x16()
    save_lut_image(neutral_lut, "neutral_lut.png")
    
    # Generate base palette LUT
    print("Converting to base palette LUT...")
    base_palette_lut = convert_lut_to_palette(neutral_lut, base_palette)
    save_lut_image(base_palette_lut, "duke3d_base_lut.png")
    
    # Generate palette swap LUTs (applied to base palette)
    for swap_index, swap_table in palette_swaps:
        print(f"Generating palette swap {swap_index} LUT...")
        swapped_palette = apply_palette_swap(base_palette, swap_table)
        swap_lut = convert_lut_to_palette(neutral_lut, swapped_palette)
        save_lut_image(swap_lut, f"duke3d_swap_{swap_index}_lut.png")
    
    # Generate alternate palette LUTs (underwater, animation, etc.)
    for alt_index, alt_palette in alt_palettes:
        print(f"Generating alternate palette {alt_index} LUT...")
        alt_lut = convert_lut_to_palette(neutral_lut, alt_palette)
        save_lut_image(alt_lut, f"duke3d_alt_{alt_index}_lut.png")
        
        # Also generate palette swaps applied to alternate palettes
        for swap_index, swap_table in palette_swaps:
            print(f"Generating alternate palette {alt_index} with swap {swap_index} LUT...")
            swapped_alt_palette = apply_palette_swap(alt_palette, swap_table)
            swap_alt_lut = convert_lut_to_palette(neutral_lut, swapped_alt_palette)
            save_lut_image(swap_alt_lut, f"duke3d_alt_{alt_index}_swap_{swap_index}_lut.png")
    
    total_luts = 1 + len(palette_swaps) + len(alt_palettes) + (len(alt_palettes) * len(palette_swaps))
    print(f"\nGenerated {total_luts} LUT files total")
    print("LUT dimensions: 256x16")
    print("Use with lutSize = 16.0 in shader")
    print("All images saved with nearest neighbor sampling (no interpolation)")
    
    # Print summary
    print(f"\nSummary:")
    print(f"- Base palette: 1 LUT")
    print(f"- Palette swaps: {len(palette_swaps)} LUTs")
    print(f"- Alternate palettes: {len(alt_palettes)} LUTs")
    print(f"- Alt palettes with swaps: {len(alt_palettes) * len(palette_swaps)} LUTs")

if __name__ == "__main__":
    main()
