'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement an orbit camera controlled by mouse drag
// An orbit camera keeps a fixed look-at target (origin) and moves the eye
// around it on a sphere defined by (radius, azimuth, elevation).
//
// Key steps:
//   1. Track mouse state: on 'mousedown' set isDragging=true and store lastX/lastY.
//      On 'mousemove', if dragging, compute dx/dy deltas and update:
//        azimuth   += dx * sensitivity   (left/right orbit)
//        elevation += dy * sensitivity   (up/down orbit)
//      Clamp elevation to [-PI/2 + eps, PI/2 - eps] to avoid gimbal flip.
//      On 'mouseup' / 'mouseleave' set isDragging=false.
//   2. Convert spherical to Cartesian eye position each frame:
//        eye.x = radius * cos(elevation) * sin(azimuth)
//        eye.y = radius * sin(elevation)
//        eye.z = radius * cos(elevation) * cos(azimuth)
//   3. Rebuild view matrix: Mat4.lookAt(eye, [0,0,0], [0,1,0])
//      Combine with static projection for the MVP uniform.

// --- Orbit camera state ---
let azimuth   = 0.3;          // radians, horizontal angle
let elevation = 0.4;          // radians, vertical angle
let radius    = 4.0;
const sensitivity = 0.005;

let isDragging = false;
let lastX = 0, lastY = 0;

// TODO: attach mouse event listeners to canvas
// canvas.addEventListener('mousedown', e => { isDragging = true; lastX = e.clientX; lastY = e.clientY; });
// canvas.addEventListener('mousemove', e => {
//     if (!isDragging) return;
//     const dx = e.clientX - lastX;
//     const dy = e.clientY - lastY;
//     lastX = e.clientX; lastY = e.clientY;
//     azimuth   += dx * sensitivity;
//     elevation = Math.max(-Math.PI/2 + 0.01, Math.min(Math.PI/2 - 0.01, elevation - dy * sensitivity));
// });
// canvas.addEventListener('mouseup',    () => isDragging = false);
// canvas.addEventListener('mouseleave', () => isDragging = false);

// --- Shaders (same as module 08) ---
const vsSource = `#version 300 es
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;
uniform mat4 u_mvp;
out vec3 v_color;
void main() {
    v_color = a_color;
    gl_Position = u_mvp * vec4(a_position, 1.0);
}`;

const fsSource = `#version 300 es
precision mediump float;
in vec3 v_color;
out vec4 fragColor;
void main() { fragColor = vec4(v_color, 1.0); }`;

// TODO: compile program, cache loc_mvp, build cube VAO (same as module 08)

// TODO: build static projection once
// const proj = Mat4.perspective(Math.PI / 3, canvas.width / canvas.height, 0.1, 100.0);

function frame(now) {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);

    // TODO: compute eye from spherical coordinates
    // const eye = [
    //     radius * Math.cos(elevation) * Math.sin(azimuth),
    //     radius * Math.sin(elevation),
    //     radius * Math.cos(elevation) * Math.cos(azimuth),
    // ];

    // TODO: rebuild view matrix each frame (cheap, just 1 lookAt)
    // const view = Mat4.lookAt(eye, [0, 0, 0], [0, 1, 0]);
    // const mvp  = Mat4.multiply(proj, view);  // model = identity for static scene

    // TODO: upload and draw
    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
