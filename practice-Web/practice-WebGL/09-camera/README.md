# 09 - Camera

## Goal

Implement a mouse-driven orbit camera that lets the user drag to rotate around a 3D scene by converting spherical coordinates (azimuth, elevation, radius) into a `lookAt` view matrix every frame.

## Key API Calls / Concepts

| Item | Description |
|---|---|
| `canvas.addEventListener('mousedown/mousemove/mouseup')` | Track drag state and compute delta pixels each move event |
| Spherical-to-Cartesian | `eye = [r·cos(el)·sin(az), r·sin(el), r·cos(el)·cos(az)]` — converts orbit angles to world-space eye position |
| `Mat4.lookAt(eye, center, up)` | Rebuilds the view matrix from the new eye position each frame |
| Elevation clamping | Clamp to `[-π/2+ε, π/2-ε]` to avoid the camera flipping at the poles |
| `gl.enable(gl.DEPTH_TEST)` | Required for correct 3D rendering; clear `DEPTH_BUFFER_BIT` each frame |

## Implementation Steps

1. Set up mouse event listeners on the canvas; track `isDragging`, `lastX`, `lastY`, and update `azimuth` and `elevation` from pixel deltas scaled by a sensitivity constant.
2. Each frame compute the Cartesian eye position from the spherical orbit parameters.
3. Rebuild `view = Mat4.lookAt(eye, origin, up)`, multiply `mvp = proj * view`, upload and draw the static scene.

## Expected Output

A 3D object (cube or grid) that can be freely orbited by clicking and dragging the mouse — panning left/right changes azimuth, dragging up/down changes elevation.
