#ifndef DESERIALIZE_SCENE_H
#define DESERIALIZE_SCENE_H

#include <string>
#include <lib/miniz.c>
#include <scene_object.h>
#include <../general/util.h>

#include <json_property_parsers.h>

inline bool DeserializeScene(unsigned char* data, size_t size, SceneObject& scene)
{
    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));
    mz_zip_reader_init_mem(&archive, data, size, 0);

    mz_uint num_files = mz_zip_reader_get_num_files(&archive);
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
            std::string res_name = z_filename.substr(resource_prefix.size());
            size_t res_size = (size_t)file_stat.m_uncomp_size;
            std::vector<char> buf(res_size);
            mz_zip_reader_extract_file_to_mem(
                &archive, 
                z_filename.c_str(), 
                (void*)buf.data(), 
                res_size, 0
            );
            //g_resourceRegistry.Add(res_name, new ResourceRawMemory(buf.data(), buf.size()));
            GlobalDataRegistry().Add(res_name, DataSourceRef(new DataSourceMemory(buf.data(), buf.size())));
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
        };
        lambda_component = [&](
            const std::string& t, 
            SceneObject*& s
        ){
            SceneObject::Component* c = s->Get(t);
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

            nlohmann::json json;
            try
            {
                json = nlohmann::json::parse(buf);
            }
            catch(std::exception& ex)
            {
                std::cout << t << " - invalid component json: " << ex.what() << std::endl;
                return;
            }
            
            for(auto it = json.begin(); it != json.end(); ++it)
            {
                if(!it.value().is_object()) continue;
                // TODO:
                rttr::property prop = type.get_property(it.key());
                rttr::variant var = JsonPropertyToVariant(it.value());
                prop.set_value(c, var);
            }
        };
        lambda_child = [&](
            const std::string& t, 
            SceneObject*& s
        ){            
            SceneObject* child = s->FindChild(t);
            if(!child)
            {
                child = s->CreateObject();
                child->Name(t);
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

    return true;
}

inline bool DeserializeScene(const std::string& filename, SceneObject& scene)
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

    DeserializeScene((unsigned char*)buffer.data(), buffer.size(), scene);

    f.close();
    return true;
}

#endif
