#pragma once
#include "Entity.h"

namespace Snail
{

class Decal : public Entity
{
public:
    struct Params : Entity::Params
    {
        Params();
    };

    Decal(const Params& params = {});
    std::string GetJsonType() override;
};

}
