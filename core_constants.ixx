module;

#include <limits>

export module plapper:core_constants;

import :core_types;

namespace plapper
{

    export constexpr uint_t char_size = sizeof(char_t);
    export constexpr uint_t cell_size = sizeof(int_t);

    export constexpr int_t min_int{ std::numeric_limits<int_t>::min() };
    export constexpr int_t max_int{ std::numeric_limits<int_t>::max() };

    export constexpr uint_t max_uint{ std::numeric_limits<uint_t>::max() };
    export constexpr uint_t mid_uint{ max_uint >> 1 };

    export constexpr flag_t yes{ std::numeric_limits<flag_t>::max() };
    export constexpr flag_t no{ std::numeric_limits<flag_t>::min() };

}