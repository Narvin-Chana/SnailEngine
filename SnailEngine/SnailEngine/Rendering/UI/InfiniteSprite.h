#pragma once
#include "Sprite.h"

namespace Snail
{
class InfiniteSprite : public Sprite
{
    static constexpr const char* INFINITE_UV_MACRO = "INFINITE_UV";
public:
    InfiniteSprite(const Params& params = {});
    InfiniteSprite(const InfiniteSprite&) = delete;
    InfiniteSprite(InfiniteSprite&&) = default;

    void InitShaders() override;
};
}