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

        dictionary() = default;

        ~dictionary() noexcept
        {
            std::free(this->data_);
        }

        dictionary(const dictionary&) = delete;
        dictionary& operator=(const dictionary&) = delete;

        dictionary(dictionary&& that) noexcept
            : dictionary{}
        {
            this->swap(that);
        }

        dictionary& operator=(dictionary&& that) noexcept
        {
            this->swap(that);

            return *this;
        }

        static std::expected<dictionary, error_status> of_size(std::size_t byte_capacity) noexcept
        {
            dictionary dict;

            auto stat = dict.reserve(byte_capacity);

            if (stat != error_status::success)
                return std::unexpected(stat);

            return dict;
        }

        error_status reserve(std::size_t new_byte_capacity) noexcept
        {
            const auto new_data = static_cast<byte_t*>(std::realloc(this->data_, new_byte_capacity));

            if (!new_data)
                return error_status::out_of_memory;

            this->data_ = new_data;
            this->mem_size_ = new_byte_capacity;
            this->here_ = this->data_;

            return error_status::success;
        }

        [[nodiscard]] auto* here(this auto& self) noexcept
        {
            return self.here_;
        }

        template <typename T>
        [[nodiscard]] T* allot(const std::size_t count = 1) noexcept
        {
            const auto size = count * sizeof(T);

            if (this->here_ + size >= this->data_ + this->mem_size_)
                return nullptr;

            auto old_here = this->here_;

            this->here_ += size;

            return reinterpret_cast<T*>(old_here);
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
            entry* mem = this->allot<entry>();

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

        [[nodiscard]] entry* find(const std::string_view word) noexcept
        {
            entry* current = this->top();

            while (current != nullptr && !case_insensitive_compare(current->word, word))
                current = current->next;

            return current;
        }

    private:

        void swap(dictionary& that) noexcept
        {
            using std::swap;

            swap(this->mem_size_, that.mem_size_);
            swap(this->data_, that.data_);
            swap(this->here_, that.here_);
            swap(this->top_, that.top_);
        }

        std::size_t mem_size_ = 0;
        byte_t* data_ = nullptr;
        byte_t* here_ = nullptr;
        entry* top_ = nullptr;

    };

}