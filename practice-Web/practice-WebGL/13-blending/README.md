# 13 - Blending

## Goal

Implement standard over-compositing alpha blending for multiple semi-transparent quads, demonstrating why back-to-front sort order and disabling depth writes matters for correct transparency.

## Key API Calls / Concepts

| Item | Description |
|---|---|
| `gl.enable(gl.BLEND)` | Activates the blending stage; without this, alpha in the output color has no effect |
| `gl.blendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA)` | Standard "over" compositing: `result = src.rgb * src.a + dst.rgb * (1 - src.a)` |
| `gl.depthMask(false)` | Prevents transparent fragments from writing to the depth buffer, avoiding occlusion artifacts |
| Back-to-front sort | Transparent objects must be drawn farthest-first so the blending composites correctly |
| Draw order rule | Opaque objects first (depth mask on), then transparent back-to-front (depth mask off) |

## Implementation Steps

1. Enable `gl.BLEND` and set `blendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA)`; keep `DEPTH_TEST` on throughout.
2. Draw the opaque background quad with `gl.depthMask(true)`.
3. Sort the transparent quads by Z distance (farthest first), set `gl.depthMask(false)`, draw each quad with its color's alpha < 1, then restore `gl.depthMask(true)`.

## Expected Output

Three overlapping semi-transparent colored quads (red, green, blue with alpha 0.6) layered correctly over an opaque background; pressing `B` disables blending (hard edges), pressing `S` disables sorting (wrong color mixing order).
