#ifndef ECHO_TYPE_HPP
#define ECHO_TYPE_HPP

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include <typeinfo>

#include "property.hpp"

namespace echo {

template<typename T>
uint32_t _get_type_index()
{
    static uint32_t index = _new_type_index();
    return index;
}

inline uint32_t _new_type_index()
{
    static uint32_t id = 0;
    return id++;
}

class registration;
class type {
    friend registration;
public:
    type(const type& other) {
        *this = other;
    }

    uint32_t get_index() const { return index; }
    std::string get_name() const { return infos[index].name; }

    const std::vector<property>& get_properties() const { return infos[index].properties; }
    property get_property(const std::string& name) const {
        auto it = infos[index].name_to_property.find(name);
        if(it == infos[index].name_to_property.end()) {
            return property();
        } 
        return infos[index].properties[it->second]; 
    }

    bool operator==(const type& other) const {
        return index == other.index;
    }
    bool operator<(const type& other) const {
        return index < other.index;
    }

    template<typename T>
    static type get() {
        uint32_t type_index = _get_type_index<T>();
        if(type_index >= types.size()) {
            type new_type;
            new_type.index = type_index;
            types.emplace_back(new_type);
            infos.emplace_back(info(typeid(T).name()));
        }
        return types[type_index];
    }
    static type get(const std::string& name) {
        auto it = name_to_type.find(name);
        if(it == name_to_type.end()) {
            return get<void>();
        } else {
            return types[it->second];
        }
    }
private:
    class info {
    public:
        info() {}
        info(const std::string& name)
        : name(name) {

        }
        std::string name;
        std::vector<property> properties;
        std::map<std::string, size_t> name_to_property;
    };

    uint32_t index;

    static std::vector<echo::type> types;
    static std::vector<info> infos;
    static std::map<uint32_t, std::string> type_to_name;
    static std::map<std::string, size_t> name_to_type;

    type() {}

    void set_name(const std::string& name) {
        infos[index].name = name;
    }
};

} // echo

#endif
