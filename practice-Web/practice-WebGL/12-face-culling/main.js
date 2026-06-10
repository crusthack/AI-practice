'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement face culling to skip rendering back-facing triangles
// The GPU determines "front" vs "back" by the winding order of vertices in screen space:
//   CCW (counter-clockwise) = front face  (WebGL default via gl.frontFace(gl.CCW))
//   CW  (clockwise)         = back face   → culled when gl.cullFace(gl.BACK)
//
// Key steps:
//   1. Place TWO quads side by side:
//        Left quad  — vertices in CCW order (front-facing, should be visible)
//        Right quad — vertices in CW  order (back-facing, should be culled)
//      Keep the rest of the scene (a spinning cube) to show full-object culling.
//   2. Enable culling:
//        gl.enable(gl.CULL_FACE);
//        gl.cullFace(gl.BACK);    // cull back faces
//        gl.frontFace(gl.CCW);    // CCW winding = front (default, explicit is clearer)
//   3. Key bindings:
//        'C' — toggle gl.enable / gl.disable (gl.CULL_FACE)
//        'F' — toggle gl.frontFace between gl.CCW and gl.CW (flips which face is "front")

let cullingEnabled = true;
let frontFaceCCW   = true;

document.addEventListener('keydown', e => {
    if (e.key === 'c' || e.key === 'C') {
        cullingEnabled = !cullingEnabled;
    }
    if (e.key === 'f' || e.key === 'F') {
        frontFaceCCW = !frontFaceCCW;
    }
    document.getElementById('info').textContent =
        `12 - Face Culling | C: culling ${cullingEnabled ? 'ON' : 'OFF'} | F: front=${frontFaceCCW ? 'CCW' : 'CW'}`;
});

// --- Shaders ---
const vsSource = `#version 300 es
layout(location = 0) in vec3 a_position;
uniform mat4 u_mvp;
uniform vec4 u_color;
out vec4 v_color;
void main() {
    v_color = u_color;
    gl_Position = u_mvp * vec4(a_position, 1.0);
}`;

const fsSource = `#version 300 es
precision mediump float;
in vec4 v_color;
out vec4 fragColor;
void main() { fragColor = v_color; }`;

// TODO: 1. Compile program, cache loc_mvp and loc_color.

// TODO: 2. Create two VAOs:
//   vao_ccw: quad with vertices listed counter-clockwise in NDC
//     e.g. BL(-0.4,-0.4,0), BR(0.4,-0.4,0), TR(0.4,0.4,0), TL(-0.4,0.4,0)
//     indices [0,1,2, 2,3,0] — CCW in screen space
//   vao_cw: same geometry but indices [0,3,2, 2,1,0] — CW in screen space
//
//   Also create a cube VAO (from module 08) to demonstrate per-face culling.

// TODO: 3. Build projection + view matrices.

function frame(now) {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);

    // TODO: apply culling state from keyboard toggles
    if (cullingEnabled) {
        gl.enable(gl.CULL_FACE);
        gl.cullFace(gl.BACK);
        gl.frontFace(frontFaceCCW ? gl.CCW : gl.CW);
    } else {
        gl.disable(gl.CULL_FACE);
    }

    // TODO: draw CCW quad (green — always visible when culling is ON with CCW front face)
    // TODO: draw CW  quad (red   — culled when culling is ON with CCW front face)
    // TODO: draw spinning cube (back faces of cube disappear with culling ON)

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
