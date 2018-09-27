#include "scene_serializer.h"

#include <resources/resource/resource_ref.h>

SceneSerializer::SceneSerializer() {
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
    prop_serializers[rttr::type::get<gfxm::quat>()] = &ToJson<gfxm::quat>;
    prop_serializers[rttr::type::get<gfxm::mat3>()] = &ToJson<gfxm::mat3>;
    prop_serializers[rttr::type::get<gfxm::mat4>()] = &ToJson<gfxm::mat4>;
    prop_serializers[rttr::type::get<std::string>()] = &ToJson<std::string>;
    prop_serializers[rttr::type::get<SceneObject*>()] = [this](nlohmann::json& j, rttr::variant& value)->bool {
        SceneObject* so = value.get_value<SceneObject*>();
        if(so == 0) {
            j = 0;
        } else {
            j = so->Uid();
        }
        return true;
    };
    prop_serializers[rttr::type::get<i_resource_ref>()] = [this](nlohmann::json& j, rttr::variant& value)->bool {
        /*
        if(!value.can_convert<std::shared_ptr<Resource>>()) {
            std::cout << "Serializing property: Can't convert variant of type " << 
                value.get_type().get_name().to_string() << " to " << rttr::type::get<std::shared_ptr<Resource>>().get_name().to_string() << std::endl;
        }
        */
        i_resource_ref& ref = value.get_value<i_resource_ref>();
        std::shared_ptr<Resource> res = ref.base_ptr();
        if(!res) {
            std::cout << "Serializing property: resource is null" << std::endl;
            j["storage"] = Resource::LOCAL;
            j["name"] = "";
        }
        else {
            j["storage"] = res->Storage();
            j["name"] = res->Name();
            if(res->Storage() == Resource::LOCAL) {
                embedded_resources.insert(res);
            }
        }
        return true;
    };
    parsers = InitPropertyParsers();
}

bool SceneSerializer::Serialize(const SceneObject* scene, const std::string& filename) {
    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));

    if(!mz_zip_writer_init_file(&archive, filename.c_str(), 65537)) {
        LOG("Failed to create archive file " << filename);
        return false;
    }

    SerializeScene_(scene, archive);

    SerializeEmbeddedResources(archive);

    mz_zip_writer_finalize_archive(&archive);
    mz_zip_writer_end(&archive);

    return true;
}

bool SceneSerializer::Deserialize(const std::string& filename, SceneObject& scene)
{
    std::ifstream f(filename, std::ios::binary | std::ios::ate);
    if(!f.is_open())
    {
        std::cout << "Failed to open " << filename << std::endl;
        return false;
    }

    std::streamsize size = f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<char> buffer((unsigned int)size);
    if(!f.read(buffer.data(), (unsigned int)size))
    {
        f.close();
        return false;
    }

    importData.Clear();
    DeserializeScene((unsigned char*)buffer.data(), buffer.size(), scene);
    for(auto& task : importData.deferred_tasks) {
        task();
    }

    f.close();
    return true;
}

// PRIVATE

