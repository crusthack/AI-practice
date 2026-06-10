'use strict';

// ─── Context ───────────────────────────────────────────────────────────────
const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// ─── Explicit Shader Compilation ──────────────────────────────────────────
// This module implements compile/link manually (without the common helper)
// to expose every step and error-reporting call.
function compileShader(gl, type, source) {
    const shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        const log = gl.getShaderInfoLog(shader);
        gl.deleteShader(shader);
        throw new Error(`[Shader Compile]\n${log}`);
    }
    return shader;
}

function buildProgram(gl, vsSource, fsSource) {
    const vs   = compileShader(gl, gl.VERTEX_SHADER,   vsSource);
    const fs   = compileShader(gl, gl.FRAGMENT_SHADER, fsSource);
    const prog = gl.createProgram();
    gl.attachShader(prog, vs);
    gl.attachShader(prog, fs);
    gl.linkProgram(prog);
    // Shaders are no longer needed once the program is linked
    gl.detachShader(prog, vs); gl.deleteShader(vs);
    gl.detachShader(prog, fs); gl.deleteShader(fs);
    if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
        const log = gl.getProgramInfoLog(prog);
        gl.deleteProgram(prog);
        throw new Error(`[Program Link]\n${log}`);
    }
    // Validate in the current GL state (optional but informative during development)
    gl.validateProgram(prog);
    if (!gl.getProgramParameter(prog, gl.VALIDATE_STATUS)) {
        console.warn('Program validation:', gl.getProgramInfoLog(prog));
    }
    return prog;
}

// ─── Shaders ──────────────────────────────────────────────────────────────
// The vertex shader accepts pixel-space positions and converts them to NDC.
// This avoids hardcoding NDC coordinates and feels more intuitive during layout.
const VS = `#version 300 es
in vec2 a_position;
uniform vec2 u_resolution;  // canvas size in pixels
void main() {
    // Map [0, resolution] → [-1, 1] with Y flipped (top-left origin)
    vec2 ndc = (a_position / u_resolution) * 2.0 - 1.0;
    gl_Position = vec4(ndc * vec2(1.0, -1.0), 0.0, 1.0);
}`;

const FS = `#version 300 es
precision mediump float;
uniform float u_time;
uniform vec4  u_color;
out vec4 fragColor;
void main() {
    // Pulse brightness with a sine wave driven by u_time
    float pulse = 0.5 + 0.5 * sin(u_time * 3.0);
    fragColor = mix(u_color, vec4(1.0), pulse * 0.25);
}`;

const prog = buildProgram(gl, VS, FS);

// Cache all uniform locations up front — querying them per-frame is wasteful
const u = getUniformLocations(gl, prog, ['u_resolution', 'u_time', 'u_color']);

// ─── Geometry (Vertex Buffer) ──────────────────────────────────────────────
// Triangle vertices in pixel space (origin = top-left of canvas)
const positions = new Float32Array([
    400, 80,   // top
    80,  520,  // bottom-left
    720, 520,  // bottom-right
]);

const vao = gl.createVertexArray();
const vbo = gl.createBuffer();
gl.bindVertexArray(vao);
gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
gl.bufferData(gl.ARRAY_BUFFER, positions, gl.STATIC_DRAW);
const posLoc = gl.getAttribLocation(prog, 'a_position');
gl.enableVertexAttribArray(posLoc);
gl.vertexAttribPointer(posLoc, 2, gl.FLOAT, false, 0, 0);
gl.bindVertexArray(null);

// ─── Render Loop ───────────────────────────────────────────────────────────
function frame(now) {
    const t = now * 0.001;
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(prog);
    gl.uniform2f(u.u_resolution, canvas.width, canvas.height);
    gl.uniform1f(u.u_time,  t);
    gl.uniform4f(u.u_color, 0.2, 0.6, 1.0, 1.0);  // base blue

    gl.bindVertexArray(vao);
    gl.drawArrays(gl.TRIANGLES, 0, 3);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
