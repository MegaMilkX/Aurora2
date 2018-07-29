#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "math.h"

namespace Au{
namespace Math{

class Transform
{
public:
    Transform();

    void Translate(float x, float y, float z);
    void Translate(Vec3f trans);
    void Rotate(float angle, float x, float y, float z);
    void Rotate(float angle, Vec3f axis);
    void Scale(float x, float y, float z);
    void Scale(Vec3f scale);
    void Scale(float scale);

    Vec3f Position();
    Quat Rotation();
    Vec3f Scale();

    void Position(const Vec3f& position);
    void Rotation(const Quat& rotation);

    Mat4f GetTransform();
private:
    Vec3f position;
    Quat  rotation;
    Vec3f scale;
    Mat4f transform;
};

}
}

#endif
