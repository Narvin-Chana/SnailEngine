#pragma once
#include <type_traits>
#include <cstddef>
#include <tuple>
#include <string>
#include <memory>
#include <ranges>
#include <unordered_map>

namespace Snail
{
template <class T>
class GenericAssetManager
{
public:

    struct AssetCacheEntry
    {
        bool isPersistent;
        std::unique_ptr<T> asset;
    };

protected:
    std::mutex assetManagerMutex;
    std::unordered_map<std::string, AssetCacheEntry> assetCache;

public:
    GenericAssetManager() = default;
    virtual ~GenericAssetManager() = default;

    virtual void Init() = 0;

    template<class TChild> requires std::is_base_of_v<T, TChild>
    TChild* GetAsset(const std::string& assetName, const bool isPersistent = false)
    {
        auto it = assetCache.find(assetName);
        if (it != assetCache.end())
        {
            if (isPersistent)
                it->second.isPersistent = true;

            return dynamic_cast<TChild*>(it->second.asset.get());
        }

        return nullptr;
    }

    virtual bool DoesAssetExist(const std::string& name)
    {
        return assetCache.contains(name);
    }

    virtual bool IsAssetPersistent(const std::string& name)
    {
        if (!DoesAssetExist(name))
            return false;

        return assetCache[name].isPersistent;
    }

    virtual T* SaveAsset(const std::string& filename, bool isPersistent) = 0;

    virtual void DeleteAsset(const std::string& name)
    {
        assetCache.erase(name);
    }

    // This cleans up all assets that are not persistent
    virtual void SoftCleanup()
    {
        std::erase_if(assetCache, [](const auto& val){
            return !val.second.isPersistent;
        });
    }

    virtual void Cleanup()
    {
        assetCache.clear();
    }

    virtual std::vector<T*> GetAllAssets() const;

    virtual void RenderImGui() = 0;
};

template <class T>
std::vector<T*> GenericAssetManager<T>::GetAllAssets() const
{
    std::vector<T*> assets;
    assets.reserve(assetCache.size());
    for (auto& entry : assetCache)
        assets.push_back(entry.second.asset.get());
    return assets;
}

// Template to get AssetManager's variadic tuple index of the passed type T
template <int N, typename T, typename... Ts>
struct Index;

template <int N, class T>
struct Index<N, T> : std::integral_constant<int, -1>
{};

template <int N, typename T, typename... Ts>
struct Index<N, T, T, Ts...> : std::integral_constant<int, N>
{};

template <int N, typename T, typename U, typename... Ts>
struct Index<N, T, U, Ts...> : std::integral_constant<int, Index<N + 1, T, Ts...>::value>
{};

template <typename T, typename... Ts>
constexpr std::size_t IndexValue = Index<0, T, Ts...>::value;

template<class T>
concept InitializableModule = requires (T module)
{
    module.Init();
};

template <class... Ts>
class ModuleManager
{
    std::tuple<std::unique_ptr<Ts>...> instances;

    template <class T>
    std::unique_ptr<T>& GetInstanceUniquePtr()
    {
        static_assert(IndexValue<T, Ts...> != -1, "Type doesn't exist in the asset manager!");
        constexpr int index = IndexValue<T, Ts...>;
        return std::get<index>(instances);
    }

public:
    template <class T>
    T& Get()
    {
        return *GetInstanceUniquePtr<T>();
    }

    template <class T, typename... Args>
    void RegisterModule(Args&&... args)
    {
        auto& ptr = GetInstanceUniquePtr<T>();
        ptr = std::make_unique<T>(std::forward<Args>(args)...);
        if constexpr (InitializableModule<T>)
            ptr->Init();
    }
};
}
