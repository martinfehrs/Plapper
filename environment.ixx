module;

#include <utility>

export module plapper:environment;

import :error;
import :input_buffer;
import :terminal;
import :stack;
import :dictionary;
import :core_types;
import :core_constants;
import :settings;

namespace plapper
{

    export struct environment
    {
        environment(dictionary dict, data_stack dstack, return_stack rstack, input_buffer tib) noexcept
            : dict(std::move(dict))
            , dstack(std::move(dstack))
            , rstack(std::move(rstack))
            , tib(std::move(tib))
        { }

        dictionary dict;
        data_stack dstack;
        return_stack rstack;
        input_buffer tib;
        terminal term;
        bool running = true;
        int_t base = 10;
        flag_t state = no;
        execution_token*** instruction_ptr = nullptr;

    };

}
