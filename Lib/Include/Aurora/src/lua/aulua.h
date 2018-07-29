#ifndef AU_LUA_H
#define AU_LUA_H

#include <fstream>
#include <streambuf>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "auluafunc.h"

namespace Au{

int _luaProc(lua_State* L);

class Lua
{
public:
    Lua()
    : L(0) {}
    ~Lua()
    { if(L) lua_close(L); }
    void Init()
    {
        L = luaL_newstate();
        /*
        lua_createtable(L, 0, 0);
        lua_pushnumber(L, 10);
        lua_setfield(L, -2, "boopValue");
        lua_pushnumber(L, 3);
        lua_setfield(L, -2, "boopValue2");
        lua_setglobal(L, "Table");
        */
        
        //lua_pushstring(L, "Pepperoni");
        //lua_setglobal(L, "MyString");
    }
    void Cleanup()
    {
        
    }
    
    bool LoadSource(const std::string& source)
    {
        int ret;
        ret = luaL_loadbuffer(
            L, 
            source.c_str(), 
            source.size(), 
            "script"
        );
        if(ret != 0)
        {
            
            return false;
        }
        
        lua_pcall(L, 0, 0, 0);
        
        return true;
    }
    
    void DoFile(const std::string& filename)
    {
        std::ifstream file(filename);
        std::string source((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());
                 
        LoadSource(source);
        
        const char* err = lua_tostring(L, -1);
        if(err)
            std::cout << err << std::endl;
        
        file.close();
    }
    
    void PushArgs() { }
    
    template<typename Arg, typename... Args>
    void PushArgs(const Arg& arg, const Args&... args)
    { 
        LuaType::GetPtr<Arg>()->LuaPush(L, (void*)&arg);
        PushArgs(args...);
    }
    
    template<typename... Args>
    void Call(const std::string& funcName, const Args&... args)
    {
        lua_getglobal(L, funcName.c_str());
        if(lua_isfunction(L, -1) == 0)
        {
            lua_pop(L, 1);
            return;
        }
        PushArgs(args...);
        lua_pcall(L, sizeof...(Args), LUA_MULTRET, 0);
        
        if(lua_gettop(L) && lua_isstring(L, -1))
        {
            std::cout << lua_tostring(L, -1) << std::endl;
        }
    }

    template<typename... Args>
    void CallMember(const std::string& tableName, const std::string& funcName, const Args&... args)
    {
        lua_getglobal(L, tableName.c_str());
        if(lua_isnil(L, -1) == 1)
        {
            lua_pop(L, 1);
            return;
        }
        lua_getfield(L, -1, funcName.c_str());
        if(lua_isfunction(L, -1) == 0)
        {
            lua_pop(L, 2);
            return;
        }
        PushArgs(args...);
        lua_pcall(L, sizeof...(Args), LUA_MULTRET, 0);
        if(lua_gettop(L) && lua_isstring(L, -1))
        {
            std::cout << lua_tostring(L, -1) << std::endl;
        }
        lua_pop(L, 1);
    }
    
    // Type shenanigans ==================
    
    template<typename T>
    LuaType& BindType()
    { return LuaType::Get<T>(); }
    
    template<typename T>
    void SetGlobal(const std::string& name, T* value)
    {
        LuaType* type = LuaType::GetPtr<T*>();
        type->LuaPush(L, (void*)value);
        lua_setglobal(L, name.c_str());
    }

    template<typename T>
    void SetGlobal(const std::string& name, const T& value)
    {
        LuaType* type = LuaType::GetPtr<T*>();
        type->LuaPush(L, (void*)value);
        lua_setglobal(L, name.c_str());
    }
    
    template<typename T>
    T GetGlobal(const std::string& name)
    {
        LuaType* type = LuaType::GetPtr<T*>();
        LuaValue val = T();
        lua_getglobal(L, name.c_str());
        type->LuaPop(L, val);
        return val.Get<T>();
    }
    
    LuaFunc& GetFunction(unsigned id)
    { return _funcs[id]; }
    template<typename Ret, typename... Args>
    void Bind(Ret (*fn)(Args... args), const std::string& name)
    {
        LuaFunc func = LuaFunc(fn);
        _luaBindFunc(_funcs.size(), name);
        _funcs.push_back(func);
    }
    
    template<typename Type, typename Class>
    void Bind(Type Class::* member, const std::string& name)
    {
        LuaType* t = LuaType::GetPtr<Class>();
        LuaType* tptr = LuaType::GetPtr<Class*>();
        t->Member(member, name);
        tptr->Member(member, name);
    }
    
    template<typename Class, typename Ret, typename... Args>
    void Bind(Ret (Class::*fn)(Args... args), const std::string& name)
    {
        LuaType* t = LuaType::GetPtr<Class>();
        LuaType* tptr = LuaType::GetPtr<Class*>();
        LuaFunc func = LuaFunc(fn);
        t->Function(LuaType::MemberFunc(name, this, _funcs.size(), &_luaProc));
        tptr->Function(LuaType::MemberFunc(name, this, _funcs.size(), &_luaProc));
        _funcs.push_back(func);
    }
    
private:
    void _luaBindFunc(unsigned funcIndex, const std::string& name)
    {
        lua_pushnumber(L, funcIndex);
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, &_luaProc, 2);
        lua_setglobal(L, name.c_str());
    }
    lua_State* L;
    
    std::vector<LuaFunc> _funcs;
};

inline int _luaProc(lua_State* L)
{
    Lua* lua = (Lua*)lua_touserdata(L, lua_upvalueindex(2));
    LuaFunc func = lua->GetFunction((int)lua_tonumber(L, lua_upvalueindex(1)));

    if(!func.GrabArgs(L))
        return 0;
    
    return func.Call(L);
}

}

#endif
