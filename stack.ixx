module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <ranges>
#include <span>
#include <type_traits>

export module plapper:stack;

import :error;
import :constant_size_literals;
import :core_constants;

namespace rng = std::ranges;

namespace plapper
{

    template <std::size_t extent>
    struct size_storage
    {
        consteval size_storage() noexcept = default;

        static constexpr std::size_t size_ = extent;
    };

    template <>
    struct size_storage<std::dynamic_extent>
    {
        constexpr size_storage() noexcept
            : size_{ 0 }
        { };

        constexpr size_storage(std::size_t size) noexcept
            : size_{ size }
        { };

        const std::size_t size_;
    };

    template <typename Value>
    concept stack_value = requires
    {
        requires std::same_as<std::remove_cvref_t<Value>, Value>;
        requires std::is_trivially_copyable_v<Value>;
    };

    template <typename Value>
    concept stack_element = requires
    {
        requires std::same_as<std::remove_volatile_t<std::remove_reference_t<Value>>, Value>;
        requires std::is_trivially_copyable_v<Value>;
    };

    template <typename Type1, typename Type2>
    concept equally_sized = sizeof(Type1) == sizeof(Type2);

    template <typename Type1, typename Type2>
    concept equally_sized_stack_values = requires
    {
        requires stack_value<Type1>;
        requires stack_value<Type2>;
        requires equally_sized<Type1, Type2>;
    };

    template <typename Type1, typename Type2>
    concept equally_sized_stack_elements = requires
    {
        requires stack_element<Type1>;
        requires stack_element<Type2>;
        requires equally_sized<Type1, Type2>;
    };

    export template <typename T>
    struct stack_conversion_traits
    {
        using source_type = T;
        using target_type = T;

        [[nodiscard]] static constexpr target_type convert(source_type val) noexcept
        {
            return val;
        }
    };

    template <typename T>
    struct stack_conversion_traits<const T> : stack_conversion_traits<T>
    { };

    template <>
    struct stack_conversion_traits<char_t>
    {
        using source_type = char_t;
        using target_type = uint_t;

        [[nodiscard]] static constexpr target_type convert(source_type val) noexcept
        {
            return val;
        }
    };

    template <>
    struct stack_conversion_traits<char>
    {
        using source_type = char;
        using target_type = uint_t;

        [[nodiscard]] static constexpr target_type convert(source_type val) noexcept
        {
            return static_cast<unsigned char>(val);
        }
    };

    template <>
    struct stack_conversion_traits<int>
    {
        using source_type = int;
        using target_type = int_t;

        [[nodiscard]] static constexpr target_type convert(source_type val) noexcept
        {
            return val;
        }
    };

    template <>
    struct stack_conversion_traits<unsigned int>
    {
        using source_type = unsigned int;
        using target_type = uint_t;

        [[nodiscard]] static constexpr target_type convert(source_type val) noexcept
        {
            return val;
        }
    };

    template <>
    struct stack_conversion_traits<bool>
    {
        using source_type = bool;
        using target_type = flag_t;

        [[nodiscard]] static constexpr target_type convert(source_type val) noexcept
        {
            return val ? yes : no;
        }
    };

    template <typename Source, typename... Targets>
    concept stack_compatible_value = requires
    {
        requires stack_value<Source>;
        requires (stack_value<Targets> && ...);
        requires (std::same_as<typename stack_conversion_traits<Source>::target_type, Targets> || ...);
    };

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

    export template <stack_element Element, std::size_t extent, stack_value... StackValues>
    class stack_range : size_storage<extent>
    {

        template <stack_element, std::size_t, stack_value...> friend class stack_range;

    public:

        using element_type = Element;
        using value_type = std::remove_cv_t<Element>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = Element*;
        using const_pointer = const Element*;
        using reference = Element&;
        using const_reference = const Element&;
        using iterator = Element*;
        using const_iterator = std::const_iterator<iterator>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::const_iterator<reverse_iterator>;

        constexpr stack_range() noexcept
            : size_storage<extent>()
            , data_{}
        { }

        constexpr stack_range(pointer first) noexcept requires(extent != std::dynamic_extent)
            : size_storage<extent>()
            , data_{ first }
        { }

