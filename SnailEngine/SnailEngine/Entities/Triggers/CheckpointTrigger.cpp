#include "stdafx.h"
#include "CheckpointTrigger.h"

#include "Core/WindowsEngine.h"

namespace Snail
{

CheckpointTrigger::CheckpointTrigger(const Params& params)
    : TriggerBox{params}
    , checkpointIndex{params.checkpointIndex}
{
    static auto& gm = WindowsEngine::GetModule<GameManager>();
    gm.AddCheckpoint();

#ifdef _DEBUG
    triggerColor = checkpointIndex == 0 ? Color{ 1, 0, 0, 1 } : Color{ 1, 1, 0, 1 };
#endif

}
void CheckpointTrigger::OnTriggerEnter()
{
    TriggerBox::OnTriggerEnter();

    static auto& gm = WindowsEngine::GetModule<GameManager>();
    gm.TriggerCheckpoint(checkpointIndex);
}
}
