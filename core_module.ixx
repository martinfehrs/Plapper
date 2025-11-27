module;

#include <string>
#include <variant>

export module plapper:core_module;

import :core_types;
import :error;

namespace plapper
{

    struct module_entry
    {
        std::string word;
        execution_token_t impl;
        bool immediate;
        std::variant<int_t, uint_t, int_t*, uint_t*> data;
    };

}