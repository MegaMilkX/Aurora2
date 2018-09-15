#ifndef SCENE_SERIALIZER_H
#define SCENE_SERIALIZER_H

#include <rttr/registration>
#include <rttr/type>
#include <external/json.hpp>
#include <util/gfxm.h>
#include <scene_object.h>
#define MINIZ_HEADER_FILE_ONLY
#include <lib/miniz.c>
#include <functional>
#include <resources/resource/resource_ref.h>

template<typename T>
void ToJsonArray(nlohmann::json& j, T* data, int count)
{
    for(int i = 0; i < count; ++i)
    {
        j[i] = data[i];
    }
}

template<typename T> bool ToJson(nlohmann::json& j, rttr::variant& value){
    j = value.get_value<T>();
    return true;
}
template<> inline bool ToJson<gfxm::vec2>(nlohmann::json& j, rttr::variant& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value.get_value<gfxm::vec2>(), 2);
    return true;
}
template<> inline bool ToJson<gfxm::vec3>(nlohmann::json& j, rttr::variant& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value.get_value<gfxm::vec3>(), 3);
    return true;
}
template<> inline bool ToJson<gfxm::vec4>(nlohmann::json& j, rttr::variant& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value.get_value<gfxm::vec4>(), 4);
    return true;
}
template<> inline bool ToJson<gfxm::mat3>(nlohmann::json& j, rttr::variant& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value.get_value<gfxm::mat3>(), 9);
    return true;
}
template<> inline bool ToJson<gfxm::mat4>(nlohmann::json& j, rttr::variant& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value.get_value<gfxm::mat4>(), 16);
    return true;
}

class SceneSerializer {
public:
    typedef std::function<bool(nlohmann::json&, rttr::variant&)> serialize_prop_f;
    typedef std::function<void(rttr::variant&, nlohmann::json&)> json_prop_parser_t;

    SceneSerializer();

    bool Serialize(const SceneObject* scene, const std::string& filename);
    bool Deserialize(const std::string& filename, SceneObject& scene);
private:
    std::map<rttr::type, serialize_prop_f> prop_serializers;
    std::map<rttr::type, json_prop_parser_t> parsers;
    std::set<std::shared_ptr<Resource>> embedded_resources;
    std::map<std::string, DataSourceRef> data_sources;
    std::map<std::string, std::shared_ptr<Resource>> resources;

    bool SerializeEmbeddedResources(mz_zip_archive& archive);
    bool SerializeScene_(const SceneObject* scene, mz_zip_archive& archive, std::string& file_prefix);
    std::string SerializeComponentToJson(mz_zip_archive& archive, rttr::type t, SceneObject::Component* c);

    bool DeserializeScene(unsigned char* data, size_t size, SceneObject& scene);
    rttr::variant JsonPropertyToVariant(rttr::variant& var, nlohmann::json& j);
    std::map<rttr::type, json_prop_parser_t> InitPropertyParsers();
};

#endif
