#ifndef SCENE_SERIALIZER_H
#define SCENE_SERIALIZER_H

#include <rttr/registration>
#include <rttr/type>
#include <external/json.hpp>
#include <util/gfxm.h>
#include <component.h>
#define MINIZ_HEADER_FILE_ONLY
#include <lib/miniz.c>
#include <functional>
#include <resources/resource/resource_factory.h>

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
template<> inline bool ToJson<gfxm::quat>(nlohmann::json& j, rttr::variant& value){
    j = nlohmann::json::array();
    ToJsonArray(j, (float*)&value.get_value<gfxm::quat>(), 4);
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
    class ImportData {
    public:
        void Clear() {
            oldUidToObject.clear();
        }

        void AddObject(int64_t imported_uid, SceneObject* so) {
            oldUidToObject[imported_uid] = so;
        }
        SceneObject* GetObjectByImportUid(int64_t uid) {
            auto it = oldUidToObject.find(uid);
            if(it == oldUidToObject.end()) return 0;
            return it->second;
        }

        std::map<std::string, DataSourceRef> data_sources;
    private:
        std::map<int64_t, SceneObject*> oldUidToObject;
    };

    class ExportData {
    public:
    private:
    };

    typedef std::function<bool(nlohmann::json&, rttr::variant&)> serialize_prop_f;
    typedef std::function<void(rttr::variant&, nlohmann::json&)> json_prop_parser_t;
    typedef std::function<void(Component*, nlohmann::json&)> custom_component_writer_f;
    typedef std::function<void(Component*, nlohmann::json&)> custom_component_reader_f;

    SceneSerializer();

    bool Serialize(const SceneObject* scene, const std::string& filename);
    bool Deserialize(const std::string& filename, SceneObject& scene);

    template<typename T>
    void CustomComponentWriter(custom_component_writer_f f);
    template<typename T>
    void CustomComponentReader(custom_component_reader_f f);
private:
    std::map<rttr::type, serialize_prop_f> prop_serializers;
    std::map<rttr::type, json_prop_parser_t> parsers;
    std::set<std::shared_ptr<Resource>> embedded_resources;
    std::map<std::string, std::shared_ptr<Resource>> resources;

    ImportData importData;
    ExportData exportData;

    std::map<rttr::type, custom_component_writer_f> custom_component_writers;
    std::map<rttr::type, custom_component_reader_f> custom_component_readers;

    bool SerializeEmbeddedResources(mz_zip_archive& archive);
    bool SerializeScene_(const SceneObject* scene, mz_zip_archive& archive);
    std::string SerializeComponentToJson(mz_zip_archive& archive, rttr::type t, Component* c);

    bool DeserializeScene(unsigned char* data, size_t size, SceneObject& scene);
    rttr::variant JsonPropertyToVariant(rttr::variant& var, nlohmann::json& j);
    std::map<rttr::type, json_prop_parser_t> InitPropertyParsers();
};

template<typename T>
void SceneSerializer::CustomComponentWriter(SceneSerializer::custom_component_writer_f f) {
    custom_component_writers[rttr::type::get<T>()] = f;
}
template<typename T>
void SceneSerializer::CustomComponentReader(SceneSerializer::custom_component_reader_f f) {
    custom_component_readers[rttr::type::get<T>()] = f;
}

#endif
