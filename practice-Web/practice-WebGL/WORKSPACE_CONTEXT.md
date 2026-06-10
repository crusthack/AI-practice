# WebGL Workspace Context

## Tech Stack

- **API**: WebGL2 (`WebGL2RenderingContext`)
- **Shading Language**: GLSL ES 3.00 (`#version 300 es`)
- **Host Language**: Vanilla JavaScript (ES2017+ syntax, no bundler)
- **Math Library**: `common/math.js` — thin Float32Array-based Vec3/Mat4
- **Utility Library**: `common/webgl-utils.js` — `createProgram`, `createBuffer`, `createTexture2D`, `getUniformLocations`
- **Build System**: None — open `index.html` directly or via local HTTP server

## Per-Module Conventions

Every module folder contains exactly:
```
index.html   — canvas, script tags (math.js → webgl-utils.js → main.js)
main.js      — all WebGL logic for that concept
README.md    — concept summary, key API calls, pitfalls
```
No transpilation, no bundler, no npm. Every sample runs in a browser with zero setup.

## WebGL2 Object Lifecycle Pattern

```
Create   → gl.createBuffer / createTexture / createFramebuffer / createVertexArray
Bind     → gl.bindBuffer / bindTexture / bindFramebuffer / bindVertexArray
Configure/Upload → gl.bufferData / texImage2D / framebufferTexture2D / vertexAttribPointer
Draw     → gl.drawArrays / drawElements / drawArraysInstanced
Delete   → gl.deleteBuffer / deleteTexture (on cleanup)
```

## GLSL ES 3.00 Cheatsheet

```glsl
#version 300 es
precision mediump float;      // required in fragment shader

// Vertex shader I/O
in  vec2 a_position;          // attribute (location qualifier optional in WebGL2)
out vec2 v_uv;                // to fragment shader

// Fragment shader I/O
in  vec2 v_uv;
out vec4 fragColor;           // explicit output (replaces gl_FragColor)

// Uniforms
uniform mat4 u_mvp;
uniform sampler2D u_tex;
uniform sampler2DArray u_texArray;
uniform samplerCube u_cube;
```

## Common Pitfalls

- `gl_FragColor` is not available in GLSL ES 3.00 — use `out vec4 fragColor`.
- Textures with non-power-of-two dimensions require `CLAMP_TO_EDGE` wrap mode and cannot use mipmaps with the default filter in WebGL1; in WebGL2 NPOT is fully supported.
- The element array buffer binding is **part of VAO state** — bind the EBO while the target VAO is bound.
- WebGL2 has no compute shader — use Transform Feedback or MRT to perform GPU-side computation.
- `Float32Array` is column-major matching GLSL `mat4` — pass directly to `gl.uniformMatrix4fv`.
- `gl.uniformMatrix4fv(loc, false, mat)` — the `false` (transpose) must always be `false` in WebGL.
