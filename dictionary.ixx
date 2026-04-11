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

    class execution_token
    {

    public:

        [[nodiscard]] virtual error_status operator()(environment&, void* data) noexcept = 0;
        virtual ~execution_token() = default;

    };

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
            execution_token* xt;

            [[nodiscard]] void* data() noexcept
            {
                return reinterpret_cast<byte_t*>(this) + sizeof(entry);
            }
        };

        using key_type = std::string;
        using mapped_type = execution_token*;
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
        [[nodiscard]] std::expected<T*, error_status> allot(const std::size_t count = 1) noexcept
        {
            const auto size_diff = count * sizeof(T);

            if (this->buffer_.resize(this->buffer_.size() + size_diff) != error_status::success)
                return std::unexpected(error_status::out_of_memory);

            return reinterpret_cast<T*>(this->buffer_.end() - size_diff);
        }

        template <typename T, typename... Args>
        [[nodiscard]] std::expected<T*, error_status> create(Args&&... args) noexcept
        {
            auto mem = this->allot<T>();

            if (!mem)
                return mem;

            return new(*mem) T{ std::forward<Args>(args)... };
        }

        template <typename Value>
        std::expected<Value*, error_status> append(Value value) noexcept
        {
            return this->create<Value>(value);
        }

        [[nodiscard]] entry* top() const noexcept
        {
            return this->top_;
        }

        [[nodiscard]] std::expected<entry*, error_status> create(
            key_type name, mapped_type execution_token, const bool immediate
        ) noexcept
        {
            auto mem = this->allot<entry>();

            if (!mem)
                return std::unexpected(mem.error());

            this->top_ = new(*mem) entry{ std::move(name), immediate, this->top_, execution_token };

            return mem;
        }

        [[nodiscard]] std::expected<entry*, error_status> create(
            key_type name, mapped_type execution_token
        ) noexcept
        {
            return this->create(std::move(name), execution_token, false);
        }

        template <typename Value>
        [[nodiscard]] std::expected<entry*, error_status> create(
            key_type name, mapped_type execution_token, Value value
        ) noexcept
        {
            auto entry = this->create(std::move(name), execution_token);

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
                const auto status = std::visit(
                    [this, it](std::derived_from<execution_token> auto& token)
                    {
                        if (const auto entry = this->create(it->word, &token, it->immediate); !entry)
                            return entry.error();

                        return error_status::success;
                    },
                    it->token
                );

                if (status != error_status::success)
                    return status;
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