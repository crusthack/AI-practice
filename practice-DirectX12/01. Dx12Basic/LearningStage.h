#pragma once

// Learning goal: Create a Win32 window, initialize the DX12 device/swap chain/command queue, clear a back buffer, and present.
// Implementation status: Implemented.

#include <d3d12.h>

struct LearningStageState
{
    float ClearColor[4] = { 0.08f, 0.13f, 0.20f, 1.0f };
};

struct LearningStageRenderContext
{
    ID3D12GraphicsCommandList* CommandList = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = {};
};

inline void ApplyStageSpecificSetup(LearningStageState& stage, ID3D12Device* device)
{
    (void)stage;
    (void)device;
    // This first sample intentionally creates no stage-owned GPU resources.
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)timeSeconds;
    stage.ClearColor[0] = 0.08f;
    stage.ClearColor[1] = 0.13f;
    stage.ClearColor[2] = 0.20f;
    stage.ClearColor[3] = 1.0f;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    context.CommandList->ClearRenderTargetView(context.RenderTargetView, stage.ClearColor, 0, nullptr);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    (void)stage;
}
