#include "transform.h"

#include <algorithm>

typedef gfxm::vec4 vec4;
typedef gfxm::vec3 vec3;
typedef gfxm::vec2 vec2;
typedef gfxm::mat4 mat4;
typedef gfxm::mat3 mat3;
typedef gfxm::quat quat;

void Transform::Dirty()
{
    dirty = true;
    for(auto c : _children)
    {
        c->Dirty();
    }
}

void Transform::Translate(float x, float y, float z)
{ Translate(gfxm::vec3(x, y, z)); }
void Transform::Translate(const gfxm::vec3& vec)
{
    _position = _position + vec;
    Dirty();
}
void Transform::Rotate(float angle, float axisX, float axisY, float axisZ)
{ Rotate(angle, gfxm::vec3(axisX, axisY, axisZ)); }
void Transform::Rotate(float angle, const gfxm::vec3& axis)
{
    Rotate(gfxm::angle_axis(angle, axis));
}

void Transform::Rotate(const gfxm::quat& q)
{
    _rotation = 
        gfxm::normalize(
            q * 
            _rotation
        );
    Dirty();
}

void Transform::LookDir(const gfxm::vec3& dir, const gfxm::vec3& forward, const gfxm::vec3& up, float f)
{
    LookAt(WorldPosition() - dir, forward, up, f);
}

void Transform::LookAt(const gfxm::vec3& target, const gfxm::vec3& forward, const gfxm::vec3& up, float f)
{
    Dirty();
	f = std::max(-1.0f, std::min(f, 1.0f));
	
    Transform* trans = GetObject()->GetComponent<Transform>();
    gfxm::mat4 mat = trans->GetTransform();
    gfxm::vec3 pos = mat[3];
    
    gfxm::vec3 newFwdUnit = gfxm::normalize(target - pos);
    gfxm::vec3 rotAxis = gfxm::normalize(gfxm::cross(forward, newFwdUnit));
    
    gfxm::quat q;
    float dot = gfxm::dot(forward, newFwdUnit);
	
    const float eps = 0.01f;
    if(fabs(dot + 1.0f) <= eps)
    {
        q = gfxm::angle_axis(gfxm::pi * f, trans->Up());
    }/*
    else if(fabs(dot - 1.0f) <= eps)
    {
        q = gfxm::quat(0.0f, 0.0f, 0.0f, 1.0f);
    }*/
    else
	{
        float rotAngle = acosf(std::max(-1.0f, std::min(dot, 1.0f))) * f;
        q = gfxm::angle_axis(rotAngle, rotAxis);
    }
    
    trans->Rotate(q);
}

void Transform::LookAtChain(const gfxm::vec3& target, const gfxm::vec3& forward, const gfxm::vec3& up, float f, int chain)
{
    float fStep = f / (float)chain;
    if(chain<0)
        return;
    chain--;
    if(_parent)
    {
       _parent->LookAtChain(target, forward, up, fStep, chain);
    }
    
    LookAt(target, forward, up, fStep);
}

void Transform::Track(const gfxm::vec3& target, float f)
{
    Track(target, Forward(), f);
}

void Transform::Track(const gfxm::vec3& target, const gfxm::vec3& forward, float f)
{
    vec3 vforward = gfxm::normalize(forward);
    vec3 vtarget = gfxm::normalize(target - WorldPosition());
    vec3 rotationAxis = gfxm::normalize(gfxm::cross(vforward, vtarget));
    rotationAxis = gfxm::inverse(GetTransform()) * vec4(rotationAxis.x, rotationAxis.y, rotationAxis.z, 0.0f);
    float dot = gfxm::dot(vforward, vtarget);
    float angle = (float)acos(std::max(-1.0f, std::min(dot, 1.0f)));
    
    quat q = gfxm::angle_axis(angle, rotationAxis);
    Rotate(q);
}

void IKCollectChain(Transform* t, std::vector<Transform*>& r, int chainLength)
{
    if(!t)
        return;
    if(chainLength > 0)
    {
        r.push_back(t);
        chainLength--;
        IKCollectChain(t->ParentTransform(), r, chainLength);
    }
}

void Transform::IKChain(const gfxm::vec3& target, int chain)
{
    std::vector<Transform*> bones;
    IKCollectChain(this, bones, chain);
    if(bones.empty())
        return;
    Transform* origin = bones.back();
    Transform* end = bones.front();
    
    vec3 vforward = gfxm::normalize(end->WorldPosition() - origin->WorldPosition());
    vec3 vtarget = gfxm::normalize(target - origin->WorldPosition());
    vec3 rotationAxis = gfxm::normalize(gfxm::cross(vforward, vtarget));
    rotationAxis = gfxm::inverse(origin->GetTransform()) * vec4(rotationAxis.x, rotationAxis.y, rotationAxis.z, 0.0f);
    float dot = gfxm::dot(vforward, vtarget);
    float angle = (float)acos(std::max(-1.0f, std::min(dot, 1.0f)));
    quat q = gfxm::angle_axis(angle, rotationAxis);
    origin->Rotate(q);
}

struct ChainBone
{
    vec3 pos;
    vec3 dir;
    float length;
    Transform* t;
};

void IKCollectBones(Transform* t, std::vector<ChainBone>& r, int chainLength)
{
    if(!t)
        return;
    if(chainLength > 0)
    {
        ChainBone bone;
        bone.pos = t->WorldPosition();
        if(r.empty())
            bone.length = 1.0f;
        else
        {
            bone.length = gfxm::length((r.back().pos - bone.pos));
            bone.dir = gfxm::normalize(bone.pos - r.back().pos);
        }
        bone.t = t;
        r.push_back(bone);
        chainLength--;
        IKCollectBones(t->ParentTransform(), r, chainLength);
    }
}

