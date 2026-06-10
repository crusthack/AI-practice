# WebGL2 Quick Reference

## Context Setup

```js
const gl = canvas.getContext('webgl2');
// Check support
if (!gl) throw new Error('WebGL2 not supported');
// Query limits
gl.getParameter(gl.MAX_TEXTURE_SIZE);       // usually 16384
gl.getParameter(gl.MAX_VERTEX_ATTRIBS);     // usually 16
gl.getParameter(gl.MAX_TEXTURE_IMAGE_UNITS);// usually 16
```

## Shader Compilation

```js
function createProgram(gl, vsSrc, fsSrc) {
    const vs = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vs, vsSrc);
    gl.compileShader(vs);
    // ... same for fs ...
    const prog = gl.createProgram();
    gl.attachShader(prog, vs); gl.attachShader(prog, fs);
    gl.linkProgram(prog);
    gl.deleteShader(vs); gl.deleteShader(fs);
    return prog;
}
```

## GLSL ES 3.00 Skeleton

**Vertex Shader:**
```glsl
#version 300 es
in  vec3  a_position;
in  vec2  a_uv;
out vec2  v_uv;
uniform mat4 u_mvp;
void main() {
    gl_Position = u_mvp * vec4(a_position, 1.0);
    v_uv = a_uv;
}
```

**Fragment Shader:**
```glsl
#version 300 es
precision mediump float;
in  vec2      v_uv;
out vec4      fragColor;          // NOT gl_FragColor
uniform sampler2D u_tex;
void main() {
    fragColor = texture(u_tex, v_uv);
}
```

## Buffer Objects

```js
// VBO
const vbo = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([...]), gl.STATIC_DRAW);

// EBO (must be bound while VAO is bound)
const ebo = gl.createBuffer();
gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ebo);
gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array([...]), gl.STATIC_DRAW);

// Partial update (dynamic geometry)
gl.bufferSubData(gl.ARRAY_BUFFER, byteOffset, data);
```

## Vertex Array Object

```js
const vao = gl.createVertexArray();
gl.bindVertexArray(vao);
    // all vertexAttribPointer + enableVertexAttribArray calls here
    // EBO bind here too
gl.bindVertexArray(null);

// Per frame:
gl.bindVertexArray(vao);
gl.drawArrays(gl.TRIANGLES, 0, count);
// or
gl.drawElements(gl.TRIANGLES, indexCount, gl.UNSIGNED_SHORT, 0);
```

## Uniforms

```js
const loc = gl.getUniformLocation(prog, 'u_mvp');
gl.useProgram(prog);                          // must be bound first
gl.uniformMatrix4fv(loc, false, mat4Array);   // false = do NOT transpose
gl.uniform1f(loc, 1.5);
gl.uniform2f(loc, x, y);
gl.uniform3fv(loc, new Float32Array([r,g,b]));
gl.uniform4f(loc, r, g, b, a);
gl.uniform1i(loc, textureUnit);               // sampler2D
```

## Textures

```js
const tex = gl.createTexture();
gl.activeTexture(gl.TEXTURE0);               // select texture unit
gl.bindTexture(gl.TEXTURE_2D, tex);
gl.texImage2D(gl.TEXTURE_2D, 0,              // mip level 0
    gl.RGBA8, width, height, 0,
    gl.RGBA, gl.UNSIGNED_BYTE, pixelData);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
// In shader: uniform sampler2D u_tex;  → gl.uniform1i(loc, 0);
```

## Framebuffer Object

```js
const fbo = gl.createFramebuffer();
gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
// Attach a color texture
gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, colorTex, 0);
// Attach a renderbuffer for depth
const rbo = gl.createRenderbuffer();
gl.bindRenderbuffer(gl.RENDERBUFFER, rbo);
gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, w, h);
gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, rbo);
// Check completeness
if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) !== gl.FRAMEBUFFER_COMPLETE)
    throw new Error('Incomplete framebuffer');
gl.bindFramebuffer(gl.FRAMEBUFFER, null); // restore default
```

## Depth & Blending State

```js
// Depth test
gl.enable(gl.DEPTH_TEST);
gl.depthFunc(gl.LESS);
gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

// Blending (standard alpha)
gl.enable(gl.BLEND);
gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);

// Face culling
gl.enable(gl.CULL_FACE);
gl.cullFace(gl.BACK);
gl.frontFace(gl.CCW);
```

## Instancing

```js
// Per-instance buffer
gl.vertexAttribDivisor(instanceLoc, 1);  // advance once per instance (not per vertex)
gl.drawArraysInstanced(gl.TRIANGLES, 0, vertexCount, instanceCount);
gl.drawElementsInstanced(gl.TRIANGLES, indexCount, gl.UNSIGNED_SHORT, 0, instanceCount);
```

## Transform Feedback

```js
// Before link
gl.transformFeedbackVaryings(prog, ['v_position', 'v_velocity'], gl.INTERLEAVED_ATTRIBS);
// Then link program

const tf = gl.createTransformFeedback();
gl.bindTransformFeedback(gl.TRANSFORM_FEEDBACK, tf);
gl.bindBufferBase(gl.TRANSFORM_FEEDBACK_BUFFER, 0, outputBuf);

gl.enable(gl.RASTERIZER_DISCARD);        // update-only pass, skip rasterization
gl.beginTransformFeedback(gl.POINTS);
gl.drawArrays(gl.POINTS, 0, count);
gl.endTransformFeedback();
gl.disable(gl.RASTERIZER_DISCARD);
```

## UBO (Uniform Buffer Object)

```glsl
// GLSL
layout(std140) uniform FrameData {
    mat4 u_view;
    mat4 u_proj;
    float u_time;
};
```

```js
const ubo = gl.createBuffer();
gl.bindBuffer(gl.UNIFORM_BUFFER, ubo);
gl.bufferData(gl.UNIFORM_BUFFER, byteSize, gl.DYNAMIC_DRAW);

const blockIdx = gl.getUniformBlockIndex(prog, 'FrameData');
gl.uniformBlockBinding(prog, blockIdx, 0);          // bind point 0
gl.bindBufferBase(gl.UNIFORM_BUFFER, 0, ubo);       // connect buffer to point 0

// Update per frame
gl.bindBuffer(gl.UNIFORM_BUFFER, ubo);
gl.bufferSubData(gl.UNIFORM_BUFFER, 0, frameDataFloat32Array);
```

## Common Pitfalls

| Pitfall | Fix |
|---|---|
| Blank screen | Check `gl.getError()`, verify viewport size, check shader compile |
| `gl_FragColor` undefined | Use `out vec4 fragColor;` in GLSL ES 3.00 |
| Texture shows as black | Check min filter; NPOT needs CLAMP_TO_EDGE + LINEAR |
| EBO ignored | Bind EBO *while* VAO is bound |
| `uniformMatrix4fv` no effect | `useProgram` must be called before uploading uniforms |
| Skybox shows through objects | Use `LEQUAL` depth func for skybox; write `gl_Position.z = gl_Position.w` |
| MRT no output | Call `gl.drawBuffers([...])` after binding FBO |
