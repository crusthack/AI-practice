# 01 - WebGL Context

## Goal

Initialize a WebGL2 rendering context from an HTML canvas element, animate the clear color, and build a `requestAnimationFrame` loop with a live FPS counter.

## Key API Calls

| Call | Purpose |
|---|---|
| `canvas.getContext('webgl2')` | Obtain a `WebGL2RenderingContext`; returns `null` if unsupported |
| `WEBGL_debug_renderer_info` | Extension to query the actual GPU name |
| `gl.viewport(0, 0, w, h)` | Set the pixel region that NDC maps to |
| `gl.clearColor(r, g, b, a)` | Set the value written by `gl.clear` |
| `gl.clear(gl.COLOR_BUFFER_BIT)` | Fill the framebuffer with the clear color |
| `requestAnimationFrame(cb)` | Schedule the next frame, passing a DOMHighResTimeStamp |

## Concepts

- **Canvas backing-store vs CSS size**: the canvas element has two independent sizes — the CSS display size (set in CSS) and the backing-store pixel resolution (`canvas.width / canvas.height`). Multiplying by `devicePixelRatio` prevents blurring on high-DPI screens.
- **requestAnimationFrame**: the browser calls the callback once per display refresh (~16.7 ms at 60 Hz). The callback receives a monotonically increasing timestamp in milliseconds.
- **FPS counter**: `FPSCounter` in `common/webgl-utils.js` uses a rolling window of 60 timestamps to compute a smooth average.

## Expected Output

A 800×600 canvas whose background cycles smoothly through all hues, with an FPS/size readout underneath.
