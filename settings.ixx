module;

#include <cstddef>
#include <utility>

export module plapper:settings;

namespace plapper
{

    export enum class modules
    {
        core_extension = 0x1,
        programming_tools = 0x4,
    };

    export [[nodiscard]] constexpr auto operator|(const modules mod1, const modules mod2) noexcept
    {
        return modules{ std::to_underlying(mod1) | std::to_underlying(mod2) };
    }

    export [[nodiscard]] constexpr auto operator&(const modules mod1, const modules mod2) noexcept
    {
        return modules{ std::to_underlying(mod1) & std::to_underlying(mod2) };
    }

    export struct settings
    {
        std::size_t dict_capacity;
        std::size_t dstack_capacity;
        std::size_t rstack_capacity;
        modules additional_modules;
    };

}