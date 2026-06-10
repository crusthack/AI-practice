# 12 - Face Culling

## Goal

Show how vertex winding order determines front vs back face, and how `CULL_FACE` discards back-facing triangles — demonstrated with two quads (one CCW, one CW) and an interactive toggle.

## Key API Calls / Concepts

| Item | Description |
|---|---|
| `gl.enable(gl.CULL_FACE)` | Activates face culling — invisible faces are skipped before the fragment shader runs |
| `gl.cullFace(gl.BACK)` | Tells the GPU which face to discard (`BACK`, `FRONT`, or `FRONT_AND_BACK`) |
| `gl.frontFace(gl.CCW)` | Declares that counter-clockwise screen-space winding is the "front" face (WebGL default) |
| CCW vs CW winding | The sign of the signed area of the screen-space triangle determines winding order |
| Performance benefit | Culling typically discards ~50% of cube faces, halving fragment shader invocations |

## Implementation Steps

1. Create two quads with the same geometry but opposite index ordering (CCW and CW); assign them distinct colors (green = CCW, red = CW).
2. Enable `CULL_FACE`, set `cullFace(BACK)` and `frontFace(CCW)`; the CW quad should disappear while CCW remains visible.
3. Add key listeners: `C` toggles culling on/off (both quads visible vs one hidden), `F` flips `frontFace` between CCW and CW (swaps which quad is culled).

## Expected Output

Two quads where the green (CCW) one is visible and the red (CW) one is culled when back-face culling is active; pressing `C` makes both visible, pressing `F` inverts which one disappears.
