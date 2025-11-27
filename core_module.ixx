module;

#include <string_view>
#include <variant>

export module plapper:core_module;

import :core_types;
import :error;

namespace plapper
{

    struct module_entry
    {
        std::string_view word;
        execution_token_t xt;
        bool immediate;
        std::variant<int_t, uint_t, int_t*, uint_t*> data;
    };

}