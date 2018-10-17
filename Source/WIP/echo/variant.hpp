#ifndef ECHO_VARIANT_HPP
#define ECHO_VARIANT_HPP

#include "type.hpp"

namespace echo {

class variant {
public:
    variant()
    : t(type::get<void>()) {}
    template<typename T>
    variant(const T& value)
    : t(type::get<T>()) {}
    template<typename T>
    variant& operator=(const T& value) {
        *this = variant(value);
        return *this;
    }
    
    type get_type() const { return t; }
    template<typename T>
    bool is() const { return t == type::get<T>(); }
    bool is_array() const;
private:
    type t;
};

} // echo

#endif
