# 05 - Index Buffer

## Goal

Draw a quad (2 triangles) using an element buffer object (EBO). Share 4 vertices instead of repeating them, and use `drawElements` instead of `drawArrays`.

## Key API Calls

| Call | Purpose |
|---|---|
| `gl.bindBuffer(ELEMENT_ARRAY_BUFFER, ebo)` | Bind the index buffer (stored in VAO state) |
| `gl.bufferData(ELEMENT_ARRAY_BUFFER, indices, STATIC_DRAW)` | Upload `Uint16Array` of indices |
| `gl.drawElements(TRIANGLES, count, UNSIGNED_SHORT, 0)` | Draw using the index buffer |

## Index Types

| Type constant | JS type | Max index |
|---|---|---|
| `gl.UNSIGNED_BYTE` | `Uint8Array` | 255 |
| `gl.UNSIGNED_SHORT` | `Uint16Array` | 65 535 |
| `gl.UNSIGNED_INT` | `Uint32Array` | 4 294 967 295 |

## Vertex Reuse

A quad without an EBO requires 6 vertices (duplicating 2 corner positions).  
With an EBO: 4 unique vertices + 6 indices = less GPU memory and vertex cache hits.

```
Without EBO: TL, BL, BR,  TL, BR, TR   (6 vertices, 2 duplicates)
With    EBO: TL, TR, BR, BL  + [0,3,2, 0,2,1]   (4 vertices + 6 indices)
```

## VAO + EBO Binding Rule

The `ELEMENT_ARRAY_BUFFER` binding is part of the VAO's state. **Always bind the EBO while the target VAO is active** so the association is captured:

```js
gl.bindVertexArray(vao);
gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ebo);   // ← stored in vao
gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, ...);
```

## Expected Output

A large quad filling most of the canvas, showing an animated checkerboard pattern that pulses between orange and blue.
