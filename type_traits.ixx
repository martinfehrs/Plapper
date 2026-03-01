module;

#include <cstddef>
#include <type_traits>

export module plapper:type_traits;

namespace plapper
{

    template<typename Type, template<typename...> typename Template>
    inline constexpr bool is_instance_of_v = std::false_type{};

    template<template<typename...> typename Template, typename... Args>
    inline constexpr bool is_instance_of_v<Template<Args...>,Template> = std::true_type{};


    template <std::size_t pos, typename... types>
    struct nth_type;

    template <std::size_t pos, typename... types>
    using nth_type_t = nth_type<pos, types...>::type;

    template <typename first_type, typename... remaining_types>
    struct nth_type<0, first_type, remaining_types...>
    {
        using type = first_type;
    };

    template <std::size_t pos, typename first_type, typename... remaining_types>
    struct nth_type<pos, first_type, remaining_types...>
    {
        using type = nth_type_t<pos - 1, remaining_types...>;
    };

}