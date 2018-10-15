#ifndef SKELETON_MOTOR_H
#define SKELETON_MOTOR_H

#include <animation.h>
#include <lib/ozz/include/ozz/animation/runtime/animation.h>
#include <lib/ozz/include/ozz/animation/runtime/skeleton.h>
#include <lib/ozz/include/ozz/animation/offline/raw_animation.h>
#include <lib/ozz/include/ozz/animation/offline/raw_skeleton.h>
#include <lib/ozz/include/ozz/animation/offline/animation_builder.h>
#include <lib/ozz/include/ozz/animation/offline/skeleton_builder.h>
#include <lib/ozz/include/ozz/animation/runtime/sampling_job.h>
#include <lib/ozz/include/ozz/base/memory/allocator.h>

class SkeletonMotor {
public:
    void Length(float len) { length = len; }
    float Length() const { return length; }
    void FrameRate(float fps) { frameRate = fps; }
    float FrameRate() const { return frameRate; }

    void Build(SceneObject* root, Animation* anim) {
        Length(anim->Length());
        FrameRate(anim->FrameRate());
        
        ozz::animation::offline::RawSkeleton raw_skeleton;
        ozz::animation::offline::RawAnimation raw_animation;
        raw_animation.duration = Length() / FrameRate();
        
        for(size_t i = 0; i < anim->TargetCount(); ++i) {
            auto target = anim->GetTarget(i);
            auto so = root->FindObject(target->name);
            if(!so) continue;
            for(auto& comp_node : target->children) {
                rttr::type comp_type = rttr::type::get_by_name(comp_node.second.name);
                if(comp_type.get_name() != rttr::type::get<Transform>().get_name())
                    continue;

                Transform* transform = so->Get<Transform>();
                gfxm::vec3 translation = transform->Position();
                gfxm::quat rotation = transform->Rotation();
                gfxm::vec3 scale = transform->Scale();
                ozz::animation::offline::RawSkeleton::Joint joint;
                joint.name = so->Name().c_str();
                joint.transform.translation = ozz::math::Float3(
                    translation.x, translation.y, translation.z
                );
                joint.transform.rotation = ozz::math::Quaternion(
                    rotation.x, rotation.y, rotation.z, rotation.w
                );
                joint.transform.scale = ozz::math::Float3(
                    scale.x, scale.y, scale.z
                );
                raw_skeleton.roots.emplace_back(joint);

                for(auto& prop_node : comp_node.second.children) {
                    rttr::property prop = comp_type.get_property(prop_node.second.name);
                    if(!prop.is_valid()) continue;
                    if(prop.get_name().to_string() != "Position" &&
                        prop.get_name().to_string() != "Rotation" && 
                        prop.get_name().to_string() != "Scale")
                        continue;

                    ozz::animation::offline::RawAnimation::JointTrack track;
                    
                    
                }
            }
        }

        if(!raw_skeleton.Validate()) {
            std::cout << "Motion skeleton is not valid!" << std::endl;
        }
        ozz::animation::offline::SkeletonBuilder builder;
        skeleton = builder(raw_skeleton);

        if(!raw_animation.Validate()) {
            std::cout << "Motion animation is not valid!" << std::endl;
        }
    }

    void Tick(float dt) {
        if(!skeleton || !animation) return;

        ozz::animation::SamplingJob sampling_job;
        sampling_job.animation = animation;
        sampling_job.cache = cache;
        sampling_job.ratio = 1.0f;
        sampling_job.output = locals;

        if(!sampling_job.Run()) {
            std::cout << "Sampling job failed" << std::endl;
        }
    }
private:
    float length = 0.0f;
    float frameRate = 60.0f;

    ozz::animation::Skeleton* skeleton = 0;
    ozz::animation::Animation* animation = 0;
    ozz::animation::SamplingCache* cache;
    ozz::Range<ozz::math::SoaTransform> locals;
};

#endif
