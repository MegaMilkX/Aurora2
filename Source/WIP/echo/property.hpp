#ifndef ECHO_PROPERTY_HPP
#define ECHO_PROPERTY_HPP

namespace echo {

class type;
class variant;
class property {
public:
    type get_type() const;
    variant get_variant() const;
};

} // echo

#endif
