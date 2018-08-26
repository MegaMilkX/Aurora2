#ifndef SERIALIZE_SCENE_H
#define SERIALIZE_SCENE_H

#include <rttr/registration>
#include <rttr/type>
#include <external/json.hpp>
#include <util/gfxm.h>
#include <scene_object.h>
#include <lib/miniz.c>

template<typename T>
void ToJsonArray(nlohmann::json& j, T* data, int count)
{
    for(int i = 0; i < count; ++i)
    {
        j[i] = data[i];
    }
}

template<typename T>
void ToJson(nlohmann::json& j, T& value){
    j = value;
}
template<>
inline void ToJson(nlohmann::json& j, gfxm::vec2& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value, 2);
}
template<>
inline void ToJson(nlohmann::json& j, gfxm::vec3& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value, 3);
}
template<>
inline void ToJson(nlohmann::json& j, gfxm::vec4& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value, 4);
}
template<>
inline void ToJson(nlohmann::json& j, gfxm::mat3& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value, 9);
}
template<>
inline void ToJson(nlohmann::json& j, gfxm::mat4& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value, 16);
}
template<>
inline void ToJson(nlohmann::json& j, ResourceRef& value){
    j = value.GetTargetName();
}

template<typename T>
bool TrySetJsonMember(nlohmann::json& j, rttr::property p, SceneObject::Component* c)
{
    if(p.get_type() == rttr::type::get<T>())
    {
        j[p.get_name().to_string()]["type"] = rttr::type::get<T>().get_name().to_string();

        T v = p.get_value(c).get_value<T>();

        ToJson(
            j[p.get_name().to_string()]["value"], 
            v
        );
        return true;
    }
    return false;
}

inline void SetJsonMember(nlohmann::json& j, rttr::property p, SceneObject::Component* c)
{
#define X(TYPE) if(TrySetJsonMember<TYPE>(j, p, c)) return;
    X(uint8_t)
    X(int8_t)
    X(uint16_t)
    X(int16_t)
    X(uint32_t)
    X(int32_t)
    X(uint64_t)
    X(int64_t)
    X(float)
    X(double)
    X(gfxm::vec2)
    X(gfxm::vec3)
    X(gfxm::vec4)
    X(gfxm::mat3)
    X(gfxm::mat4)
    X(std::string)
    X(ResourceRef)
#undef X
}

inline std::string SerializeComponentToJson(mz_zip_archive& archive, rttr::type t, SceneObject::Component* c, bool embed_resources)
{
    using json = nlohmann::json;

    std::string result;
    json j = json::object();

    auto props = t.get_properties();
    for(auto p : props)
    {
        SetJsonMember(j, p, c);
        if(embed_resources)
        {
            if(p.get_type() == rttr::type::get<ResourceRef>())
            {
                rttr::variant var = p.get_value(c);
                ResourceRef ref = var.get_value<ResourceRef>();
                ResourceRaw* raw = ref.GetRaw();
                if(raw && raw->Size() > 0)
                {
                    std::vector<char> buf;
                    buf.resize(raw->Size());
                    raw->ReadAll((char*)buf.data());
                    mz_zip_writer_add_mem(
                        &archive,
                        ("resources/" + ref.GetTargetName()).c_str(),
                        buf.data(), buf.size(), 0
                    );
                }
            }
        }
    }

    result = j.dump();

    return result;
}

inline bool SerializeScene_(
    const SceneObject* scene, 
    mz_zip_archive& archive,
    std::string& file_prefix, 
    bool embed_resources = false
){
    int comp_count = scene->ComponentCount();
    for(int i = 0; i < comp_count; ++i)
    {
        SceneObject::Component* comp = 
            scene->GetComponent(i);

        rttr::type type = comp->GetType();

        if(type.get_name() == 
            rttr::type::get(UnserializableComponentToken()).get_name())
            continue;

        if(!type.is_valid()) continue;

        std::string data = 
            SerializeComponentToJson(archive, type, comp, embed_resources);

        if(!mz_zip_writer_add_mem(
            &archive, 
            (file_prefix + "components/" + type.get_name()).c_str(), 
            data.c_str(), 
            data.size(), 
            0
        )){
            LOG_ERR("Failed to mz_zip_writer_add_mem() ");
        }
    }

    //LOG(file_prefix + "hello");

    int child_count = scene->ChildCount();
    for(int i = 0; i < child_count; ++i)
    {
        std::string prefix = 
            file_prefix + "objects/" + scene->GetChild(i)->Name() + "/";
        SerializeScene_(scene->GetChild(i), archive, prefix, embed_resources);
    }

    return true;
}

inline bool SerializeScene(const SceneObject* scene, const std::string& filename, bool embed_resources = false)
{
    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));

    if(!mz_zip_writer_init_file(&archive, filename.c_str(), 65537))
    {
        LOG("Failed to create archive file " << filename);
        return false;
    }

    std::string file_prefix;
    SerializeScene_(scene, archive, file_prefix, embed_resources);

    mz_zip_writer_finalize_archive(&archive);
    mz_zip_writer_end(&archive);

    return true;
}

#endif
