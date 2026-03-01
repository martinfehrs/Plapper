module;

#include <cctype>
#include <expected>
#include <iterator>
#include <string_view>
#include <ranges>
#include <stack>
#include <variant>
#include <print>
#include <concepts>

export module plapper:dictionary;

import :error;
import :core_types;
import :core_constants;
import :memory_buffer;

namespace rng = std::ranges;

namespace plapper
{

    export struct module_entry;

    [[nodiscard]] bool case_insensitive_compare(std::string_view str1, std::string_view str2) noexcept
    {
        return std::ranges::equal(
            str1,
            str2,
            [](const char c1, const char c2){ return std::tolower(c1) == std::tolower(c2); });
    }

    export class dictionary
    {

    public:

        struct entry
        {
            std::string word;
            bool immediate;
            entry* next;
            execution_token_t xt;

            [[nodiscard]] void* data() noexcept
            {
                return reinterpret_cast<byte_t*>(this) + sizeof(entry);
            }
        };

        using key_type = std::string;
        using mapped_type = execution_token_t;
        using value_type = entry;
        using buffer_type = memory_buffer<byte_t>;

        dictionary(const dictionary&) = delete;
        dictionary(dictionary&& that) noexcept = default;
        dictionary& operator=(const dictionary&) = delete;
        dictionary& operator=(dictionary&& that) noexcept = default;

        static std::expected<dictionary, error_status> of_capacity(const std::size_t capacity) noexcept
        {
            auto buffer = buffer_type::of_capacity(capacity);

            if (!buffer)
                return std::unexpected(buffer.error());

            return dictionary{ std::move(*buffer) };
        }

        [[nodiscard]] auto* here(this auto& self) noexcept
        {
            return self.buffer_.end();
        }

        template <typename T>
        [[nodiscard]] T* allot(const std::size_t count = 1) noexcept
        {
            const auto size_diff = count * sizeof(T);

            if (this->buffer_.resize(this->buffer_.size() + size_diff) != error_status::success)
                return nullptr;

            return reinterpret_cast<T*>(this->buffer_.end() - size_diff);
        }

        template <typename T, typename... Args>
        [[nodiscard]] T* create(Args&&... args)
        {
            auto mem = this->allot<T>();

            if (!mem)
                return nullptr;

            return new(mem) T{ std::forward<Args>(args)... };
        }

        template <typename Value>
        Value* append(Value value)
        {
            return this->create<Value>(value);
        }

        [[nodiscard]] entry* top() const noexcept
        {
            return this->top_;
        }

        [[nodiscard]] entry* create(key_type name, const mapped_type execution_token, const bool immediate) noexcept
        {
            auto* mem = this->allot<entry>();

            if (!mem)
                return nullptr;

            this->top_ = new(mem) entry{ std::move(name), immediate, this->top_, execution_token };

            return mem;
        }

        [[nodiscard]] entry* create(key_type name, const mapped_type execution_token) noexcept
        {
            return this->create(std::move(name), execution_token, false);
        }

        template <typename Value>
        [[nodiscard]] entry* create(key_type name, const mapped_type execution_token, Value value) noexcept
        {
            entry* entry = this->create(std::move(name), execution_token);

            if (entry)
                this->append(value);

            return entry;
        }

        template<
            std::input_iterator I, std::sentinel_for<I> O> requires std::same_as<std::iter_value_t<I>, module_entry
        >
        error_status load(I begin, O end) noexcept
        {
            for (auto it = begin; it != end; ++it)
            {
                const auto* entry = this->create(it->word, it->impl, it->immediate);

                if (!entry)
                    return error_status::out_of_memory;

                if (std::holds_alternative<int_t>(it->data) && !this->append(std::get<int_t>(it->data)))
                    return error_status::out_of_memory;

                if (std::holds_alternative<uint_t>(it->data) && !this->append(std::get<uint_t>(it->data)))
                    return error_status::out_of_memory;

                if (std::holds_alternative<int_t*>(it->data) && !this->append(std::get<int_t*>(it->data)))
                    return error_status::out_of_memory;

                if (std::holds_alternative<uint_t*>(it->data) && !this->append(std::get<uint_t*>(it->data)))
                    return error_status::out_of_memory;
            }

            return error_status::success;
        }

        template <rng::input_range Range> requires std::same_as<rng::range_value_t<Range>, module_entry>
        error_status load(Range&& module) noexcept
        {
            return this->load(rng::begin(module), rng::end(module));
        }

        [[nodiscard]] entry* find(const std::string_view word) const noexcept
        {
            entry* current = this->top();

            while (current != nullptr && !case_insensitive_compare(current->word, word))
                current = current->next;

            return current;
        }

    private:

        explicit dictionary(buffer_type&& buffer) noexcept
            : buffer_{ std::move(buffer) }
        { }

        buffer_type buffer_;
        entry* top_ = nullptr;

    };

}