'use strict';

const canvas = document.getElementById('canvas');
const gl     = canvas.getContext('webgl2');
canvas.width  = 800;
canvas.height = 600;

// TODO: Implement two-pass shadow mapping with PCF soft shadows
// Shadow mapping is a two-pass algorithm:
//   PASS 1 (shadow map): render the scene from the LIGHT's point of view.
//          Only write depth — no color needed. The resulting depth texture
//          records "how far from the light each surface is."
//   PASS 2 (main render): for each fragment, reconstruct its position in
//          light space and compare its depth against the shadow map.
//          If the fragment's depth > shadowMap depth + bias → it's in shadow.
//
// Key steps:
//   1. Create an FBO with a DEPTH_COMPONENT24 texture, no color attachment.
//   2. Pass 1: bind the FBO, render geometry with a simple depth-only shader.
//   3. Pass 2: bind default FBO, use the shadow texture. In the FS compute
//      the shadow coordinate (NDC → [0,1]), sample the depth texture, compare.
//      Apply PCF: average multiple nearby samples to soften shadow edges.

const SHADOW_SIZE = 1024;  // shadow map resolution

// ----- Depth-only (pass 1) shader -----
const DEPTH_VS = `#version 300 es
layout(location=0) in vec3 a_pos;
uniform mat4 u_lightMVP;
void main() { gl_Position = u_lightMVP * vec4(a_pos, 1.0); }`;
const DEPTH_FS = `#version 300 es
precision mediump float;
// No output — depth is written automatically
void main() {}`;

// ----- Main (pass 2) shader -----
const MAIN_VS = `#version 300 es
layout(location=0) in vec3 a_pos;
layout(location=1) in vec3 a_normal;
uniform mat4 u_mvp;
uniform mat4 u_lightMVP;  // used to project into shadow map space
uniform mat3 u_normalMat;
out vec3 v_normal;
out vec4 v_shadowCoord; // position in light clip space
void main() {
    v_normal = normalize(u_normalMat * a_normal);
    // NDC shadow coord: transform vertex by light's full MVP then to [0,1]
    v_shadowCoord = u_lightMVP * vec4(a_pos, 1.0);
    gl_Position   = u_mvp * vec4(a_pos, 1.0);
}`;

const MAIN_FS = `#version 300 es
precision mediump float;
uniform sampler2D   u_shadowMap;
uniform vec3        u_lightDir;
in vec3 v_normal;
in vec4 v_shadowCoord;
out vec4 outColor;

// PCF: sample a 3×3 neighborhood of the shadow map and average results.
// This softens the harsh aliased shadow edge into a smooth penumbra.
float pcfShadow(sampler2D shadowMap, vec4 sc) {
    // Perspective divide → NDC, then shift from [-1,1] to [0,1]
    vec3 proj = sc.xyz / sc.w;
    proj = proj * 0.5 + 0.5;

    if (proj.z > 1.0) return 0.0; // outside light frustum, no shadow

    float bias  = 0.003;   // depth bias prevents self-shadowing ("shadow acne")
    float shadow = 0.0;
    vec2  texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    // 3×3 PCF kernel
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcfDepth = texture(shadowMap, proj.xy + vec2(x,y)*texelSize).r;
            shadow += (proj.z - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    vec3 N    = normalize(v_normal);
    float NdL = max(dot(N, u_lightDir), 0.0);
    float shadow = pcfShadow(u_shadowMap, v_shadowCoord);

    // Base floor color
    vec3 color = vec3(0.6, 0.6, 0.7);
    color *= 0.2 + 0.8 * NdL * (1.0 - shadow * 0.8);
    outColor = vec4(color, 1.0);
}`;

function compileShader(type, src) {
    const s = gl.createShader(type);
    gl.shaderSource(s, src); gl.compileShader(s);
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(s));
    return s;
}
function makeProgram(vs, fs) {
    const p = gl.createProgram();
    gl.attachShader(p, compileShader(gl.VERTEX_SHADER, vs));
    gl.attachShader(p, compileShader(gl.FRAGMENT_SHADER, fs));
    gl.linkProgram(p);
    return p;
}
const depthProg = makeProgram(DEPTH_VS, DEPTH_FS);
const mainProg  = makeProgram(MAIN_VS,  MAIN_FS);

