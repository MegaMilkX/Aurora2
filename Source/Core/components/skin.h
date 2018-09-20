#ifndef SKIN_H
#define SKIN_H

#include <skeleton.h>
#include <transform.h>
#include <scene_object.h>

class SceneObjectRef {
public:
    SceneObjectRef() {}
    SceneObjectRef(SceneObject* origin)
    : origin(origin) {}

    void Set(SceneObject* target) {
        if(!target) return;
        std::vector<SceneObject*> origin_parents;
        std::vector<SceneObject*> target_parents;
        SceneObject* current = target;
        while(current) {
            if(current) target_parents.emplace_back(current);
            current = target->Parent();
        }
        current = origin;
        while(current) {
            if(current) origin_parents.emplace_back(current);
            current = origin->Parent();
        }

        for(size_t i = 0; i < target_parents.size(); ++i) {
            for(size_t j = 0; j < origin_parents.size(); ++j) {
                if(target_parents[i] == origin_parents[j]) {
                    upwardDepth = j;
                    for(size_t k = 0; k < j; ++k) {
                        childChain.emplace_back(target_parents[k]->Name());
                    }
                    std::reverse(childChain.begin(), childChain.end());
                }
            }
        }
    }
    SceneObject* Get() {
        if(!origin) return 0;
        SceneObject* current = origin;
        for(int i = 0; i < upwardDepth; ++i) {
            current = current->Parent();
            if(!current) return 0;
        }
        for(size_t i = 0; i < childChain.size(); ++i) {
            current = current->FindObject(childChain[i]);
            if(!current) return 0;
        }
        return current;
    }
private:
    SceneObject* origin = 0;
    int upwardDepth = 0;
    std::vector<std::string> childChain;
};

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

    void SetArmatureRoot(SceneObject* root) {
        armatureRoot.Set(root);
    }
    SceneObject* GetArmatureRoot() {
        return armatureRoot.Get();
    }

    const std::vector<gfxm::mat4>& Update() {
        if(skeletonDirty) {
            // TODO: Rebuild transformObjects and transforms arrays

            skeletonDirty = false;
        }

        // TODO: Fill out transforms array

        return transforms;
    }

    virtual void OnInit() {
        armatureRoot = SceneObjectRef(Object());
    }
private:
    resource_ref<Skeleton> skeleton;
    SceneObjectRef armatureRoot;
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
