#include "stdafx.h"
#include "Transform2D.h"

namespace Snail
{
Matrix Transform2D::GetTransformMatrix() const
{
    Matrix transformation;
    transformation *= Matrix::CreateScale(Vector3{scale.x, scale.y, 0});
    transformation *= Matrix::CreateRotationZ(rotation);
    transformation *= Matrix::CreateTranslation(Vector3{ position.x, position.y, 0});
    return transformation;
}
}
