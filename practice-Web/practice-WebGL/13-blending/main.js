'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement alpha blending for transparent overlapping quads
// Standard alpha blending formula (over compositing):
//   result = src.rgb * src.a  +  dst.rgb * (1 - src.a)
// The GPU applies this when: gl.enable(gl.BLEND) + gl.blendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA)
//
// CRITICAL SORT ORDER RULE: opaque objects first, then transparent back-to-front.
// Depth writes from a transparent quad would block farther transparent quads.
// Solutions:
//   (a) Disable depth writes for transparent objects: gl.depthMask(false) while drawing them.
//   (b) Sort transparent objects by distance to camera, farthest first.
//
// Key steps:
//   1. Enable blending with standard over-compositing blend function.
//      Keep depth test ON but disable depth writes for transparent objects.
//   2. Draw the opaque background quad first (alpha = 1.0, depth mask ON).
//   3. Sort transparent quads by Z (farthest first), enable depthMask(false),
//      draw them back-to-front, then restore depthMask(true).

let blendEnabled  = true;
let sortEnabled   = true;  // front-to-back vs back-to-front

document.addEventListener('keydown', e => {
    if (e.key === 'b' || e.key === 'B') blendEnabled = !blendEnabled;
    if (e.key === 's' || e.key === 'S') sortEnabled  = !sortEnabled;
    document.getElementById('info').textContent =
        `13 - Blending | B: blend ${blendEnabled ? 'ON' : 'OFF'} | S: sort ${sortEnabled ? 'back-to-front' : 'UNSORTED'}`;
});

// --- Shaders ---
const vsSource = `#version 300 es
layout(location = 0) in vec2 a_position;
uniform mat4 u_mvp;
out vec2 v_pos;
void main() {
    v_pos = a_position;
    gl_Position = u_mvp * vec4(a_position, 0.0, 1.0);
}`;

const fsSource = `#version 300 es
precision mediump float;
uniform vec4 u_color; // rgba — alpha < 1 = transparent
out vec4 fragColor;
void main() { fragColor = u_color; }`;

// Scene: one opaque background + three translucent overlapping quads
// Each quad has [x_offset, z_depth, color with alpha]
const quads = [
    // opaque background (drawn first, always)
    { x: 0.0, z: -1.0, color: [0.15, 0.15, 0.20, 1.00], size: 1.8, opaque: true },
    // transparent quads (draw back-to-front)
    { x: -0.3, z:  0.0, color: [1.0, 0.2, 0.2, 0.6], size: 0.7, opaque: false },
    { x:  0.1, z:  0.2, color: [0.2, 1.0, 0.2, 0.6], size: 0.7, opaque: false },
    { x:  0.4, z:  0.4, color: [0.2, 0.2, 1.0, 0.6], size: 0.7, opaque: false },
];

// TODO: 1. Compile shaders, link program, cache loc_mvp and loc_color.

// TODO: 2. Create a reusable unit quad VAO (centered at origin, size 1x1).

// TODO: 3. Build projection + view matrices.

function frame(now) {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);

    if (blendEnabled) {
        gl.enable(gl.BLEND);
        gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
    } else {
        gl.disable(gl.BLEND);
    }

    // TODO: draw opaque quads first (depth write ON)
    // gl.depthMask(true);
    // ... draw quads where opaque === true

    // TODO: sort transparent quads
    // const transparent = quads.filter(q => !q.opaque);
    // if (sortEnabled) transparent.sort((a, b) => a.z - b.z); // farthest (lowest z) first

    // TODO: draw transparent quads (disable depth writes to avoid blocking each other)
    // gl.depthMask(false);
    // ... draw each transparent quad
    // gl.depthMask(true); // restore

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
