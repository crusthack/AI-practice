'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement MVP (Model-View-Projection) matrix transforms
// The three matrices bring a 3D object from local space into clip space:
//   Model      — where the object is in the world (translation, rotation, scale)
//   View       — where the camera is looking (lookAt)
//   Projection — how 3D depth maps to 2D screen (perspective divide)
//
// Key steps:
//   1. Build a STATIC projection matrix once (aspect ratio 800/600, ~60° fov, near=0.1, far=100).
//      Build a STATIC view matrix using lookAt: eye=[0,0,3], center=[0,0,0], up=[0,1,0].
//   2. Each frame: build a model matrix that rotates around Y (and optionally X) by u_time.
//      Use Mat4.rotateY(t) or compose Mat4.trs(translation, rotation, scale).
//   3. Multiply: mvp = projection * view * model  (column-major order used by WebGL).
//      Upload the single 4x4 result: gl.uniformMatrix4fv(loc_mvp, false, mvp).

// --- Shaders ---
// The vertex shader only needs the MVP uniform and a_position attribute.
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
void main() {
    fragColor = vec4(v_color, 1.0);
}`;

// TODO: 1. Compile shaders, link program
// const program = ...
// const loc_mvp = gl.getUniformLocation(program, 'u_mvp');

// TODO: 2. Create a colored cube (8 unique vertices + index list of 36 indices = 12 triangles)
// Cube vertex positions span [-0.5, +0.5] on each axis.
// Interleave [x,y,z, r,g,b] per vertex.

// TODO: 3. Compute static projection and view matrices (outside frame loop)
// const aspect = canvas.width / canvas.height;
// const proj = Mat4.perspective(Math.PI / 3, aspect, 0.1, 100.0);
// const view = Mat4.lookAt([0, 0.5, 3], [0, 0, 0], [0, 1, 0]);

function frame(now) {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);

    const t = now / 1000.0;

    // TODO: build per-frame model matrix
    // const model = Mat4.rotateY(t);  // spins 1 rad/sec around Y

    // TODO: multiply MVP = proj * view * model
    // const mvp = Mat4.multiply(proj, Mat4.multiply(view, model));

    // TODO: upload and draw
    // gl.useProgram(program);
    // gl.uniformMatrix4fv(loc_mvp, false, mvp);
    // gl.bindVertexArray(vao);
    // gl.drawElements(gl.TRIANGLES, 36, gl.UNSIGNED_SHORT, 0);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
