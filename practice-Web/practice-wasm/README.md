---
title: 'WebAssembly practice'
description: 'WebAssembly runtime, browser integration, WASI, and host interop practice'
---

# WebAssembly Practice

This workspace is for learning WebAssembly from the binary/runtime model up to browser and WASI integration.

## Goals

- Understand what a `.wasm` module exports and imports.
- Load and call WebAssembly from JavaScript.
- Practice memory sharing between host code and WebAssembly.
- Learn the WAT text format as a debugging and teaching tool.
- Build small Rust/C modules for browser and WASI targets.
- Compare WebAssembly's sandbox, linear memory, and host ABI with native code.

## Workspace Layout

```text
practice-wasm/
  README.md
  WASM_LEARNING_ROADMAP.md
  examples/
    00-js-raw-module/
    01-wat-add/
    02-memory-from-js/
    03-rust-wasm-bindgen/
    04-c-wasi-cli/
    05-browser-canvas/
```

## Quick Start

The first example does not require Rust, C, Node packages, or external tools.

```console
cd practice-wasm/examples/00-js-raw-module
```

Open `index.html` in a browser. It creates a tiny WebAssembly module from raw bytes and calls an exported `add` function.

For later examples, install only the toolchain needed by the example:

- WAT examples: WABT, especially `wat2wasm`
- Rust browser examples: `rustup`, `wasm-pack`
- C/WASI examples: `wasi-sdk` or a Clang build with WASI support

## Recommended Order

1. `00-js-raw-module`: instantiate raw bytes and call an exported function.
2. `01-wat-add`: read and compile a WAT module.
3. `02-memory-from-js`: share linear memory between JavaScript and WebAssembly.
4. `03-rust-wasm-bindgen`: expose Rust functions to JavaScript.
5. `04-c-wasi-cli`: build a command-line WASI module.
6. `05-browser-canvas`: use WebAssembly for browser-side computation.

