# 11 - Depth Buffer

## Goal

Demonstrate depth testing by rendering multiple colored quads at different Z positions and showing that nearer quads correctly occlude farther ones regardless of draw order — with a toggleable comparison to see the painter-order artifacts when depth testing is disabled.

## Key API Calls / Concepts

| Item | Description |
|---|---|
| `gl.enable(gl.DEPTH_TEST)` | Enables per-fragment depth comparison against the depth buffer |
| `gl.depthFunc(gl.LESS)` | Sets the comparison function: pass the fragment only if its Z is less than (closer than) the stored value |
| `gl.clear(gl.DEPTH_BUFFER_BIT)` | Resets all depth values to 1.0 (far plane) at the start of each frame |
| `gl.disable(gl.DEPTH_TEST)` | Turns off depth testing — objects appear in draw order (useful for comparison) |
| Z translation in model matrix | Moving quads to different Z positions in world space to create depth differences |

## Implementation Steps

1. Enable `gl.DEPTH_TEST` with `depthFunc(gl.LESS)`; clear both `COLOR_BUFFER_BIT` and `DEPTH_BUFFER_BIT` each frame.
2. Create a reusable flat quad mesh; draw three instances at `z = -0.6, 0.0, +0.4` by changing the model matrix translation and uploading a per-object color uniform.
3. Add a keypress listener (key `D`) to toggle depth testing on/off so the occlusion difference is immediately visible.

## Expected Output

Three overlapping colored quads (red behind, green middle, blue front) that correctly layer in depth — pressing D disables depth testing and reveals the incorrect painter-order rendering.
