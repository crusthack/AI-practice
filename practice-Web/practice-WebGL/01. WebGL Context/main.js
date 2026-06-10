'use strict';

// ─── Context Initialization ────────────────────────────────────────────────
const canvas = document.getElementById('canvas');
const gl = canvas.getContext('webgl2');

if (!gl) {
    document.body.innerHTML = `
        <p style="color:#f66;font-family:monospace;padding:2rem">
            WebGL2 is not supported in this browser.<br>
            Try Chrome 56+, Firefox 51+, or Edge 79+.
        </p>`;
    throw new Error('WebGL2 not supported');
}

// Log renderer info — useful for verifying the GPU being used
const dbgInfo = gl.getExtension('WEBGL_debug_renderer_info');
if (dbgInfo) {
    console.log('Renderer:', gl.getParameter(dbgInfo.UNMASKED_RENDERER_WEBGL));
    console.log('Vendor:  ', gl.getParameter(dbgInfo.UNMASKED_VENDOR_WEBGL));
}
console.log('WebGL version:', gl.getParameter(gl.VERSION));
console.log('GLSL version: ', gl.getParameter(gl.SHADING_LANGUAGE_VERSION));

// ─── Resize Handling ───────────────────────────────────────────────────────
// Sync the canvas backing-store size to its CSS display size.
// Without this the image is stretched/blurred on high-DPI screens.
function handleResize() {
    const dpr = window.devicePixelRatio || 1;
    const w   = Math.round(canvas.clientWidth  * dpr);
    const h   = Math.round(canvas.clientHeight * dpr);
    if (canvas.width !== w || canvas.height !== h) {
        canvas.width  = w;
        canvas.height = h;
        gl.viewport(0, 0, w, h);
    }
}

// ─── FPS Counter ───────────────────────────────────────────────────────────
const fpsCounter = new FPSCounter();
const infoEl     = document.getElementById('info');

// ─── Render Loop ───────────────────────────────────────────────────────────
// Animate the clear color through the HSL color wheel using sin waves
// offset by 120° (2π/3 ≈ 2.094 rad) for R, G, B channels.
function frame(now) {
    handleResize();
    fpsCounter.tick(now);

    const t = now * 0.001;
    const r = 0.5 + 0.5 * Math.sin(t);
    const g = 0.5 + 0.5 * Math.sin(t + 2.094);
    const b = 0.5 + 0.5 * Math.sin(t + 4.189);

    gl.clearColor(r, g, b, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    infoEl.textContent =
        `01 - WebGL Context  |  ${fpsCounter.fps} FPS  |  ` +
        `${canvas.width}×${canvas.height} px  |  WebGL2`;

    requestAnimationFrame(frame);
}

requestAnimationFrame(frame);
