#pragma once
#include "Animation.h"
#include "UIElement.h"

namespace Snail
{
class InfiniteSprite;

class SceneTransition : public UIElement, IAnimation
{
    InfiniteSprite* stencil{};
    const Vector2 initialScale{1,1};
    const Vector2 finalScale{ 200,200 };
protected:
    void Animate(float t) override;
public:
    SceneTransition();
    SceneTransition(SceneTransition&&) = default;

    void Update(float dt) override;

    Animation anim;
};
}
