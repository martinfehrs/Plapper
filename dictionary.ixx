module;

#include <cctype>
#include <expected>
#include <iterator>
#include <string_view>
#include <stack>
#include <print>

export module plapper:dictionary;

import :error;
import :core_types;
import :core_constants;
import :memory_buffer;
import :stack;
import :type_params;

namespace rng = std::ranges;

namespace plapper
{

    [[nodiscard]] bool case_insensitive_compare(std::string_view str1, std::string_view str2) noexcept
    {
        return std::ranges::equal(
            str1,
            str2,
            [](const char c1, const char c2){ return std::tolower(c1) == std::tolower(c2); });
    }

    enum class execution_time_t
    {
        immediate,
        delayed
    };

    struct dictionary_entry : execution_token
    {

        explicit dictionary_entry(const execution_time_t execution_time = execution_time_t::delayed) noexcept
            : execution_time{ execution_time }
        { }

        [[nodiscard]] inline void* data() noexcept
        {
            return reinterpret_cast<byte_t*>(this) + sizeof(dictionary_entry);
        }

        [[nodiscard]] virtual std::string_view word() const noexcept = 0;

        execution_time_t execution_time;
        dictionary_entry* next;
    };

    class core_word_entry : public dictionary_entry
    {

    public:

        explicit core_word_entry(
            const std::string_view word, const execution_time_t execution_time = execution_time_t::delayed
        ) noexcept
            : dictionary_entry{ execution_time }
            , word_{ word }
        { }

        [[nodiscard]] inline std::string_view word() const noexcept final
        {
            return this->word_;
        }

    private:

        std::string_view word_;

    };

    class user_dictionary_entry : public dictionary_entry
    {

    public:

        user_dictionary_entry(
            std::string_view word, const execution_time_t execution_time = execution_time_t::delayed
        ) noexcept
            : dictionary_entry{ execution_time }
            , word_{ std::string{ word } }
        { }

        [[nodiscard]] std::string_view word() const noexcept final
        {
            return this->word_;
        }

    private:

        std::string word_;

    };

    export class dictionary
    {

    public:

        using key_type = std::string;
        using mapped_type = execution_token*;
        using value_type = dictionary_entry;
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

        [[nodiscard]] dictionary_entry* top() const noexcept
        {
            return this->top_;
        }

        void push_entry(dictionary_entry* entry) noexcept
        {
            entry->next = this->top_;
            this->top_ = entry;
        }

        template <typename Entry, typename... Args>
        error_status emplace_entry(typename_param<Entry>, const std::string_view word, Args&&... args) noexcept
        {
            auto new_entry = this->create<Entry>(word, std::forward<Args>(args)...);

            if (!new_entry)
                return new_entry.error();

            return push_entry(*new_entry), error_status::success;
        }

        template <template <typename...> typename EntryTemplate, typename... ConstructionArgs>
        error_status emplace_entry(
            template_param<EntryTemplate>, const std::string_view word, ConstructionArgs&&... construction_args
        ) noexcept
        {
            return this->emplace_entry(
                typename_v<decltype(EntryTemplate{ word, std::forward<ConstructionArgs>(construction_args)... })>,
                word,
                std::forward<ConstructionArgs>(construction_args)...
            );
        }

        template <template<typename...> typename Entry, typename ConstructionArg>
        error_status emplace_entries(
            template_param<Entry> entry_type, const std::string_view word, ConstructionArg&& construction_arg
        ) noexcept
        {
            return this->emplace_entry(entry_type, word, std::forward<ConstructionArg>(construction_arg));
        }

        template <
            template<typename...> typename EntryType,
            typename ConstructionArg,
            template <typename...> typename NextEntryType,
            typename... FurtherArgs
        >
        error_status emplace_entries(
            template_param<EntryType> entry_type,
            const std::string_view word,
            ConstructionArg&& construction_arg,
            template_param<NextEntryType> next_entry_type,
            FurtherArgs&&... further_args
        ) noexcept
        {
            if (
                const auto stat = this->emplace_entries(next_entry_type, std::forward<FurtherArgs>(further_args)...);
                stat != error_status::success
            )
            {
                return stat;
            }

            return this->emplace_entry(entry_type, word, std::forward<ConstructionArg>(construction_arg));
        }

        template <
            template <typename...> typename EntryType,
            typename ConstructionArg1,
            typename ConstructionArg2,
            template <typename...> typename NextEntryType,
            typename... FurtherArgs
        >
        error_status emplace_entries(
            template_param<EntryType> entry_type,
            const std::string_view word,
            ConstructionArg1&& construction_arg_1,
            ConstructionArg2&& construction_arg_2,
            template_param<NextEntryType> next_entry_type,
            FurtherArgs&&... further_args
        ) noexcept
        {
            if (const auto stat = this->emplace_entries(next_entry_type, std::forward<FurtherArgs>(further_args)...);
                stat != error_status::success
            )
            {
                return stat;
            }

            return this->emplace_entry(
                entry_type,
                word,
                std::forward<ConstructionArg1>(construction_arg_1),
                std::forward<ConstructionArg2>(construction_arg_2)
            );
        }

        [[nodiscard]] dictionary_entry* find(const std::string_view word) const noexcept
        {
            dictionary_entry* current = this->top();

            while (current != nullptr && !case_insensitive_compare(current->word(), word))
                current = current->next;

            return current;
        }

    private:

        explicit dictionary(buffer_type&& buffer) noexcept
            : buffer_{ std::move(buffer) }
        { }

        buffer_type buffer_;
        dictionary_entry* top_ = nullptr;

    };

}