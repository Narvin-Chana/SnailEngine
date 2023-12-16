#pragma once
#include "TriggerBox.h"

namespace Snail
{

class KeyTrigger : public TriggerBox
{
public:
    KeyTrigger(const Params& params);
    void OnTriggerEnter() override;
};

}