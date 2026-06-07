# OpenGL Learning Guide - 01~09

This phase covers the minimum Windows OpenGL runtime, first draw calls, shader programs, vertex input, matrix transforms, and camera movement.

Key comparison with the DirectX samples:

- WGL context creation replaces D3D device/swap-chain setup.
- OpenGL state is context-global, so samples should be careful about what state each stage changes.
- Buffer and shader ownership still belongs in `LearningStage.h`, matching the DirectX12 stage-hook contract.
