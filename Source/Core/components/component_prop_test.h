#ifndef COMPONENT_PROP_TEST_H
#define COMPONENT_PROP_TEST_H

#include "../component.h"
#include <util/gfxm.h>
#include <mesh.h>
#include <material.h>
#include <animation.h>
#include <texture2d.h>

#include "editor_gui_property.h"

class ComPropTest : public Component {
    RTTR_ENABLE(Component)
public:
    std::string string;
    unsigned char uchar_;
    char char_;
    unsigned short ushort_;
    short short_;
    unsigned int uint_;
    int int_;
    unsigned long ulong_;
    long long_;
    unsigned long long ulonglong_;
    long long longlong_;
    float float_;
    double double_;
    bool bool_;
    gfxm::vec2 vec2_;
    gfxm::vec3 vec3_;
    gfxm::vec4 vec4_;
    gfxm::quat quat_;
    gfxm::mat3 mat3_;
    gfxm::mat4 mat4_;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    std::shared_ptr<Animation> animation;
    std::shared_ptr<Texture2D> texture2d;
    std::shared_ptr<SceneObject> sceneObject;
    std::map<std::string, gfxm::vec2> map;
    std::vector<int> vector;
private:
};

STATIC_RUN(ComPropTest) {
    rttr::registration::class_<ComPropTest>("ComPropTest")
        .constructor<>()(
            rttr::policy::ctor::as_raw_ptr
        )
        .property("string", &ComPropTest::string)
        .property("uchar_", &ComPropTest::uchar_)
        .property("char_", &ComPropTest::char_)
        .property("ushort_", &ComPropTest::ushort_)
        .property("short_", &ComPropTest::short_)
        .property("uint_", &ComPropTest::uint_)
        .property("int_", &ComPropTest::int_)
        .property("ulong_", &ComPropTest::ulong_)
        .property("long_", &ComPropTest::long_)
        .property("ulonglong_", &ComPropTest::ulonglong_)
        .property("longlong_", &ComPropTest::longlong_)
        .property("float_", &ComPropTest::float_)
        .property("double_", &ComPropTest::double_)
        .property("bool_", &ComPropTest::bool_)
        .property("vec2_", &ComPropTest::vec2_)
        .property("vec3_", &ComPropTest::vec3_)
        .property("vec4_", &ComPropTest::vec4_)
        .property("quat_", &ComPropTest::quat_)
        .property("mat3_", &ComPropTest::mat3_)
        .property("mat4_", &ComPropTest::mat4_)
        .property("mesh", &ComPropTest::mesh)
        .property("material", &ComPropTest::material)
        .property("animation", &ComPropTest::animation)
        .property("texture2d", &ComPropTest::texture2d)
        .property("sceneObject", &ComPropTest::sceneObject)
        .property("map", &ComPropTest::map)
        .property("vector", &ComPropTest::vector);
}

#endif
