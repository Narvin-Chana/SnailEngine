#include "stdafx.h"

#include "Firefly.h"

#include "Core/WindowsEngine.h"

namespace Snail
{
Firefly::Params::Params()
{
    name = "Fireflies";
}

void Firefly::Draw(DrawContext& ctx)
{
    for (const auto& instanceTransform : instanceTransforms)
    {
        billboard.SetTransform(instanceTransform);
        billboard.Draw(ctx);
    }
}

void Firefly::Update(const float deltaTime) noexcept
{
    elapsedTime += deltaTime;
    InstancedEntity::Update(deltaTime);

    //update lights
    for (int i = 0; i < instanceTransforms.size(); ++i)
    {
        Vector3 calculatedPosition = instanceTransforms[i].position;
        calculatedPosition.y += cos(elapsedTime + billboardOffsets[i]) * 0.01f;
        instanceTransforms[i].position = calculatedPosition;
        lights[i]->Position = calculatedPosition;
    }

}

Firefly::Firefly(const Params& params)
    : InstancedEntity(params)
    , billboard{params.billboard}
    , color{params.color}
    , coefficients{params.coefficients}
{
    billboardOffsets.resize(instanceTransforms.size());
    for (int i = 0; i < instanceTransforms.size(); ++i)
    {
        billboardOffsets[i] = rand() % 1000 / 1000.0f * 3.14f;
    }
}

void Firefly::addLights(SceneData* sceneData)
{
    //add lights to the scene
    for (int i = 0; i < instanceTransforms.size(); ++i)
    {
        instanceTransforms[i].position;
        PointLight light(instanceTransforms[i].position, color, coefficients, true);
        sceneData->pointLights.push_back(light);
        //size is reserved at the start of the scene, so we can have a pointer to the light
        lights.push_back(&sceneData->pointLights[sceneData->pointLights.size() - 1]);
    }
}

std::string Firefly::GetJsonType()
{
    return "firefly";
}
}
