'use strict';

// ─── Context ───────────────────────────────────────────────────────────────
const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// ─── Shaders ──────────────────────────────────────────────────────────────
// Two vertex attributes: a_position (vec2) and a_color (vec3)
// Color is passed through to the fragment shader via a varying
const VS = `#version 300 es
in vec2 a_position;
in vec3 a_color;
out vec3 v_color;
void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
    v_color = a_color;
}`;

const FS = `#version 300 es
precision mediump float;
in  vec3 v_color;
out vec4 fragColor;
void main() {
    fragColor = vec4(v_color, 1.0);
}`;

const prog = createProgram(gl, VS, FS);

// ─── Interleaved Vertex Buffer ─────────────────────────────────────────────
// Layout: [x, y, r, g, b] per vertex — all floats, stride = 5 × 4 = 20 bytes
//
//  Vertex 0 ─────────────────── offset 0
//  Vertex 1 ─────────────────── offset 20
//  Vertex 2 ─────────────────── offset 40
//
//  a_position: size=2, offset=0   (bytes 0..7 of each stride block)
//  a_color   : size=3, offset=8   (bytes 8..19 of each stride block)

const STRIDE = 5 * Float32Array.BYTES_PER_ELEMENT; // 20

const vertices = new Float32Array([
//    x      y      r     g     b
    0.0,   0.65,  1.0,  0.2,  0.2,   // top    — red
   -0.65, -0.55,  0.2,  1.0,  0.2,   // left   — green
    0.65, -0.55,  0.2,  0.2,  1.0,   // right  — blue
]);

// ─── VAO Setup ────────────────────────────────────────────────────────────
const vao = gl.createVertexArray();
const vbo = gl.createBuffer();

gl.bindVertexArray(vao);
gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

const posLoc = gl.getAttribLocation(prog, 'a_position');
gl.enableVertexAttribArray(posLoc);
// vertexAttribPointer(index, size, type, normalized, stride, offset)
gl.vertexAttribPointer(posLoc, 2, gl.FLOAT, false, STRIDE, 0);

const colLoc = gl.getAttribLocation(prog, 'a_color');
gl.enableVertexAttribArray(colLoc);
gl.vertexAttribPointer(colLoc, 3, gl.FLOAT, false, STRIDE, 2 * Float32Array.BYTES_PER_ELEMENT);

gl.bindVertexArray(null);

// ─── Render Loop ───────────────────────────────────────────────────────────
function frame() {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(prog);
    gl.bindVertexArray(vao);
    gl.drawArrays(gl.TRIANGLES, 0, 3);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
