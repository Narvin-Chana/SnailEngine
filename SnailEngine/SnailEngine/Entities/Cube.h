#pragma once

#include "Entity.h"

namespace Snail
{
	class D3D11Device;

	class Cube : public Entity
	{
    public:
        struct Params : Entity::Params
        {
            Params();
        };

		Cube(const Params& params = {});
        std::string GetJsonType() override;
	};
}
