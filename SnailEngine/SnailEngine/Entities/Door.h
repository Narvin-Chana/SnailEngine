#pragma once
#include "Entity.h"

namespace Snail
{
class D3D11Device;

class Door : public Entity
{
public:
    struct Params : Entity::Params
    {
        Params();
    };

    Door(const Params& params = {});
};
}
