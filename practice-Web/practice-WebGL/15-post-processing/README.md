# 15 - Post Processing

## Goal

Apply switchable screen-space shader effects (grayscale, invert, vignette) in a second fullscreen pass over the FBO output from module 14, using a single `uniform int u_effect` to select the active effect without changing the program.

## Key API Calls / Concepts

| Item | Description |
|---|---|
| FBO pipeline (same as module 14) | Pass 1 renders scene to texture; pass 2 processes that texture |
| `uniform int u_effect` | Integer uniform selects which branch of the post-process shader runs |
| `gl.uniform1i(loc, value)` | Uploads an integer uniform — used for both `u_effect` and sampler units |
| Grayscale (luma) | `dot(rgb, vec3(0.299, 0.587, 0.114))` — perceptual luminance weights for BT.601 |
| Vignette | Darken by `1 - dot(uv - 0.5, uv - 0.5) * k` where `uv` is centered at origin |

## Implementation Steps

1. Reuse the FBO setup from module 14; render the scene into `fboTex` each frame in pass 1.
2. Write the post-process fragment shader with an `if/else` block on `u_effect` implementing all four effects (none, grayscale, invert, vignette) in one shader.
3. Listen for number keys `1`-`4` to update `effectIndex`; upload it via `gl.uniform1i(loc_effect, effectIndex)` before the fullscreen draw call in pass 2.

## Expected Output

A 3D scene (spinning cube) displayed with a selectable visual effect: key `1` = normal, key `2` = black-and-white grayscale, key `3` = inverted colors, key `4` = darkened vignette corners.
