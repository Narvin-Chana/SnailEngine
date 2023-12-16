#include "stdafx.h"
#include "RapidObjUtil.h"

#include "Core/WindowsEngine.h"

namespace Snail
{

TexturedMaterial ConvertRapidObjMatToTexturedMaterial(const rapidobj::Material& rapidMat, const std::string& pathPrefix, bool isPersistent)
{
    static TextureManager& tm = WindowsEngine::GetModule<TextureManager>();

    TexturedMaterial mat{};
    // General material parameters
    mat.material.ambient = Vector3{rapidMat.ambient[0], rapidMat.ambient[1], rapidMat.ambient[2]};
    mat.material.diffuse = Vector3{rapidMat.diffuse[0], rapidMat.diffuse[1], rapidMat.diffuse[2]};

    // Default diffuse at 1 if is 0 to avoid black textures
    if (mat.material.diffuse == Vector3::Zero)
    {
        mat.material.diffuse = Vector3::One;
    }

    mat.material.specular = Vector3{rapidMat.specular[0], rapidMat.specular[1], rapidMat.specular[2]};
    mat.material.shininess = rapidMat.shininess;
    mat.material.emission = Vector3{rapidMat.emission[0], rapidMat.emission[1], rapidMat.emission[2]};

    // Import textures if they exist
    if (!rapidMat.ambient_texname.empty())
    {
        mat.ambientTexture = pathPrefix + rapidMat.ambient_texname;
        if (!tm.DoesAssetExist(mat.ambientTexture))
        {
            tm.SaveAsset<Texture2D>(mat.ambientTexture, isPersistent);
        }
    }

    if (!rapidMat.diffuse_texname.empty())
    {
        // Avoid duplicate diffuses
        mat.diffuseTexture = pathPrefix + rapidMat.diffuse_texname;
        if (!tm.DoesAssetExist(mat.diffuseTexture))
        {
            tm.SaveAsset<Texture2D>(mat.diffuseTexture, isPersistent);
        }
    }

    if (!rapidMat.specular_texname.empty())
    {
        mat.specularTexture = pathPrefix + rapidMat.specular_texname;
        if (!tm.DoesAssetExist(mat.specularTexture))
        {
            tm.SaveAsset<Texture2D>(mat.specularTexture, isPersistent);
        }
    }

    if (!rapidMat.normal_texname.empty())
    {
        mat.normalMapTexture = pathPrefix + rapidMat.normal_texname;
        if (!tm.DoesAssetExist(mat.normalMapTexture))
        {
            tm.SaveAsset<Texture2D>(mat.normalMapTexture, isPersistent);
        }
    }

    // Add other material parameters if needed -> https://github.com/guybrush77/rapidobj#materials
    // Other features that would be needed are :
    // - Bump Mapping
    // - Alpha Mapping
    // - Displacement Mapping

    return mat;
}

}
