module;

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <utility>
#include <iterator>

export module plapper:memory_buffer;

import :error;
import :uninitialized;

namespace plapper
{

    template <typename Element>
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

        explicit constexpr dynamic_buffer(uninitialized_t) noexcept
        { }

        constexpr dynamic_buffer() noexcept
            : data_{ nullptr }
            , size_{ 0uz }
            , capacity_{ 0uz }
        { }

        dynamic_buffer(const dynamic_buffer& that) = delete;

        constexpr dynamic_buffer(dynamic_buffer&& that) noexcept
            : dynamic_buffer{}
        {
            this->swap(that);
        }

        static std::expected<dynamic_buffer, error_status> of_capacity(const std::size_t capacity) noexcept
        {
            dynamic_buffer instance{ uninitialized };

            instance.data_ = static_cast<Element*>(malloc(capacity * sizeof(Element)));

            if (instance.data_ == nullptr)
                return std::unexpected(error_status::out_of_memory);

            instance.size_ = 0uz;
            instance.capacity_ = capacity;

            return instance;
        }

        dynamic_buffer& operator=(const dynamic_buffer& that) = delete;



        constexpr dynamic_buffer& operator=(dynamic_buffer&& that) noexcept
        {
            this->swap(that);

            return *this;
        }

        ~dynamic_buffer() noexcept
        {
            std::free(this->data_);
        }

        [[nodiscard]] constexpr const_pointer data() const noexcept
        {
            return this->data_;
        }

        [[nodiscard]] constexpr pointer data() noexcept
        {
            return const_cast<pointer>(std::as_const(*this).data_);
        }

        [[nodiscard]] constexpr std::size_t size() const noexcept
        {
            return this->size_;
        }

        [[nodiscard]] constexpr std::size_t capacity() const noexcept
        {
            return this->capacity_;
        }

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return this->size_ == 0uz;
        }

        [[nodiscard]] constexpr const_reference operator[](std::size_t pos) const noexcept
        {
            return this->data[pos];
        }

        [[nodiscard]] constexpr reference operator[](std::size_t pos) noexcept
        {
            return const_cast<reference>(std::as_const(*this).data_[pos]);
        }

        [[nodiscard]] bool operator==(const dynamic_buffer& that) const noexcept
        {
            return this->size_ == that.size_
                && std::memcmp(this->data_, that.data_, this->size_) == 0;
        }

        [[nodiscard]] bool operator!=(const dynamic_buffer& that) const noexcept
        {
            return !(*this == that);
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

        error_status resize(std::size_t size) noexcept
        {
            if (size > this->capacity_)
                return error_status::out_of_memory;

            this->size_ = size;

            return error_status::success;
        }

        void clear()
        {
            this->size_ = 0uz;
        }

    private:

        void swap(dynamic_buffer& that) noexcept
        {
            std::swap(this->data_    , that.data_);
            std::swap(this->size_    , that.size_);
            std::swap(this->capacity_, that.capacity_);
        }

        Element* data_;
        std::size_t size_;
        std::size_t capacity_;

    };
}