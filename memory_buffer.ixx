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

        static std::expected<dynamic_buffer, error_status> of_size(std::size_t size) noexcept
        {
            dynamic_buffer instance{ size };

            if (instance.data_ == nullptr)
                return std::unexpected(error_status::out_of_memory);

            return instance;
        }

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

    private:

        explicit dynamic_buffer(const std::size_t size) noexcept
            : data_{ static_cast<Element*>(std::malloc(size * sizeof(Element))) }
            , size_{ size }
        { }

        void swap(dynamic_buffer& that) noexcept
        {
            std::swap(this->data_, that.data_);
            std::swap(this->size_, that.size_);
        }

        Element* data_ = nullptr;
        std::size_t size_{};

    };
}