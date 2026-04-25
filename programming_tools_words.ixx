module;

#include <cassert>
#include <format>
#include <ranges>

export module plapper:programming_tools_words;

import :environment;
import :core_word_entries;

namespace plapper
{

    error_status dot_s(environment& env) noexcept
    {
        return env.dstack.select(range).and_then(
            [&env](const auto xs)
            {
                env.term.write("istack:\n");

                const auto size = rng::size(xs);

                if (size == 0uz)
                    return;

                static constexpr auto max_chars_to_write = std::numeric_limits<std::size_t>::digits10 + 1
                                                                    + std::numeric_limits<int_t>::digits10 + 1
                                                                    + std::char_traits<char_t>::length(" []: -\n");

                const auto max_index_width = std::formatted_size("{}", size - 1);

                static char_t format_buffer[max_chars_to_write + 1];

                for (const auto[i, x] : xs | rng::views::reverse | rng::views::enumerate)
                {
                    auto out = std::format_to(format_buffer, " [{:{}}]: {: }\n", i, max_index_width, x);
                    env.term.write({ format_buffer, static_cast<std::size_t>(out - format_buffer) });
                }
            }
        );
    }

    void bye(environment& env) noexcept
    {
        env.running = false;
    }

    error_status load_programming_tool_words(dictionary& dict)
    {
        return dict.emplace_entries(
            template_v<procedure>, ".S" , dot_s,
            template_v<procedure>, "BYE", bye
        );
    }
}