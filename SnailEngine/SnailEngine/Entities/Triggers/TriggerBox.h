#pragma once

#include "Entities/Entity.h"

namespace Snail
{
class D3D11Device;
class DrawContext;

class TriggerBox : public Entity
{
public:
    struct TriggerUserData : UserData
    {
        bool alreadyIn = false;

        TriggerUserData(TriggerBox* trigger, Entity* entity);

        void OnTrigger(TriggerBox*) override;
        TriggerBox* triggerBox;
    };

private:
    std::unique_ptr<TriggerUserData> triggerUD = std::make_unique<TriggerUserData>(this, this);

public:

    struct Params : Entity::Params
    {
        Params();
    };

#ifdef _DEBUG
    Color triggerColor = Color{1,1,1,1};
#endif

    bool triggerActive = true;

    TriggerBox(const Params& params = {});

    void InitPhysics() override;
    void Draw(DrawContext& ctx) override;

    virtual void OnTriggerEnter();
    virtual void OnTriggerExit();
    std::string GetJsonType() override;
};
}
