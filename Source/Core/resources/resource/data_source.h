#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

#include <string>
#include <memory>

class DataSource {
public:
    virtual ~DataSource() {}

    virtual bool ReadAll(char* dest) = 0;
    virtual uint64_t Size() const = 0;

    void SetName(const std::string& name) { this->name = name; }
    const std::string& Name() const { return name; }
private:
    std::string name;
};

#define MINIZ_HEADER_FILE_ONLY
#include "../../../lib/miniz.c"

class DataSourceArchive : public DataSource
{
public:
    DataSourceArchive(uint32_t file_index, std::shared_ptr<mz_zip_archive> archive)
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

class DataSourceMemory : public DataSource
{
public:
    DataSourceMemory() {}
    DataSourceMemory(const char* src, size_t size) {
        Fill(src, size);
    }
    ~DataSourceMemory() {}

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

class DataSourceFilesystem : public DataSource {
public:
    DataSourceFilesystem(const std::string& path) {

    }
    virtual bool ReadAll(char* dest) { return false; }
    virtual uint64_t Size() const { return 0; }
private:

};

typedef std::shared_ptr<DataSource> DataSourceRef;

#endif
