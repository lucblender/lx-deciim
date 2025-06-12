import svgwrite
import random

def generate_square_pattern_svg(
    filename='pattern.svg',
    width=607,
    height=1286,
    grid_rows=20,
    grid_cols=20,
    density=0.5,  # 0 to 1
    min_size=2,
    max_size=20
):
    dwg = svgwrite.Drawing(filename, size=(width, height))
    cell_width = width / grid_cols
    cell_height = height / grid_rows

    for row in range(grid_rows):
        for col in range(grid_cols):
            if random.random() <= density:
                # Y-dependent size (bigger at top, smaller at bottom)
                y_ratio = 1 - row / (grid_rows - 1)  # 1 at top, 0 at bottom
                size = min_size + (max_size - min_size) * y_ratio
                
                # Center the square in the cell
                cx = col * cell_width + cell_width / 2
                cy = row * cell_height + cell_height / 2
                top_left_x = cx - size / 2
                top_left_y = cy - size / 2
                
                dwg.add(dwg.rect(
                    insert=(top_left_x, top_left_y),
                    size=(size, size),
                    fill='black'
                ))

    dwg.save()
    print(f"Pattern saved as {filename}")

# Example usage
generate_square_pattern_svg(
    filename='squares_pattern.svg',
    grid_rows=31,
    grid_cols=14,
    density=0.7,
    min_size=8,
    max_size=43
)