module;

#include <utility>

export module plapper:environment;

import :error;
import :input_buffer;
import :output_buffer;
import :stack;
import :dictionary;
import :core_types;
import :core_constants;
import :settings;

namespace plapper
{

    export using data_stack = stack<int_t, uint_t, flag_t, int_t*, uint_t*, flag_t*>;
    export using return_stack = stack<execution_token_t**>;

    export struct environment
    {
        environment() = default;

        environment(dictionary dict, data_stack dstack, return_stack rstack) noexcept
            : dict(std::move(dict))
            , dstack(std::move(dstack))
            , rstack(std::move(rstack))
        { }

        dictionary dict;
        data_stack dstack;
        return_stack rstack;
        input_buffer tib;
        output_buffer tob;
        bool running = true;
        int_t base = 10;
        flag_t state = no;
        execution_token_t** instruction_ptr = nullptr;

    };

}
