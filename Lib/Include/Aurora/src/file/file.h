#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <windows.h>
#include <Shlwapi.h>
#include <string>

class File
{
public:
    enum MODE
    {
        READ = OF_READ,
        WRITE = OF_WRITE,
        READWRITE = OF_READWRITE,
        CREATE = OF_CREATE,
    };

    enum SEEKFROM
    {
        BEGIN = FILE_BEGIN,
        CURRENT = FILE_CURRENT,
        END = FILE_END
    };

    File(){}
    ~File(){ if (readBuffer.sz > 0) readBuffer.Free(); }

    static File Create(std::string path, File::MODE mode = READWRITE);
    static File Open(std::string path, MODE mode = READWRITE);
    static bool Exists(std::string path);
    unsigned char* Read(size_t sz, unsigned int& bytes_read);
    template<typename T>
    T Read(unsigned int& bytes_read);
    void Write(void* data, size_t sz);
    void Write(std::string data);
    void Seek(unsigned int dist, SEEKFROM from = BEGIN);
    void Close();
    size_t Size() { return sz; }
	std::string Name() { return name; }
	std::string Path() { return path; }
protected:
    struct Buffer
    {
        Buffer() : sz(0){}
        void Alloc(size_t sz)
        {
            data = (unsigned char*)malloc(sz);
            this->sz = sz;
        }
        void Free()
        {
            free(data);
            this->sz = 0;
        }
        unsigned char* data;
        size_t sz;
    };
    Buffer readBuffer;

    HANDLE file;
    size_t sz;
	std::string name;
	std::string path;
};

template<typename T>
T File::Read(unsigned int& bytes_read)
{
    T value;
    unsigned int bytesRead;
    unsigned char* data = Read(sizeof(T), bytesRead);
    if(data && bytesRead >= sizeof(T))
    {
        value = *((T*)data);
    }
    
    return value;
}

template<>
std::string File::Read<std::string>(unsigned int& bytes_read);

#endif
