#include "editor_gui_property.h"

namespace Editor {
namespace GUI {

std::map<rttr::type, Property::property_gui_func_t> Property::functions;

}
}