#include "stdafx.h"
#include "Animation.h"

namespace Snail
{

Animation::Animation(const Params& params)
    : animationLength{params.animationLength}
    , isPaused{params.shouldStartPaused}
{
}
void Animation::Update(float dt, IAnimation* animated)
{
    if (isPaused)
        return;

    elapsedTime = std::min(elapsedTime + dt, animationLength);
    animated->Animate(elapsedTime / animationLength);
}

void Animation::Start()
{
    isPaused = false;
}

void Animation::Stop()
{
    isPaused = true;
}

void Animation::Reset()
{
    elapsedTime = 0;
}
bool Animation::IsDone() const
{
    return elapsedTime == animationLength;
}
}
