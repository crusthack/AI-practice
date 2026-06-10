'use strict';

// createShader: compile a single GLSL ES shader; throws on compile error
function createShader(gl, type, source) {
    const shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        const log = gl.getShaderInfoLog(shader);
        gl.deleteShader(shader);
        throw new Error(`Shader compile error:\n${log}`);
    }
    return shader;
}

// createProgram: compile + link VS and FS; shaders are detached/deleted after link
function createProgram(gl, vsSource, fsSource) {
    const vs = createShader(gl, gl.VERTEX_SHADER, vsSource);
    const fs = createShader(gl, gl.FRAGMENT_SHADER, fsSource);
    const prog = gl.createProgram();
    gl.attachShader(prog, vs);
    gl.attachShader(prog, fs);
    gl.linkProgram(prog);
    gl.deleteShader(vs);
    gl.deleteShader(fs);
    if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
        const log = gl.getProgramInfoLog(prog);
        gl.deleteProgram(prog);
        throw new Error(`Program link error:\n${log}`);
    }
    return prog;
}

// createBuffer: allocate a GPU buffer and upload data
function createBuffer(gl, target, data, usage = gl.STATIC_DRAW) {
    const buf = gl.createBuffer();
    gl.bindBuffer(target, buf);
    gl.bufferData(target, data, usage);
    gl.bindBuffer(target, null);
    return buf;
}

// createTexture2D: create a 2D texture from a pixel data array (or null for FBO attachment)
function createTexture2D(gl, width, height, data = null,
                          internalFormat = gl.RGBA8,
                          format = gl.RGBA,
                          type = gl.UNSIGNED_BYTE) {
    const tex = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, tex);
    gl.texImage2D(gl.TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.bindTexture(gl.TEXTURE_2D, null);
    return tex;
}

// getUniformLocations: batch-query uniform locations into a plain object
function getUniformLocations(gl, prog, names) {
    const locs = {};
    for (const name of names) locs[name] = gl.getUniformLocation(prog, name);
    return locs;
}

// resizeCanvasToDisplaySize: sync canvas pixel size to CSS display size
function resizeCanvasToDisplaySize(canvas, dpr = window.devicePixelRatio || 1) {
    const w = Math.round(canvas.clientWidth  * dpr);
    const h = Math.round(canvas.clientHeight * dpr);
    if (canvas.width !== w || canvas.height !== h) {
        canvas.width  = w;
        canvas.height = h;
        return true; // viewport needs updating
    }
    return false;
}

// FPSCounter: rolling average over 60 frames
class FPSCounter {
    constructor() { this._times = []; this.fps = 0; }
    tick(now) {
        this._times.push(now);
        if (this._times.length > 60) this._times.shift();
        const dt = this._times[this._times.length - 1] - this._times[0];
        this.fps = dt > 0 ? Math.round((this._times.length - 1) * 1000 / dt) : 0;
    }
}
