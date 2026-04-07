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

    class procedure final : public execution_token
    {

    public:

        using callback_type = error_status(*)(environment&, void* data) noexcept;

        explicit constexpr procedure(const callback_type callback) noexcept
            : callback{ callback }
        { }

        [[nodiscard]] inline error_status operator()(environment& env, void* data) const noexcept override
        {
            return this->callback(env, data);
        }

    private:

        callback_type callback;

    };

    template <typename DataType>
    class closure final : public execution_token
    {
    public:

        using callback_type = error_status(*)(environment&, DataType data) noexcept;
        using data_type = DataType;

        explicit constexpr closure(const callback_type callback, DataType data) noexcept
            : callback{ callback }
            , data{ data }
        { }

        [[nodiscard]] inline error_status operator()(environment& env, void*) const noexcept override
        {
            return this->callback(env, this->data);
        }

    private:

        callback_type callback;
        DataType data;

    };

    struct module_entry
    {
        std::string word;
        std::variant<
            variable<int_t>,
            variable<uint_t>,
            constant<int_t>,
            constant<uint_t>,
            procedure,
            closure<int_t*>,
            closure<uint_t*>
        > token;
        bool immediate;
    };

}