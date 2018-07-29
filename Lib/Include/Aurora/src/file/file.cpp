#include "file.h"
#include <iostream>
File File::Create(std::string path, File::MODE mode)
{
    OFSTRUCT ofstruct;
    File file;
    file.file = (HANDLE)OpenFile(path.c_str(), &ofstruct, OF_CREATE | mode);
    DWORD dwFileSizeHigh = 0;
    DWORD dwFileSizeLow = GetFileSize(file.file, &dwFileSizeHigh);
    file.sz = dwFileSizeLow | (__int64)dwFileSizeHigh << 32;

	file.path = path;
	size_t first = path.find_last_of("\\");
	if (first == path.npos)
		first = 0;
	size_t last = path.find_last_of(".");
	file.name = path.substr(first, last - first);
    return file;
}

File File::Open(std::string path, File::MODE mode)
{
    OFSTRUCT ofstruct;
    File file;
    file.file = (HANDLE)OpenFile(path.c_str(), &ofstruct, mode);
    DWORD dwFileSizeHigh = 0;
    DWORD dwFileSizeLow = GetFileSize(file.file, &dwFileSizeHigh);
    file.sz = dwFileSizeLow | (__int64)dwFileSizeHigh << 32;

	file.path = path;
	size_t first = path.find_last_of("\\");
	if (first == path.npos)
		first = 0;
	size_t last = path.find_last_of(".");
	file.name = path.substr(first, last - first);
    return file;
}

bool File::Exists(std::string path)
{
    std::wstring wpath(path.begin(), path.end());
    if(PathFileExists(wpath.c_str()))
    {
        DWORD file_attrib = GetFileAttributes(wpath.c_str());
        if(file_attrib & FILE_ATTRIBUTE_DIRECTORY)
            return false;
        else
            return true;
    }
    else
        return false;
}

unsigned char* File::Read(size_t sz, unsigned int& bytes_read)
{
    if (readBuffer.sz != 0)
    {
        if (readBuffer.sz < sz)
        {
            readBuffer.Free();
            readBuffer.Alloc(sz);
        }
    }
    else
    {
        readBuffer.Alloc(sz);
    }

    memset(readBuffer.data, 0, sizeof(readBuffer.sz));

    DWORD dwBytesRead = 0;
    if (!ReadFile(file, readBuffer.data, sz, &dwBytesRead, NULL))
    {
        return 0;
    }
    bytes_read = dwBytesRead;
    return readBuffer.data;
}

template<>
std::string File::Read<std::string>(unsigned int& bytes_read)
{
    std::string str;
    unsigned char* data = Read(bytes_read, bytes_read);
    if(data)
    {
        str = std::string((char*)data, (char*)data + bytes_read);
    }
    
    return str;
}

void File::Write(void* data, size_t sz)
{
    DWORD dwBytesWritten;
    WriteFile(file, data, sz, &dwBytesWritten, NULL);
}

void File::Write(std::string data)
{
    DWORD dwBytesWritten;
    WriteFile(file, (void*)data.data(), data.length(), &dwBytesWritten, NULL);
}

void File::Seek(unsigned int dist, SEEKFROM from)
{
    LARGE_INTEGER large_int;
    large_int.QuadPart = dist;
    SetFilePointerEx(file, large_int, NULL, from);
}

void File::Close()
{
    CloseHandle((HANDLE)file);
}