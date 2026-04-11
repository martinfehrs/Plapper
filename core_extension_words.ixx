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

    export error_status zero_greater(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([&dstack](auto& n){ n = dstack[0] > 0 ? yes : no; });
    }

    export void hex(int_t& base) noexcept
    {
        base = 16;
    }

    export error_status roll(data_stack& dstack) noexcept
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

    export class core_extension_words_t
    {

        static auto& create_entries(const core_words_t& core_words)
        {
            static module_entry entries_[]{
                { "0>"  , procedure{ zero_greater                }, false  },
                { "HEX" , closure  { hex, core_words.base() }, false  },
                { "ROLL", procedure{ roll                        }, false  },
            };

            return entries_;
        }

        std::span<module_entry> entries;

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