#ifndef RESOURCE_RAW_H
#define RESOURCE_RAW_H

#include <vector>
#include <memory>
#include <string>

class ResourceRaw
{
public:
    virtual ~ResourceRaw() {}

    virtual bool ReadAll(char* dest) = 0;
    virtual uint64_t Size() const = 0;

    void SetName(const std::string& name) { this->name = name; }
    const std::string& Name() const { return name; }
private:
    std::string name;
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

class ResourceRawMemory : public ResourceRaw
{
public:
    ResourceRawMemory() {}
    ResourceRawMemory(const char* src, size_t size) {
        Fill(src, size);
    }
    ~ResourceRawMemory() {}

    void Fill(const char* src, size_t size) {
        data.resize(size);
        memcpy((void*)data.data(), src, size);
    }

    virtual bool ReadAll(char* dest) {
        memcpy(dest, data.data(), data.size());
        return true;
    }
    virtual uint64_t Size() const {
        return data.size();
    }
private:
    std::vector<char> data;
};

class ResourceRawFilesystem : public ResourceRaw
{
public:
    ResourceRawFilesystem(const std::string& name)
    : path(name) {

    }

    virtual bool ReadAll(char* dest) {
        return false;
    }
    virtual uint64_t Size() const {
        return 0;
    }
private:
    std::string path;
};

#endif
