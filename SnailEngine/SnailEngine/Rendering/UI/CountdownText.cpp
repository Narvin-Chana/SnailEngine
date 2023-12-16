#include "stdafx.h"
#include "CountdownText.h"

namespace Snail
{

void CountdownText::Animate(float t)
{
    transform.rotation = log(pow(1 - t, 4.0f) + 1.0f) / log(2.0f) * DirectX::XM_PI;
    transform.scale = Vector2::Lerp(initScale, finalScale, log(pow(t, 6.0f) + 1.0f) / log(2.0f));
}

CountdownText::CountdownText(const Params& params)
    : Text{params}
    , anim{Animation::Params{
        .animationLength = 1,
        .shouldStartPaused = true
    }}
{
}

void CountdownText::Update(float dt)
{
    Text::Update(dt);
    anim.Update(dt, this);
}

}
