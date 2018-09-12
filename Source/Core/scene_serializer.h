#ifndef SCENE_SERIALIZER_H
#define SCENE_SERIALIZER_H

#include <rttr/registration>
#include <rttr/type>
#include <external/json.hpp>
#include <util/gfxm.h>
#include <scene_object.h>
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

    SceneSerializer() {
        prop_serializers[rttr::type::get<uint8_t>()] = &ToJson<uint8_t>;
        prop_serializers[rttr::type::get<int8_t>()] = &ToJson<int8_t>;
        prop_serializers[rttr::type::get<uint16_t>()] = &ToJson<uint16_t>;
        prop_serializers[rttr::type::get<int16_t>()] = &ToJson<int16_t>;
        prop_serializers[rttr::type::get<uint32_t>()] = &ToJson<uint32_t>;
        prop_serializers[rttr::type::get<int32_t>()] = &ToJson<int32_t>;
        prop_serializers[rttr::type::get<uint64_t>()] = &ToJson<uint64_t>;
        prop_serializers[rttr::type::get<int64_t>()] = &ToJson<int64_t>;
        prop_serializers[rttr::type::get<float>()] = &ToJson<float>;
        prop_serializers[rttr::type::get<double>()] = &ToJson<double>;
        prop_serializers[rttr::type::get<gfxm::vec2>()] = &ToJson<gfxm::vec2>;
        prop_serializers[rttr::type::get<gfxm::vec3>()] = &ToJson<gfxm::vec3>;
        prop_serializers[rttr::type::get<gfxm::vec4>()] = &ToJson<gfxm::vec4>;
        prop_serializers[rttr::type::get<gfxm::mat3>()] = &ToJson<gfxm::mat3>;
        prop_serializers[rttr::type::get<gfxm::mat4>()] = &ToJson<gfxm::mat4>;
        prop_serializers[rttr::type::get<std::string>()] = &ToJson<std::string>;
        prop_serializers[rttr::type::get<ResourceRef>()] = [this](nlohmann::json& j, rttr::variant& value)->bool {
            ResourceRef& ref = value.get_value<ResourceRef>();
            if(!ref) return false;
            std::shared_ptr<Resource> res = ref.Get();
            if(!res) return false;
            j["storage"] = res->Storage();
            j["name"] = res->Name();
            if(res->Storage() == Resource::LOCAL) {
                embedded_resources.insert(res);
            }
            return true;
        };
    }

    bool Serialize(const SceneObject* scene, const std::string& filename) {
        mz_zip_archive archive;
        memset(&archive, 0, sizeof(archive));

        if(!mz_zip_writer_init_file(&archive, filename.c_str(), 65537)) {
            LOG("Failed to create archive file " << filename);
            return false;
        }

        std::string file_prefix;
        SerializeScene_(scene, archive, file_prefix);

        SerializeEmbeddedResources(archive);

        mz_zip_writer_finalize_archive(&archive);
        mz_zip_writer_end(&archive);

        return true;
    }
private:
    std::map<rttr::type, serialize_prop_f> prop_serializers;
    std::set<std::shared_ptr<Resource>> embedded_resources;

    bool SerializeEmbeddedResources(mz_zip_archive& archive) {
        for(auto& r : embedded_resources) {
            std::vector<unsigned char> data;
            r->Serialize(data);
            mz_zip_writer_add_mem(
                &archive, 
                MKSTR("resources/" << r->Name()).c_str(), 
                (void*)data.data(),
                data.size(), 0
            );
        }        
        return true;
    }

    bool SerializeScene_(
        const SceneObject* scene, 
        mz_zip_archive& archive,
        std::string& file_prefix
    ){
        int comp_count = scene->ComponentCount();
        for(int i = 0; i < comp_count; ++i)
        {
            SceneObject::Component* comp = 
                scene->GetComponent(i);

            rttr::type type = comp->GetType();

            if(type == rttr::type::get<void>())
                continue;

            if(!type.is_valid()) continue;

            std::string data = 
                SerializeComponentToJson(archive, type, comp);

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
            SerializeScene_(scene->GetChild(i), archive, prefix);
        }

        return true;
    }

    std::string SerializeComponentToJson(mz_zip_archive& archive, rttr::type t, SceneObject::Component* c)
    {
        using json = nlohmann::json;

        std::string result;
        json j = json::object();

        auto props = t.get_properties();
        for(auto p : props)
        {
            auto it = prop_serializers.find(p.get_type());
            if(it == prop_serializers.end()) continue;
            it->second(j, p.get_value(c));
        }

        result = j.dump();

        return result;
    }
};

#endif
