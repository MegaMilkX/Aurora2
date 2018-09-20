#ifndef SKIN_H
#define SKIN_H

#include <skeleton.h>
#include <transform.h>
#include <scene_object.h>

class Skin : public SceneObject::Component {
    CLONEABLE(Skin)
    RTTR_ENABLE(SceneObject::Component)
public:
    Skin() {

    }

    void SetSkeleton(resource_ref<Skeleton>& skel) { 
        skeleton = skel;
        skeletonDirty = true;
    }
    resource_ref<Skeleton>& GetSkeleton() {
        return skeleton;
    }

    void SetArmatureRoot(std::weak_ptr<SceneObject> obj) {
        armatureRoot = obj;
    }
    std::weak_ptr<SceneObject> GetArmatureRoot() {
        return armatureRoot;
    }

    const std::vector<gfxm::mat4>& Update() {
        if(skeletonDirty) {
            if(skeleton) {
                transformObjects.clear();
                transforms.clear(); 
                for(auto& b : skeleton.bones) {
                    SceneObject* so = Object()->FindObject(b);
                    if(so) {
                        transformObjects.emplace_back(so->Get<Transform>());
                    } else {
                        transformObjects.emplace_back(Get<Transform>());
                    }
                    transforms.emplace_back(gfxm::mat4(1.0f));
                }
            }
            skeletonDirty = false;
        }

        for(size_t i = 0; i < transformObjects.size(); ++i) {
            transforms[i] = transformObjects[i]->GetTransform();
        }

        return transforms;
    }

    virtual void OnInit() {
    }
private:
    resource_ref<Skeleton> skeleton;
    std::weak_ptr<SceneObject> armatureRoot;
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
        )
        .property(
            "armature", &Skin::GetArmatureRoot, &Skin::SetArmatureRoot
        );
}

#endif
