#ifndef FBX_PROP_H
#define FBX_PROP_H

#include <vector>
#include <string>

class FbxProp
{
public:
    void SetType(char t);
    char GetType();
    void SetArrayLen(unsigned len);
    void SetData(const std::vector<char>& data);
    std::string GetString();
    int64_t GetInt64();
    int32_t GetInt32();
    float GetFloat();
    double GetDouble();
    template<typename T>
    std::vector<T> GetArray();
    
    void Print(std::ostringstream& sstr, unsigned level = 0);
private:
    char type;
    unsigned arraySize;
    std::vector<char> data;
};

template<typename T>
std::vector<T> FbxProp::GetArray()
{
    std::vector<T> result;
        
    unsigned byteLength = data.size();
    unsigned arrayLength = 0;
    unsigned targetElementSize = sizeof(T);
    
    switch(type)
    {
    case 'f': case 'i': arrayLength = byteLength / 4; break;
    case 'd': case 'l': arrayLength = byteLength / 8; break;
    }
    
    if(arrayLength == 0)
        return result;
    
    switch(type)
    {
    case 'f':
        {
            float* dataptr = (float*)(data.data());
            for(unsigned i = 0; i < arrayLength; ++i)
                result.push_back((T)(dataptr[i]));
        }
        break;
    case 'i':
        {
            int32_t* dataptr = (int32_t*)(data.data());
            for(unsigned i = 0; i < arrayLength; ++i)
                result.push_back((T)(dataptr[i]));
        }
        break;
    case 'd':
        {
            double* dataptr = (double*)(data.data());
            for(unsigned i = 0; i < arrayLength; ++i)
                result.push_back((T)(dataptr[i]));
        }
        break;
    case 'l':
        {
            int64_t* dataptr = (int64_t*)(data.data());
            for(unsigned i = 0; i < arrayLength; ++i)
                result.push_back((T)(dataptr[i]));
        }
        break;
    }
    
    return result;
}


#endif
