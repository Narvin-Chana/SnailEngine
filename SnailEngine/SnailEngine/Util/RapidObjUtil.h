#pragma once
#include "rapidobj.hpp"
#include "Rendering/TexturedMaterial.h"

namespace Snail
{

TexturedMaterial ConvertRapidObjMatToTexturedMaterial(const rapidobj::Material& rapidMat, const std::string& pathPrefix, bool isPersistent);

}
