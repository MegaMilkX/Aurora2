#ifndef SKELETON_H
#define SKELETON_H

#include "resource/resource.h"
#include <util/gfxm.h>
#include <map>
#include <vector>

class Skeleton : public Resource {
    RTTR_ENABLE(Resource)
public:
    struct BonePose {
        std::string name;
        gfxm::mat4 pose;
        bool operator<(const BonePose& b) const {
            return name < b.name;
        }
    };

    void AddBone(const std::string& name, const gfxm::mat4& bindTransform) {
        //bones.emplace_back(name);
        //bindPose[name] = bindTransform;
        bonePoses.emplace_back(
            BonePose{name, bindTransform}
        );
        //std::sort(bonePoses.begin(), bonePoses.end());
    }

    virtual bool Build(DataSourceRef r) {
        if(r->Size() == 0) return false;
        std::vector<char> buffer;
        buffer.resize((size_t)r->Size());
        r->ReadAll((char*)buffer.data());

        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        if(!mz_zip_reader_init_mem(&zip, buffer.data(), buffer.size(), 0))
        {
            //LOG_ERR("Failed to build mesh");
            return false;
        }

        // TODO Read skeleton
        mz_uint num_files = mz_zip_reader_get_num_files(&zip);
        for(mz_uint i = 0; i < num_files; ++i) {
            mz_zip_archive_file_stat file_stat;
            if(!mz_zip_reader_file_stat(&zip, i, &file_stat))
            {
                LOG("Failed to get file stat for file at index " << i);
                continue;
            }
            std::string z_filename = file_stat.m_filename;
            gfxm::mat4 bindTransform;
            mz_zip_reader_extract_file_to_mem(
                &zip, 
                z_filename.c_str(), 
                (void*)&bindTransform, 
                sizeof(gfxm::mat4), 0
            );
            
            AddBone(z_filename, bindTransform);
        }

        mz_zip_reader_end(&zip);
        return true;
    }
    virtual bool Serialize(std::vector<unsigned char>& data) {
        mz_zip_archive zip;
        memset(&zip, 0, sizeof(zip));
        mz_zip_writer_init_heap(&zip, 0, 0);

        // TODO Write skeleton
        for(auto& bonePose : bonePoses) {
            mz_zip_writer_add_mem(&zip, bonePose.name.c_str(), (void*)&bonePose.pose, sizeof(gfxm::mat4), 0);
        }

        void* bufptr;
        size_t sz;
        mz_zip_writer_finalize_heap_archive(&zip, &bufptr, &sz);
        data = std::vector<unsigned char>((unsigned char*)bufptr, (unsigned char*)bufptr + sz);
        mz_zip_writer_end(&zip);
        return true;
    }

    std::vector<BonePose> bonePoses;

    //std::vector<std::string> bones;
    //std::map<std::string, gfxm::mat4> bindPose;
};

#endif
