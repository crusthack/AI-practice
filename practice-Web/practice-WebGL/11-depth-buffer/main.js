'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement depth buffer to correctly occlude overlapping 3D objects
// Without depth testing, draw order determines visibility (painter's algorithm).
// With depth testing enabled the GPU discards fragments that are behind already-drawn ones.
//
// Key steps:
//   1. Enable depth testing once (or toggle with a keypress to show the difference):
//        gl.enable(gl.DEPTH_TEST);
//        gl.depthFunc(gl.LESS);   // pass fragment if its Z < stored Z (default)
//      IMPORTANT: also clear the depth buffer each frame:
//        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
//   2. Place several quads at different Z values (e.g. z = -0.5, 0.0, 0.5) in world space.
//      Each quad has a distinct solid color.
//      Draw them in arbitrary order — depth test should still produce correct result.
//   3. Press 'D' to toggle gl.enable/disable(gl.DEPTH_TEST) to see painter-order artifacts.

let depthEnabled = true;

document.addEventListener('keydown', e => {
    if (e.key === 'd' || e.key === 'D') {
        depthEnabled = !depthEnabled;
        document.getElementById('info').textContent =
            `11 - Depth Buffer | depth test: ${depthEnabled ? 'ON' : 'OFF'} (press D to toggle)`;
    }
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

// TODO: 1. Compile shaders, link program, cache loc_mvp and loc_color

// TODO: 2. Create a flat quad mesh (2 triangles, z=0 in local space)
// Then per-object, set z translation in the model matrix:
//   const quads = [
//     { z: -0.6, color: [1.0, 0.3, 0.3, 1.0], scale: 0.8 },
//     { z:  0.0, color: [0.3, 1.0, 0.3, 1.0], scale: 0.6 },
//     { z:  0.4, color: [0.3, 0.3, 1.0, 1.0], scale: 0.4 },
//   ];

// TODO: 3. Build projection + view matrices (similar to module 08)

function frame(now) {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);

    // TODO: conditionally enable/disable depth test and clear appropriately
    if (depthEnabled) {
        gl.enable(gl.DEPTH_TEST);
        gl.depthFunc(gl.LESS);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    } else {
        gl.disable(gl.DEPTH_TEST);
        gl.clear(gl.COLOR_BUFFER_BIT);
    }

    // TODO: draw each quad with its own model matrix (translation along Z) and color uniform
    // Draw in reverse Z order when depth test is OFF to see the bug clearly;
    // draw in any order when depth test is ON to prove it works.

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
