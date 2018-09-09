#ifndef JSON_PROPERTY_PARSERS_H
#define JSON_PROPERTY_PARSERS_H

#include <scene_object.h>
#include <util/gfxm.h>
#include <util/static_run.h>
#include <resources/resource/resource_factory.h>

typedef std::function<void(rttr::variant&, nlohmann::json&)> json_prop_parser_t;
inline std::map<rttr::type, json_prop_parser_t> InitPropertyParsers()
{
    std::map<rttr::type, json_prop_parser_t> parsers;

    parsers[rttr::type::get<uint8_t>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_number()) return;
        v = j.get<uint8_t>();
    };
    parsers[rttr::type::get<int8_t>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_number()) return;
        v = j.get<int8_t>();
    };
    parsers[rttr::type::get<uint16_t>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_number()) return;
        v = j.get<uint16_t>();
    };
    parsers[rttr::type::get<int16_t>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_number()) return;
        v = j.get<int16_t>();
    };
    parsers[rttr::type::get<uint32_t>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_number()) return;
        v = j.get<uint32_t>();
    };
    parsers[rttr::type::get<int32_t>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_number()) return;
        v = j.get<int32_t>();
    };
    parsers[rttr::type::get<uint64_t>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_number()) return;
        v = j.get<uint64_t>();
    };
    parsers[rttr::type::get<int64_t>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_number()) return;
        v = j.get<int64_t>();
    };
    parsers[rttr::type::get<float>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_number()) return;
        v = j.get<float>();
    };
    parsers[rttr::type::get<double>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_number()) return;
        v = j.get<double>();
    };
    parsers[rttr::type::get<std::string>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_string()) return;
        v = j.get<std::string>();
    };
    parsers[rttr::type::get<gfxm::vec2>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_array()) return;
        v = gfxm::vec2(j[0].get<float>(), j[1].get<float>());
    };
    parsers[rttr::type::get<gfxm::vec3>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_array()) return;
        v = gfxm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
    };
    parsers[rttr::type::get<gfxm::vec4>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_array()) return;
        v = gfxm::vec4(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>());
    };
    parsers[rttr::type::get<gfxm::mat3>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_array()) return;
        gfxm::mat3 m;
        m[0][0] = j[0].get<float>();
        m[0][1] = j[1].get<float>();
        m[0][2] = j[2].get<float>();
        m[1][0] = j[3].get<float>();
        m[1][1] = j[4].get<float>();
        m[1][2] = j[5].get<float>();
        m[2][0] = j[6].get<float>();
        m[2][1] = j[7].get<float>();
        m[2][2] = j[8].get<float>();
        v = m;
    };
    parsers[rttr::type::get<gfxm::mat4>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_array()) return;
        gfxm::mat4 m;
        m[0][0] = j[0].get<float>();
        m[0][1] = j[1].get<float>();
        m[0][2] = j[2].get<float>();
        m[0][3] = j[3].get<float>();
        m[1][0] = j[4].get<float>();
        m[1][1] = j[5].get<float>();
        m[1][2] = j[6].get<float>();
        m[1][3] = j[7].get<float>();
        m[2][0] = j[8].get<float>();
        m[2][1] = j[9].get<float>();
        m[2][2] = j[10].get<float>();
        m[2][3] = j[11].get<float>();
        m[3][0] = j[12].get<float>();
        m[3][1] = j[13].get<float>();
        m[3][2] = j[14].get<float>();
        m[3][3] = j[15].get<float>();
        v = m;
    };
    parsers[rttr::type::get<ResourceRef>()] = [](rttr::variant& v, nlohmann::json& j){
        if(!j.is_string()) return;
        
        //ResourceRef ref(j.get<std::string>());
        //v = ref;

        v.get_value<ResourceRef>().Set(j.get<std::string>());
    };

    return parsers;
}

inline rttr::variant JsonPropertyToVariant(nlohmann::json& j)
{
    static auto parsers = InitPropertyParsers();
    using json = nlohmann::json;
    rttr::variant var;

    json jtype = j["type"];
    json jvalue = j["value"];
    if(!jtype.is_string()) return var;

    rttr::type t = rttr::type::get_by_name(jtype.get<std::string>());
    if(!t.is_valid()) return var;

    auto it = parsers.find(t);
    if(it == parsers.end()) return var;

    it->second(var, jvalue);
    
    return var;
}

#endif
