#pragma once

#include "Util/Singleton.h"

#include "Mouse.h"
#include "Keyboard.h"
#include "Controller.h"

namespace Snail {

struct InputModule : Singleton<InputModule> {
	ControllerManager& Controller = ControllerManager::GetInstance();
	Mouse& Mouse = Mouse::GetInstance();
	Keyboard& Keyboard = Keyboard::GetInstance();

    void PreUpdate()
    {
        Mouse.Update();
        Controller.Update();
    }

    void PostUpdate()
    {
        Keyboard.Update();
    }
};

}