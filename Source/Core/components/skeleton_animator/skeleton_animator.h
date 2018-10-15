#ifndef SKELETON_ANIMATOR_H
#define SKELETON_ANIMATOR_H

#include <updatable.h>

#include "skeleton_motor.h"

class SkeletonAnimator : public Updatable {
    RTTR_ENABLE(Updatable)
public:
    SkeletonAnimator() {}

    size_t LayerCount() const;
    void GetLayer(size_t i);
    void AddLayer();
    void RemoveLayer(size_t i);
    

    virtual void OnUpdate() {
        
    }
    void AddAnim(const std::string& name, const std::string& resource) {
        Animation* anim = ResourceRef(resource).Get<Animation>();
        SkeletonMotor& motor = motors[name];
        motor.Build(Object(), anim);
    }
private:
    std::map<std::string, SkeletonMotor> motors;
};
STATIC_RUN(SkeletonAnimator) {
    rttr::registration::class_<SkeletonAnimator>("SkeletonAnimator")
        .constructor<>()(
            rttr::policy::ctor::as_raw_ptr
        );
}

#endif
