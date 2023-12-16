#pragma once
#include <memory>

#include "Core/Math/Quad.h"
#include "Core/Math/Transform2D.h"
#include "Rendering/Buffers/D3D11Buffer.h"
#include "Rendering/Shaders/EffectsShader.h"

namespace Snail
{
class UIElement
{
    mutable D3D11Buffer positionBuffer;

public:
    enum class SizeType
    {
        TYPE_ABSOLUTE   = 0,
        TYPE_PERCENTAGE = 1
    };

    enum class Alignment
    {
        MIDDLE = 0,
        START,
        END,
    };

    struct Params
    {
        // Position is always relative to (bottom left corner of screen)
        // right = x+, up = y+, left = x-, down = y-
        Transform2D transform{};

        SizeType sizeType = SizeType::TYPE_ABSOLUTE;

        // Size is either the size in screen space pixels or in percentage depending on sizeType
        Vector2 size{};

        // Local alignment determins where your own center point is
        // START = near 0 (bottom , MIDDLE = middle of shape, END = end of shape
        // o--o--o
        // |  |  |
        // o--o--x
        // |  |  |
        // o--o--o
        // Here x would be vertical anchor = MIDDLE, horizontal anchor = END
        Alignment localVerticalAnchor = Alignment::START;
        Alignment localHorizontalAnchor = Alignment::START;

        // Parent alignment anchor means your position relative to the parents origin
        // START = near 0 (bottom , MIDDLE = middle of shape, END = end of shape
        // Same as above
        Alignment parentVerticalAnchor = Alignment::START;
        Alignment parentHorizontalAnchor = Alignment::START;

        int zOrder = 0;
        std::string name{};
    };

protected:
    mutable struct UIElementPosition
    {
        Matrix toNormalizedSpace;
        Quad corners;
    } positionData;

    bool isActive = true;

    Transform2D transform;
    Vector2 originalSize, size;

    Alignment parentVerticalAnchor, parentHorizontalAnchor;
    Alignment localVerticalAnchor, localHorizontalAnchor;
    SizeType sizeType;


    UIElement* parent{};
    std::vector<std::unique_ptr<UIElement>> childElements;

    std::unique_ptr<EffectsShader> drawEffect;

    void ApplyLocalOffset(Vector2& transformOut, Alignment vertAlignment, Alignment horizAlignment) const;
    void ApplyParentOffset(Transform2D& transformOut, Alignment vertAlignment, Alignment horizAlignment, const Vector2& parentSize) const;
    Vector2 GetParentSize() const;

    virtual void PrepareDraw() const;
    virtual void DrawGeometry() const;

public:
    UIElement(const Params& params = {});
    UIElement(UIElement&& sprite) = default;
    UIElement& operator=(UIElement&& sprite) = default;
    virtual ~UIElement() = default;
    void Rotate(float dr);
    virtual void Update(float);
    virtual void InitShaders();
    void SetActive(bool isActive);

    void SetSize(const Vector2& newSize);
    Vector2 GetSize() const;
    Vector2 GetOriginalSize() const { return originalSize; };

    void Draw() const;

    Transform2D GetScreenTransform() const;
    Matrix GetTransformMatrix() const;

    Quad GetScreenSpaceQuad() const;

    UIElement* AddChild(std::unique_ptr<UIElement> element);
    void RemoveChild(UIElement* element);

    int zOrder;
    std::string name;

    template <class T> requires std::is_base_of_v<UIElement, T>
    static T Create(const typename T::Params& params = {})
    {
        T sprite{params};
        sprite.InitShaders();
        return sprite;
    }

    template <class T> requires std::is_base_of_v<UIElement, T>
    static std::unique_ptr<T> CreatePtr(const typename T::Params& params = {})
    {
        auto ptr = std::make_unique<T>(params);
        ptr->InitShaders();
        return std::move(ptr);
    }
};
}