// TODO Step 1: Create FBO with depth-only texture
const shadowTex = gl.createTexture();
gl.bindTexture(gl.TEXTURE_2D, shadowTex);
gl.texImage2D(gl.TEXTURE_2D, 0, gl.DEPTH_COMPONENT24, SHADOW_SIZE, SHADOW_SIZE,
              0, gl.DEPTH_COMPONENT, gl.UNSIGNED_INT, null);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

const shadowFBO = gl.createFramebuffer();
gl.bindFramebuffer(gl.FRAMEBUFFER, shadowFBO);
// Attach the depth texture to the depth attachment point; no color attachment.
gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, shadowTex, 0);
// Explicitly tell WebGL there is no color draw/read buffer
gl.drawBuffers([gl.NONE]);
gl.readBuffer(gl.NONE);
gl.bindFramebuffer(gl.FRAMEBUFFER, null);

// ----- Build floor and occluder geometry -----
function buildFloor() {
    const v = [-5,0,-5, 5,0,-5, 5,0,5, -5,0,5];
    const n = [0,1,0, 0,1,0, 0,1,0, 0,1,0];
    const verts = []; for(let i=0;i<4;i++) verts.push(v[i*3],v[i*3+1],v[i*3+2],n[i*3],n[i*3+1],n[i*3+2]);
    return {verts:new Float32Array(verts),idx:new Uint16Array([0,1,2,0,2,3]),count:6};
}
function buildCube() {
    // Simple cube with flat normals (6 faces × 4 verts)
    const f=[[0,0,-1],[0,0,1],[-1,0,0],[1,0,0],[0,-1,0],[0,1,0]];
    const v=[]; const idx=[];
    const corners=[-1,-1,-1,1,-1,-1,1,1,-1,-1,1,-1,-1,-1,1,1,-1,1,1,1,1,-1,1,1];
    // Build manually for simplicity: 36 verts
    const faceVerts = [
        [2,1,0,3],[4,5,6,7],[0,4,7,3],[1,2,6,5],[0,1,5,4],[3,7,6,2]
    ];
    let vi=0;
    for(let fi=0;fi<6;fi++){
        const fn=f[fi]; const fc=faceVerts[fi];
        for(let i=0;i<4;i++){
            const c=corners.slice(fc[i]*3,fc[i]*3+3);
            v.push(...c,...fn);
        }
        idx.push(vi,vi+1,vi+2,vi,vi+2,vi+3); vi+=4;
    }
    return {verts:new Float32Array(v),idx:new Uint16Array(idx),count:idx.length};
}

function makeVAO(mesh) {
    const vao=gl.createVertexArray(); gl.bindVertexArray(vao);
    const vbo=gl.createBuffer(); gl.bindBuffer(gl.ARRAY_BUFFER,vbo);
    gl.bufferData(gl.ARRAY_BUFFER,mesh.verts,gl.STATIC_DRAW);
    const S=6*4;
    gl.enableVertexAttribArray(0); gl.vertexAttribPointer(0,3,gl.FLOAT,false,S,0);
    gl.enableVertexAttribArray(1); gl.vertexAttribPointer(1,3,gl.FLOAT,false,S,12);
    const ibo=gl.createBuffer(); gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,ibo);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,mesh.idx,gl.STATIC_DRAW);
    return vao;
}
const floor = buildFloor(); const floorVAO = makeVAO(floor);
const cube  = buildCube();  const cubeVAO  = makeVAO(cube);

