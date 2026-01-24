module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <expected>
#include <ranges>
#include <type_traits>

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

    template <typename Stack, typename... Elements>
    class selection
    {

    public:

        explicit selection(Stack& stack) noexcept
            : stack{ std::addressof(stack) }
            , first_param{ stack.top() - this->size() + 1 }
            , error_{ error_status::success }
        {}

        explicit selection(const error_status error) noexcept
            : stack{ nullptr }
            , first_param{ nullptr }
            , error_{ error }
        {}

        template <std::size_t pos>
        [[nodiscard]] auto& at() const noexcept requires(pos < sizeof...(Elements))
        {
            return *reinterpret_cast<Elements...[pos]*>(this->first_param + pos);
        }

        [[nodiscard]] error_status error() const noexcept
        {
            return this->error_;
        }

        // ReSharper disable once CppNonExplicitConversionOperator
        [[nodiscard]] operator error_status() const noexcept
        {
            return this->error_;
        }

        template<typename Func> requires(std::same_as<std::invoke_result_t<Func, Elements&...>, void>)
        [[nodiscard]] auto and_then(Func func) const noexcept
        {
            static const auto impl = [this]<std::size_t... indices>(Func func_, std::index_sequence<indices...>)
            {
                func_(this->at<indices>()...);
            };

            if (this->error_ != error_status::success)
                return *this;

            impl(func, std::make_index_sequence<sizeof...(Elements)>{});

            return *this;
        }

        template<typename Func> requires(
            std::same_as<std::invoke_result_t<Func, Elements&..., std::span<typename Stack::value_type>>, void>
        )
        [[nodiscard]] auto and_then(Func func) const noexcept
        {
            static const auto impl = [this]<std::size_t... indices>(Func func_, std::index_sequence<indices...>)
            {
                func_(
                    this->at<indices>()...,
                    std::span<typename Stack::value_type>{
                        this->stack->data(), this->stack->size() - this->size()
                    }
                );
            };

            if (this->error_ != error_status::success)
                return *this;

            impl(func, std::make_index_sequence<sizeof...(Elements)>{});

            return *this;
        }

        template<typename Func> requires(std::same_as<std::invoke_result_t<Func, Elements&...>, error_status>)
        [[nodiscard]] auto and_then(Func func) const noexcept
        {
            static const auto impl = [this]<std::size_t... indices>(Func func_, std::index_sequence<indices...>)
            {
                return func_(this->at<indices>()...);
            };

            if (this->error_ != error_status::success)
                return *this;

            const auto new_error_status = impl(func, std::make_index_sequence<sizeof...(Elements)>{});

            if (new_error_status != error_status::success)
                return selection{ new_error_status };

            return *this;
        }

        template<typename Func> requires(
            std::same_as<std::invoke_result_t<Func, Elements&..., std::span<typename Stack::value_type>>, error_status>
        )
        [[nodiscard]] auto and_then(Func func) const noexcept
        {
            static const auto impl = [this]<std::size_t... indices>(Func func_, std::index_sequence<indices...>)
            {
                return func_(
                    this->at<indices>()...,
                    std::span<typename Stack::value_type>{
                        this->stack->data(), this->stack->size() - this->size()
                    }
                );
            };

            if (this->error_ != error_status::success)
                return *this;

            const auto new_error_status = impl(func, std::make_index_sequence<sizeof...(Elements)>{});

            if (new_error_status != error_status::success)
                return selection{ new_error_status };

            return *this;
        }

        template <typename Func> requires(std::same_as<std::invoke_result_t<Func>, void>)
        [[nodiscard]] auto or_else(Func func) noexcept
        {
            if (this->error_ == error_status::success)
                return *this;

            func();

            return *this;
        }

        template <typename Func> requires(std::same_as<std::invoke_result_t<Func>, error_status>)
        [[nodiscard]] auto or_else(Func func) noexcept
        {
            if (this->error_ == error_status::success)
                return *this;

            return selection{ func() };
        }

        [[nodiscard]] static consteval std::size_t size() noexcept
        {
            return sizeof...(Elements);
        }

    private:

        Stack* stack;
        decltype(std::declval<Stack>().top()) first_param;
        error_status error_;

    };

    template <typename... Elements>
    struct selection_filter
    {
        template <typename Stack>
        [[nodiscard]] static constexpr auto select(Stack& stack) noexcept
        {
            using result_type = selection<
                Stack,
                std::conditional_t<std::same_as<Elements, void>, typename Stack::value_type, Elements>...
            >;

            return stack.has(sizeof...(Elements))
                ? result_type{ stack }
                : result_type{ error_status::stack_underflow };
        }
    };

    export template <typename Element>
    inline constexpr selection_filter<Element> value_of{};

    export inline constexpr selection_filter<void> value;

    template <typename... Elements1, typename... Elements2>
    [[nodiscard]] consteval auto operator+(
       selection_filter<Elements1...>, selection_filter<Elements2...>
    ) noexcept
    {
        return selection_filter<Elements1..., Elements2...>{};
    }

    export template <typename... Elements, std::size_t multiplicator>
    [[nodiscard]] consteval auto operator*(
        size_constant<multiplicator>, selection_filter<Elements...> description
    ) noexcept
    {
        if constexpr(multiplicator == 1uz)
        {
            return description;
        }
        else
        {
            return description + size_constant<multiplicator - 1uz>{} * description;
        }
    }

    export template <stack_value DefaultValue, equally_sized_stack_values<DefaultValue>... FurtherValues> class stack
    {

        template <stack_value ThatDefaultValue, equally_sized_stack_values<ThatDefaultValue>...> friend class stack;

    public:

        using size_type = std::size_t;
        using value_type = DefaultValue;
        using reference = DefaultValue&;
        using const_reference = const DefaultValue&;
        using pointer = DefaultValue*;
        using const_pointer = const DefaultValue*;
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

        [[nodiscard]] const_pointer data() const noexcept
        {
            return this->data_;
        }

        [[nodiscard]] pointer data() noexcept
        {
            return const_cast<pointer>(std::as_const(*this).data());
        }

        [[nodiscard]] const_pointer top() const noexcept
        {
            return this->data_ + this->size_ - 1uz;
        }

        [[nodiscard]] pointer top() noexcept
        {
            return const_cast<pointer>(std::as_const(*this).top());
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

        template <typename Self, typename... Filter> requires (is_instance_of_v<Filter, selection_filter> && ...)
        [[nodiscard]] auto select(this Self& self, Filter... filter) noexcept
        {
            return (filter + ... + selection_filter{}).select(self);
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

                this->select(value).template at<0>() = new_value;

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