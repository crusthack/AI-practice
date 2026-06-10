'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement a scene graph with parent-child transform hierarchy
// A scene graph is a tree of nodes, each holding a LOCAL transform.
// To get the WORLD transform of a node: multiply every ancestor's local
// transform from root to leaf (worldMatrix = parent.world * local).
//
// Key steps:
//   1. Define SceneNode: { localMatrix, worldMatrix, mesh, children[] }
//   2. render(node, parentWorld): compute worldMatrix = parentWorld * local,
//      draw the node's mesh with worldMatrix, then recurse for each child.
//   3. Build a solar-system hierarchy:
//        root → sun (stationary)
//             → earthPivot (rotates around origin) → earth
//                                                  → moonPivot → moon
//
// Why this pattern? Each node's local transform is simple (just its own motion).
// The world transform automatically accumulates ancestor transforms through
// recursion — you never manually track absolute positions.

// ----- Shaders -----
const VS = `#version 300 es
layout(location=0) in vec3 a_pos;
uniform mat4 u_mvp;
uniform vec3 u_color;
out vec3 v_color;
void main() {
    v_color = u_color;
    gl_Position = u_mvp * vec4(a_pos, 1.0);
}`;
const FS = `#version 300 es
precision mediump float;
in  vec3 v_color;
out vec4 outColor;
void main() { outColor = vec4(v_color, 1.0); }`;

function compileShader(type, src) {
    const s = gl.createShader(type);
    gl.shaderSource(s, src); gl.compileShader(s);
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(s));
    return s;
}
const prog = gl.createProgram();
gl.attachShader(prog, compileShader(gl.VERTEX_SHADER,   VS));
gl.attachShader(prog, compileShader(gl.FRAGMENT_SHADER, FS));
gl.linkProgram(prog);
const uMVP   = gl.getUniformLocation(prog, 'u_mvp');
const uColor = gl.getUniformLocation(prog, 'u_color');

// ----- Sphere mesh (reused by all nodes) -----
function buildSphere(stacks, slices) {
    const v=[],idx=[];
    for(let i=0;i<=stacks;i++){
        const phi=Math.PI*i/stacks,y=Math.cos(phi),r=Math.sin(phi);
        for(let j=0;j<=slices;j++){
            const t=2*Math.PI*j/slices;
            v.push(r*Math.cos(t),y,r*Math.sin(t));
        }
    }
    for(let i=0;i<stacks;i++) for(let j=0;j<slices;j++){
        const a=i*(slices+1)+j,b=a+(slices+1);
        idx.push(a,b,a+1,b,b+1,a+1);
    }
    return {verts:new Float32Array(v),idx:new Uint16Array(idx),count:idx.length};
}
const sphere = buildSphere(16, 16);
const vao = gl.createVertexArray();
gl.bindVertexArray(vao);
const vbo = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
gl.bufferData(gl.ARRAY_BUFFER, sphere.verts, gl.STATIC_DRAW);
gl.enableVertexAttribArray(0);
gl.vertexAttribPointer(0,3,gl.FLOAT,false,0,0);
const ibo = gl.createBuffer();
gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, sphere.idx, gl.STATIC_DRAW);

// ----- Matrix math -----
function identity() { return new Float32Array([1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]); }
function mul(a,b) {
    const o=new Float32Array(16);
    for(let i=0;i<4;i++) for(let j=0;j<4;j++) for(let k=0;k<4;k++) o[i*4+j]+=a[i*4+k]*b[k*4+j];
    return o;
}
function rotY(t){const c=Math.cos(t),s=Math.sin(t);return new Float32Array([c,0,-s,0,0,1,0,0,s,0,c,0,0,0,0,1]);}
function scale(s){return new Float32Array([s,0,0,0,0,s,0,0,0,0,s,0,0,0,0,1]);}
function translate(x,y,z){return new Float32Array([1,0,0,0,0,1,0,0,0,0,1,0,x,y,z,1]);}
function perspective(fov,asp,n,f){
    const t=Math.tan(fov*Math.PI/360);
    return new Float32Array([1/(asp*t),0,0,0,0,1/t,0,0,0,0,(f+n)/(n-f),-1,0,0,(2*f*n)/(n-f),0]);
}

const proj = perspective(45, canvas.width/canvas.height, 0.1, 100);
const cam  = translate(0, 0, -14);

// TODO Step 1: SceneNode class
// localMatrix: transform relative to parent space.
// worldMatrix: computed each frame = parent.world * localMatrix.
// color: used for shading since we share one sphere mesh.
class SceneNode {
    constructor(color=[1,1,1]) {
        this.localMatrix = identity();
        this.worldMatrix = identity();
        this.color    = color;
        this.visible  = true;   // false = pivot node (no geometry)
        this.children = [];
    }
    add(child) { this.children.push(child); return child; }
}

// TODO Step 2: Recursive render function
// Accumulates world transforms as it traverses the tree.
function renderNode(node, parentWorld) {
    // World = parent's accumulated transform × this node's local transform
    node.worldMatrix = mul(parentWorld, node.localMatrix);

    if (node.visible) {
        const mvp = mul(mul(proj, cam), node.worldMatrix);
        gl.uniformMatrix4fv(uMVP,   false, mvp);
        gl.uniform3fv(uColor, node.color);
        gl.drawElements(gl.TRIANGLES, sphere.count, gl.UNSIGNED_SHORT, 0);
    }

    // Recurse: each child inherits this node's world transform
    for (const child of node.children) renderNode(child, node.worldMatrix);
}

// TODO Step 3: Build the solar-system scene graph
//   root (identity)
//   ├── sun  (scale 1.5, yellow)
//   └── earthPivot (invisible, just rotates around origin)
//       ├── earth (translated 5 units out, scale 0.7, blue)
//       └── moonPivot (invisible, child of earth, rotates around earth)
//           └── moon (translated 1.5 units out, scale 0.3, grey)

const root       = new SceneNode(); root.visible = false;
const sun        = root.add(new SceneNode([1.0, 0.9, 0.2]));
const earthPivot = root.add(new SceneNode()); earthPivot.visible = false;
const earth      = earthPivot.add(new SceneNode([0.2, 0.5, 1.0]));
const moonPivot  = earth.add(new SceneNode()); moonPivot.visible = false;
const moon       = moonPivot.add(new SceneNode([0.7, 0.7, 0.7]));

function frame(now) {
    const t = now * 0.001;

    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.04, 0.04, 0.08, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);

    // --- Update local transforms each frame ---
    // Sun: just scales in place (no orbit)
    sun.localMatrix = scale(1.5);

    // earthPivot rotates around the scene origin — earth orbits the sun.
    // The pivot itself has no geometry, only a rotation.
    earthPivot.localMatrix = rotY(t * 0.8);

    // Earth: translated from pivot origin + small self-rotation
    earth.localMatrix = mul(translate(5, 0, 0), mul(rotY(t * 2.0), scale(0.7)));

    // moonPivot is a child of EARTH, so it rotates around the earth's origin.
    moonPivot.localMatrix = rotY(t * 3.5);

    // Moon: small, offset from moonPivot by 1.5 units.
    moon.localMatrix = mul(translate(1.5, 0, 0), scale(0.3));

    // --- Render tree starting from root with identity as the parent world ---
    gl.useProgram(prog);
    gl.bindVertexArray(vao);
    renderNode(root, identity());

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
