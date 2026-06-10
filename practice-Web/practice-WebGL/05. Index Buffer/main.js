'use strict';

// ─── Context ───────────────────────────────────────────────────────────────
const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// ─── Shaders ──────────────────────────────────────────────────────────────
const VS = `#version 300 es
in vec2 a_position;
in vec2 a_uv;
out vec2 v_uv;
void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
    v_uv = a_uv;
}`;

const FS = `#version 300 es
precision mediump float;
uniform float u_time;
in  vec2 v_uv;
out vec4 fragColor;
void main() {
    // Animated checkerboard — demonstrates UV coordinates without a real texture
    float scl   = 6.0;
    float check = mod(floor(v_uv.x * scl) + floor(v_uv.y * scl), 2.0);
    float anim  = 0.5 + 0.5 * sin(u_time * 1.5);
    vec3  colA  = vec3(0.95, 0.35, 0.15);
    vec3  colB  = vec3(0.15, 0.35, 0.95);
    // Swap colors over time
    vec3  col   = mix(mix(colA, colB, check), mix(colB, colA, check), anim);
    fragColor   = vec4(col, 1.0);
}`;

const prog   = createProgram(gl, VS, FS);
const uTime  = gl.getUniformLocation(prog, 'u_time');

// ─── Geometry ─────────────────────────────────────────────────────────────
// A quad requires only 4 unique vertices (corners), not 6.
// The EBO tells the GPU which vertices to use for each of the 2 triangles.
//
//   0 ─────── 1
//   │  ╲      │
//   │    ╲    │
//   │      ╲  │
//   3 ─────── 2
//
// Triangle 1: 0 → 3 → 2
// Triangle 2: 0 → 2 → 1  (CCW winding, viewed from +Z)

const STRIDE = 4 * Float32Array.BYTES_PER_ELEMENT; // [x, y, u, v] × 4 bytes

const vertices = new Float32Array([
//    x      y     u    v
   -0.75,  0.75,  0.0, 1.0,   // 0 top-left
    0.75,  0.75,  1.0, 1.0,   // 1 top-right
    0.75, -0.75,  1.0, 0.0,   // 2 bottom-right
   -0.75, -0.75,  0.0, 0.0,   // 3 bottom-left
]);

const indices = new Uint16Array([
    0, 3, 2,   // triangle 1
    0, 2, 1,   // triangle 2
]);

// ─── VAO + VBO + EBO ──────────────────────────────────────────────────────
// IMPORTANT: the ELEMENT_ARRAY_BUFFER binding is stored inside the VAO.
// Always bind the EBO while the target VAO is bound.
const vao = gl.createVertexArray();
const vbo = gl.createBuffer();
const ebo = gl.createBuffer();

gl.bindVertexArray(vao);

gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

// Bind EBO inside the VAO — this association is remembered by the VAO
gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ebo);
gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);

const posLoc = gl.getAttribLocation(prog, 'a_position');
gl.enableVertexAttribArray(posLoc);
gl.vertexAttribPointer(posLoc, 2, gl.FLOAT, false, STRIDE, 0);

const uvLoc = gl.getAttribLocation(prog, 'a_uv');
gl.enableVertexAttribArray(uvLoc);
gl.vertexAttribPointer(uvLoc, 2, gl.FLOAT, false, STRIDE, 2 * Float32Array.BYTES_PER_ELEMENT);

gl.bindVertexArray(null);

// ─── Render Loop ───────────────────────────────────────────────────────────
function frame(now) {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(prog);
    gl.uniform1f(uTime, now * 0.001);

    gl.bindVertexArray(vao);
    // drawElements: uses index buffer to assemble triangles from shared vertices
    // (count=6 indices, UNSIGNED_SHORT type, offset 0 into the EBO)
    gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
