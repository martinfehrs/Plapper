module;

#include <algorithm>
#include <ranges>

export module plapper:core_extension_words;

import :dictionary;
import :environment;
import :core_word_entries;
import :core_words;

namespace rng = std::ranges;

namespace plapper
{

    error_status zero_greater(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([&dstack](auto& n){ n = dstack[0] > 0 ? yes : no; });
    }

    void hex(int_t& base) noexcept
    {
        base = 16;
    }

    error_status roll(data_stack& dstack) noexcept
    {
        return dstack.select(range, value).and_then(
            [&dstack](const auto xs, const auto n)
            {
                const auto roll_range = xs
                    | rng::views::reverse
                    | rng::views::take(n + 1);

                rng::rotate(roll_range, rng::next(rng::begin(roll_range)));
                dstack.pop_unchecked();
            }
        );
    }

    error_status load_core_extension_words(dictionary& dict, shared_core_word_data& shared_data)
    {
        return dict.emplace_entries(
            template_v<procedure>, "0>"  , zero_greater          ,
            template_v<closure  >, "HEX" , hex, *shared_data.base,
            template_v<procedure>, "ROLL", roll
        );
    }
}