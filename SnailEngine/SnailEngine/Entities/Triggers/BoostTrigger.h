#pragma once
#include "TriggerBox.h"

namespace Snail
{

class BoostTrigger : public TriggerBox
{
    float timeUntilReappear = 0;
    static constexpr float respawnTime = 10; 
public:
    BoostTrigger(const Params& params);
    void Update(float) noexcept override;
    void OnTriggerEnter() override;
};

}
