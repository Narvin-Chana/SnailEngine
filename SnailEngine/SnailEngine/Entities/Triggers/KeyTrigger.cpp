#include "stdafx.h"
#include "KeyTrigger.h"

#include "Core/WindowsEngine.h"

namespace Snail
{

KeyTrigger::KeyTrigger(const Params& params)
    : TriggerBox{params}
{
#ifdef _DEBUG
    triggerColor = Color{1, 0, 1, 1};
#endif
}
void KeyTrigger::OnTriggerEnter()
{
    TriggerBox::OnTriggerEnter();

    static auto* scene = WindowsEngine::GetScene();
    static auto& gm = WindowsEngine::GetModule<GameManager>();

    gm.CollectKey();
    scene->RemoveEntity(this);
}
}
