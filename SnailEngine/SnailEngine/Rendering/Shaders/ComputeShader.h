#pragma once
#include <unordered_set>

#include "Shader.h"

namespace Snail
{
class Texture;

class ComputeShader : public Shader
{
    ID3D11ComputeShader* internalShader = nullptr;
    std::unordered_set<std::string> defines;

public:
    ComputeShader(const std::wstring& filename, const std::unordered_set<std::string>& defines = {});
    ~ComputeShader() override;
    void Bind() override;
    void Execute(int threadGroupCountX, int threadGroupCountY, int threadGroupCountZ);
    void Unbind();

    /**
     * \brief Binds a UAV to this CS.
     *
     * <b>DOES NOT MANAGE IF THERE ARE MORE THAN ONE UAV TO BIND!!</b>
     */
    void BindComputedUAV(ID3D11UnorderedAccessView* uav);
    void BindSRVAndSampler(int slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* samplerState);
    void BindSampler(int slot, ID3D11SamplerState* samplerState);
    void BindSRV(int slot, ID3D11ShaderResourceView* srv);
    void UnbindSRV(int slot);
};
}
