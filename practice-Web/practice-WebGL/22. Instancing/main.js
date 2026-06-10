'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement hardware instancing with vertexAttribDivisor
// Without instancing: N objects → N draw calls → N state changes → slow.
// With instancing: N objects → 1 draw call → only per-instance data changes.
//
// Key steps:
//   1. Create a per-instance buffer containing offset (vec2) + color (vec3)
//      for each instance. Fill with a grid of N×N instances.
//   2. Set gl.vertexAttribDivisor(loc, 1) on the instance attributes.
//      Divisor=0 means "advance every vertex" (normal attributes).
//      Divisor=1 means "advance every INSTANCE" (instance attributes).
//   3. Call gl.drawArraysInstanced(TRIANGLES, 0, vertexCount, instanceCount).
//      The GL reuses the same base geometry for every instance, substituting
//      the next row of the instance buffer for each instance.

// ----- Shaders -----
const VS = `#version 300 es
// Per-vertex attributes (divisor = 0, advance every vertex)
layout(location=0) in vec2 a_pos;
// Per-instance attributes (divisor = 1, advance every instance)
layout(location=1) in vec2 a_offset;  // grid position offset
layout(location=2) in vec3 a_color;   // unique color per instance

uniform mat4  u_vp;          // view-projection matrix
uniform float u_time;
out vec3 v_color;

void main() {
    // Animate: each instance bobs up and down slightly, phased by offset.x
    float bob = sin(u_time * 2.0 + a_offset.x * 0.5) * 0.05;
    vec2 worldPos = a_pos * 0.3 + a_offset + vec2(0.0, bob);
    v_color = a_color;
    gl_Position = u_vp * vec4(worldPos, 0.0, 1.0);
}`;

const FS = `#version 300 es
precision mediump float;
in  vec3 v_color;
out vec4 outColor;
void main() { outColor = vec4(v_color, 1.0); }`;

function compileShader(type, src) {
    const s = gl.createShader(type);
    gl.shaderSource(s, src); gl.compileShader(s);
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(s));
    return s;
}
const prog = gl.createProgram();
gl.attachShader(prog, compileShader(gl.VERTEX_SHADER, VS));
gl.attachShader(prog, compileShader(gl.FRAGMENT_SHADER, FS));
gl.linkProgram(prog);
const uVP   = gl.getUniformLocation(prog, 'u_vp');
const uTime = gl.getUniformLocation(prog, 'u_time');

// ----- Base triangle geometry (per-vertex, shared by all instances) -----
const triVerts = new Float32Array([
     0.0,  0.5,
    -0.5, -0.5,
     0.5, -0.5,
]);
const vao = gl.createVertexArray();
gl.bindVertexArray(vao);

// Attribute 0: base geometry — divisor stays 0 (default)
const vbo = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
gl.bufferData(gl.ARRAY_BUFFER, triVerts, gl.STATIC_DRAW);
gl.enableVertexAttribArray(0);
gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0);

// TODO Step 1: Build per-instance data — offset (vec2) + color (vec3) = 5 floats
const GRID    = 30;          // 30×30 = 900 instances
const SPACING = 0.7;
const instances = new Float32Array(GRID * GRID * 5);
let idx = 0;
for (let row = 0; row < GRID; row++) {
    for (let col = 0; col < GRID; col++) {
        const ox = (col - GRID/2 + 0.5) * SPACING;
        const oy = (row - GRID/2 + 0.5) * SPACING;
        // Color: hue varies across the grid using HSL-like math
        const h  = (row + col) / (GRID * 2);
        const r  = 0.5 + 0.5 * Math.sin(h * Math.PI * 2);
        const g  = 0.5 + 0.5 * Math.sin(h * Math.PI * 2 + 2.09);
        const b  = 0.5 + 0.5 * Math.sin(h * Math.PI * 2 + 4.19);
        instances[idx++] = ox; instances[idx++] = oy;
        instances[idx++] = r;  instances[idx++] = g; instances[idx++] = b;
    }
}
const INSTANCE_COUNT = GRID * GRID;

// TODO Step 2: Upload instance buffer and set vertex attrib divisors
const instVBO = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, instVBO);
gl.bufferData(gl.ARRAY_BUFFER, instances, gl.STATIC_DRAW);

const INST_STRIDE = 5 * 4;  // 5 floats per instance

// Attribute 1: offset (vec2), starts at byte 0 of each instance
gl.enableVertexAttribArray(1);
gl.vertexAttribPointer(1, 2, gl.FLOAT, false, INST_STRIDE, 0);
// divisor=1 → this attribute advances once per INSTANCE, not per vertex
gl.vertexAttribDivisor(1, 1);

// Attribute 2: color (vec3), starts at byte 8 (after the 2-float offset)
gl.enableVertexAttribArray(2);
gl.vertexAttribPointer(2, 3, gl.FLOAT, false, INST_STRIDE, 8);
gl.vertexAttribDivisor(2, 1);

// ----- Orthographic projection -----
function ortho(l, r, b, t, n, f) {
    return new Float32Array([
        2/(r-l), 0, 0, 0,
        0, 2/(t-b), 0, 0,
        0, 0, -2/(f-n), 0,
        -(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1
    ]);
}
const half = (GRID * SPACING) / 2 + 0.5;
const vp   = ortho(-half, half, -half, half, -1, 1);

function frame(now) {
    const t = now * 0.001;
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(prog);
    gl.uniformMatrix4fv(uVP, false, vp);
    gl.uniform1f(uTime, t);

    gl.bindVertexArray(vao);

    // TODO Step 3: Single draw call renders all 900 instances
    // The base triangle (3 verts) is drawn once per instance.
    // GL reads per-instance data from instVBO, advancing by INST_STRIDE
    // each time the instance index increments.
    gl.drawArraysInstanced(gl.TRIANGLES, 0, 3, INSTANCE_COUNT);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
