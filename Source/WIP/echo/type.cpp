#include "type.hpp"

namespace echo {

std::vector<echo::type> type::types;
std::vector<type::info> type::infos;
std::map<uint32_t, std::string> type::type_to_name;
std::map<std::string, size_t> type::name_to_type;

}