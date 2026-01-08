module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <expected>
#include <ranges>
#include <type_traits>
#include <catch2/generators/catch_generators.hpp>

export module plapper:stack;

import :error;
import :constant_size_literals;
import :core_constants;
import :type_traits;

namespace rng = std::ranges;

namespace plapper
{

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

    export template <stack_element Element, std::size_t extent>
    class value_selection
    {

        template <stack_element, std::size_t> friend class value_selection;

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

        constexpr value_selection() noexcept
            : data_{}
        { }

        constexpr value_selection(pointer first) noexcept
            : data_{ first }
        { }

        constexpr value_selection(const value_selection& other) noexcept = default;

        [[nodiscard]] element_type& operator[](size_type pos) const noexcept
        {
            assert(pos < extent);

            return *(data_ + pos);
        }

        [[nodiscard]] static constexpr size_type size() noexcept
        {
            return extent;
        }

        [[nodiscard]] static constexpr bool empty() noexcept
        {
            return extent == 0;
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
            return self.data_ + extent;
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

        template<typename Func> requires(std::same_as<invoke_result_n_t<Func, Element&, extent>, void>)
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

        template<typename Func> requires(std::same_as<invoke_result_n_t<Func, Element&, extent>, error_status>)
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

    export template <stack_element Element>
    class range_selection
    {

        template <stack_element> friend class range_selection;

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

        constexpr range_selection() noexcept
            : size_{}
            , data_{}
        { }

        constexpr range_selection(pointer first, size_type count) noexcept
            : size_{ count }
            , data_{ first }
        { }

        constexpr range_selection(pointer first, pointer last) noexcept
            : size_{ static_cast<std::size_t>(last - first) }
            , data_{ first }
        { }

        constexpr range_selection(const range_selection& other) noexcept = default;

        [[nodiscard]] element_type& operator[](size_type pos) const noexcept
        {
            assert(pos < this->size_);

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

        template<typename Func>
        [[nodiscard]] error_status apply(Func func) const noexcept requires(
            std::same_as<std::invoke_result_t<Func, decltype(*this)>, void>
        )
        {
            if (!this->data_)
                return error_status::stack_underflow;

            func(*this);

            return error_status::success;
        }

        template<typename Func>
        [[nodiscard]] error_status apply(Func func) const noexcept requires(
            std::same_as<std::invoke_result_t<Func, decltype(*this)>, error_status>
        )
        {
            if (!this->data_)
                return error_status::stack_underflow;

            return func(*this);
        }

    private:

        size_type size_;
        pointer data_;

    };

    template <typename Filter, typename StackPointer>
    concept selection_filter = requires(Filter filter, StackPointer stack_pointer)
    {
        typename Filter::template result_type<StackPointer>;
        { filter.count() } -> std::same_as<std::size_t>;
        { filter.select(stack_pointer) } -> std::same_as<typename Filter::template result_type<StackPointer>>;
    };

    export template <std::size_t count_> requires (count_> 0)
    struct values_t
    {
        template <typename StackPointer>
        using result_type = value_selection<std::remove_pointer_t<StackPointer>, count_>;

        template <typename StackPointer>
        [[nodiscard]] static auto select(StackPointer stack_pointer) noexcept
        {
            return result_type<StackPointer>{ stack_pointer - count_ };
        }

        [[nodiscard]] static std::size_t count() noexcept
        {
            return count_;
        }
    };

    export template <typename ValueType, std::size_t count_> requires (count_> 0)
    struct values_of_t
    {
        template <typename StackPointer>
        using result_type = value_selection<std::remove_pointer_t<replace_base_t<StackPointer, ValueType>>, count_>;

        template <typename StackPointer>
        [[nodiscard]] static auto select(StackPointer stack_pointer) noexcept
        {
            return result_type<StackPointer>{
                reinterpret_cast<replace_base_t<StackPointer, ValueType>>(stack_pointer - count_)
            };
        }

        [[nodiscard]] static std::size_t count() noexcept
        {
            return count_;
        }
    };

    export inline constexpr values_t<1> value;

    export template <typename ValueType>
    inline constexpr values_of_t<ValueType, 1> value_of{};

    export template <std::size_t multiplicator, std::size_t count>
    [[nodiscard]] consteval auto operator*(size_constant<multiplicator>, values_t<count>) noexcept
    {
        return values_t<multiplicator * count>{};
    }

    export template <typename ValueType, std::size_t multiplicator, std::size_t count>
    [[nodiscard]] consteval auto operator*(size_constant<multiplicator>, values_of_t<ValueType, count>) noexcept
    {
        return values_of_t<ValueType, multiplicator * count>{};
    }

    export struct subrange_t
    {
        template <typename StackPointer>
        using result_type = range_selection<std::remove_pointer_t<StackPointer>>;

        explicit subrange_t(const std::size_t count) noexcept
            : count_{ count }
        { }

        template <typename StackPointer>
        [[nodiscard]] auto select(StackPointer stack_pointer) const noexcept
        {
            return result_type<StackPointer>{ stack_pointer - this->count_, this->count_ };
        }

        [[nodiscard]] std::size_t count() const noexcept
        {
            return this->count_;
        }

    private:

        std::size_t count_;
    };

    export template <typename ValueType>
    struct subrange_of_t
    {
        template <typename StackPointer>
        using result_type = range_selection<std::remove_pointer_t<replace_base_t<StackPointer, ValueType>>>;

        explicit subrange_of_t(const std::size_t count) noexcept
            : count_{ count }
        { }

        template <typename StackPointer>
        [[nodiscard]] auto select(StackPointer stack_pointer) const noexcept
        {
            return result_type<StackPointer>{
                reinterpret_cast<replace_base_t<StackPointer, ValueType>>(stack_pointer - this->count_), this->count_
            };
        }

        [[nodiscard]] std::size_t count() const noexcept
        {
            return this->count_;
        }

    private:

        std::size_t count_;
    };

    export [[nodiscard]] constexpr auto subrange(const std::size_t count) noexcept
    {
        return subrange_t{ count };
    }

    export template <typename ValueType>
    [[nodiscard]] constexpr auto subrange_of(const std::size_t count) noexcept
    {
        return subrange_of_t<ValueType>{ count };
    }

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

        template <typename Filter>
        [[nodiscard]] auto select(this auto& self, Filter filter) noexcept
            requires selection_filter<Filter, decltype(self.data_)>
        {
            if (self.size_ < filter.count())
                return typename Filter::template result_type<decltype(self.data_)>{};

            return filter.select(self.data_ + self.size_);
        }

        template <typename Filter>
        [[nodiscard]] auto select_at(this auto& self, size_type start, Filter filter) noexcept
            requires selection_filter<Filter, decltype(self.data_)>
        {
            if (self.size_ < start + filter.count())
                return typename Filter::template result_type<decltype(self.data_)>{};

            return filter.select(self.data_ + self.size_ - start);
        }

        template <size_type count>
        error_status replace(value_type new_value)
        {
            if constexpr (count == 0)
            {
                return this->push(new_value);
            }
            else
            {
                if constexpr(count > 1)
                {
                    this->pop_n_unchecked(count - 1);
                }

                this->select(value)[0] = new_value;

                return error_status::success;
            }
        }

        template <size_type count, stack_compatible_value<DefaultValue, FurtherValues...>... Values>
        error_status replace(value_type new_value, Values... new_values)
        {
            assert(count <= this->size());

            if constexpr (count == 0)
            {
                return this->push(new_value, new_values...);
            }
            else
            {
                this->data_[this->size_ - count] = new_value;
                return replace<count - 1>(new_values...);
            }
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