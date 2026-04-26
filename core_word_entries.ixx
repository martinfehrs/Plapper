module;

#include <string_view>
#include <type_traits>
#include <expected>

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
            else if constexpr (std::is_same_v<Callback, void(terminal&) noexcept>)
            {
                return this->callback(env.term), error_status::success;
            }
            else if constexpr (std::is_same_v<Callback, error_status(environment&, int_t) noexcept>)
            {
                return env.dstack.select(value).and_then(
                    [this, &env](auto a){ return this->callback(env, a); }
                );
            }
            else if constexpr (std::is_same_v<Callback, void(environment&, int_t) noexcept>)
            {
                return env.dstack.select(value).and_then(
                    [this, &env](auto a){ return this->callback(env, a), error_status::success; }
                );
            }
            else if constexpr (std::is_same_v<Callback, error_status(data_stack&, int_t) noexcept>)
            {
                return env.dstack.select(value).and_then(
                    [this, &env](auto a){ return this->callback(env.dstack, a); }
                );
            }
            else if constexpr (std::is_same_v<Callback, error_status(data_stack&, int_t, int_t) noexcept>)
            {
                return env.dstack.select(2_cuz * value).and_then(
                    [this, &env](auto a, auto b){ return this->callback(env.dstack, a, b); }
                );
            }
            else if constexpr (std::is_same_v<Callback, error_status(data_stack&, int_t, int_t, int_t) noexcept>)
            {
                return env.dstack.select(3_cuz * value).and_then(
                    [this, &env](auto a, auto b, auto c){ return this->callback(env.dstack, a, b, c); }
                );
            }
            else if constexpr (std::is_same_v<Callback, int_t(int_t) noexcept>)
            {
                return env.dstack.select(value).and_then(
                    [this, &env](auto a){ return env.dstack.replace<1>(this->callback(a)); }
                );
            }
            else if constexpr (std::is_same_v<Callback, int_t(int_t, int_t) noexcept>)
            {
                return env.dstack.select(2_cuz * value).and_then(
                    [this, &env](auto a, auto b){ return env.dstack.replace<2>(this->callback(a, b)); }
                );
            }
            else if constexpr (std::is_same_v<Callback, uint_t(uint_t, uint_t) noexcept>)
            {
                return env.dstack.select(2_cuz * value_of<uint_t>).and_then(
                    [this, &env](auto a, auto b){ return env.dstack.replace<2>(this->callback(a, b)); }
                );
            }
            else if constexpr (std::is_same_v<Callback, flag_t(int_t, int_t) noexcept>)
            {
                return env.dstack.select(2_cuz * value).and_then(
                    [this, &env](auto a, auto b){ return env.dstack.replace<2>(this->callback(a, b)); }
                );
            }
            else if constexpr (std::is_same_v<Callback, flag_t(uint_t, uint_t) noexcept>)
            {
                return env.dstack.select(2_cuz * value_of<uint_t>).and_then(
                    [this, &env](auto a, auto b){ return env.dstack.replace<2>(this->callback(a, b)); }
                );
            }
            else if constexpr (std::is_same_v<Callback, std::expected<int_t, error_status>(int_t, int_t) noexcept>)
            {
                return env.dstack.select(2_cuz * value).and_then(
                    [this, &env](auto a, auto b)
                    {
                        auto ret = this->callback(a, b);

                        if (!ret)
                            return ret.error();

                        return env.dstack.replace<2>(*ret);
                    }
                );
            }
            else if constexpr (std::is_same_v<Callback, std::expected<std::tuple<int_t, int_t>, error_status>(int_t, int_t) noexcept>)
            {
                return env.dstack.select(2_cuz * value).and_then(
                    [this, &env](auto a, auto b)
                    {
                        auto ret = this->callback(a, b);

                        if (!ret)
                            return ret.error();

                        return env.dstack.replace<2>(std::get<0>(*ret), std::get<1>(*ret));
                    }
                );
            }
            else if constexpr (std::is_same_v<Callback, int_t(int_t, int_t, int_t) noexcept>)
            {
                return env.dstack.select(3_cuz * value).and_then(
                    [this, &env](auto a, auto b, auto c){ return env.dstack.replace<3>(this->callback(a, b, c)); }
                );
            }
            else if constexpr (std::is_same_v<Callback, std::tuple<int_t, int_t>(int_t, int_t, int_t) noexcept>)
            {
                return env.dstack.select(3_cuz * value).and_then(
                    [this, &env](auto a, auto b, auto c)
                    {
                        const auto ret = this->callback(a, b, c);

                        return env.dstack.replace<3>(std::get<0>(ret), std::get<1>(ret));
                    }
                );
            }
            else if constexpr (std::is_same_v<Callback, void(int_t&) noexcept>)
            {
                return env.dstack.select(value).and_then(
                    [this](auto& a){ return this->callback(a), error_status::success; }
                );
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