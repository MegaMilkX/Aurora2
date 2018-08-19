#ifndef RESOURCE_RAW_H
#define RESOURCE_RAW_H

#include <vector>
#include <memory>

class ResourceRaw
{
public:
    virtual ~ResourceRaw() {}

    virtual bool ReadAll(char* dest) = 0;
    virtual uint64_t Size() const = 0; 
};

#define MINIZ_HEADER_FILE_ONLY
#include "../../lib/miniz.c"

class ResourceRawArchive : public ResourceRaw
{
public:
    ResourceRawArchive(uint32_t file_index, std::shared_ptr<mz_zip_archive> archive)
    : file_index(file_index), archive(archive) {}
    virtual bool ReadAll(char* dest) {
        mz_zip_archive_file_stat f_stat;
        mz_zip_reader_file_stat(archive.get(), file_index, &f_stat);
        mz_zip_reader_extract_file_to_mem(archive.get(), f_stat.m_filename, dest, (size_t)Size(), 0);
        return true;
    }
    virtual uint64_t Size() const {
        mz_zip_archive_file_stat f_stat;
        mz_zip_reader_file_stat(archive.get(), file_index, &f_stat);
        return f_stat.m_uncomp_size;
    }
private:
    uint32_t file_index;
    std::shared_ptr<mz_zip_archive> archive;
};

class ResourceRawFilesystem : public ResourceRaw
{
public:
    ResourceRawFilesystem(const std::string& name);
};

#endif
