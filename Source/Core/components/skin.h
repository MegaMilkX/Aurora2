#ifndef SKIN_H
#define SKIN_H

#include <skeleton.h>
#include <transform.h>
#include <scene_object.h>

class Skin : public SceneObject::Component {
    CLONEABLE(Skin)
    RTTR_ENABLE(SceneObject::Component)
public:
    void SetSkeleton(resource_ref<Skeleton>& skel) { 
        skeleton = skel;
        skeletonDirty = true;
    }
    resource_ref<Skeleton>& GetSkeleton() {
        return skeleton;
    }

    const std::vector<gfxm::mat4>& Update() {
        if(skeletonDirty) {
            // TODO: Rebuild transformObjects and transforms arrays

            skeletonDirty = false;
        }

        // TODO: Fill out transforms array

        return transforms;
    }

    virtual void OnInit() {}
private:
    resource_ref<Skeleton> skeleton;
    std::vector<Transform*> transformObjects;
    std::vector<gfxm::mat4> transforms;
    bool skeletonDirty = true;
};
STATIC_RUN(Skin)
{
    rttr::registration::class_<Skin>("Skin")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr)
        .property(
            "skeleton", &Skin::GetSkeleton, &Skin::SetSkeleton
        );
}

#endif
