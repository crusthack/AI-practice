# 07 - Uniforms And Time

## Goal

Animate geometry by passing a `u_time` uniform each frame, and demonstrate multiple uniform types (`float`, `vec2`, `vec4`) to drive position offset and color modulation.

## Key API Calls / Concepts

| Item | Description |
|---|---|
| `gl.getUniformLocation(program, name)` | Returns an opaque location handle; call once after linking and cache the result |
| `gl.uniform1f(loc, value)` | Uploads a single float uniform (used for u_time) |
| `gl.uniform2f(loc, x, y)` | Uploads a vec2 uniform (used for u_offset) |
| `gl.uniform4fv(loc, Float32Array)` | Uploads a vec4 uniform from a JS array (used for u_color) |
| `requestAnimationFrame(frame)` | Schedules the next frame; `now` argument is DOMHighResTimeStamp in ms |

## Implementation Steps

1. Declare `uniform float u_time`, `uniform vec2 u_offset`, and `uniform vec4 u_color` in the GLSL shaders; use them to animate position and tint the output color.
2. After `gl.linkProgram`, call `gl.getUniformLocation` for each uniform and store the handles in JS variables — never query locations inside the render loop.
3. Each frame convert `now` to seconds, compute animated values (sin/cos), and upload them with `gl.uniform*` before calling `gl.drawArrays`.

## Expected Output

A triangle that oscillates horizontally and bobs vertically, with its color pulsing between hues — all driven by elapsed time on the CPU and read by the GPU shaders.
