module;

#include <cassert>
#include <format>
#include <ranges>

export module plapper:programming_tools_words;

import :environment;
import :core_module;

namespace plapper
{

    export [[nodiscard]] error_status dot_s(environment& env, void*) noexcept
    {
        return env.dstack.select(range).and_then(
            [&env](const auto xs)
            {
                env.odev.write("istack:\n");

                const auto size = rng::size(xs);

                if (size == 0uz)
                    return;

                static constexpr auto max_chars_to_write = std::numeric_limits<std::size_t>::digits10 + 1
                                                                    + std::numeric_limits<int_t>::digits10 + 1
                                                                    + std::char_traits<char_t>::length("\t[]: -\n");

                const auto max_index_width = std::formatted_size("{}", size - 1);

                static char_t format_buffer[max_chars_to_write + 1];

                for (const auto[i, x] : xs | rng::views::reverse | rng::views::enumerate)
                {
                    auto out = std::format_to(format_buffer, "\t[{:{}}]: {: }\n", i, max_index_width, x);
                    env.odev.write(format_buffer, out - format_buffer);
                }
            }
        );
    }

    export [[nodiscard]] error_status bye(environment& env, void*) noexcept
    {
        env.running = false;
        return error_status::success;
    }

    export constexpr std::array programming_tool_words{
        module_entry{ ".S" , &dot_s, false },
        module_entry{ "BYE", &bye  , false },
    };

}