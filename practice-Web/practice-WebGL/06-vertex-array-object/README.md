# 06 - Vertex Array Object

## Goal

Encapsulate all vertex attribute state (VBO bindings, attrib pointers, EBO binding) inside a VAO so that the per-frame draw call is reduced to a single `bindVertexArray` + `drawElements`.

## Key API Calls / Concepts

| Item | Description |
|---|---|
| `gl.createVertexArray()` | Allocates a new VAO object on the GPU |
| `gl.bindVertexArray(vao)` | Makes vao the active VAO; all subsequent attrib/buffer calls are recorded into it |
| `gl.bindVertexArray(null)` | Unbinds the VAO to prevent accidental state changes after setup |
| `gl.vertexAttribPointer` | Describes layout of one attribute inside the currently bound VBO — recorded into the active VAO |
| `gl.drawElements` | Issues an indexed draw call using the EBO recorded in the VAO |

## Implementation Steps

1. Create and compile vertex + fragment shaders; link program.
2. Create VAO with `gl.createVertexArray()`; bind it; inside the binding create the VBO (position + color interleaved) and EBO, then call `gl.vertexAttribPointer` + `gl.enableVertexAttribArray` for each attribute; unbind the VAO.
3. Per-frame: clear the canvas, call `gl.useProgram`, `gl.bindVertexArray(vao)`, `gl.drawElements`, then `gl.bindVertexArray(null)`.

## Expected Output

A colored quad (two triangles forming a rectangle) identical to module 05, but the draw loop contains no buffer rebinding — all attribute state lives in the VAO.
