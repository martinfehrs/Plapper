module;

#include <string_view>
#include <type_traits>

export module plapper:core_word_entries;

import :dictionary;
import :environment;

namespace plapper
{

    template <typename DataType>
    class constant_entry final : public core_word_entry
    {

    public:

        explicit constexpr constant_entry(const std::string_view word, const DataType value) noexcept
            : core_word_entry{ word }
            , value{ value }
        { }

        [[nodiscard]] inline error_status operator()(environment& env, void* data) noexcept override
        {
            return env.dstack.push(this->value);
        }

    private:

        std::string_view word_;
        DataType value;

    };

    template <typename DataType>
    class variable final : public core_word_entry
    {

    public:

        explicit constexpr variable(const std::string_view word, DataType& reference) noexcept
            : core_word_entry{ word }
            , address{ std::addressof(reference) }
        { }

        [[nodiscard]] inline error_status operator()(environment& env, void* data) noexcept override
        {
            return env.dstack.push(reinterpret_cast<int_t>(address));
        }

    private:

        DataType* address;

    };

    template <typename Callback>
    class procedure final : public core_word_entry
    {

    public:

        explicit constexpr procedure(
            const std::string_view word,
            Callback* callback,
            const execution_time_t execution_time = execution_time_t::delayed
        ) noexcept
            : core_word_entry{ word, execution_time }
            , callback{ callback }
        { }

        [[nodiscard]] inline error_status operator()(environment& env, [[maybe_unused]] void* data) noexcept override
        {
            if constexpr (std::is_same_v<Callback, void(environment&) noexcept>)
            {
                return this->callback(env), error_status::success;
            }
            else if constexpr (std::is_same_v<Callback, error_status(environment&) noexcept>)
            {
                return this->callback(env);
            }
            else if constexpr (std::is_same_v<Callback, void(data_stack&) noexcept>)
            {
                return this->callback(env.dstack), error_status::success;
            }
            else if constexpr (std::is_same_v<Callback, error_status(data_stack&) noexcept>)
            {
                return this->callback(env.dstack);
            }
            else
            {
                return this->callback(env);
            }
        }

    private:

        Callback* callback;

    };

    template <typename Callback, typename DataType>
    class closure final : public core_word_entry
    {
    public:

        explicit constexpr closure(const std::string_view word, Callback* callback, DataType& data) noexcept
            : core_word_entry{ word }
            , callback{ callback }
            , data{ std::addressof(data) }
        { }

        [[nodiscard]] inline error_status operator()(environment& env, void*) noexcept override
        {
            if constexpr (std::is_same_v<Callback, void(DataType&) noexcept>)
            {
                return this->callback(*this->data), error_status::success;
            }
            else if constexpr (std::is_same_v<Callback, error_status(DataType&) noexcept>)
            {
                return this->callback(*this->data);
            }
            else if constexpr (std::is_same_v<Callback, void(environment&, DataType&) noexcept>)
            {
                return this->callback(env, *this->data), error_status::success;
            }
            else
            {
                return this->callback(env, *this->data);
            }
        }

    private:

        Callback* callback;
        DataType* data;

    };

}