#pragma once
#include "Entity.h"

namespace Snail
{
class InvisibleWall : public Entity
{
#ifdef _IMGUI_
    bool showColliders = false;
#endif
public:
    InvisibleWall(const Params&);
    void InitPhysics() override;
    std::string GetJsonType() override;
    void RenderImGui(int idNumber) override;
    void Draw(DrawContext& ctx) override;
};

}
