#include "transform.h"

namespace Au{
namespace Math{

Transform::Transform()
: transform(1.0f),
position(0.0f, 0.0f, 0.0f),
rotation(0.0f, 0.0f, 0.0f, 1.0f),
scale(1.0f, 1.0f, 1.0f)
{

}

void Transform::Translate(float x, float y, float z)
{
    Translate(Vec3f(x, y, z));
}

void Transform::Translate(Vec3f trans)
{
    position = position + trans;
    /*
    if (space == LOCAL)
    {
        trans = toMat3(rotation) * trans;
        position = position + trans;
    }
    else if (space == PARENT)
    {
        
    }
    else if (space == WORLD)
    {

    }*/
}

void Transform::Rotate(float angle, float x, float y, float z)
{
    Rotate(angle, Vec3f(x, y, z));
}

void Transform::Rotate(float angle, Vec3f axis)
{
    Quat q = AngleAxis(angle, axis);
    rotation = q * rotation;
    /*
    if (space == LOCAL)
    {
        
    }
    else if (space == PARENT)
    {

    }
    else if (space == WORLD)
    {
        Quat q = angleAxis(angle, axis);
        rotation = rotation * q;
    }*/
    
    rotation = Normalize(rotation);
}

void Transform::Scale(float x, float y, float z)
{
    scale = Vec3f(x, y, z);
}

void Transform::Scale(Vec3f scale)
{
    this->scale = scale;
}

void Transform::Scale(float scale)
{
    this->scale = Vec3f(scale, scale, scale);
}

Vec3f Transform::Position() { return position; }
Quat Transform::Rotation() { return rotation; }
Vec3f Transform::Scale() { return scale; }

void Transform::Position(const Vec3f& position) { this->position = position; }
void Transform::Rotation(const Quat& rotation) { this->rotation = rotation; }

Mat4f Transform::GetTransform()
{
    transform = Mat4f(1.0f);
    return transform = 
        Math::Translate(transform, position) * 
        Math::ToMat4(rotation) * 
        Math::Scale(transform, scale);
}

}
}

