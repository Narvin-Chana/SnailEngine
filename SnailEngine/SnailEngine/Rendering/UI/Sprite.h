#pragma once
#include "UIElement.h"

namespace Snail
{

    class EffectsShader;
    class Texture2D;

    class Sprite : public UIElement
    {
    public:
        
        struct Params : UIElement::Params
        {
            Params();
            Texture2D* texture{};
        };

    protected:
        Texture2D* texture;

        void PrepareDraw() const override;
        void DrawGeometry() const override;
    public:
        Sprite(const Params& params = {});
        Sprite(const Sprite&) = delete;
        Sprite(Sprite&&) = default;

        Sprite& operator=(Sprite&&) = default;
        Sprite& operator=(const Sprite&) = delete;

        void SetTexture(Texture2D* tex);

        ~Sprite() override;
        void InitShaders() override;
    };
}
