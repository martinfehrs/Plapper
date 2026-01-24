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

}