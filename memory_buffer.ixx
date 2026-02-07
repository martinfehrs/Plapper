module;

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <utility>
#include <iterator>

export module plapper:memory_buffer;

import :error;

namespace plapper
{

    enum class iterator_validity
    {
        valid,
        invalid
    };

    template <typename Element, std::size_t grow_factor = 2>
    class dynamic_buffer
    {

    public:

        using value_type = Element;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = Element*;
        using const_pointer = const Element*;
        using reference = Element&;
        using const_reference = const Element&;
        using iterator = Element*;
        using const_iterator = const Element*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        static std::expected<dynamic_buffer, error_status> with_capacity_of(std::size_t capacity) noexcept
        {
            dynamic_buffer instance{ capacity };

            if (instance.data_ == nullptr)
                return std::unexpected(error_status::out_of_memory);

            return instance;
        }

        consteval dynamic_buffer() noexcept = default;

        constexpr dynamic_buffer(dynamic_buffer&& that) noexcept
        {
            this->swap(that);
        }

        constexpr dynamic_buffer& operator=(dynamic_buffer&& that) noexcept
        {
            this->swap(that);

            return *this;
        }

        ~dynamic_buffer() noexcept
        {
            std::free(data);
        }

        std::expected<iterator_validity, error_status> reserve(std::size_t new_capacity) noexcept
        {
            if (new_capacity <= this->capacity)
                return iterator_validity::valid;

            if constexpr(grow_factor > 1)
            {
                Element* new_data = std::realloc(this->data_, this->capacity * sizeof(Element) * grow_factor);

                if (new_data == nullptr)
                    return std::unexpected(error_status::out_of_memory);

                const auto iterator_validity = new_data != this->data
                    ? iterator_validity::invalid
                    : iterator_validity::valid;

                this->data = new_data;
                this->capacity = capacity;

                return iterator_validity;
            }
            else
            {
                return std::unexpected(error_status::out_of_memory);
            }
        }

        [[nodiscard]] constexpr const_pointer data() const noexcept
        {
            return this->data_;
        }

        [[nodiscard]] constexpr pointer data() noexcept
        {
            return const_cast<pointer>(std::as_const(*this).data_);
        }

        [[nodiscard]] constexpr std::size_t capacity() const noexcept
        {
            return this->capacity_;
        }

        [[nodiscard]] constexpr std::size_t size() const noexcept
        {
            return this->size_;
        }

        [[nodiscard]] constexpr const_reference operator[](std::size_t pos) const noexcept
        {
            return this->data[pos];
        }

        [[nodiscard]] constexpr reference operator[](std::size_t pos) noexcept
        {
            return const_cast<reference>(std::as_const(*this).data[pos]);
        }

        [[nodiscard]] const_iterator cbegin() const noexcept
        {
            return this->data_;
        }

        [[nodiscard]] const_iterator cend() const noexcept
        {
            return this->data_ + this->size_;
        }

        [[nodiscard]] const_iterator begin() const noexcept
        {
            return this->cbegin();
        }

        [[nodiscard]] const_iterator end() const noexcept
        {
            return this->cend();
        }

        [[nodiscard]] iterator begin() noexcept
        {
            return const_cast<iterator>(this->cbegin());
        }

        [[nodiscard]] iterator end() noexcept
        {
            return const_cast<iterator>(this->cend());
        }

        [[nodiscard]] const_reverse_iterator crbegin() const noexcept
        {
            return std::make_reverse_iterator(this->cend());
        }

        [[nodiscard]] const_reverse_iterator crend() const noexcept
        {
            return std::make_reverse_iterator(this->cbegin());
        }

        [[nodiscard]] const_reverse_iterator rbegin() const noexcept
        {
            return this->crbegin();
        }

        [[nodiscard]] const_reverse_iterator rend() const noexcept
        {
            return this->crend();
        }

        [[nodiscard]] reverse_iterator rbegin() noexcept
        {
            return std::make_reverse_iterator(this->end());
        }

        [[nodiscard]] reverse_iterator rend() noexcept
        {
            return std::make_reverse_iterator(this->begin());
        }

        [[nodiscard]] std::expected<iterator_validity, error_status> push_back(const Element& element) noexcept
        {
            auto result = this->reserve(this->capacity_ + 1uz);

            if (result)
                *(this->data_ + this->size_) = element;

            return result;
        }

        [[nodiscard]] std::expected<iterator_validity, error_status> push_back(Element&& element) noexcept
        {
            auto result = this->reserve(this->capacity_ + 1uz);

            if (result)
                *(this->data_ + this->size_) = std::move(element);

            return result;
        }

        error_status pop_back() noexcept
        {
            if (this->size_ == 0uz)
                return error_status::out_of_range;

            --this->size_;

            return error_status::success;
        }

        std::expected<std::pair<iterator, iterator_validity>, error_status> insert(
            const_iterator pos, const_reference value
        ) noexcept
        {
            const auto offset = pos - this->data_;

            const auto new_allocation_result = this->reserve(
                this->capacity_ + 1uz
            );

            if (!new_allocation_result)
                return new_allocation_result.error();

            const auto iterator_validity = *new_allocation_result;

            if (iterator_validity == iterator_validity::invalid)
                pos = this->data_ + offset;

            std::memmove(pos + 1, pos, end() - pos);

            *pos = value;

            return { pos, iterator_validity };
        }

    private:

        explicit dynamic_buffer(const std::size_t capacity) noexcept
            : data_{ static_cast<Element*>(std::malloc(capacity * sizeof(Element))) }
            , capacity_{ capacity }
        { }

        void swap(dynamic_buffer& that) noexcept
        {
            std::swap(this->data_, that.data_);
            std::swap(this->capacity_, that.capacity_);
            std::swap(this->size_, that.size_);
        }

        Element* data_ = nullptr;
        std::size_t capacity_{};
        std::size_t size_{};

    };
}