void FABRIK_impl(std::vector<ChainBone>& bones, const gfxm::vec3& target)
{
    vec3 tgt = target;
    for(unsigned i = 1; i < bones.size() - 1; ++i)
    {
        ChainBone& b = bones[i];
        ChainBone& next = bones[i + 1];
        
        b.pos = tgt;
        tgt = tgt + gfxm::normalize(next.pos - b.pos) * next.length;
    }
    
    tgt = bones.back().pos;
    for(int i = (int)bones.size() - 2; i >= 1; --i)
    {
        ChainBone& b = bones[i];
        ChainBone& next = bones[i - 1];
        
        b.pos = tgt;
        tgt = tgt + gfxm::normalize(b.pos - next.pos) * b.length;
    }
}

void Transform::FABRIK(const gfxm::vec3& target, int chainLength)
{
    std::vector<ChainBone> bones;
    IKCollectBones(this, bones, chainLength);
    if(bones.empty())
        return;
    
    ChainBone b;
    b.pos = target;
    b.length = 1.0f;
    bones.insert(bones.begin(), b);
    b.pos = bones.back().pos;
    b.length = gfxm::length(bones.back().pos);
    bones.push_back(b);
    
    for(unsigned i = 0; i < 5; ++i)
    {
        FABRIK_impl(bones, target);
    }
    
    for(int i = (int)bones.size() - 2; i >= 1; --i)
    {
        ChainBone& b = bones[i];
        ChainBone& next = bones[i - 1];
        b.t->Track(next.pos, b.dir);
    }
}

void Transform::Position(float x, float y, float z)
{ Position(gfxm::vec3(x, y, z)); }
void Transform::Position(const gfxm::vec3& position)
{ _position = position; Dirty(); }
void Transform::Rotation(float x, float y, float z)
{ _rotation = gfxm::euler_to_quat(gfxm::vec3(x, y, z)); Dirty(); }
void Transform::Rotation(gfxm::vec3 euler)
{ _rotation = gfxm::euler_to_quat(gfxm::vec3(euler.x, euler.y, euler.z)); Dirty(); }
void Transform::Rotation(float x, float y, float z, float w)
{ Rotation(gfxm::quat(x, y, z, w)); }
void Transform::Rotation(const gfxm::quat& rotation)
{ _rotation = rotation; Dirty(); }

void Transform::Scale(float scale)
{ Scale(gfxm::vec3(scale, scale, scale)); }
void Transform::Scale(float x, float y, float z)
{ Scale(gfxm::vec3(x, y, z)); }
void Transform::Scale(const gfxm::vec3& scale)
{ _scale = scale; Dirty(); }
void Transform::ScaleIncrement(const gfxm::vec3& scale)
{
    _scale = _scale + scale;
}

gfxm::vec3 Transform::WorldPosition()
{
    return GetTransform()[3];
}
const gfxm::vec3& Transform::Position()
{ return _position; }
gfxm::quat Transform::WorldRotation()
{
    gfxm::mat3 m3 = gfxm::to_mat3(GetTransform());
    m3[0] /= gfxm::length(m3[0]);
    m3[1] /= gfxm::length(m3[1]);
    m3[2] /= gfxm::length(m3[2]);
    return gfxm::to_quat(m3);
}
const gfxm::quat& Transform::Rotation()
{ return _rotation; }
gfxm::vec3 Transform::RotationEuler()
{
    return gfxm::to_euler(_rotation);
}
const gfxm::vec3& Transform::Scale()
{ return _scale; }

gfxm::vec3 Transform::Right()
{ return GetTransform()[0]; }
gfxm::vec3 Transform::Up()
{ return GetTransform()[1]; }
gfxm::vec3 Transform::Back()
{ return GetTransform()[2]; }
gfxm::vec3 Transform::Left()
{ return -Right(); }
gfxm::vec3 Transform::Down()
{ return -Up(); }
gfxm::vec3 Transform::Forward()
{ return -Back(); }

gfxm::quat Transform::GetParentRotation()
{
    if(_parent)
        return gfxm::normalize(_parent->GetParentRotation() * _parent->Rotation());
    else
        return gfxm::quat(0.0f, 0.0f, 0.0f, 1.0f);
}

void Transform::SetTransform(gfxm::mat4 t)
{
    _position = gfxm::vec3(t[3].x, t[3].y, t[3].z);
    gfxm::mat3 rotMat = gfxm::to_orient_mat3(t);
    _rotation = gfxm::to_quat(rotMat);
    gfxm::vec3 right = t[0];
    gfxm::vec3 up = t[1];
    gfxm::vec3 back = t[2];
    _scale = gfxm::vec3(right.length(), up.length(), back.length());
    Dirty();
}

gfxm::mat4 Transform::GetLocalTransform() const
{
    return 
        gfxm::translate(gfxm::mat4(1.0f), _position) * 
        gfxm::to_mat4(_rotation) * 
        gfxm::scale(gfxm::mat4(1.0f), _scale);
}

gfxm::mat4 Transform::GetParentTransform()
{
    if(_parent)
        return _parent->GetTransform();
    else
        return gfxm::mat4(1.0f);
}

gfxm::mat4 Transform::GetTransform()
{
    if(dirty)
    {
        dirty = false;
        gfxm::mat4 localTransform = GetLocalTransform();           
        if(_parent)
            _transform = _parent->GetTransform() * localTransform;
        else
            _transform = localTransform;
    }
    return _transform;
}

gfxm::mat4 Transform::GetTransformForRoot(Transform* root) {
    gfxm::mat4 tr = GetTransform();
    if(root) {
        return gfxm::inverse(root->GetTransform()) * tr;
    } else {
        return GetTransform();
    }
}