////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///   ██████╗ ██╗      █████╗ ██████╗ ██████╗ ███████╗██████╗
///   ██╔══██╗██║     ██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔══██╗
///   ██████╔╝██║     ███████║██████╔╝██████╔╝█████╗  ██████╔╝
///   ██╔═══╝ ██║     ██╔══██║██╔═══╝ ██╔═══╝ ██╔══╝  ██╔══██╗
///   ██║     ███████╗██║  ██║██║     ██║     ███████╗██║  ██║
///   ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝     ╚══════╝╚═╝  ╚═╝
///
///      - Ein typsicheres deutschsprachiges FORTH -
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @author    Martin Fehrs
/// @copyright MIT Lizenz
/// @brief     Kompilierzeitkonstanten und -Literale für std::size_t
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
module;

#include <charconv>
#include <iterator>

export module plapper:constant_size_literals;

namespace plapper
{

    template<char ...chars>
    consteval std::size_t parse_size()
    {
        const char arr[]{ chars... };

        auto base = 10;
        auto offset = 0;

        if (arr[0] == '0' && 2 < std::size(arr))
        {
            if (arr[1] == 'x' || arr[1] == 'X')
            {
                base = 16;
                offset = 2;
            }
            else if (arr[1] == 'b')
            {
                base = 2;
                offset = 2;
            }
            else
            {
                base = 8;
                offset = 1;
            }
        }

        const auto first = std::begin(arr) + offset;
        const auto last = std::end(arr);

        std::size_t size{};

        if (
            const auto result = std::from_chars(first, last, size, base);
            result.ptr != last || result.ec != std::errc{}
        )
            throw std::logic_error{ "" };

        return size;
    }

    export template <std::size_t size>
    using size_constant = std::integral_constant<std::size_t, size>;

    export template<char... chars>
    consteval size_constant<parse_size<chars...>()> operator""_cuz()
    {
        return {};
    }

}
