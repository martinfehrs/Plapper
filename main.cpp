#include <print>
#include <iostream>

import plapper;

namespace rng = std::ranges;
using namespace plapper::literals;


[[noreturn]] void critical_error(const plapper::error_status stat) noexcept
{
    std::puts(plapper::error_message_for(stat));
    std::exit(1);
}

int main(const int argc, const char** argv)
{
    using enum plapper::modules;

    static constexpr plapper::settings settings{
        .dict_capacity = 65_KiB,
        .dstack_capacity = 64_cells,
        .rstack_capacity = 64_cells,
        .additional_modules = core_extension | programming_tools
    };

    auto interpreter = plapper::interpreter::from_settings(settings);

    if (!interpreter)
        critical_error(interpreter.error());

    return interpreter->run(argc, argv);
}