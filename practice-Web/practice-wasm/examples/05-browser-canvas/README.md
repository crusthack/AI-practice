# 05. Browser Canvas

This is a staged project for using WebAssembly as a compute module and JavaScript as the browser host.

The first implementation target:

1. Export a Rust function that fills an RGBA pixel buffer.
2. Read the generated buffer from JavaScript.
3. Draw it with `CanvasRenderingContext2D.putImageData`.

Suggested build flow:

```console
wasm-pack build --target web
python -m http.server 8000
```

Learning points:

- Keep DOM and canvas APIs in JavaScript.
- Use WebAssembly for deterministic computation.
- Measure copy cost between WebAssembly memory and canvas buffers.

