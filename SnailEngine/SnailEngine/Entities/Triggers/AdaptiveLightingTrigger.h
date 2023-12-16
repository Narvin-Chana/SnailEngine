#pragma once
#include "TriggerBox.h"

namespace Snail
{
    class AdaptiveLightingTrigger : public TriggerBox
    {
        float upperBound = 0.7f;
        float lowerBound = 0.2f;
        float incrementValue = 0.2f;
        bool shouldIncrement = false;
    public:
        AdaptiveLightingTrigger(const Params& params);
        void Update(float) noexcept override;
        void OnTriggerEnter() override;
        void OnTriggerExit() override;

        void RenderImGui(int idNumber) override;
    };
}
