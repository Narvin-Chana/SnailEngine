#pragma once
#include "TriggerBox.h"

namespace Snail
{
class CheckpointTrigger : public TriggerBox
{
    const int checkpointIndex = 0;
public:
    struct Params : TriggerBox::Params
    {
        int checkpointIndex = 0;
    };

    CheckpointTrigger(const Params& params);
    void OnTriggerEnter() override;
};
}
