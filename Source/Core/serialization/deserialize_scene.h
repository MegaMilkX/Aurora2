#ifndef DESERIALIZE_SCENE_H
#define DESERIALIZE_SCENE_H

#include <scene_object.h>
#define MINIZ_HEADER_FILE_ONLY
#include <lib/miniz.c>
#include <util/has_suffix.h>
#include <util/split.h>
#include "import_data.h"

inline bool DeserializeScene(SceneObject* scene, std::vector<char>& data) {
    const std::string ext_scene = ".scn";
    const std::string ext_comp = ".comp";
    
    ImportData importData;
    
    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));
    mz_zip_reader_init_mem(&archive, data.data(), data.size(), 0);

    mz_uint num_files = mz_zip_reader_get_num_files(&archive);
    for(mz_uint i = 0; i < num_files; ++i) {
        mz_zip_archive_file_stat file_stat;
        if(!mz_zip_reader_file_stat(&archive, i, &file_stat)) {
            LOG("Failed to get file stat for file at index " << i);
            continue;
        }
        std::string z_filename = file_stat.m_filename;
        if(z_filename == "uid") {
            int64_t uid = 0;
            if(!mz_zip_reader_extract_to_mem(&archive, i, (void*)&uid, sizeof(uid), 0)) {
                LOG("Failed to extract file '" << z_filename << "'");
                continue;
            }
            //importData.AddObject(uid, &scene);
            continue;
        } else if(z_filename == "name") {
            std::string name;
            name.resize(file_stat.m_uncomp_size);
            if(!mz_zip_reader_extract_to_mem(&archive, i, (void*)name.data(), file_stat.m_uncomp_size, 0)) {
                LOG("Failed to extract file '" << z_filename << "'");
                continue;
            }
            scene->Name(name);
            continue;
        } else if(has_suffix(z_filename, ext_scene)) {
            // Reading a scene object
            std::vector<char> buf(file_stat.m_uncomp_size);
            if(!mz_zip_reader_extract_to_mem(&archive, i, (void*)buf.data(), file_stat.m_uncomp_size, 0)) {
                LOG("Failed to extract file '" << z_filename << "'");
                continue;
            }
            DeserializeScene(scene->CreateObject(), buf);
            
            continue;
        } else if(has_suffix(z_filename, ext_comp)) {
            // Reading component
            std::vector<char> buf((size_t)file_stat.m_uncomp_size);
            if(!mz_zip_reader_extract_to_mem(&archive, i, (void*)buf.data(), file_stat.m_uncomp_size, 0)) {
                LOG("Failed to extract file '" << z_filename << "'");
                continue;
            }
            std::vector<std::string> tokens = split(z_filename, '.');
            if(tokens.size() != 2) {
                LOG("Invalid component file name: " << z_filename);
                continue;
            }
            std::string component_name = tokens[0];
            Component* c = scene->Get(component_name);
            if(!c) {
                LOG("Failed to create component '" << component_name << "'");
                continue;
            }
            std::stringstream ss;
            ss.write(buf.data(), buf.size());
            size_t comp_data_sz = buf.size();
            if(!c->_read(ss, comp_data_sz, importData)) {
                LOG("Component " + component_name + " read failed. Expect default state")
            }

            continue;
        }
    }
    
    mz_zip_reader_end(&archive);
    return true;
}

inline bool DeserializeScene(SceneObject* scene, const std::string& filename) {
    std::ifstream f(filename, std::ios::binary | std::ios::ate);
    if(!f.is_open()) {
        std::cout << "Failed to open " << filename << std::endl;
        return false;
    }
    std::streamsize size = f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<char> buffer((unsigned int)size);
    if(!f.read(buffer.data(), (unsigned int)size)) {
        f.close();
        return false;
    }

    DeserializeScene(scene, buffer);
    
    return true;
}

#endif
