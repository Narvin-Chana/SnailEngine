#include "stdafx.h"
#include "BoostTrigger.h"

#include "Core/WindowsEngine.h"

namespace Snail
{

BoostTrigger::BoostTrigger(const Params& params)
    : TriggerBox{params}
{
#ifdef _DEBUG
    triggerColor = Color{ 0, 1, 1, 1 };
#endif
}

void BoostTrigger::Update(float dt) noexcept
{
    TriggerBox::Update(dt);

    if (!triggerActive)
    {
        timeUntilReappear -= dt;
        if (timeUntilReappear < 0)
        {
            triggerActive = true;
            timeUntilReappear = 0;
        }
    }
    else
    {
        SetRotation(transform.rotation * Quaternion::CreateFromAxisAngle(Vector3::UnitY, dt));
    }
}

void BoostTrigger::OnTriggerEnter()
{
    TriggerBox::OnTriggerEnter();

    static auto& gm = WindowsEngine::GetModule<GameManager>();
    if (gm.HasBoost())
        return;

    gm.CollectBoost();
    triggerActive = false;
    timeUntilReappear = respawnTime;
}

}
