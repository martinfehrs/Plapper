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
    concept stack_element = requires
    {
        requires std::same_as<std::remove_volatile_t<std::remove_reference_t<Value>>, Value>;
        requires std::is_trivially_copyable_v<Value>;
    };

    template <typename Value>
    concept stack_value = requires
    {
        requires std::same_as<std::remove_cvref_t<Value>, Value>;
        requires std::is_trivially_copyable_v<Value>;
    };

    template <typename Source, typename Target>
    concept fits_into = sizeof(Source) <= sizeof(Target);

    template <typename Source, typename Target>
    concept compatible_stack_value = requires(Source value)
    {
        requires stack_value<Source>;
        requires stack_value<Target>;
        requires std::constructible_from<Target, Source>;
        requires fits_into<Source, Target>;
    };

    export template <stack_element Element, std::size_t extent = std::dynamic_extent>
    class stack_range : size_storage<extent>
    {

        template <stack_element, std::size_t> friend class stack_range;

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

    private:

        pointer data_;

    };

    template <stack_element Element>
    class stack_pointer
    {

        template <stack_element> friend class stack_pointer;

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

        template <typename Target>
        [[nodiscard]] stack_pointer<Target> as() const noexcept
        {
            return { reinterpret_cast<Target*>(this->element_addr) };
        }

        [[nodiscard]] auto operator+(std::intptr_t offset) const noexcept
        {
            return stack_pointer{ this->element_addr + offset };
        }

    private:

        pointer element_addr;

    };

    export template <stack_element Element> using const_stack_pointer = stack_pointer<const Element>;

    template <typename Element>
    stack_pointer(Element* element_addr) -> stack_pointer<Element>;


    export template <stack_value Value> class stack
    {

        template <stack_value ThatValue> friend class stack;

    public:

        using size_type = std::size_t;
        using value_type = Value;
        using reference = Value&;
        using const_reference = const Value&;
        using const_iterator = const Value*;

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

        [[nodiscard]] static std::expected<stack<Value>, error_status> of_size(const std::size_t cell_capacity) noexcept
        {
            stack<Value> stack{};

            auto stat = stack.reserve(cell_capacity);

            if (stat != error_status::success)
                return std::unexpected(stat);

            return stack;
        }

        template <compatible_stack_value<Value> ... Values>
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
                const auto new_data = static_cast<Value*>(
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

        template <compatible_stack_value<value_type> ... Values, std::size_t... indices>
        void push_impl(std::index_sequence<indices...>, Values... values) noexcept
        {
            ((this->data_[this->size_ + indices] = values), ...);
        }

        template <compatible_stack_value<value_type> ... Values>
        void unchecked_push(Values... values) noexcept
        {
            this->push_impl(std::index_sequence_for<Values...>{}, values...);

            this->size_ += sizeof...(Values);
        }

        template <compatible_stack_value<value_type> ... Values>
        [[nodiscard]] error_status push(Values... values) noexcept
        {
            if (this->size_ + sizeof...(Values) > this->capacity_)
                return error_status::stack_overflow;

            unchecked_push(values...);

            return error_status::success;
        }

        void unchecked_pop() noexcept
        {
            assert(this->size_ > 0);

            --this->size_;
        }

        void unchecked_pop_n(size_type count) noexcept
        {
            assert(this->size_ >= count);

            this->size_ -= count;
        }

        [[nodiscard]] error_status pop() noexcept
        {
            if (this->size_ == 0)
                return error_status::stack_underflow;

            this->unchecked_pop();

            return error_status::success;
        }

        [[nodiscard]] error_status pop_n(size_type count) noexcept
        {
            if (this->size_ < count)
                return error_status::stack_underflow;

            this->unchecked_pop_n(count);

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
                    this->unchecked_pop_n(count - 1);
                }

                *this->top() = value;

                return error_status::success;
            }
        }

        template <size_type count, compatible_stack_value<value_type>... Values>
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

        [[nodiscard]] auto top(this auto& self) noexcept
            -> stack_pointer<std::remove_pointer_t<decltype(self.data_)>>
        {
            if (self.empty())
                return {};

            return { self.data_ + self.size_ - 1 };
        }


        error_status for_top(this auto& self, auto action) noexcept
        {
            if (self.empty())
                return error_status::stack_underflow;

            return action(*(self.data_ + self.size_ - 1));
        }

        template <std::size_t count, std::size_t... indices>
        error_status for_args_impl(this auto& self, auto action, std::index_sequence<indices...>) noexcept
        {
            auto range = self.template top_n<count>();

            if (!range)
                return error_status::stack_underflow;

            return action(range[indices]...);
        }

        template <std::size_t count>
        error_status for_args(this auto& self, auto action) noexcept
        {
            return self.template for_args_impl<count>(action, std::make_index_sequence<count>{});
        }

        [[nodiscard]] auto nth(this auto& self, size_type pos) noexcept
            -> stack_pointer<std::remove_pointer_t<decltype(self.data_)>>
        {
            if (pos >= self.size_)
                return {};

            return { &self.data_[self.size_ - 1 - pos] };
        }

        [[nodiscard]] auto get_n(this auto& self, size_type start, size_type count) noexcept
            ->  stack_range<std::remove_pointer_t<decltype(self.data_)>>
        {
            if (self.size_ < start + count)
                return {};

            auto last = self.data_ + self.size_ - start;

            return { last - count, last };
        }

        template <size_type count>
        [[nodiscard]] auto get_n(this auto& self, size_type start) noexcept
            ->  stack_range<std::remove_pointer_t<decltype(self.data_)>, count>
        {
            if (self.size_ < start + count)
                return {};

            auto last = self.data_ + self.size_ - start;

            return { last - count };
        }

        error_status for_top_2(auto action) noexcept
        {
            auto range = this->top_n<2>();

            if (!range)
                return error_status::stack_underflow;

            return action(range[0], range[1]);
        }

        [[nodiscard]] auto top_n(this auto& self, size_type count) noexcept
            ->  stack_range<std::remove_pointer_t<decltype(self.data_)>>
        {
            if (self.size_ < count)
                return {};

            auto last = self.data_ + self.size_;

            return { last - count, last };
        }

        template <size_type count>
        [[nodiscard]] auto top_n(this auto& self) noexcept
            -> stack_range<std::remove_pointer_t<decltype(self.data_)>, count>
        {
            if (self.size_ < count)
                return {};

            return { self.data_ + self.size_ - count };
        }

    private:

        void swap(stack& that) noexcept
        {
            using std::swap;

            swap(this->data_, that.data_);
            swap(this->capacity_, that.capacity_);
            swap(this->size_, that.size_);
        }

        Value* data_ = nullptr;
        std::size_t capacity_ = 0;
        std::size_t size_ = 0;

    };

}