#pragma once
#include <memory>
#include <optional>
#include <array>

#include "Util/Util.h"

#include "ShadowMap.h"

#include "Core/RendererModule.h"
#include "Core/SceneParser.h"
#include "Core/DataStructures/FixedVector.h"
#include "Rendering/DrawContext.h"
#include "Rendering/Buffers/D3D11Buffer.h"
#include "Rendering/Shaders/PixelShader.h"
#include "Rendering/Shaders/VertexShader.h"
#include "Rendering/Lights/DirectionalLight.h"

struct ID3D11DepthStencilView;

namespace Snail
{
	class Cube;
	class Entity;
	class D3D11Device;
	class Texture2D;

	class DirectionalShadowMap : public ShadowMap
	{
		static constexpr uint8_t CASCADE_COUNT = 6;

		std::vector<std::pair<std::optional<float>, std::optional<float>>> cascades = {
			{{}, 10.0f},
			{7.5f, 20.0f},
			{15.0f, 50.0f},
			{37.5f, 100.0f},
            {75.0f, 200.0f},
			{180.0f, {}}
		};

		std::unique_ptr<Texture2D> shadowMap;
		std::vector<ID3D11DepthStencilView*> depthStencilView;
        ID3D11RasterizerState* shadowRS;
		D3D11Device* renderDevice;
		D3D11_VIEWPORT viewport;

		VertexShader vsShader;
		PixelShader psShader;

		struct DX_ALIGN CascadeShadowData
		{
			DX_ALIGN DirectX::XMFLOAT4X4 matrix;
			Vector2 cascadeBound;
		};

		D3D11Buffer cascadeShadowBuffer;
		D3D11Buffer viewProjBuffer;

#ifdef _IMGUI_
        bool drawCascades = false;
#endif

        std::array<std::pair<Matrix, DirectX::BoundingOrientedBox>, CASCADE_COUNT> cascadeInfo;

	public:
		DirectionalShadowMap(D3D11Device* device);
		~DirectionalShadowMap();

		Texture2D* GetDepthTexture() const;
        std::pair<Matrix, DirectX::BoundingOrientedBox> GetLightSpaceMatrix(const DirectionalLight& light, int cascadeId);
		const D3D11Buffer& GetViewProjBuffer(const std::vector<DirectionalLight>& lights);
		void Render(const FixedVector<DirectionalLight, SceneData::MAX_DIR_LIGHTS>& lights);
        void RenderImGui();
	};

    class ShadowDrawContext : public DrawContext
    {
        const DirectX::BoundingOrientedBox& sumObb;
    public:
        ShadowDrawContext(RendererModule* rm, D3D11Device* dev, const DirectX::BoundingOrientedBox& obb);

        bool ShouldBeCulled(const DirectX::BoundingBox&) override;
        bool ShouldBeCulled(const DirectX::BoundingOrientedBox&) override;
        bool ShouldBeCulled(const DirectX::BoundingSphere&) override;
    };
}
