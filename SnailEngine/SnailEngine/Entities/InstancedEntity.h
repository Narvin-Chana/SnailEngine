#pragma once
#include "Entity.h"

namespace Snail
{

class InstancedEntity : public Entity
{
public:
    struct Params : Entity::Params
    {
        std::vector<Transform> instanceTransforms;

        Params();
    };

#ifdef _IMGUI_
    std::vector<bool> hiddenInstances;
#endif

    std::vector<Transform> instanceTransforms;

    InstancedEntity(const Params&);

    void Draw(DrawContext& ctx) override;

    void RenderImGui(int idNumber) override;

    std::string GetJsonType() override;
};

}