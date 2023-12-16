#pragma once

#include "InstancedEntity.h"
#include "Billboard.h"
#include "Core/SceneParser.h"
#include "Rendering/Lights/PointLight.h"

namespace Snail
{
	class D3D11Device;

	class Firefly : public InstancedEntity
	{
    public:
        struct Params : InstancedEntity::Params
        {
            Params();
            Billboard::Params billboard;
            Vector3 color{1, 1, 1};
            Vector3 coefficients{1, 0.045f, 0.0075f};
        };

        Billboard billboard;
        std::vector<PointLight*> lights;
        std::vector<float> billboardOffsets;
        Vector3 color{1, 1, 1};
        Vector3 coefficients{1, 0.045f, 0.0075f};
        float elapsedTime = 0;


        void Draw(DrawContext& ctx) override;
        void Update(const float deltaTime) noexcept override;

        Firefly(const Params& params = {});

        void addLights(SceneData* sceneData);
        std::string GetJsonType() override;
	};
}
