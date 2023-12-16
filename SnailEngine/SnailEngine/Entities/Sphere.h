#pragma once

#include "Entity.h"

namespace Snail
{

class Sphere : public Entity
{
public:
    struct Params : Entity::Params
    {
        int stacks = 50, slices = 50;

        Params();
    };

    Sphere(const Params& params = {});

    std::string GetJsonType() override;
};

};
