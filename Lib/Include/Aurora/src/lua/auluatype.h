#ifndef AU_LUA_TYPE_H
#define AU_LUA_TYPE_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <typeinfo>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "auluavalue.h"

namespace Au{
    
template<typename T>
static int LuaTypeIndex()
{
    static int i = NextLuaTypeIndex();
    return i;
}
static int NextLuaTypeIndex()
{
    static int id;
    return ++id;
}

template<typename T> 
inline int _pushFn(lua_State* L, void* data) 
{
    void** userDataPtr = (void**)lua_newuserdata(L, sizeof(void*));
    (*userDataPtr) = data;
    return 0;
}
template<> 
inline int _pushFn<char>(lua_State* L, void* data) 
{ lua_pushnumber(L, (lua_Number)*(char*)data); return 1; }
template<> 
inline int _pushFn<unsigned char>(lua_State* L, void* data) 
{ lua_pushnumber(L, (lua_Number)*(unsigned char*)data); return 1; }
template<> 
inline int _pushFn<short>(lua_State* L, void* data) 
{ lua_pushnumber(L, (lua_Number)*(short*)data); return 1; }
template<> 
inline int _pushFn<unsigned short>(lua_State* L, void* data) 
{ lua_pushnumber(L, (lua_Number)*(unsigned short*)data); return 1; }
template<> 
inline int _pushFn<int>(lua_State* L, void* data) 
{ lua_pushnumber(L, (lua_Number)*(int*)data); return 1; }
template<> 
inline int _pushFn<unsigned int>(lua_State* L, void* data) 
{ lua_pushnumber(L, (lua_Number)*(unsigned int*)data); return 1; }
template<> 
inline int _pushFn<long>(lua_State* L, void* data) 
{ lua_pushnumber(L, (lua_Number)*(long*)data); return 1; }
template<> 
inline int _pushFn<unsigned long>(lua_State* L, void* data) 
{ lua_pushnumber(L, (lua_Number)*(unsigned long*)data); return 1; }
template<> 
inline int _pushFn<long long>(lua_State* L, void* data) 
{ lua_pushnumber(L, (lua_Number)*(long long*)data); return 1; }
template<> 
inline int _pushFn<unsigned long long>(lua_State* L, void* data) 
{ lua_pushnumber(L, (lua_Number)*(unsigned long long*)data); return 1; }
template<> 
inline int _pushFn<float>(lua_State* L, void* data) 
{ lua_pushnumber(L, *(float*)data); return 1; }
template<> 
inline int _pushFn<double>(lua_State* L, void* data) 
{ lua_pushnumber(L, *(double*)data); return 1; }
template<> 
inline int _pushFn<char*>(lua_State* L, void* data) 
{ lua_pushstring(L, (char*)data); return 1; }
template<> 
inline int _pushFn<std::string>(lua_State* L, void* data) 
{ lua_pushstring(L, (*(std::string*)data).c_str()); return 1; }
template<> 
inline int _pushFn<bool>(lua_State* L, void* data) 
{ lua_pushboolean(L, (int)*(bool*)data); return 1; }

template<typename T>
inline void _popFn(lua_State* L, LuaValue& data) 
{ 
    void** userDataPtr = (void**)lua_touserdata(L, -1);
    data = *userDataPtr;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<char>(lua_State* L, LuaValue& data) 
{
    char v = (char)lua_tonumber(L, -1); 
    data = v; 
    lua_pop(L, 1); 
}
template<>
inline void _popFn<unsigned char>(lua_State* L, LuaValue& data) 
{
    unsigned char v = (unsigned char)lua_tonumber(L, -1); 
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<short>(lua_State* L, LuaValue& data) 
{
    short v = (short)lua_tonumber(L, -1); 
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<unsigned short>(lua_State* L, LuaValue& data) 
{
    unsigned short v = (unsigned short)lua_tonumber(L, -1); 
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<int>(lua_State* L, LuaValue& data) 
{
    int v = (int)lua_tonumber(L, -1);
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<unsigned int>(lua_State* L, LuaValue& data) 
{
    unsigned int v = (unsigned int)lua_tonumber(L, -1); 
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<long>(lua_State* L, LuaValue& data) 
{
    long v = (long)lua_tonumber(L, -1); 
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<unsigned long>(lua_State* L, LuaValue& data) 
{
    unsigned long v = (unsigned long)lua_tonumber(L, -1); 
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<long long>(lua_State* L, LuaValue& data) 
{
    long long v = (long long)lua_tonumber(L, -1); 
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<unsigned long long>(lua_State* L, LuaValue& data) 
{
    unsigned long long v = (unsigned long long)lua_tonumber(L, -1);
    data = v; 
    lua_pop(L, 1); 
}
template<>
inline void _popFn<float>(lua_State* L, LuaValue& data) 
{
    float v = (float)lua_tonumber(L, -1); 
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<double>(lua_State* L, LuaValue& data) 
{
    double v = (double)lua_tonumber(L, -1);
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<char*>(lua_State* L, LuaValue& data) 
{
    char* v = (char*)lua_tostring(L, -1); 
    data = v;
    lua_pop(L, 1); 
}
template<>
inline void _popFn<std::string>(lua_State* L, LuaValue& data) 
{
    data = std::string(lua_tostring(L, -1)); 
    lua_pop(L, 1); 
}
template<>
inline void _popFn<bool>(lua_State* L, LuaValue& data) 
{
    bool v = lua_toboolean(L, -1) != 0;
    data = v;
    lua_pop(L, 1); 
}

template<typename T>
static int _userDataGC(lua_State* L)
{
    T** data = (T**)lua_touserdata(L, 1);
    if(*data != 0)
        delete *data;
    return 0;
}

template<typename T>
inline void _pushDeleterFn(lua_State* L)
{
    lua_pushcfunction(L, _userDataGC<T>);
    lua_setfield(L, -2, "__gc");
}

struct LuaType
{    
    struct MemberField
    {
        MemberField()
        : type(0), offset(0) {}
        MemberField(LuaType* type, char* offset, const std::string& name)
        : type(type), offset(offset), name(name) {}
        LuaType* type;
        char* offset;
        std::string name;
    };
    
    struct MemberFunc
    {
        MemberFunc()
        : statePtr(0), fnIndex(0) {}
        MemberFunc(const std::string& name, void* statePtr, int fnIndex, int (*cfn)(lua_State*))
        : name(name), statePtr(statePtr), fnIndex(fnIndex), cfn(cfn) {}
        std::string name;
        void* statePtr;
        int fnIndex;
        int (*cfn)(lua_State*);
    };

    LuaType()
    : _typeIndex(0),
      _push(0),
      _pop(0),
      _pushDeleter(0)
    {}
    
    template<typename T>
    LuaType(T* dummy)
    : _typeIndex(LuaTypeIndex<bare_type<T>::type>()),
      _push(&_pushFn<bare_type<T>::type>),
      _pop(&_popFn<bare_type<T>::type>),
      _pushDeleter(_pushDeleterFn<bare_type<T>::type>)
    {}
    
    template<typename T>
    LuaType(T** dummy)
    : _typeIndex(LuaTypeIndex<bare_type<T>::type>()),
      _push(&_pushFn<bare_type<T>::type>),
      _pop(&_popFn<bare_type<T>::type>),
      _pushDeleter(0)
    {}
    
    int _typeIndex;
    std::vector<MemberField> members;
    std::vector<MemberFunc> functions;
    
    template<typename T>
    static LuaType& Get()
    {
        static LuaType type = LuaType((bare_type<T>::type*)0);
        return type;
    }
    
    template<typename T>
    static LuaType* GetPtr()
    { return &(Get<T>()); }
    
    template<typename Type, typename Class>
    LuaType& Member(Type Class::* member, const std::string& name)
    {
        if(LuaTypeIndex<Class>() != _typeIndex)
            return *this;
        MemberField m;
        m.type = LuaType::GetPtr<Type>();
        Class* c = 0;
        m.offset = (char*)((char*)&(c->*member) - (char*)c);
        m.name = name;
        members.push_back(m);
        return *this;
    }
    
    void Function(const MemberFunc& func)
    {
        functions.push_back(func);
    }
    
    void LuaPush(lua_State* L, void* data, bool member = false)
    {
        if(!_push)
        {
            lua_pushnil(L);
            return;
        }
        
        _push(L, data);
        
        lua_createtable(L, 0, 0);
        
        lua_createtable(L, 0, 0);
        
        for(unsigned i = 0; i < members.size(); ++i)
        {
            _luaPushMember(L, data, members[i]);            
        }
        
        for(unsigned i = 0; i < functions.size(); ++i)
        {
            lua_pushnumber(L, functions[i].fnIndex);
            lua_pushlightuserdata(L, functions[i].statePtr);
            lua_pushcclosure(L, functions[i].cfn, 2);
            lua_setfield(L, -2, functions[i].name.c_str());
        }
        
        lua_setfield(L, -2, "__index");
        
        if(!member)
            if(_pushDeleter) 
                _pushDeleter(L);
        
        lua_setmetatable(L, -2);
    }
    
    void _luaPushMember(lua_State* L, void* data, MemberField& m)
    {
        m.type->LuaPush(L, (void*)((char*)data + (int)m.offset), true);
        lua_setfield(L, -2, m.name.c_str());
    }
    
    void LuaPop(lua_State* L, LuaValue& data)
    {
        if(!_pop)
            return;
        
        _pop(L, data);
        /*
        if(!members.empty())
        {
            lua_getmetatable(L, -1);
            lua_pushstring(L, "__index");
            lua_gettable(L, -2);
        }
        
        for(unsigned i = 0; i < members.size(); ++i)
        {
            lua_pushstring(L, members[i].name.c_str());
            lua_gettable(L, -2);
            members[i].type->LuaPop(L, (void*)((char*)data.Ptr() + (int)members[i].offset));
        }
        */
    }
private:
    int (*_push)(lua_State*, void*);
    void (*_pop)(lua_State*, LuaValue&);
    void (*_pushDeleter)(lua_State*);
};

}

#endif