// ----- Matrix math -----
function mul(a,b){const o=new Float32Array(16);for(let i=0;i<4;i++)for(let j=0;j<4;j++)for(let k=0;k<4;k++)o[i*4+j]+=a[i*4+k]*b[k*4+j];return o;}
function perspective(fov,asp,n,f){const t=Math.tan(fov*Math.PI/360);return new Float32Array([1/(asp*t),0,0,0,0,1/t,0,0,0,0,(f+n)/(n-f),-1,0,0,(2*f*n)/(n-f),0]);}
function ortho(l,r,b,t,n,f){return new Float32Array([2/(r-l),0,0,0,0,2/(t-b),0,0,0,0,-2/(f-n),0,-(r+l)/(r-l),-(t+b)/(t-b),-(f+n)/(f-n),1]);}
function lookAt(eye,center,up){
    const f=norm([center[0]-eye[0],center[1]-eye[1],center[2]-eye[2]]);
    const s=norm(cross(f,up)); const u=cross(s,f);
    return new Float32Array([s[0],u[0],-f[0],0,s[1],u[1],-f[1],0,s[2],u[2],-f[2],0,
        -dot(s,eye),-dot(u,eye),dot(f,eye),1]);
}
function cross(a,b){return[a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]];}
function dot(a,b){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}
function norm(a){const l=Math.sqrt(dot(a,a));return[a[0]/l,a[1]/l,a[2]/l];}
function translate(x,y,z){return new Float32Array([1,0,0,0,0,1,0,0,0,0,1,0,x,y,z,1]);}
function rotY(t){const c=Math.cos(t),s=Math.sin(t);return new Float32Array([c,0,-s,0,0,1,0,0,s,0,c,0,0,0,0,1]);}
function nm3(m){return new Float32Array([m[0],m[1],m[2],m[4],m[5],m[6],m[8],m[9],m[10]]);}

const camProj  = perspective(50, canvas.width/canvas.height, 0.1, 100);
const camView  = lookAt([6,8,8],[0,0,0],[0,1,0]);
const lightPos = [4, 8, 4];

function drawScene(prog, lightMVP) {
    const uLMVP = gl.getUniformLocation(prog, 'u_lightMVP');
    const uMVP  = gl.getUniformLocation(prog, 'u_mvp');
    const uNM   = gl.getUniformLocation(prog, 'u_normalMat');
    const now   = performance.now() * 0.001;

    // Floor
    const floorMV = mul(camView, new Float32Array([1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]));
    if (uMVP)   gl.uniformMatrix4fv(uMVP,  false, mul(camProj, floorMV));
    if (uLMVP)  gl.uniformMatrix4fv(uLMVP, false, lightMVP);
    if (uNM)    gl.uniformMatrix3fv(uNM,   false, nm3(floorMV));
    gl.bindVertexArray(floorVAO);
    gl.drawElements(gl.TRIANGLES, floor.count, gl.UNSIGNED_SHORT, 0);

    // Spinning cube above floor
    const cubeModel = mul(translate(0, 1.2, 0), rotY(now));
    const cubeMV    = mul(camView, cubeModel);
    if (uMVP)   gl.uniformMatrix4fv(uMVP,  false, mul(camProj, cubeMV));
    if (uLMVP)  gl.uniformMatrix4fv(uLMVP, false, mul(lightMVP, cubeModel));
    if (uNM)    gl.uniformMatrix3fv(uNM,   false, nm3(cubeMV));
    gl.bindVertexArray(cubeVAO);
    gl.drawElements(gl.TRIANGLES, cube.count, gl.UNSIGNED_SHORT, 0);
}

function frame() {
    const lightView = lookAt(lightPos, [0,0,0], [0,1,0]);
    const lightProj = ortho(-8,8,-8,8,0.1,30);
    const lightVP   = mul(lightProj, lightView);
    // The light MVP for static floor = lightVP * identity
    const lightMVP  = lightVP;

    // TODO Step 2: SHADOW PASS — render from light, depth only
    gl.bindFramebuffer(gl.FRAMEBUFFER, shadowFBO);
    gl.viewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
    gl.clear(gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);
    gl.useProgram(depthProg);
    gl.uniform1i(gl.getUniformLocation(depthProg,'u_lightMVP'), 0);
    gl.uniformMatrix4fv(gl.getUniformLocation(depthProg,'u_lightMVP'), false, lightMVP);
    drawScene(depthProg, lightMVP);

    // TODO Step 3: MAIN PASS — render with shadow comparison
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.07, 0.07, 0.10, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.useProgram(mainProg);

    // Bind shadow map to texture unit 0
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, shadowTex);
    gl.uniform1i(gl.getUniformLocation(mainProg,'u_shadowMap'), 0);

    const ld = norm([lightPos[0], lightPos[1], lightPos[2]]);
    gl.uniform3fv(gl.getUniformLocation(mainProg,'u_lightDir'), ld);

    // Build a bias matrix: shifts NDC [-1,1] → [0,1] for texture lookup
    const biasM = new Float32Array([.5,0,0,0,0,.5,0,0,0,0,.5,0,.5,.5,.5,1]);
    const biasedLightMVP = mul(biasM, lightMVP);
    drawScene(mainProg, biasedLightMVP);

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
