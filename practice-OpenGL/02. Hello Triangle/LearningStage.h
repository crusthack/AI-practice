#pragma once

// Learning goal: draw the first visible triangle after the WGL context exists.
// Implementation status: Implemented.
//
// This sample uses OpenGL's compatibility immediate-mode draw path on purpose.
// Shader programs and buffer objects start in later samples, so this file keeps
// the new idea limited to primitive submission and vertex attributes.

#include <gl/GL.h>

#include <cmath>

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
    float ClearColor[4] = { 0.045f, 0.060f, 0.080f, 1.0f };
    float Pulse = 0.0f;
};

inline void ApplyStageSpecificSetup(LearningStageState& stage, const LearningStageSetupContext& context)
{
    (void)stage;
    glViewport(0, 0, context.Width, context.Height);
}

inline void UpdateStageSpecificDemo(LearningStageState& stage, double timeSeconds)
{
    stage.Pulse = static_cast<float>(0.5 + 0.5 * std::sin(timeSeconds * 1.5));
}

inline void ApplyStageSpecificRender(LearningStageState& stage, const LearningStageRenderContext& context)
{
    glViewport(0, 0, context.Width, context.Height);
    glClearColor(stage.ClearColor[0], stage.ClearColor[1], stage.ClearColor[2], stage.ClearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_TRIANGLES);
        glColor3f(0.95f, 0.25f + stage.Pulse * 0.25f, 0.18f);
        glVertex2f(0.0f, 0.65f);

        glColor3f(0.18f, 0.72f, 0.95f);
        glVertex2f(-0.70f, -0.55f);

        glColor3f(0.25f, 0.90f, 0.35f + stage.Pulse * 0.15f);
        glVertex2f(0.70f, -0.55f);
    glEnd();
}

inline void ApplyStageSpecificCleanup(LearningStageState& stage)
{
    (void)stage;
}
