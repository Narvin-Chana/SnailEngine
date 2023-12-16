#pragma once

#include <bitset>
#include "Core/Math/Transform.h"
#include "Core/Mesh/Mesh.h"

namespace Snail
{
class DrawContext;
class PhysicsObject;
class EffectsShader;
class TriggerBox;

class Entity
{
public:
    struct Params
    {
        std::string name = "Entity";
        BaseMesh* mesh = nullptr;
        Transform transform = {};
        bool castsShadows = true;
        PhysicsObject* physicsObject = nullptr;
    };

    std::string entityName;

protected:
    Entity* parent = nullptr;
    BaseMesh* mesh = nullptr;

    /**
     * DirtyFlags field signification:
     * 0: the transform is dirty
     * 1: the bounding box is dirty
     * 2: the world transform is dirty
     * All fields should default to true for first lazy loading
     */
    std::bitset<3> dirtyFlags = std::bitset<3>().set();

    Transform transform;

    // TODO: Hierarchy: must also update the dirty flags of all children when this entity's dirtyFlag is toggled
    Transform worldTransform;
    Matrix worldTransformMatrix;

    DirectX::BoundingBox boundingBox;

    std::unique_ptr<PhysicsObject> physicsObject;

    bool castsShadows = false;
    bool shouldFrustumCull = true;

public:
    Entity(const Params& params = {}) noexcept;
    Entity(Entity&&) noexcept;
    Entity& operator=(Entity&&) noexcept;
    // Must be defined in cpp for smart pointers to work
    virtual ~Entity();

    virtual void InitPhysics();
    virtual void Update(float) noexcept;
    virtual void Draw(DrawContext& ctx);
    virtual void PrepareShadows();

    [[nodiscard]] Entity* GetParent() const noexcept;
    void SetParent(Entity* pParent) noexcept;

    virtual Transform GetWorldTransform();
    virtual const Transform& GetTransform() const;
    virtual void SetTransform(const Transform& t);
    virtual void SetPosition(const Vector3& pos);
    virtual void SetRotation(const Vector3& euler);
    virtual void SetRotation(const Quaternion& quat);
    virtual void SetScale(const Vector3& scale);

    [[nodiscard]] const BaseMesh* GetMesh() const noexcept;
    [[nodiscard]] const PhysicsObject* GetPhysicsObject() const noexcept;
    [[nodiscard]] virtual bool ShouldCastShadows() const noexcept;
    [[nodiscard]] virtual Matrix GetWorldTransformMatrix();
    [[nodiscard]] bool ShouldFrustumCull() const noexcept;
    [[nodiscard]] Vector3 GetExtents() const noexcept;
    [[nodiscard]] Vector3 GetBoundsLocalCenter() const noexcept;
    [[nodiscard]] const DirectX::BoundingBox& GetBoundingBox();

    template <class T> requires std::is_base_of_v<Entity, T>
    static std::unique_ptr<T> CreateObject(const typename T::Params& params = {})
    {
        T* ptr = new T{params};
        ptr->InitPhysics();
        LOGF("Object created with name {}", params.name);
        return std::unique_ptr<T>(ptr);
    }

    struct UserData
    {
        UserData(Entity* entity)
            : entity(entity) {}

        virtual ~UserData() = default;
        Entity* entity;

        virtual void OnTrigger(TriggerBox*)
        {
            LOG("Triggered");
        };
    };

    std::unique_ptr<UserData> userData = std::make_unique<UserData>(this);

    template <class T> requires std::is_base_of_v<Entity, T>
    static std::unique_ptr<T> CreateObject(const nlohmann::json& obj) { return CreateObject<T>(obj.get<typename T::Params>()); }

    virtual std::string GetJsonType();
    virtual void RenderImGui(int idNumber);
};
}
