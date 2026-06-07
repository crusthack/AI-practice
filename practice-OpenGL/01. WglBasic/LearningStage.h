#pragma once

// Learning goal: create a Win32 window, attach a WGL OpenGL context,
// clear the default framebuffer, and present with SwapBuffers.
// Implementation status: Implemented.

#include <gl/GL.h>

struct LearningStageSetupContext
{
    int Width = 0;
    int Height = 0;
};

struct LearningStageRenderContext
{
    int Width = 0;
    int Height = 0;
};

struct LearningStageState
{
    float ClearColor[4] = { 0.055f, 0.075f, 0.095f, 1.0f };
};

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    (void)stage;
    glViewport(0, 0, context.Width, context.Height);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    (void)timeSeconds;
    stage.ClearColor[0] = 0.055f;
    stage.ClearColor[1] = 0.075f;
    stage.ClearColor[2] = 0.095f;
    stage.ClearColor[3] = 1.0f;
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    glViewport(0, 0, context.Width, context.Height);
    glClearColor(stage.ClearColor[0], stage.ClearColor[1], stage.ClearColor[2], stage.ClearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    (void)stage;
}
