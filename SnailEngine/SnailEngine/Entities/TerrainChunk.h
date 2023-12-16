#pragma once
#include "Entity.h"

namespace Snail
{
class Terrain;

class TerrainChunk : public Entity
{
public:
    struct Params : Entity::Params
    {
        Vector3 chunkSize;
    };

    Vector3 chunkSize;

    TerrainChunk(const Params& params);
    void InitPhysics() override;

    void PrepareShadows() override;
};
}
