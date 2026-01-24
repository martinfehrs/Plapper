module;

#include <format>
#include <ranges>

export module plapper:programming_tools_words;

import :environment;
import :core_module;

namespace plapper
{

    export [[nodiscard]] error_status dot_s(environment& env, void*) noexcept
    {
        return env.dstack.select().and_then(
            [&env](const auto xs)
            {
                env.tob.write("istack:\n");

                const auto size = rng::size(xs);

                if (size == 0uz)
                    return;

                const auto index_width = rng::size(std::format("{}", size - 1));

                for (const auto[i, x] : xs | rng::views::reverse | rng::views::enumerate)
                    env.tob.write(std::format("\t[{:{}}]: {: }\n", i, index_width, x));
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