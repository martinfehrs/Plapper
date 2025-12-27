module;

#include <algorithm>
#include <ranges>

export module plapper:core_extension_words;

import :dictionary;
import :environment;
import :core_module;
import :core_words;

namespace rng = std::ranges;

namespace plapper
{

    export error_status zero_greater(environment& env, void*) noexcept
    {
        return env.dstack.top().apply([&env](auto& n){ n = env.dstack[0] > 0 ? yes : no; });
    }

    export error_status hex(environment& env, void*) noexcept
    {
        env.base = 16;

        return error_status::success;
    }

    export error_status roll(environment& env, void*) noexcept
    {
        return env.dstack.top().apply(
            [&env](const auto n)
            {
                auto values = env.dstack.access_n(1, n + 1);

                if (!values)
                    return error_status::stack_underflow;

                rng::rotate(values, rng::next(rng::begin(values)));
                env.dstack.pop_unchecked();

                return error_status::success;
            }
        );
    }

    export class core_extension_words_t
    {

        static auto& create_entries(const core_words_t& core_words)
        {
            static const module_entry entries_[]{
                { "0>"  , &zero_greater , false                                 },
                { "HEX" , &hex          , false, core_words.base_addr() },
                { "ROLL", &roll         , false                                 },
            };

            return entries_;
        }

        std::span<const module_entry> entries;

    public:

        explicit core_extension_words_t(const core_words_t& core_words) noexcept
            : entries{ create_entries(core_words) }
        { }

        [[nodiscard]] auto begin() const noexcept
        {
            return rng::begin(this->entries);
        }

        [[nodiscard]] auto end() const noexcept
        {
            return rng::end(this->entries);
        }

    };

}