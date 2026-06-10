'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement Uniforms And Time animation
// Uniforms are per-draw constants set by the CPU each frame.
// u_time drives animation; u_offset shifts position; u_color tints the geometry.
//
// Key steps:
//   1. Declare uniform variables in GLSL:
//        uniform float u_time;
//        uniform vec2  u_offset;
//        uniform vec4  u_color;
//      In the VS apply: a_position + vec2(sin(u_time) * 0.5, 0.0) + u_offset
//      In the FS output u_color (possibly modulated by u_time)
//   2. After gl.linkProgram, cache ALL uniform locations with gl.getUniformLocation.
//      Querying locations every frame is expensive — cache them once.
//   3. Each frame: compute elapsed seconds (now / 1000.0), then upload with:
//        gl.uniform1f(loc_time, t)
//        gl.uniform2f(loc_offset, 0.0, sin(t * 0.5) * 0.3)
//        gl.uniform4fv(loc_color, [r, g, b, 1.0])

// --- Shaders ---
const vsSource = `#version 300 es
layout(location = 0) in vec2 a_position;
uniform float u_time;
uniform vec2  u_offset;
void main() {
    vec2 pos = a_position + vec2(sin(u_time) * 0.5, 0.0) + u_offset;
    gl_Position = vec4(pos, 0.0, 1.0);
}`;

const fsSource = `#version 300 es
precision mediump float;
uniform vec4 u_color;
uniform float u_time;
out vec4 fragColor;
void main() {
    // TODO: modulate color channels with u_time for a pulsing effect
    // e.g. fragColor = vec4(u_color.rgb * (0.5 + 0.5 * sin(u_time)), u_color.a);
    fragColor = u_color;
}`;

// TODO: 1. Compile shaders and link program
// const program = compileProgram(gl, vsSource, fsSource);

// TODO: 2. Cache uniform locations after linking (do NOT do this inside frame())
// const loc_time   = gl.getUniformLocation(program, 'u_time');
// const loc_offset = gl.getUniformLocation(program, 'u_offset');
// const loc_color  = gl.getUniformLocation(program, 'u_color');

// TODO: 3. Create VAO + VBO for a simple triangle (reuse knowledge from modules 05-06)

function frame(now) {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    // TODO: compute time in seconds
    // const t = now / 1000.0;

    // TODO: upload uniforms each frame
    // gl.useProgram(program);
    // gl.uniform1f(loc_time,   t);
    // gl.uniform2f(loc_offset, 0.0, Math.sin(t * 0.5) * 0.3);
    // gl.uniform4fv(loc_color, [0.2 + 0.8 * Math.abs(Math.sin(t)), 0.4, 0.9, 1.0]);

    // TODO: bind VAO and draw
    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
