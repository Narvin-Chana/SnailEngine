#include "stdafx.h"
#include "TextureManager.h"

#include "Util/Util.h"

namespace Snail
{

template <class T> requires std::is_base_of_v<Texture, T>
T* TextureManager::SaveAsset(const std::string& filename, bool isPersistent)
{
    return SaveAsset(filename, std::make_unique<T>(filename), isPersistent);
}

template <class T> requires std::is_base_of_v<Texture, T>
T* TextureManager::SaveAsset(const std::string& filename, std::unique_ptr<T>&& asset, bool isPersistent)
{
    std::unique_lock lock(saveMutex);
    assetCache[filename] = { isPersistent, std::move(asset) };
    LOGF("Saved texture \"{}\" to cache", filename);
    return dynamic_cast<T*>(assetCache[filename].asset.get());
}

Texture2D* TextureManager::GetTexture2D(const std::wstring& str, const bool isPersistent) { return GetTexture2D(WStringToString(str), isPersistent); }

void TextureManager::Init()
{
    // Initialise default textures

    assetCache[DEFAULT_DIFFUSE_TEXTURE_NAME] = {
       .isPersistent = true,
       .asset = std::make_unique<Texture2D>(1, 1, Color{ 0xff, 0xff, 0xff, 0xff })
    };

    assetCache[DEFAULT_BLEND_TEXTURE_NAME] = {
       .isPersistent = true,
       .asset = std::make_unique<Texture2D>(1, 1, Color{ 0xff, 0xff, 0xff, 0xff })
    };

    assetCache[DEFAULT_AMBIENT_TEXTURE_NAME] = {
       .isPersistent = true,
       .asset = std::make_unique<Texture2D>(1, 1, Color{ 0xff, 0xff, 0xff, 0xff })
    };

    assetCache[DEFAULT_SPECULAR_TEXTURE_NAME] = {
        .isPersistent = true,
        .asset = std::make_unique<Texture2D>(1, 1, Color{ 0xff, 0xff, 0xff, 0xff })
    };

    assetCache[DEFAULT_NORMAL_MAP_TEXTURE_NAME] = {
        .isPersistent = true,
        .asset = std::make_unique<Texture2D>(1, 1, Color{ 0x80, 0x80, 0xff, 0xff })
    };
}

Texture2D* TextureManager::GetTexture2D(const std::string& str, const bool isPersistent)
{
    // Get texture from cache
    auto texture = GetAsset<Texture2D>(str, isPersistent);
    if (!texture)
    {
        // Create texture by importing
        texture = SaveAsset<Texture2D>(str, isPersistent);
    }

    return texture;
}

TextureCube* TextureManager::GetTextureCube(const std::wstring& str, const bool isPersistent) { return GetTextureCube(WStringToString(str), isPersistent); }

void TextureManager::RenderImGui()
{
#ifdef _IMGUI_
    if (ImGui::CollapsingHeader("TextureManager"))
    {
        for (const auto& [textureName, texture] : assetCache)
        {
            if (ImGui::TreeNode(textureName.c_str()))
            {
                texture.asset->RenderImGui();
                ImGui::TreePop();
            }
        }
    }
#endif
}

TextureCube* TextureManager::GetTextureCube(const std::string& str, const bool isPersistent)
{
    // Get texture from cache
    TextureCube* texture = GetAsset<TextureCube>(str, isPersistent);
    if (!texture)
    {
        // Create texture by importing
        texture = SaveAsset<TextureCube>(str, isPersistent);
    }

    return texture;
}
}
