module;

#include <cstddef>
#include <type_traits>

export module plapper:type_traits;

namespace plapper
{

    export template <typename, typename NewBase>
    struct replace_base
    {
        using type = NewBase;
    };

    template <typename UndecoratedType, typename NewBase>
    struct replace_base<const UndecoratedType, NewBase>
    {
        using type = const NewBase;
    };

    template <typename UndecoratedType, typename NewBase>
    struct replace_base<volatile UndecoratedType, NewBase>
    {
        using type = volatile NewBase;
    };

    template <typename UndecoratedType, typename NewBase>
    struct replace_base<const volatile UndecoratedType, NewBase>
    {
        using type = const volatile NewBase;
    };

    template <typename Base, typename NewBase>
    struct replace_base<Base*, NewBase>
    {
        using type = replace_base<Base, NewBase>::type*;
    };

    template <typename Base, typename NewBase>
    struct replace_base<Base* const, NewBase>
    {
        using type = replace_base<Base, NewBase>::type* const;
    };

    template <typename Base, typename NewBase>
    struct replace_base<Base* volatile, NewBase>
    {
        using type = replace_base<Base, NewBase>::type* volatile;
    };

    template <typename Base, typename NewBase>
    struct replace_base<Base* const volatile, NewBase>
    {
        using type = replace_base<Base, NewBase>::type* const volatile;
    };

    template <typename UndecoratedType, typename NewBase>
    struct replace_base<UndecoratedType&, NewBase>
    {
        using type = replace_base<UndecoratedType, NewBase>::type&;
    };

    template <typename UndecoratedType, typename NewBase>
    struct replace_base<UndecoratedType[], NewBase>
    {
        using type = replace_base<UndecoratedType, NewBase>::type[];
    };

    template <typename UndecoratedType, typename NewBase>
    struct replace_base<const UndecoratedType[], NewBase>
    {
        using type = const replace_base<UndecoratedType, NewBase>::type[];
    };

    template <typename UndecoratedType, typename NewBase>
    struct replace_base<volatile UndecoratedType[], NewBase>
    {
        using type = volatile replace_base<UndecoratedType, NewBase>::type[];
    };

    template <typename UndecoratedType, typename NewBase>
    struct replace_base<const volatile UndecoratedType[], NewBase>
    {
        using type = const volatile replace_base<UndecoratedType, NewBase>::type[];
    };

    template <typename UndecoratedType, std::size_t size, typename NewBase>
    struct replace_base<UndecoratedType[size], NewBase>
    {
        using type = replace_base<UndecoratedType, NewBase>::type[];
    };

    template <typename UndecoratedType, std::size_t size, typename NewBase>
    struct replace_base<const UndecoratedType[size], NewBase>
    {
        using type = const replace_base<UndecoratedType, NewBase>::type[size];
    };

    template <typename UndecoratedType, std::size_t size, typename NewBase>
    struct replace_base<volatile UndecoratedType[size], NewBase>
    {
        using type = volatile replace_base<UndecoratedType, NewBase>::type[size];
    };

    template <typename UndecoratedType, std::size_t size, typename NewBase>
    struct replace_base<const volatile UndecoratedType[size], NewBase>
    {
        using type = const volatile replace_base<UndecoratedType, NewBase>::type[size];
    };

    export template <typename Type, typename NewBase>
    using replace_base_t = replace_base<Type, NewBase>::type;


    template <typename... Ts>
    struct type_sequence
    { };

    template<typename Type, std::size_t counter, std::size_t count, typename... ExpandedTypes>
    struct make_type_sequence_helper
    {
        using type = make_type_sequence_helper<Type, counter + 1, count, Type, ExpandedTypes...>::type;
    };

    template<typename Type, std::size_t count, typename... ExpandedTypes>
    struct make_type_sequence_helper<Type, count, count, ExpandedTypes...>
    {
        using type = type_sequence<ExpandedTypes...>;
    };

    template <typename Type, std::size_t count>
    using make_type_sequence = make_type_sequence_helper<Type, 0, count>::type;

    template <typename Func, typename Arg, std::size_t count>
    struct invoke_result_n
    {

    private:

        template <typename Func_, typename TypeSequence>
        struct impl;

        template <typename Func_, template<typename...> typename TypeSequence, typename...Args>
        struct impl<Func_, TypeSequence<Args...>>
        {
            using type = std::invoke_result_t<Func_, Args...>;
        };

    public:

        using type = impl<Func, make_type_sequence<Arg, count>>::type;

    };

    template <typename Func, typename Arg, std::size_t count>
    using invoke_result_n_t = invoke_result_n<Func, Arg, count>::type;
}