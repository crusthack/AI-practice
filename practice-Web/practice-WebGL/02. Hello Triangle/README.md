# 02 - Hello Triangle

## Goal

Draw the first triangle. Skip buffer management entirely by embedding positions in the vertex shader using `gl_VertexID`, so the focus stays on the shader/program lifecycle.

## Key API Calls

| Call | Purpose |
|---|---|
| `gl.createShader(type)` | Allocate a shader object (via `createProgram` helper) |
| `gl.shaderSource(shader, src)` | Upload GLSL ES 3.00 source |
| `gl.compileShader(shader)` | Compile to GPU IR |
| `gl.createProgram()` | Allocate a program object |
| `gl.attachShader / linkProgram` | Combine VS + FS into a drawable program |
| `gl.useProgram(prog)` | Bind the program for subsequent draw calls |
| `gl.drawArrays(gl.TRIANGLES, 0, 3)` | Submit 3 vertices → 1 triangle |

## GLSL ES 3.00 Required Changes

| OpenGL GLSL | GLSL ES 3.00 |
|---|---|
| `varying` | `in` / `out` |
| `gl_FragColor` | `out vec4 fragColor;` (any name) |
| `attribute` | `in` |
| `texture2D()` | `texture()` |

## Expected Output

A solid orange triangle centered in the canvas on a dark background.
