#pragma once
#include "Animation.h"
#include "Text.h"

namespace Snail
{

class CountdownText : public Text, IAnimation
{
    Vector2 initScale{7, 7};
    Vector2 finalScale{1, 1};
protected:
    void Animate(float t) override;
public:
    CountdownText(const Params& params = {});
    void Update(float) override;

    Animation anim;
};

}