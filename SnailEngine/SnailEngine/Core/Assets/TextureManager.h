#pragma once
#include <mutex>
#include <string>

#include "ModuleManager.h"
#include "Rendering/Texture2D.h"
#include "Rendering/TextureCube.h"

namespace Snail
{

class TextureManager : public GenericAssetManager<Texture>
{
    std::mutex saveMutex;

    // Don't use standard GenericAssetManager::SaveToCache
    Texture* SaveAsset(const std::string&, bool) override
    {
        LOG(Logger::FATAL, "Abstract load to cache should not be called.");
// release complains that there is unreachable code and debug complains that the function doesnt return anything
#ifdef _DEBUG
        return nullptr;
#endif
    }
public:
    static inline const std::string DEFAULT_DIFFUSE_TEXTURE_NAME = "DefaultDiffuse";
    static inline const std::string DEFAULT_BLEND_TEXTURE_NAME = "DefaultBlend";
    static inline const std::string DEFAULT_AMBIENT_TEXTURE_NAME = "DefaultAmbient";
    static inline const std::string DEFAULT_SPECULAR_TEXTURE_NAME = "DefaultSpecular";
    static inline const std::string DEFAULT_NORMAL_MAP_TEXTURE_NAME = "DefaultNormalMap";

    void Init() override;

    // Templated one allows to generate Cubemap or Texture2D
    template<class T> requires std::is_base_of_v<Texture, T>
    T* SaveAsset(const std::string& filename, bool isPersistent = false);

    // Templated one allows to generate Cubemap or Texture2D
    template<class T> requires std::is_base_of_v<Texture, T>
    T* SaveAsset(const std::string& filename, std::unique_ptr<T>&& asset, bool isPersistent = false);

    Texture2D* GetTexture2D(const std::string& str, bool isPersistent = false);
    Texture2D* GetTexture2D(const std::wstring& str, bool isPersistent = false);

    TextureCube* GetTextureCube(const std::string& str, bool isPersistent = false);
    TextureCube* GetTextureCube(const std::wstring& str, bool isPersistent = false);

    void RenderImGui() override;
};
}
