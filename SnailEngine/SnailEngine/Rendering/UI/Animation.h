#pragma once

namespace Snail
{
class Animation;

class IAnimation
{
    friend Animation;
protected:
    virtual void Animate(float t) = 0;
public:
    virtual ~IAnimation() = default;
};

class Animation final
{
    float elapsedTime{};
    float animationLength{};
    bool isPaused = true;

public:
    struct Params
    {
        float animationLength = 0;
        bool shouldStartPaused = true;

    };

    Animation(const Params& params = {});

    void Update(float dt, IAnimation* animated);
    void Start();
    void Stop();
    void Reset();
    bool IsDone() const;
};

}