bool SceneSerializer::SerializeEmbeddedResources(mz_zip_archive& archive) {
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

bool SceneSerializer::SerializeScene_(
    const SceneObject* scene,
    mz_zip_archive& archive
) {
    int64_t uid = scene->Uid();
    std::string name = scene->Name();

    mz_zip_writer_add_mem(
        &archive, MKSTR("UID").c_str(), (void*)&uid, sizeof(scene->Uid()), 0
    );
    mz_zip_writer_add_mem(
        &archive, MKSTR("Name").c_str(), name.c_str(), name.size(), 0
    );    

    int child_count = scene->ChildCount();
    for(int i = 0; i < child_count; ++i) {
        mz_zip_archive zip_object;
        memset(&zip_object, 0, sizeof(zip_object));
        if(!mz_zip_writer_init_heap(&zip_object, 0, 65537)) {
            LOG("Failed to create archive file in memory");
            return false;
        }
        
        SerializeScene_(scene->GetChild(i), zip_object);
        
        void* buf = 0;
        size_t sz = 0;
        mz_zip_writer_finalize_heap_archive(&zip_object, &buf, &sz);
        mz_zip_writer_add_mem(
            &archive, 
            MKSTR(scene->GetChild(i)->Name() << "_" << scene->GetChild(i)->Uid() << ".scn").c_str(), 
            buf,
            sz, 0
        );
        mz_zip_writer_end(&zip_object);
    }

    int comp_count = scene->ComponentCount();
    for(int i = 0; i < comp_count; ++i)
    {
        Component* comp = scene->GetComponent(i);
        rttr::type type = comp->GetType();
        if(type == rttr::type::get<void>())
            continue;
        if(!type.is_valid()) continue;
        std::string data = 
            SerializeComponentToJson(archive, type, comp);

        if(!mz_zip_writer_add_mem(
            &archive, 
            MKSTR(type.get_name() << ".comp").c_str(), 
            data.c_str(), 
            data.size(), 
            0
        )){
            LOG_ERR("Failed to mz_zip_writer_add_mem() ");
        }
    }
    return true;
}

/*
bool SceneSerializer::SerializeScene_(
    const SceneObject* scene, 
    mz_zip_archive& archive,
    std::string& file_prefix
){
    {
        using json = nlohmann::json;
        json jroot = json::object();
        jroot["uid"] = scene->Uid();
        jroot["name"] = scene->Name();
        std::string jobject_info = jroot.dump();
        if(!mz_zip_writer_add_mem(
            &archive, 
            (file_prefix + "object_info.json").c_str(), 
            jobject_info.c_str(), 
            jobject_info.size(), 
            0
        )){
            LOG_ERR("Failed to mz_zip_writer_add_mem() ");
        }
    }

    int comp_count = scene->ComponentCount();
    for(int i = 0; i < comp_count; ++i)
    {
        Component* comp = 
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
            MKSTR(file_prefix << "objects/" << scene->GetChild(i)->Name() << "_" << scene->GetChild(i)->Uid() << "/");
        SerializeScene_(scene->GetChild(i), archive, prefix);
    }

    return true;
}
*/
std::string SceneSerializer::SerializeComponentToJson(mz_zip_archive& archive, rttr::type t, Component* c)
{
    using json = nlohmann::json;

    std::string result;
    json jroot = json::object();
    json& j = jroot["props"];
    json& jext = jroot["ext"];

    {
        auto it = custom_component_writers.find(t);
        if(it != custom_component_writers.end()) {
            it->second(c, jext);
        }
    }

    auto props = t.get_properties();
    for(auto p : props)
    {
        serialize_prop_f serializer;
        rttr::variant value;
        rttr::type p_type = p.get_type();

        if(p_type.is_derived_from<i_resource_ref>()) {
            serializer = prop_serializers[rttr::type::get<i_resource_ref>()];
            value = p.get_value(c);
        } else if(p_type.is_wrapper()) {
            auto it = prop_serializers.find(p.get_type().get_wrapped_type());
            if(it != prop_serializers.end()) {
                serializer = it->second;
                value = p.get_value(c);
                value = value.extract_wrapped_value();
            }
        } else {
            auto it = prop_serializers.find(p.get_type());
            if(it != prop_serializers.end()) {
                serializer = it->second;
                value = p.get_value(c);
            }
        }

        if(serializer) {
            j[p.get_name().to_string()]["type"] = p.get_type().get_name().to_string();
            serializer(j[p.get_name().to_string()]["value"], value);
        } else {
            std::cout << "Can't serialize property '" << p.get_name().to_string() << "' of type " << p.get_type().get_name().to_string() << std::endl;
        }
    }

    result = jroot.dump();

    return result;
}

bool SceneSerializer::DeserializeScene(unsigned char* data, size_t size, SceneObject& scene) {
    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));
    mz_zip_reader_init_mem(&archive, data, size, 0);

    mz_uint num_files = mz_zip_reader_get_num_files(&archive);
    for(mz_uint i = 0; i < num_files; ++i) {
        mz_zip_archive_file_stat file_stat;
        if(!mz_zip_reader_file_stat(&archive, i, &file_stat)) {
            LOG("Failed to get file stat for file at index " << i);
            continue;
        }
        std::string z_filename = file_stat.m_filename;
        const std::string resource_prefix = "resources/";
        if(z_filename.compare(0, resource_prefix.size(), resource_prefix) == 0) {
            std::string res_name = z_filename.substr(resource_prefix.size());
            size_t res_size = (size_t)file_stat.m_uncomp_size;
            std::vector<char> buf(res_size);
            mz_zip_reader_extract_file_to_mem(
                &archive, 
                z_filename.c_str(), 
                (void*)buf.data(), 
                res_size, 0
            );
            importData.data_sources.insert(
                std::make_pair(
                    res_name,
                    DataSourceRef(new DataSourceMemory(buf.data(), buf.size()))
                )
            );
            LOG("Read local data source '" << res_name << "'");
            continue;
        }
    }

    for(mz_uint i = 0; i < num_files; ++i) {
        mz_zip_archive_file_stat file_stat;
        if(!mz_zip_reader_file_stat(&archive, i, &file_stat)) {
            LOG("Failed to get file stat for file at index " << i);
            continue;
        }

        std::string z_filename = file_stat.m_filename;
        const std::string resource_prefix = "resources/";
        if(z_filename.compare(0, resource_prefix.size(), resource_prefix) == 0)
            continue;

        if(z_filename == "UID") {
            int64_t uid = 0;
            if(!mz_zip_reader_extract_file_to_mem(&archive, z_filename.c_str(), (void*)&uid, sizeof(uid), 0)) {
                LOG("Failed to extract file '" << z_filename << "'");
                continue;
            }
            importData.AddObject(uid, &scene);
            continue;
        } else if(z_filename == "Name") {
            std::string name;
            name.resize(file_stat.m_uncomp_size);
            if(!mz_zip_reader_extract_file_to_mem(&archive, z_filename.c_str(), (void*)name.data(), file_stat.m_uncomp_size, 0)) {
                LOG("Failed to extract file '" << z_filename << "'");
                continue;
            }
            scene.Name(name);
            continue;
        }

        const std::string ext_scene = ".scn";
        const std::string ext_comp = ".comp";
        if(z_filename.size() >= ext_scene.size() 
            && z_filename.compare(z_filename.size() - ext_scene.size(), ext_scene.size(), ext_scene) == 0) {
            LOG("Reading scene object '" << z_filename << "'");
            std::vector<char> buf(file_stat.m_uncomp_size);
            if(!mz_zip_reader_extract_file_to_mem(&archive, z_filename.c_str(), (void*)buf.data(), file_stat.m_uncomp_size, 0)) {
                LOG("Failed to extract file '" << z_filename << "'");
                continue;
            }
            DeserializeScene((unsigned char*)buf.data(), file_stat.m_uncomp_size, *scene.CreateObject());

        } else if(z_filename.size() >= ext_comp.size() 
            && z_filename.compare(z_filename.size() - ext_comp.size(), ext_comp.size(), ext_comp) == 0) {
            LOG("Reading component '" << z_filename << "'");
            std::vector<char> buf(file_stat.m_uncomp_size);
            if(!mz_zip_reader_extract_file_to_mem(&archive, z_filename.c_str(), (void*)buf.data(), file_stat.m_uncomp_size, 0)) {
                LOG("Failed to extract file '" << z_filename << "'");
                continue;
            }
            std::vector<std::string> tokens = split(z_filename, '.');
            if(tokens.size() != 2) {
                LOG("Invalid component file name: " << z_filename);
                continue;
            }
            std::string component_name = tokens[0];
            Component* c = scene.Get(component_name);
            if(!c) {
                LOG("Failed to create component '" << component_name << "'");
                continue;
            }
            rttr::type type = rttr::type::get_by_name(component_name);
            if(!type.is_valid()) {
                LOG("Unidentified component type '" << component_name << "'");
                continue;
            }

            nlohmann::json json_root;
            try
            {
                json_root = nlohmann::json::parse(buf);
            }
            catch(std::exception& ex)
            {
                std::cout << component_name << " - invalid component json: " << ex.what() << std::endl;
                continue;
            }

            importData.deferred_tasks.emplace_back([this, json_root, type, c]() mutable {
                {
                    auto it = custom_component_readers.find(type);
                    if(it != custom_component_readers.end()) {
                        it->second(c, json_root["ext"]);
                    }
                }

                nlohmann::json json = json_root["props"];
                
                for(auto it = json.begin(); it != json.end(); ++it)
                {
                    if(!it.value().is_object()) continue;
                    rttr::property prop = type.get_property(it.key());
                    rttr::type prop_type = prop.get_type();

                    if(prop.get_type().is_derived_from<i_resource_ref>()) {
                        nlohmann::json j = it.value()["value"];
                        std::cout << j.dump() << std::endl;
                        Resource::STORAGE storage = 
                            (Resource::STORAGE)j["storage"].get<int>();
                        std::string name = 
                            j["name"].get<std::string>();

                        if(storage == Resource::LOCAL) {
                            auto res_it = resources.find(name);
                            if(res_it != resources.end()) {
                                rttr::variant v = prop.get_value(c);
                                v.get_value<i_resource_ref>().set_unsafe(res_it->second);
                                prop.set_value(c, v);
                            } else {
                                std::cout << "Searching for " << name << " local data source" << std::endl;
                                if(importData.data_sources.count(name) == 0) {
                                    std::cout << "Local resource " << name << " doesn't exist" << std::endl;
                                    continue;
                                }
                                rttr::variant v = prop.get_value(c);
                                std::shared_ptr<Resource> res = 
                                    v.get_value<i_resource_ref>().set_from_data(importData.data_sources[name]);
                                res->Name(name);
                                prop.set_value(c, v);
                                resources[name] = res;
                            }
                        } else if(storage == Resource::GLOBAL) {
                            rttr::variant v = prop.get_value(c);
                            v.get_value<i_resource_ref>().set_from_factory(GlobalResourceFactory(), name);
                            prop.set_value(c, v);
                        }
                        continue;
                    } else {
                        rttr::variant var = prop.get_value(c);
                        JsonPropertyToVariant(var, it.value());
                        prop.set_value(c, var);
                    }
                }
            });
        }
        // TODO:
    }

    mz_zip_reader_end(&archive);
    return true;
}
/*
bool SceneSerializer::DeserializeScene(unsigned char* data, size_t size, SceneObject& scene)
{
    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));
    mz_zip_reader_init_mem(&archive, data, size, 0);

    mz_uint num_files = mz_zip_reader_get_num_files(&archive);
    for(mz_uint i = 0; i < num_files; ++i) {
        mz_zip_archive_file_stat file_stat;
        if(!mz_zip_reader_file_stat(&archive, i, &file_stat))
        {
            LOG("Failed to get file stat for file at index " << i);
            continue;
        }
        std::string z_filename = file_stat.m_filename;
        const std::string resource_prefix = "resources/";
        if(z_filename.compare(0, resource_prefix.size(), resource_prefix) == 0)
        {
            std::string res_name = z_filename.substr(resource_prefix.size());
            size_t res_size = (size_t)file_stat.m_uncomp_size;
            std::vector<char> buf(res_size);
            mz_zip_reader_extract_file_to_mem(
                &archive, 
                z_filename.c_str(), 
                (void*)buf.data(), 
                res_size, 0
            );
            data_sources.insert(
                std::make_pair(
                    res_name,
                    DataSourceRef(new DataSourceMemory(buf.data(), buf.size()))
                )
            );
            continue;
        }
    }

    std::vector<std::function<void()>> deferred_tasks;

    for(mz_uint i = 0; i < num_files; ++i)
    {
        mz_zip_archive_file_stat file_stat;
        if(!mz_zip_reader_file_stat(&archive, i, &file_stat))
        {
            LOG("Failed to get file stat for file at index " << i);
            continue;
        }

        std::string z_filename = file_stat.m_filename;
        const std::string resource_prefix = "resources/";
        if(z_filename.compare(0, resource_prefix.size(), resource_prefix) == 0)
        {
            continue;
        }

        auto tokens = split(file_stat.m_filename, '/');

        std::function<void(const std::string&, SceneObject*&)> current_state;
        std::function<void(const std::string&, SceneObject*&)> lambda_object;
        std::function<void(const std::string&, SceneObject*&)> lambda_component;
        std::function<void(const std::string&, SceneObject*&)> lambda_child;

        lambda_object = [&](
            const std::string& t, 
            SceneObject*& s
        ){
            if(t == "objects") current_state = lambda_child;
            else if(t == "components") current_state = lambda_component;
            else if(t == "object_info.json") {
                std::vector<char> buf;
                buf.resize((size_t)file_stat.m_uncomp_size);
                mz_zip_reader_extract_file_to_mem(
                    &archive, 
                    file_stat.m_filename,
                    buf.data(),
                    buf.size(),
                    0
                );

                nlohmann::json json;
                try {
                    json = nlohmann::json::parse(buf);
                }
                catch(std::exception& ex) {
                    std::cout << t << " - invalid object_info json: " << ex.what() << std::endl;
                    return;
                }

                nlohmann::json juid = json["uid"];
                nlohmann::json jname = json["name"];
                if(!juid.is_number_integer()) {
                    std::cout << "object_info: uid is not an integer" << std::endl;
                    return;
                }
                if(!jname.is_string()) {
                    std::cout << "object_info: name is not a string" << std::endl;
                    return;
                }
                s->Name(jname.get<std::string>());
                importData.AddObject(juid.get<int64_t>(), s);
                std::cout << "UID: " << juid.get<int64_t>() << ", NAME: " << jname.get<std::string>() << std::endl;
            }
        };
        lambda_component = [&](
            const std::string& t, 
            SceneObject*& s
        ){
            Component* c = s->Get(t);
            if(!c) return;
            rttr::type type = rttr::type::get_by_name(t);
            if(!type.is_valid()) return;
            
            std::vector<char> buf;
            buf.resize((size_t)file_stat.m_uncomp_size);
            mz_zip_reader_extract_file_to_mem(
                &archive, 
                file_stat.m_filename,
                buf.data(),
                buf.size(),
                0
            );

            nlohmann::json json_root;
            try
            {
                json_root = nlohmann::json::parse(buf);
            }
            catch(std::exception& ex)
            {
                std::cout << t << " - invalid component json: " << ex.what() << std::endl;
                return;
            }

            deferred_tasks.emplace_back([this, json_root, type, c]() mutable {
                {
                    auto it = custom_component_readers.find(type);
                    if(it != custom_component_readers.end()) {
                        it->second(c, json_root["ext"]);
                    }
                }

                nlohmann::json json = json_root["props"];
                
                for(auto it = json.begin(); it != json.end(); ++it)
                {
                    if(!it.value().is_object()) continue;
                    rttr::property prop = type.get_property(it.key());
                    rttr::type prop_type = prop.get_type();

                    if(prop.get_type().is_derived_from<i_resource_ref>()) {
                        nlohmann::json j = it.value()["value"];
                        std::cout << j.dump() << std::endl;
                        Resource::STORAGE storage = 
                            (Resource::STORAGE)j["storage"].get<int>();
                        std::string name = 
                            j["name"].get<std::string>();

                        if(storage == Resource::LOCAL) {
                            auto res_it = resources.find(name);
                            if(res_it != resources.end()) {
                                rttr::variant v = prop.get_value(c);
                                v.get_value<i_resource_ref>().set_unsafe(res_it->second);
                                prop.set_value(c, v);
                            } else {
                                std::cout << "Searching for " << name << " local data source" << std::endl;
                                if(data_sources.count(name) == 0) {
                                    std::cout << "Local resource " << name << " doesn't exist" << std::endl;
                                    continue;
                                }
                                rttr::variant v = prop.get_value(c);
                                std::shared_ptr<Resource> res = 
                                    v.get_value<i_resource_ref>().set_from_data(data_sources[name]);
                                res->Name(name);
                                prop.set_value(c, v);
                                resources[name] = res;
                            }
                        } else if(storage == Resource::GLOBAL) {
                            rttr::variant v = prop.get_value(c);
                            v.get_value<i_resource_ref>().set_from_factory(GlobalResourceFactory(), name);
                            prop.set_value(c, v);
                        }
                        continue;
                    } else {
                        rttr::variant var = prop.get_value(c);
                        JsonPropertyToVariant(var, it.value());
                        prop.set_value(c, var);
                    }
                }
            });
        };
        lambda_child = [&](
            const std::string& t, 
            SceneObject*& s
        ){            
            SceneObject* child = s->FindChild(t);
            if(!child)
            {
                child = s->CreateObject();
            }
            if(child) s = child;

            current_state = lambda_object;
        };

        current_state = lambda_object;
        SceneObject* s = &scene;
        for(auto t : tokens)
        {
            current_state(t, s);
        }
    }

    mz_zip_reader_end(&archive);

    for(auto t : deferred_tasks) {
        t();
    }

    return true;
}
*/

