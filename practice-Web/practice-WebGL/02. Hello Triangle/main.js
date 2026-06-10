'use strict';

// ─── Context ───────────────────────────────────────────────────────────────
const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// ─── Shaders ───────────────────────────────────────────────────────────────
// The vertex positions are hardcoded inside the shader using gl_VertexID.
// No VBO is needed — this isolates the "first visible triangle" from buffer API concerns.
const VS = `#version 300 es
void main() {
    // Three positions in clip space, indexed by gl_VertexID (0, 1, 2)
    vec2 pos[3];
    pos[0] = vec2( 0.0,  0.6);
    pos[1] = vec2(-0.6, -0.5);
    pos[2] = vec2( 0.6, -0.5);
    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
}`;

const FS = `#version 300 es
precision mediump float;
out vec4 fragColor;   // explicit output — gl_FragColor does not exist in GLSL ES 3.00
void main() {
    fragColor = vec4(1.0, 0.5, 0.1, 1.0);  // solid orange
}`;

// createProgram is provided by common/webgl-utils.js
const prog = createProgram(gl, VS, FS);

// ─── Render Loop ───────────────────────────────────────────────────────────
function frame() {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.08, 0.08, 0.12, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(prog);
    // drawArrays with count=3 triggers the vertex shader 3 times (gl_VertexID = 0,1,2)
    gl.drawArrays(gl.TRIANGLES, 0, 3);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
