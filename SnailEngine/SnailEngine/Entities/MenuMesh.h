#pragma once

#include "Entity.h"

namespace Snail
{
	class D3D11Device;

	class MenuMesh : public Entity
	{
    public:
        struct Params : Entity::Params
        {
            Params();
            float turnSpeed;//in radiant per second but in json it is in degree per second
            Vector3 rotationAxis;
        };

        float turnSpeed;
        Vector3 rotationAxis;

        void Update(float dt) noexcept override;

        MenuMesh(const Params& params = {});
        std::string GetJsonType() override;
	};
}
