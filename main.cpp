#include <print>
#include <iostream>

import plapper;

namespace rng = std::ranges;

plapper::error_status literal_(plapper::environment& env, void*) noexcept
{
    env.instruction_ptr++;

    if (const auto status = env.dstack.push(reinterpret_cast<plapper::int_t>(*env.instruction_ptr));
        status != plapper::error_status::success)
        return status;

    return plapper::error_status::success;
}

[[noreturn]] void critical_error(const plapper::error_status stat) noexcept
{
    std::println("{}", plapper::error_message_for(stat));
    std::exit(1);
}

void load_words(plapper::environment& env, const auto& words)
{
    if (const auto err_stat = env.dict.load(words);
        err_stat != plapper::error_status::success)
    {
        critical_error(err_stat);
        std::exit(1);
    }
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