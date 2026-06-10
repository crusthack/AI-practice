'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement Vertex Array Object (VAO)
// A VAO remembers all vertex attribute configuration so you do not repeat
// gl.bindBuffer / gl.vertexAttribPointer / gl.enableVertexAttribArray on every frame.
//
// Key steps:
//   1. Call gl.createVertexArray() and gl.bindVertexArray(vao) ONCE during setup.
//      Inside the binding: create + fill a VBO, create + fill an EBO,
//      and call gl.vertexAttribPointer + gl.enableVertexAttribArray for each attribute.
//      Finish setup with gl.bindVertexArray(null) to prevent accidental mutation.
//   2. During setup OUTSIDE the VAO: compile shaders and link the program as usual.
//   3. Per-frame: only bind the VAO (gl.bindVertexArray(vao)) then call
//      gl.drawElements — no need to rebind buffers or reconfigure attributes.

// --- Shaders ---
// Vertex shader: position attribute (location 0) and color attribute (location 1)
// Fragment shader: receive v_color from VS and output it

const vsSource = `#version 300 es
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 a_color;
out vec3 v_color;
void main() {
    v_color = a_color;
    gl_Position = vec4(a_position, 0.0, 1.0);
}`;

const fsSource = `#version 300 es
precision mediump float;
in vec3 v_color;
out vec4 fragColor;
void main() {
    fragColor = vec4(v_color, 1.0);
}`;

// TODO: 1. Compile shaders and link program
// const program = ...

// TODO: 2. Create VAO — all buffer + attribute setup goes inside bindVertexArray()
// const vao = gl.createVertexArray();
// gl.bindVertexArray(vao);
//
//   Quad vertices: 4 corners, each with [x, y, r, g, b]
//   const vertices = new Float32Array([...]);
//   const vbo = gl.createBuffer();
//   gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
//   gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
//
//   const indices = new Uint16Array([0,1,2, 2,3,0]);
//   const ebo = gl.createBuffer();
//   gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ebo);
//   gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);
//
//   const STRIDE = 5 * 4; // 5 floats * 4 bytes
//   gl.vertexAttribPointer(0, 2, gl.FLOAT, false, STRIDE, 0);        // position
//   gl.enableVertexAttribArray(0);
//   gl.vertexAttribPointer(1, 3, gl.FLOAT, false, STRIDE, 2 * 4);    // color
//   gl.enableVertexAttribArray(1);
//
// gl.bindVertexArray(null); // unbind to prevent accidental changes

function frame(now) {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    // TODO: draw — this is the entire draw call after setup!
    // gl.useProgram(program);
    // gl.bindVertexArray(vao);
    // gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);
    // gl.bindVertexArray(null);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
