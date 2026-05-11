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
/// @brief     Typen als Reguläre Parameter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
export module plapper:type_params;

namespace plapper
{

    template <typename T>
    struct typename_param
    { };

    template <template <typename...> typename TT>
    struct template_param
    { };

    template <typename T>
    inline constexpr typename_param<T> typename_v;

    template <template <typename...> typename TT>
    inline constexpr template_param<TT> template_v;

}
