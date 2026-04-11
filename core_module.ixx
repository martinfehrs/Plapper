module;

#include <string>
#include <variant>

export module plapper:core_module;

import :core_types;
import :error;
import :dictionary;
import :environment;

namespace plapper
{

    template <typename DataType>
    class constant final : public execution_token
    {

    public:

        using data_type = DataType;

        explicit constexpr constant(const DataType value) noexcept
            : value{ value }
        { }

        [[nodiscard]] inline error_status operator()(environment& env, void* data) const noexcept override
        {
            return env.dstack.push(this->value);
        }

    private:

        DataType value;

    };

    template <typename DataType>
    class variable final : public execution_token
    {

    public:

        using data_type = DataType;

        explicit constexpr variable(DataType* address) noexcept
            : address{ address }
        { }

        explicit constexpr variable(DataType& reference) noexcept
            : variable{ std::addressof(reference) }
        { }

        [[nodiscard]] inline error_status operator()(environment& env, void* data) const noexcept override
        {
            return env.dstack.push(reinterpret_cast<int_t>(address));
        }

    private:

        DataType* address;

    };

    template <typename CallbackType>
    class procedure final : public execution_token
    {

    public:

        explicit constexpr procedure(CallbackType* callback) noexcept
            : callback{ callback }
        { }

        [[nodiscard]] inline error_status operator()(
            environment& env, [[maybe_unused]] void* data
        ) const noexcept override
        {
            if constexpr (std::is_same_v<CallbackType, void(environment&) noexcept>)
            {
                return this->callback(env), error_status::success;
            }
            else if constexpr (std::is_same_v<CallbackType, error_status(environment&) noexcept>)
            {
                return this->callback(env);
            }
            else if constexpr (std::is_same_v<CallbackType, void(data_stack&) noexcept>)
            {
                return this->callback(env.dstack), error_status::success;
            }
            else if constexpr (std::is_same_v<CallbackType, error_status(data_stack&) noexcept>)
            {
                return this->callback(env.dstack);
            }
            else
            {
                return this->callback(env);
            }
        }

    private:

        CallbackType* callback;

    };

    template <typename CallbackType, typename DataType>
    class closure final : public execution_token
    {
    public:

        using data_type = DataType;

        explicit constexpr closure(CallbackType* callback, DataType& data) noexcept
            : callback{ callback }
            , data{ std::addressof(data) }
        { }

        [[nodiscard]] inline error_status operator()(environment& env, void*) const noexcept override
        {
            if constexpr (std::is_same_v<CallbackType, void(DataType&) noexcept>)
            {
                return this->callback(*this->data), error_status::success;
            }
            else if constexpr (std::is_same_v<CallbackType, error_status(DataType&) noexcept>)
            {
                return this->callback(*this->data);
            }
            else if constexpr (std::is_same_v<CallbackType, void(environment&, DataType&) noexcept>)
            {
                return this->callback(env, *this->data), error_status::success;
            }
            else
            {
                return this->callback(env, *this->data);
            }
        }

    private:

        CallbackType* callback;
        DataType* data;

    };

    struct module_entry
    {
        std::string word;
        std::variant<
            variable<int_t>,
            variable<uint_t>,
            constant<int_t>,
            constant<uint_t>,
            procedure<error_status(environment&) noexcept>,
            procedure<void(environment&) noexcept>,
            procedure<error_status(data_stack&) noexcept>,
            procedure<void(data_stack&) noexcept>,
            closure<void(int_t&) noexcept, int_t>,
            closure<error_status(int_t&) noexcept, int_t>,
            closure<void(uint_t&) noexcept, uint_t>,
            closure<error_status(uint_t&) noexcept, uint_t>,
            closure<void(environment&, int_t&) noexcept, int_t>,
            closure<error_status(environment&, int_t&) noexcept, int_t>,
            closure<void(environment&, uint_t&) noexcept, uint_t>,
            closure<error_status(environment&, uint_t&) noexcept, uint_t>
        > token;
        bool immediate;
    };

}