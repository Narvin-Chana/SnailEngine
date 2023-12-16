#pragma once
#include "Effect.h"

namespace Snail
{

struct ScreenShakeParameters
{
    Vector2 intensity = { 0.2f, 0.1f };
    float time = 0.7f;
    float period = 0.17f;
};

class ScreenShakeEffect : public Effect
{
    ScreenShakeParameters params;
    Vector3 prevPos{};
public:
    void Update(float dt) override;
    void RenderEffectBegin();
    void RenderEffectEnd() const;

    void RenderImGui() override;
};

}

