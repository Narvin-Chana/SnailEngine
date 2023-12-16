#pragma once

#include "Core/Assets/TextureManager.h"
#include "Util/Util.h"

namespace Snail
{
struct Material
{
    DX_ALIGN Vector3 diffuse = {1.0f, 1.0f, 1.0f};
    DX_ALIGN Vector3 ambient = {0.3f, 0.3f, 0.3f};
    DX_ALIGN Vector3 emission = {0.0f, 0.0f, 0.0f};
    DX_ALIGN Vector3 specular = {1,1,1};
    float shininess = 16; // Specular exponent
    Vector2 uvScale = {1,1};
    Vector2 primaryBlendUvScale = {1,1};
    Vector2 secondaryBlendUvScale = { 1,1 };
    // Fill this in with any other needed material parameters
};

struct TexturedMaterial
{
    TexturedMaterial() = default;

    Material material;

    bool isBlending = false;

    // Textures
    std::string diffuseTexture = TextureManager::DEFAULT_DIFFUSE_TEXTURE_NAME;
	std::string primaryBlendDiffuseTexture = TextureManager::DEFAULT_DIFFUSE_TEXTURE_NAME;
	std::string primaryBlendTexture = TextureManager::DEFAULT_BLEND_TEXTURE_NAME;
    std::string secondaryBlendDiffuseTexture = TextureManager::DEFAULT_DIFFUSE_TEXTURE_NAME;
    std::string secondaryBlendTexture = TextureManager::DEFAULT_BLEND_TEXTURE_NAME;
    std::string ambientTexture = TextureManager::DEFAULT_AMBIENT_TEXTURE_NAME;
    std::string specularTexture = TextureManager::DEFAULT_SPECULAR_TEXTURE_NAME;
    std::string normalMapTexture = TextureManager::DEFAULT_NORMAL_MAP_TEXTURE_NAME;
};
}
