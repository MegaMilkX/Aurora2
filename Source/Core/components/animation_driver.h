#ifndef ANIMATION_DRIVER_H
#define ANIMATION_DRIVER_H

#include <updatable.h>
#include <resource_ref.h>
#include <util/animation/curve.h>
#include <algorithm>

class AnimationNode {
public:
    std::string name;
    std::map<std::string, AnimationNode> children;
    std::map<std::string, curve> curves;
};

class Animation : public ResourceObject {
public:
    size_t TargetCount() { return nodes.size(); }
    AnimationNode* GetTarget(size_t i) {
        auto it = nodes.begin();
        std::advance(it, i);
        if(it == nodes.end()) return 0;
        return &it->second;
    }

    float FrameRate() { return frameRate; }
    float Length() { return length; }

    virtual bool Build(ResourceRaw* raw) {
        if(raw->Size() == 0) return false;
        std::vector<char> buffer;
        buffer.resize((size_t)raw->Size());
        raw->ReadAll((char*)buffer.data());

        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        if(!mz_zip_reader_init_mem(&zip, buffer.data(), buffer.size(), 0)) {
            return false;
        }

        mz_zip_reader_extract_file_to_mem(
            &zip, "FrameRate", (void*)&frameRate, (size_t)sizeof(frameRate), 0
        );
        mz_zip_reader_extract_file_to_mem(
            &zip, "Length", (void*)&length, sizeof(length), 0
        );

        mz_uint num_files = mz_zip_reader_get_num_files(&zip);
        for(mz_uint i = 0; i < num_files; ++i) {
            mz_zip_archive_file_stat stat;
            mz_zip_reader_file_stat(&zip, i, &stat);
            std::string filename = stat.m_filename;
            if(filename.compare(0, strlen("Layers/"), "Layers/") != 0)
                continue;
            
            auto tokens = split(filename, '/');
            std::string layer = tokens[1];
            std::string target = tokens[2];
            std::string comp_target = tokens[3];
            std::string property = tokens[4];
            std::string curve_name = tokens[5];
            
            struct kf {
                float time;
                float value;
            };

            if(stat.m_uncomp_size < sizeof(kf))
                continue;

            std::vector<kf> frames;
            frames.resize((size_t)(stat.m_uncomp_size / sizeof(kf)));

            mz_zip_reader_extract_file_to_mem(
                &zip, 
                stat.m_filename, 
                (void*)frames.data(), 
                (size_t)stat.m_uncomp_size, 0
            );
            nodes[target].name = target;
            nodes[target].children[comp_target].name = comp_target;
            nodes[target].children[comp_target].children[property].name = property;
            for(auto& f : frames) {
                nodes[target].children[comp_target].children[property].curves[curve_name][f.time] = f.value;
            }
        }

        mz_zip_reader_end(&zip);

        return true;
    }
private:
    double frameRate;
    double length;
    std::map<std::string, AnimationNode> nodes;
};

class AnimMotorVec3 {
public:
    AnimMotorVec3() : prop(rttr::type::get<AnimMotorVec3>().get_property("INVALID")) {}
    AnimMotorVec3(SceneObject::Component* c, rttr::property prop, curve* x, curve* y, curve* z, float length)
    : comp(c), prop(prop), curves{x, y, z}, length(length) {

    }

    void Tick(float dt) {
        cursor += dt;
        if(cursor > length) { cursor = cursor - length; }
        gfxm::vec3 evaluated(
            curves[0]->at(cursor),
            curves[1]->at(cursor),
            curves[2]->at(cursor)
        );
        prop.set_value(comp, evaluated);
    }
private:
    float cursor = 0.0f;
    SceneObject::Component* comp;
    rttr::property prop;
    curve* curves[3];
    float length = 0.0f;
};

class AnimMotion {
    
};

class AnimationDriver : public Updatable
{
    RTTR_ENABLE(Updatable)
public:
    ResourceRef animation;

    void AddAnim(const std::string& name, const std::string& resource) {
        anim = ResourceRef(resource).Get<Animation>();
        motorsVec3.clear();
        for(size_t i = 0; i < anim->TargetCount(); ++i) {
            auto target = anim->GetTarget(i);
            auto so = Object()->FindObject(target->name);
            if(!so) continue;
            for(auto& comp_node : target->children) {
                rttr::type comp_type = rttr::type::get_by_name(comp_node.second.name);
                SceneObject::Component* comp = so->Get(comp_node.second.name);
                if(!comp) continue;
                for(auto& prop_node : comp_node.second.children) {
                    rttr::property prop = comp_type.get_property(prop_node.second.name);
                    if(!prop.is_valid()) continue;
                    if(prop.get_type() == rttr::type::get<gfxm::vec3>()) {
                        motorsVec3.emplace_back(
                            AnimMotorVec3(
                                comp, 
                                prop, 
                                &prop_node.second.curves["x"],
                                &prop_node.second.curves["y"],
                                &prop_node.second.curves["z"],
                                anim->Length()
                            )
                        );
                    } else {
                        std::cout << "Property " <<
                        prop.get_name().to_string().c_str() <<
                        " wasn't animated (unsupported type)" << std::endl;
                    }
                }
            }
        }
    }
    void Play(const std::string& name);

    virtual void OnUpdate() {
        for(auto& motor : motorsVec3) {
            motor.Tick(1.0f / 60.0f * anim->FrameRate());
        }
    }
private:
    Animation* anim;
    std::vector<AnimMotorVec3> motorsVec3;
};
STATIC_RUN(AnimationDriver) {
    rttr::registration::class_<AnimationDriver>("AnimationDriver")
        .constructor<>()(
            rttr::policy::ctor::as_raw_ptr
        )
        .property(
            "animation",
            &AnimationDriver::animation
        );
}

#endif
