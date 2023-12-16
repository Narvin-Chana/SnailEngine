#pragma once
#include "Entity.h"

namespace Snail
{

class Billboard : public Entity
{
public:
    enum BillboardType
    {
        WORLD_ALIGNED,
        SCREEN_ALIGNED,
        AXIAL_ALIGNED
    };

    class InvalidBillBoardTypeException : public SnailException {};

private:
    mutable D3D11Buffer billboardTransformMatrixBuffer;

    BillboardType type;

public:
    struct Params : Entity::Params
    {
        BillboardType billboardType;
        Params();
    };

    Billboard(const Params& params);
    [[nodiscard]] Matrix GetWorldTransformMatrix() override;
    void Draw(DrawContext& ctx) override;

    void RenderImGui(int idNumber) override;
    std::string GetJsonType() override;
};

}

