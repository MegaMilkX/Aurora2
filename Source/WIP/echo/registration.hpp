#ifndef ECHO_REGISTRATION_HPP
#define ECHO_REGISTRATION_HPP

#include <string>
#include "type.hpp"

namespace echo {

class registration {
public:
    template<typename T>
    class type {
    public:
        type(const std::string& name)
        : t(echo::type::get<T>()) {
            t.set_name(name);
        }
        ~type(){
            
        }
        template<typename CLASS, typename TYPE>
        type<T>& property(const std::string& name, TYPE CLASS::*prop) {
            t.infos[t.index].name_to_property[name] = t.infos[t.index].properties.size();
            t.infos[t.index].properties.emplace_back(echo::property());
            return *this;
        }
        template<typename CLASS, typename Ret, typename... Args>
        type<T>& method(const std::string& name, Ret (CLASS::*func)(Args... args)) {
            return *this;
        }
        type<T>& meta(const std::string& name, const std::string& value) {
            return *this;
        }
        type<T>& meta(const std::string& name, bool value) {
            return *this;
        }
    private:
        echo::type t;
    };
};

typedef registration reg;

} // echo

#endif
