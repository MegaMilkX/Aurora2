#ifndef SERIALIZE_SCENE_H
#define SERIALIZE_SCENE_H

#include <scene_object.h>
#define MINIZ_HEADER_FILE_ONLY
#include <lib/miniz.c>
#include "export_data.h"

inline bool SerializeScene(const SceneObject* scene, std::vector<char>& buf) {
    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));
    if(!mz_zip_writer_init_heap(&archive, 0, 65537)) {
        LOG("Failed to create archive file in memory");
        return false;
    }

    int64_t uid = scene->Uid();
    std::string name = scene->Name();

    mz_zip_writer_add_mem(
        &archive, MKSTR("uid").c_str(), (void*)&uid, sizeof(uid), 0
    );
    mz_zip_writer_add_mem(
        &archive, MKSTR("name").c_str(), name.c_str(), name.size(), 0
    );

    for(unsigned i = 0; i < scene->ChildCount(); ++i) {
        SceneObject* child = scene->GetChild(i);
        std::vector<char> child_buf;
        SerializeScene(child, child_buf);
        mz_zip_writer_add_mem(
            &archive, (child->Name() + ".scn").c_str(), child_buf.data(), child_buf.size(), 0
        );
    }

    ExportData exportData;
    for(unsigned i = 0; i < scene->ComponentCount(); ++i) {
        Component* comp = scene->GetComponent(i);
        rttr::type type = comp->GetType();
        std::stringstream ss;
        if(comp->_write(ss, exportData)) {
            ss.seekg(0, std::ios::end);
            size_t sz = ss.tellg();
            ss.seekg(0, std::ios::beg);
            std::vector<char> buf;
            buf.resize(sz);
            ss.read((char*)buf.data(), sz);

            if(!mz_zip_writer_add_mem(
                &archive, 
                MKSTR(type.get_name() << ".comp").c_str(), 
                buf.data(), 
                buf.size(), 
                0
            )){
                LOG_ERR("Failed to mz_zip_writer_add_mem() ");
            }
        }
    }

    void* archbuf = 0;
    size_t sz = 0;
    mz_zip_writer_finalize_heap_archive(&archive, &archbuf, &sz);
    buf = std::vector<char>((char*)archbuf, (char*)archbuf + sz);
    mz_zip_writer_end(&archive);    
    return true;
}

inline bool SerializeScene(const SceneObject* scene, const std::string& filename) {
    std::vector<char> buf;
    if(!SerializeScene(scene, buf)) {
        return false;
    }

    std::ofstream f(filename, std::ios::binary);
    if(!f.is_open()) {
        return false;
    }
    f.write(buf.data(), buf.size());
    f.close();

    return true;
}

#endif