        constexpr stack_range(pointer first, size_type count) noexcept requires(extent == std::dynamic_extent)
            : size_storage<extent>{ count }
            , data_{ first }
        { }

        constexpr stack_range(pointer first, pointer last) noexcept requires(extent == std::dynamic_extent)
            : size_storage<extent>{ static_cast<std::size_t>(last - first) }
            , data_{ first }
        { }

        constexpr stack_range(const stack_range& other) noexcept = default;

        [[nodiscard]] element_type& operator[](size_type pos) const noexcept
        {
            assert(pos < extent);

            return *(data_ + pos);
        }

        [[nodiscard]] constexpr size_type size() const noexcept
        {
            return this->size_;
        }

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return this->size_ == 0;
        }

        [[nodiscard]] explicit constexpr operator bool() const noexcept
        {
            return this->data_ != nullptr;
        }

        [[nodiscard]] auto begin(this auto& self) noexcept
        {
            return self.data_;
        }

        [[nodiscard]] auto end(this auto& self) noexcept
        {
            return self.data_ + self.size_;
        }

        [[nodiscard]] const_iterator cbegin() const noexcept
        {
            return this->begin();
        }

        [[nodiscard]] const_iterator cend() const noexcept
        {
            return this->end();
        }

        [[nodiscard]] auto rbegin(this auto& self) noexcept
        {
            return std::make_reverse_iterator(self.end());
        }

        [[nodiscard]] auto rend(this auto& self) noexcept
        {
            return std::make_reverse_iterator(self.begin());
        }

        [[nodiscard]] const_reverse_iterator crbegin() const noexcept
        {
            return this->rbegin();
        }

        [[nodiscard]] const_reverse_iterator crend() const noexcept
        {
            return this->rend();
        }

        template <typename Target> requires(std::same_as<Target, StackValues> || ...)
        [[nodiscard]] stack_range<Target, extent, StackValues...> as() const noexcept
        {
            return { reinterpret_cast<Target*>(this->data_) };
        }

        template<typename Func> requires(
            extent != std::dynamic_extent && std::same_as<invoke_result_n_t<Func, Element&, extent>, void>
        )
        [[nodiscard]] error_status apply(Func func) const noexcept
        {
            static const auto impl = [this]<std::size_t... indices>(Func func_, std::index_sequence<indices...>)
            {
                func_((this->data_[indices])...);
            };

            if (!this->data_)
                return error_status::stack_underflow;

            impl(func, std::make_index_sequence<extent>{});

            return error_status::success;
        }

        template<typename Func> requires(
            extent != std::dynamic_extent && std::same_as<invoke_result_n_t<Func, Element&, extent>, error_status>
        )
        [[nodiscard]] error_status apply(Func func) const noexcept
        {
            static const auto impl = [this]<std::size_t... indices>(Func func_, std::index_sequence<indices...>)
            {
                return func_(this->data_[indices]...);
            };

            if (!this->data_)
                return error_status::stack_underflow;

            return impl(func, std::make_index_sequence<extent>{});
        }

    private:

