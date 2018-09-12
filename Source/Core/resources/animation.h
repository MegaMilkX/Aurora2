#ifndef ANIMATION_H
#define ANIMATION_H

#include <resources/resource/resource.h>
#include <util/animation/curve.h>
#include <algorithm>

class curve_wrap_base {

};

template<typename T>
class curve_wrap : public curve_wrap_base {
public:
    curve_wrap() {}
    curve_wrap(const curve<T>& c) : c(c) {}
    curve<T> c;
};

class AnimationNode {
public:
    AnimationNode()
    : curv(std::make_pair(rttr::type::get<void>(), std::shared_ptr<curve_wrap_base>(0))) {}

    std::string name;
    std::map<std::string, AnimationNode> children;
    std::pair<rttr::type, std::shared_ptr<curve_wrap_base>> curv;
    
    template<typename T>
    void InitCurve() {
        curv.first = rttr::type::get<curve<T>>();
        curv.second.reset(new curve_wrap<T>());
    }
    template<typename T>
    void SetCurve(const curve<T>& c) {
        curv.first = rttr::type::get<curve<T>>();
        curv.second.reset(new curve_wrap<T>(c));
    }
    rttr::type CurveType() { return curv.first; }
    template<typename T>
    curve<T>* GetCurve() {
        if(CurveType() != rttr::type::get<curve<T>>()) return 0;
        return &((curve_wrap<T>*)curv.second.get())->c;
    }
};

class Animation : public Resource {
public:
    size_t TargetCount() { return nodes.size(); }
    AnimationNode* GetTarget(size_t i) {
        auto it = nodes.begin();
        std::advance(it, i);
        if(it == nodes.end()) return 0;
        return &it->second;
    }

    float FrameRate() { return (float)frameRate; }
    float Length() { return length; }

    virtual bool Build(DataSourceRef raw) {
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
            std::string curve_type = property.substr(property.find_last_of(".") + 1);
            std::string prop_name = property.substr(0, property.find_last_of("."));

            if(curve_type.empty()) {
                std::cout << "Empty curve type" << std::endl;
                return false;
            }

            if(curve_type == "curve") {
                curve<float> curv;
                if(stat.m_uncomp_size < sizeof(keyframe<float>))
                    continue;
                std::vector<keyframe<float>> frames;
                frames.resize((size_t)(stat.m_uncomp_size / sizeof(keyframe<float>)));
                mz_zip_reader_extract_file_to_mem(
                    &zip, 
                    stat.m_filename, 
                    (void*)frames.data(), 
                    (size_t)stat.m_uncomp_size, 0
                );
                nodes[target].name = target;
                nodes[target].children[comp_target].name = comp_target;
                nodes[target].children[comp_target].children[prop_name].name = prop_name;
                nodes[target].children[comp_target].children[prop_name].InitCurve<float>();
                nodes[target].children[comp_target].children[prop_name].GetCurve<float>()->set_keyframes(frames);
            } else if(curve_type == "curve2") {
                curve<gfxm::vec2> curv;
                if(stat.m_uncomp_size < sizeof(keyframe<gfxm::vec2>))
                    continue;
                std::vector<keyframe<gfxm::vec2>> frames;
                frames.resize((size_t)(stat.m_uncomp_size / sizeof(keyframe<gfxm::vec2>)));
                mz_zip_reader_extract_file_to_mem(
                    &zip, 
                    stat.m_filename, 
                    (void*)frames.data(), 
                    (size_t)stat.m_uncomp_size, 0
                );
                nodes[target].name = target;
                nodes[target].children[comp_target].name = comp_target;
                nodes[target].children[comp_target].children[prop_name].name = prop_name;
                nodes[target].children[comp_target].children[prop_name].InitCurve<gfxm::vec2>();
                nodes[target].children[comp_target].children[prop_name].GetCurve<gfxm::vec2>()->set_keyframes(frames);
            } else if(curve_type == "curve3") {
                curve<gfxm::vec3> curv;
                if(stat.m_uncomp_size < sizeof(keyframe<gfxm::vec3>))
                    continue;
                std::vector<keyframe<gfxm::vec3>> frames;
                frames.resize((size_t)(stat.m_uncomp_size / sizeof(keyframe<gfxm::vec3>)));
                mz_zip_reader_extract_file_to_mem(
                    &zip, 
                    stat.m_filename, 
                    (void*)frames.data(), 
                    (size_t)stat.m_uncomp_size, 0
                );
                nodes[target].name = target;
                nodes[target].children[comp_target].name = comp_target;
                nodes[target].children[comp_target].children[prop_name].name = prop_name;
                nodes[target].children[comp_target].children[prop_name].InitCurve<gfxm::vec3>();
                nodes[target].children[comp_target].children[prop_name].GetCurve<gfxm::vec3>()->set_keyframes(frames);
            } else if(curve_type == "curve4") {
                curve<gfxm::vec4> curv;
                if(stat.m_uncomp_size < sizeof(keyframe<gfxm::vec4>))
                    continue;
                std::vector<keyframe<gfxm::vec4>> frames;
                frames.resize((size_t)(stat.m_uncomp_size / sizeof(keyframe<gfxm::vec4>)));
                mz_zip_reader_extract_file_to_mem(
                    &zip, 
                    stat.m_filename, 
                    (void*)frames.data(), 
                    (size_t)stat.m_uncomp_size, 0
                );
                nodes[target].name = target;
                nodes[target].children[comp_target].name = comp_target;
                nodes[target].children[comp_target].children[prop_name].name = prop_name;
                nodes[target].children[comp_target].children[prop_name].InitCurve<gfxm::vec4>();
                nodes[target].children[comp_target].children[prop_name].GetCurve<gfxm::vec4>()->set_keyframes(frames);
            } else if(curve_type == "curveq") {
                curve<gfxm::quat> curv;
                if(stat.m_uncomp_size < sizeof(keyframe<gfxm::quat>))
                    continue;
                std::vector<keyframe<gfxm::quat>> frames;
                frames.resize((size_t)(stat.m_uncomp_size / sizeof(keyframe<gfxm::quat>)));
                mz_zip_reader_extract_file_to_mem(
                    &zip, 
                    stat.m_filename, 
                    (void*)frames.data(), 
                    (size_t)stat.m_uncomp_size, 0
                );
                nodes[target].name = target;
                nodes[target].children[comp_target].name = comp_target;
                nodes[target].children[comp_target].children[prop_name].name = prop_name;
                nodes[target].children[comp_target].children[prop_name].InitCurve<gfxm::quat>();
                nodes[target].children[comp_target].children[prop_name].GetCurve<gfxm::quat>()->set_keyframes(frames);
            } else {
                std::cout << "Invalid curve type: " << curve_type << std::endl;
                return false;
            }

            /*
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
            */
        }

        mz_zip_reader_end(&zip);

        return true;
    }

    virtual bool Serialize(std::vector<unsigned char>& data) {
        throw std::exception("not implemented");
        return false;
    }
private:
    double frameRate;
    double length;
    std::map<std::string, AnimationNode> nodes;
};

#endif