rttr::variant SceneSerializer::JsonPropertyToVariant(rttr::variant& var, nlohmann::json& j)
{
    using json = nlohmann::json;

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

std::map<rttr::type, SceneSerializer::json_prop_parser_t> SceneSerializer::InitPropertyParsers()
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
    parsers[rttr::type::get<SceneObject*>()] = [this](rttr::variant& v, nlohmann::json& j) {
        if(!j.is_number_integer()) return;
        int64_t uid = j.get<int64_t>();
        if(uid == 0) return;
        SceneObject* so = importData.GetObjectByImportUid(uid);
        if(!so) return;
        v = so;
    };
    parsers[rttr::type::get<std::weak_ptr<SceneObject>>()] = [this](rttr::variant& v, nlohmann::json& j) {
        if(!j.is_number_integer()) return;
        int64_t uid = j.get<int64_t>();
        if(uid == 0) return;
        SceneObject* so = importData.GetObjectByImportUid(uid);
        if(!so) return;
        v = so->WeakPtr();
    };
    /*
    parsers[rttr::type::get<ResourceRef>()] = [this](rttr::variant& v, nlohmann::json& j){
        if(!j.is_object()) return;
        if(!v.is_valid()) {
            LOG("Parsing ResourceRef property: Variant is not valid");
            return;
        }
        
        //ResourceRef ref(j.get<std::string>());
        //v = ref;

        ResourceRef& ref = v.get_value<ResourceRef>();
        if(!ref.IsValid()) {
            LOG("Parsing ResourceRef property: ResourceRef is not valid");
            return;
        }

        Resource::STORAGE storage = (Resource::STORAGE)j["storage"].get<int>();
        std::string name = j["name"].get<std::string>();

        if(storage == Resource::LOCAL) {
            auto it = data_sources.find(name);
            if(it != data_sources.end()) {
                auto rit = resources.find(name);
                std::shared_ptr<Resource> res;
                if(rit != resources.end()) {
                    res = rit->second;
                } else {
                    res = ref.MakeResource(it->second);
                    resources[name] = res;
                }
                ref.SetUnsafe(res);
            }
        } else if(storage == Resource::GLOBAL) {
            ref.Set(name);
        }
    };
    */

    return parsers;
}

template<typename T>
void DeserializeResource(std::shared_ptr<T>& res) {

}