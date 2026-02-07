module;

#include <cstddef>
#include <array>

export module plapper:error;

namespace plapper
{

    export enum class [[nodiscard]] error_status
    {
        success = 0,
        stack_underflow,
        out_of_character_range,
        division_by_zero,
        stack_overflow,
        out_of_memory,
        out_of_words,
        unknown_word,
        out_of_range
    };

    constexpr std::array error_messages {
        "out of stack range",
        "out of character range",
        "division by zero",
        "stack_overflow",
        "out_of_memory",
        "out of words",
        "unknown word"
    };

    export [[nodiscard]] constexpr const char* error_message_for(error_status stat) noexcept
    {
        return error_messages[static_cast<std::size_t>(stat) - 1];
    }

}