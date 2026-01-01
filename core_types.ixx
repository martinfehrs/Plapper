module;

#include <cstdint>

export module plapper:core_types;

import :error;

namespace plapper
{

    export struct environment;

    export using byte_t = std::uint8_t;
    export using char_t = char8_t;
    export using int_t = std::intptr_t;
    export using uint_t = std::uintptr_t;
    export using dint_t = __int128_t;
    export using duint_t = __uint128_t;

    export using flag_t = std::uintptr_t;
    export using execution_token_t = error_status(*)(environment& env, void* data) noexcept;

}