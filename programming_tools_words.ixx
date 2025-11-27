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
        env.tob.write("istack:\n");

        const auto stack_size = rng::size(env.dstack);

        if (stack_size == 0)
            return error_status::success;

        const auto index_width = rng::size(std::format("{}", stack_size - 1));

        for (const auto[i, elem] : env.dstack | rng::views::reverse | rng::views::enumerate )
            env.tob.write(std::format("\t[{:{}}]: {: }\n", i, index_width, elem));

        return error_status::success;
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