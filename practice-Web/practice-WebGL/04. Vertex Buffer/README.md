# 04 - Vertex Buffer

## Goal

Upload interleaved vertex data (position + color) into a VBO, describe the layout using `vertexAttribPointer`, and render a color-interpolated triangle.

## Key API Calls

| Call | Purpose |
|---|---|
| `gl.createBuffer()` | Allocate a GPU buffer object |
| `gl.bindBuffer(ARRAY_BUFFER, vbo)` | Make the VBO the active array buffer |
| `gl.bufferData(ARRAY_BUFFER, data, STATIC_DRAW)` | Upload the `Float32Array` to the GPU |
| `gl.getAttribLocation(prog, name)` | Query an attribute's slot index |
| `gl.enableVertexAttribArray(loc)` | Enable reading from the bound buffer for this slot |
| `gl.vertexAttribPointer(loc, size, type, norm, stride, offset)` | Describe the memory layout |

## Interleaved vs. Separate Buffers

**Interleaved** (this sample): all attributes in one buffer, using `stride` and `offset`.

```
Buffer: [ x y r g b | x y r g b | x y r g b ]
                ↑                    ↑
          stride = 20 bytes    vertexAttribPointer calls:
          a_position: size=2, offset=0
          a_color:    size=3, offset=8
```

**Separate**: one VBO per attribute — simpler to update individual streams.

## Varying Interpolation

The GPU automatically interpolates `v_color` (declared as `out vec3` in VS and `in vec3` in FS) across the triangle's surface. The result is the classic RGB gradient triangle.

## Expected Output

A triangle with red at the top vertex, green at the bottom-left, and blue at the bottom-right, smoothly interpolated across the surface.
