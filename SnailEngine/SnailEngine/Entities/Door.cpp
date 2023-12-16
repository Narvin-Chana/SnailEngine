#include "stdafx.h"
#include "Door.h"

#include "Core/WindowsEngine.h"

Snail::Door::Params::Params()
{
    name = "Door";
}

Snail::Door::Door(const Params& params)
    : Entity(params)
{
    WindowsEngine::GetModule<GameManager>().SetDoor(this);
}