        pointer data_;

    };

    template <stack_element Element, stack_value... StackValues>
    class stack_pointer
    {

        template <stack_element, stack_value...> friend class stack_pointer;

    public:

        using element_type = Element;
        using pointer = Element*;

        stack_pointer() noexcept
            : element_addr{}
        { }

        stack_pointer(Element* element_addr) noexcept
            : element_addr{ element_addr }
        { }

        [[nodiscard]] Element& operator*() const noexcept
        {
            assert(element_addr);

            return *(this->element_addr);
        }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return this->element_addr;
        }

        [[nodiscard]] stack_pointer<const Element> as_const() const noexcept
        {
            return { this->element_addr };
        }

        template <typename Target> requires(std::same_as<Target, StackValues> || ...)
        [[nodiscard]] stack_pointer<Target> as() const noexcept
        {
            return { reinterpret_cast<Target*>(this->element_addr) };
        }

        [[nodiscard]] auto operator+(std::intptr_t offset) const noexcept
        {
            return stack_pointer{ this->element_addr + offset };
        }

        template<typename Func> requires std::is_invocable_r_v<void, Func, Element&>
                                      || std::is_invocable_r_v<error_status, Func, Element&>
        [[nodiscard]] error_status apply(Func func) const noexcept
        {
            if (!this->element_addr)
                return error_status::stack_underflow;

            if constexpr (std::same_as<std::invoke_result_t<Func, Element &>, void>)
            {
                func(*this->element_addr);
                return error_status::success;
            }
            else
            {
                return func(*this->element_addr);
            }
        }

    private:

        pointer element_addr;

    };

    export template <stack_element Element> using const_stack_pointer = stack_pointer<const Element>;

    template <typename Element>
    stack_pointer(Element* element_addr) -> stack_pointer<Element>;

    export template <stack_value DefaultValue, equally_sized_stack_values<DefaultValue>... FurtherValues> class stack
    {

        template <stack_value ThatDefaultValue, equally_sized_stack_values<ThatDefaultValue>...> friend class stack;

    public:

        using size_type = std::size_t;
        using value_type = DefaultValue;
        using reference = DefaultValue&;
        using const_reference = const DefaultValue&;
        using const_iterator = const DefaultValue*;

        stack() = default;

        ~stack() noexcept
        {
            std::free(this->data_);
        }

        stack(const stack&) = delete;
        stack& operator=(const stack&) = delete;

        stack(stack&& that) noexcept
            : stack{}
        {
            this->swap(that);
        }

        stack& operator=(stack&& that) noexcept
        {
            this->swap(that);

            return *this;
        }

        [[nodiscard]] static std::expected<stack, error_status> of_size(const std::size_t cell_capacity) noexcept
        {
            stack stack{};

            auto stat = stack.reserve(cell_capacity);

            if (stat != error_status::success)
                return std::unexpected(stat);

            return stack;
        }

        template <equally_sized_stack_values<DefaultValue> ... Values>
        [[nodiscard]] static std::expected<stack, error_status> containing(Values... values) noexcept
        {
            auto stack = of_size(sizeof...(Values));

            if (stack)
                stack->unchecked_push(values...);

            return stack;
        }

        error_status reserve(const size_type new_cell_capacity) noexcept
        {
            if (new_cell_capacity > this->capacity_)
            {
                const auto new_data = static_cast<DefaultValue*>(
                    std::realloc(this->data_, new_cell_capacity * sizeof(value_type)));

                if (!new_data)
                    return error_status::out_of_memory;

                this->data_ = new_data;
                this->capacity_ = new_cell_capacity;
            }

            return error_status::success;
        }

        [[nodiscard]] size_type capacity() const noexcept
        {
            return this->capacity_;
        }

        template <std::equality_comparable_with<value_type> ThatValue>
        [[nodiscard]] bool operator==(std::initializer_list<value_type> il) const noexcept
        {
            return rng::equal(rng::views::counted(this->data_, this->size_), il);
        }

        template <std::equality_comparable_with<value_type> ThatValue>
        [[nodiscard]] bool operator==(const stack<ThatValue>& that) const noexcept
        {
            return rng::equal(rng::views::counted(this->data_, this->size_),
                              rng::views::counted(that.data_ , that.size_ ));
        }

        template <std::equality_comparable_with<value_type> ThatValue>
        [[nodiscard]] bool operator!=(const stack<ThatValue>& that) const noexcept
        {
            return !((*this) == that);
        }

        template <stack_compatible_value<DefaultValue, FurtherValues...> ... Values>
        void push_unchecked(Values... values) noexcept
        {
            this->push_impl(std::index_sequence_for<Values...>{}, values...);

            this->size_ += sizeof...(Values);
        }

        template <stack_compatible_value<DefaultValue, FurtherValues...> ... Values>
        [[nodiscard]] error_status push(Values... values) noexcept
        {
            if (this->size_ + sizeof...(Values) > this->capacity_)
                return error_status::stack_overflow;

            push_unchecked(values...);

            return error_status::success;
        }

        void pop_unchecked() noexcept
        {
            assert(this->size_ > 0);

            --this->size_;
        }

        void pop_n_unchecked(size_type count) noexcept
        {
            assert(this->size_ >= count);

            this->size_ -= count;
        }

        [[nodiscard]] error_status pop() noexcept
        {
            if (this->size_ == 0)
                return error_status::stack_underflow;

            this->pop_unchecked();

            return error_status::success;
        }

        [[nodiscard]] error_status pop_n(size_type count) noexcept
        {
            if (this->size_ < count)
                return error_status::stack_underflow;

            this->pop_n_unchecked(count);

            return error_status::success;
        }
        
        template <size_type count>
        error_status replace(value_type value)
        {
            if constexpr (count == 0)
            {
                return this->push(value);
            }
            else
            {
                if constexpr(count > 1)
                {
                    this->pop_n_unchecked(count - 1);
                }

                *this->select(1_cuz) = value;

                return error_status::success;
            }
        }

        template <size_type count, stack_compatible_value<DefaultValue, FurtherValues...>... Values>
        error_status replace(value_type value, Values... values)
        {
            assert(count <= this->size());

            if constexpr (count == 0)
            {
                return this->push(value, values...);
            }
            else
            {
                this->data_[this->size_ - count] = value;
                return replace<count - 1>(values...);
            }
        }

        [[nodiscard]] size_type size() const noexcept
        {
            return this->size_;
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return this->size_ == 0;
        }

        [[nodiscard]] auto begin(this auto& self) noexcept
        {
            return self.data_;
        }

        [[nodiscard]] auto end(this auto& self) noexcept
        {
            return self.data_ + self.size_;
        }

        [[nodiscard]] auto& operator[](this auto& self, size_type pos) noexcept
        {
            return self.data_[self.size_ - 1 - pos];
        }

        [[nodiscard]] bool has(size_type count) const noexcept
        {
            return this->size_ >= count;
        }

        void clear() noexcept
        {
            this->size_ = 0;
        }

        [[nodiscard]] auto select(this auto& self, size_type start, size_type count) noexcept
            ->  stack_range<std::remove_pointer_t<decltype(self.data_)>, std::dynamic_extent, DefaultValue, FurtherValues...>
        {
            if (self.size_ < start + count)
                return {};

            auto last = self.data_ + self.size_ - start;

            return { last - count, last };
        }

        template <size_type count> requires(count > 1)
        [[nodiscard]] auto select(this auto& self, size_type start, size_constant<count>) noexcept
            ->  stack_range<std::remove_pointer_t<decltype(self.data_)>, count, DefaultValue, FurtherValues...>
        {
            if (self.size_ < start + count)
                return {};

            auto last = self.data_ + self.size_ - start;

            return { last - count };
        }

        [[nodiscard]] auto select(this auto& self, size_type start, size_constant<1>) noexcept
            -> stack_pointer<std::remove_pointer_t<decltype(self.data_)>, DefaultValue, FurtherValues...>
        {
            if (start >= self.size_)
                return {};

            return { &self.data_[self.size_ - 1 - start] };
        }

        [[nodiscard]] auto select(this auto& self, size_type count) noexcept
            -> stack_range<std::remove_pointer_t<decltype(self.data_)>, std::dynamic_extent, DefaultValue, FurtherValues...>
        {
            if (self.size_ < count)
                return {};

            auto last = self.data_ + self.size_;

            return { last - count, last };
        }

        template <size_type count> requires(count > 1)
        [[nodiscard]] auto select(this auto& self, size_constant<count>) noexcept
            -> stack_range<std::remove_pointer_t<decltype(self.data_)>, count, DefaultValue, FurtherValues...>
        {
            if (self.size_ < count)
                return {};

            return { self.data_ + self.size_ - count };
        }

        [[nodiscard]] auto select(this auto& self, size_constant<1>) noexcept
            -> stack_pointer<std::remove_pointer_t<decltype(self.data_)>, DefaultValue, FurtherValues...>
        {
            if (self.empty())
                return {};

            return { self.data_ + self.size_ - 1 };
        }

    private:

        template <stack_compatible_value<DefaultValue, FurtherValues...> ... Values, std::size_t... indices>
        void push_impl(std::index_sequence<indices...>, Values... values) noexcept
        {
            ((this->data_[this->size_ + indices] = values), ...);
        }

        void swap(stack& that) noexcept
        {
            using std::swap;

            swap(this->data_, that.data_);
            swap(this->capacity_, that.capacity_);
            swap(this->size_, that.size_);
        }

        DefaultValue* data_ = nullptr;
        std::size_t capacity_ = 0;
        std::size_t size_ = 0;

    };

}