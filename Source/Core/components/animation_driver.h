#ifndef ANIMATION_DRIVER_H
#define ANIMATION_DRIVER_H

#include <updatable.h>
#include <resource_ref.h>
#include <util/animation/curve.h>

class AnimationNode {
public:
private:
    std::map<std::string, AnimationNode> children;
    std::map<std::string, curve> curves;
};

class Animation : public ResourceObject {
public:
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

        mz_zip_reader_end(&zip);
    }
private:
    double frameRate;
    double length;
};

class AnimationDriver : public Updatable
{
    RTTR_ENABLE(Updatable)
public:
    ResourceRef animation;

    void AddAnim(const std::string& name, const std::string& resource);
    void Play(const std::string& name);

    virtual void OnUpdate() {

    }
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
