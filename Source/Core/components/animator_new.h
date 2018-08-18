#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <scene_object.h>
#include <updatable.h>

class AnimPose {};
class AnimCursor {};

class Animator : public Updatable
{
    CLONEABLE(Animator)
    RTTR_ENABLE(Updatable)
public:
    void Play(const std::string& name);
    void CrossBlend(const std::string& to, float t);
    AnimCursor CreateCursor(const std::string& anim);

    // Find all objects that can be animated
    void Reattach();

    virtual void OnUpdate();
};

class SkeletonMotion : public Updatable
{
public:

};

class SpriteMotion : public Updatable
{
public:
    
};

#endif
