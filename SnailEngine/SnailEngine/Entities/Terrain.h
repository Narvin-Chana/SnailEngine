#pragma once

#include "Entity.h"
#include "TerrainChunk.h"
#include "Core/Mesh/TerrainMesh.h"

namespace Snail
{

class Terrain : public Entity
{
    TerrainMesh* GenerateChunkMesh(int chunkX, int chunkY, const TerrainMesh* tmesh) const;
    void GenerateChunks(const TerrainMesh* sourceMesh);
public:
    std::vector<std::unique_ptr<TerrainChunk>> chunks;
    Vector2 chunkCount;

    struct Params : Entity::Params
    {
        Params();
        Vector2 chunkSize;
    };

    Terrain(const Params& params);

    void Draw(DrawContext& ctx) override;
    void RenderImGui(int idNumber) override;
};

};
