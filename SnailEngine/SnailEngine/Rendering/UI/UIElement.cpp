#include "stdafx.h"
#include "UIElement.h"

#include "Core/WindowsEngine.h"

namespace Snail
{
UIElement::UIElement(const Params& params)
    : positionBuffer{D3D11Buffer::CreateConstantBuffer<UIElementPosition>()}
    , transform{params.transform}
    , originalSize{params.size}
    , parentVerticalAnchor{params.parentVerticalAnchor}
    , parentHorizontalAnchor{params.parentHorizontalAnchor}
    , localVerticalAnchor(params.localVerticalAnchor)
    , localHorizontalAnchor(params.localHorizontalAnchor)
    , sizeType{params.sizeType}
    , zOrder{params.zOrder}
    , name{params.name}
{}

void UIElement::Update(float dt)
{
    if (!isActive)
        return;

    if (sizeType == SizeType::TYPE_PERCENTAGE)
        size = GetParentSize() * originalSize;
    else
        size = originalSize;

    std::ranges::for_each(childElements,
        [dt](const auto& elem)
        {
            elem->Update(dt);
        });
}

void UIElement::InitShaders() {}

void UIElement::SetActive(const bool _isActive)
{
    isActive = _isActive;
}

void UIElement::SetSize(const Vector2& newSize)
{
    transform.scale = newSize;
}
Vector2 UIElement::GetSize() const { return transform.scale; }

void UIElement::PrepareDraw() const
{
    if (!drawEffect)
        return;

    static auto* device = WindowsEngine::GetInstance().GetRenderDevice();
    const DirectX::XMINT2 screenSize = device->GetWindowedResolution();

    positionData.toNormalizedSpace = (Matrix::CreateScale({2.0f / screenSize.x, 2.0f / screenSize.y, 0}) * Matrix::CreateTranslation({-1, -1, 0})).
        Transpose();
    positionData.corners = GetScreenSpaceQuad();
    positionBuffer.UpdateData(positionData);

    drawEffect->SetConstantBuffer("PositionInfo", positionBuffer.GetBuffer());
    drawEffect->Bind();
}

void UIElement::DrawGeometry() const
{
    static auto* device = WindowsEngine::GetInstance().GetRenderDevice();
    device->GetImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void UIElement::Rotate(const float dr)
{
    transform.rotation += dr;
}

void UIElement::Draw() const
{
    PrepareDraw();
    DrawGeometry();

    std::ranges::for_each(childElements,
        [](const auto& elem)
        {
            elem->Draw();
        });
}

Transform2D UIElement::GetScreenTransform() const
{
    Transform2D localTransform = transform;
    if (parent)
    {
        Vector2 pos{};
        parent->ApplyLocalOffset(pos, parent->localVerticalAnchor, parent->localHorizontalAnchor);
        localTransform.position += pos;
    }
    ApplyParentOffset(localTransform, parentVerticalAnchor, parentHorizontalAnchor, GetParentSize());
    return localTransform;
}

void UIElement::ApplyLocalOffset(Vector2& transformOut, Alignment vertAlignment, Alignment horizAlignment) const
{
    switch (vertAlignment)
    {
    case Alignment::MIDDLE:
        transformOut.y -= size.y / 2.0f;
        break;
    case Alignment::END:
        transformOut.y -= size.y;
        break;
    default: ;
    }

    switch (horizAlignment)
    {
    case Alignment::MIDDLE:
        transformOut.x -= size.x / 2.0f;
        break;
    case Alignment::END:
        transformOut.x -= size.x;
        break;
    default: ;
    }
}

void UIElement::ApplyParentOffset(Transform2D& transformOut, Alignment vertAlignment, Alignment horizAlignment, const Vector2& parentSize) const
{
    switch (vertAlignment)
    {
    case Alignment::MIDDLE:
        transformOut.position.y += parentSize.y / 2.0f;
        break;
    case Alignment::END:
        transformOut.position.y += parentSize.y;
        break;
    default: ;
    }

    switch (horizAlignment)
    {
    case Alignment::MIDDLE:
        transformOut.position.x += parentSize.x / 2.0f;
        break;
    case Alignment::END:
        transformOut.position.x += parentSize.x;
        break;
    default: ;
    }
}

Vector2 UIElement::GetParentSize() const
{
    if (!parent)
    {
        static auto* device = WindowsEngine::GetInstance().GetRenderDevice();
        const DirectX::XMINT2 screenSize = device->GetWindowedResolution();
        return Vector2((float)screenSize.x, (float)screenSize.y);
    }

    return parent->size;
}

Matrix UIElement::GetTransformMatrix() const
{
    const Transform2D localTransform = GetScreenTransform();
    if (!parent)
        return localTransform.GetTransformMatrix();

    return localTransform.GetTransformMatrix() * parent->GetTransformMatrix();
}

Quad UIElement::GetScreenSpaceQuad() const
{
    const auto toScreenSpace = GetTransformMatrix();

    Vector2 p0{0, 0};
    ApplyLocalOffset(p0, localVerticalAnchor, localHorizontalAnchor);

    Vector2 p1{size.x, 0};
    ApplyLocalOffset(p1, localVerticalAnchor, localHorizontalAnchor);

    Vector2 p2{0, size.y};
    ApplyLocalOffset(p2, localVerticalAnchor, localHorizontalAnchor);

    Vector2 p3{size.x, size.y};
    ApplyLocalOffset(p3, localVerticalAnchor, localHorizontalAnchor);

    Quad corners;
    corners.topLeft = Vector2::Transform(p0, toScreenSpace);
    corners.topRight = Vector2::Transform(p1, toScreenSpace);
    corners.botLeft = Vector2::Transform(p2, toScreenSpace);
    corners.botRight = Vector2::Transform(p3, toScreenSpace);
    return corners;
}

UIElement* UIElement::AddChild(std::unique_ptr<UIElement> element)
{
    childElements.push_back(std::move(element));
    UIElement* added = childElements.back().get();
    added->parent = this;
    return added;
}

void UIElement::RemoveChild(UIElement* element)
{
    std::erase_if(childElements, [element](const auto& ptr)
    {
        return ptr.get() == element;
    });
}

}
