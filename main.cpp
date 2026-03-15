#include <print>
#include <iostream>

import plapper;

namespace rng = std::ranges;

[[noreturn]] void critical_error(const plapper::error_status stat) noexcept
{
    std::println("{}", plapper::error_message_for(stat));
    std::exit(1);
}

int main(const int argc, const char** argv)
{
    using enum plapper::modules;

    static constexpr plapper::settings settings{
        .dict_capacity = 65'536,
        .dstack_capacity = 64,
        .rstack_capacity = 64,
        .additional_modules = core_extension | programming_tools
    };

    auto ipred = plapper::interpreter::from_settings(settings);

    if (!ipred)
        critical_error(ipred.error());

    return ipred->run(argc, argv);

    return 0;
}