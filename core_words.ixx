module;

#include <algorithm>
#include <cstring>
#include <expected>
#include <format>

export module plapper:core_words;

import :core_types;
import :dictionary;
import :environment;
import :core_word_entries;
import :type_params;

namespace rng = std::ranges;

namespace plapper
{

    error_status store(data_stack& dstack) noexcept
    {
        return dstack.select(value, value_of<int_t*>).and_then(
            [&dstack](const auto x, const auto a_addr)
            {
                *a_addr = x;
                dstack.pop_n_unchecked(2);
            }
        );
    }

    error_status times_divide(data_stack& dstack) noexcept
    {
        return dstack.select(3_cuz * value).and_then(
            [&dstack](const dint_t n1, const dint_t n2, const dint_t n3)
            {
                return dstack.replace<3>(static_cast<int_t>(n1 * n2 / n3));
            }
        );
    }

    error_status times_divide_mod(data_stack& dstack) noexcept
    {
        return dstack.select(3_cuz * value).and_then(
            [&dstack](const dint_t n1, const dint_t n2, const dint_t n3)
            {
                const dint_t intermediate_product = n1 * n2;
                const auto quotient = intermediate_product / n3;
                const auto reminder = intermediate_product % n3;

                return dstack.replace<3>(static_cast<int_t>(quotient), static_cast<int_t>(reminder));
            }
        );
    }

    error_status times(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2) { return dstack.replace<2>(n1 * n2); }
        );
    }

    error_status plus(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2) { return dstack.replace<2>(n1 + n2); }
        );
    }

    error_status plus_store(data_stack& dstack) noexcept
    {
        return dstack.select(value_of<int_t*>, value).and_then(
            [&dstack](const auto a_addr, const auto n)
            {
                *a_addr += n;
                dstack.pop_n_unchecked(2);
            }
        );
    }

    error_status comma(environment& env) noexcept
    {
        return env.dstack.select(value).and_then(
            [&env](const auto x)
            {
                if (!env.dict.append(x))
                    return error_status::out_of_memory;

                env.dstack.pop_unchecked();

                return error_status::success;
            }
        );
    }

    error_status minus(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2) { return dstack.replace<2>(n1 - n2); }
        );
    }

    error_status dot(environment& env) noexcept
    {
        return env.dstack.select(value).and_then(
            [&env](const auto n)
            {
                static constexpr auto buffer_size = std::numeric_limits<int_t>::digits10 + 2;
                static char buffer[buffer_size];

                const auto out = std::format_to(buffer, "{}", n);

                env.term.write({ buffer, static_cast<std::size_t>(out - buffer) });
                env.dstack.pop_unchecked();
            }
        );
    }

    error_status divide(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                return n2 == 0 ? error_status::division_by_zero
                               : dstack.replace<2>(n1 / n2);
            }
        );
    }

    error_status divide_mod(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                if (n2 == 0)
                    return error_status::division_by_zero;

                const auto[quot, reminder] = std::div(n1, n2);

                return dstack.replace<2>(reminder, quot);
            }
        );
    }

    error_status zero_less(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& x){ x = x < 0 ? yes : no; });
    }

    error_status zero_equals(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& x){ x = x == 0 ? yes : no; });
    }

    error_status one_plus(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n += 1; });
    }

    error_status one_minus(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n -= 1; });
    }

    error_status two_store(data_stack& dstack) noexcept
    {
        return dstack.select(value_of<int_t*>, 2_cuz * value).and_then(
            [&dstack](const auto a_addr, const auto x1, const auto x2)
            {
                a_addr[0] = x2;
                a_addr[1] = x1;
                dstack.pop_n_unchecked(3);
            }
        );
    }

    error_status two_star(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& x){ x <<= 1; });
    }

    error_status two_slash(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& x){ x >>= 1; });
    }

    error_status two_fetch(data_stack& dstack) noexcept
    {
        return dstack.select(value_of<int_t*>).and_then(
            [&dstack](const auto a_addr)
            {
                return dstack.replace<1>(a_addr[0], a_addr[1]);
            }
        );
    }

    error_status two_drop(data_stack& dstack) noexcept
    {
        return dstack.pop_n(2);
    }

    error_status two_dupe(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2){ return dstack.push(x1, x2); }
        );
    }

    error_status two_over(data_stack& dstack) noexcept
    {
        return dstack.select(4_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2, const auto, const auto)
            {
                return dstack.push(x1, x2);
            }
        );
    }

    error_status two_swap(data_stack& dstack) noexcept
    {
        return dstack.select(4_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2, const auto x3, const auto x4)
            {
                return dstack.replace<4>(x3, x4, x1, x2);
            }
        );
    }

    error_status colon(environment& env) noexcept
    {
        struct user_word_entry final : public user_dictionary_entry
        {
            using user_dictionary_entry::user_dictionary_entry;

            [[nodiscard]] error_status operator()(environment& env, void* data) noexcept override
            {
                if (env.instruction_ptr)
                {
                    if (const auto status = env.rstack.push(env.instruction_ptr); status != error_status::success)
                        return status;
                }

                env.instruction_ptr = static_cast<execution_token***>(data) - 1;

                return error_status::success;
            }
        };

        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        if (
            const auto stat = env.dict.emplace_entry(typename_v<user_word_entry>, word);
            stat != error_status::success
        )
        {
            return error_status::out_of_memory;
        }

        env.state = yes;

        return error_status::success;
    }

    error_status semicolon(environment& env) noexcept
    {
        struct semicolon_rt_t final : execution_token
        {
            [[nodiscard]] error_status operator()(environment& env, void*) noexcept override
            {
                return env.rstack.select(value)
                     .and_then(
                         [&env](const auto ptr)
                         {
                             env.instruction_ptr = ptr;
                             env.rstack.pop_unchecked();
                         }
                     ).or_else(
                         [&env]
                         {
                             env.instruction_ptr = nullptr;
                         }
                     );
            }
        };

        env.state = no;

        static semicolon_rt_t semicolon_rt{};
        static auto semicolon_rt_ptr = &semicolon_rt;

        if (!env.dict.append(&semicolon_rt_ptr))
            return error_status::out_of_memory;

        return error_status::success;
    }

    error_status less_than(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                return dstack.replace<2>(n1 < n2 ? yes : no);
            }
        );
    }

    error_status equals(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2)
            {
                return dstack.replace<2>(x1 == x2 ? yes : no);
            }
        );
    }

    error_status greater_than(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                return dstack.replace<2>(n1 > n2 ? yes : no);
            }
        );
    }

    error_status question_dupe(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then(
            [&dstack](const auto x){ return x ? dstack.push(x) : error_status::success; }
        );
    }

    error_status fetch(data_stack& dstack) noexcept
    {
        return dstack.select(value_of<int_t*>).and_then(
            [&dstack](const auto a_addr){ return dstack.replace<1>(*a_addr); }
        );
    }

    error_status abs(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n = std::abs(n); });
    }

    error_status aligned(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then(
            [](auto& a_addr)
            {
                auto offset = a_addr % cell_size;

                if (offset == 0)
                    return;

                offset = cell_size - offset;

                a_addr += offset;
            }
        );
    }

    error_status allot(environment& env) noexcept
    {
        return env.dstack.select(value).and_then(
            [&env](const auto n)
            {
                if (const auto mem = env.dict.allot<byte_t>(n); !mem)
                    return mem.error();

                env.dstack.pop_unchecked();

                return error_status::success;
            }
        );
    }

    error_status and_(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2){ return dstack.replace<2>(x1 & x2); }
        );
    }

    error_status b_l(data_stack& dstack) noexcept
    {
        return dstack.push(' ');
    }

    error_status cell_plus(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& a_addr){ a_addr += cell_size; });
    }

    error_status cells(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n *= cell_size; });
    }

    error_status char_(environment& env) noexcept
    {
        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        return env.dstack.push(word[0]);
    }

    error_status char_plus(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& c_addr){ c_addr += char_size; });
    }

    error_status chars(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n *= char_size; });
    }

    error_status constant_(environment& env) noexcept
    {
        class user_constant_entry : public user_dictionary_entry
        {

        public:

            explicit user_constant_entry(const std::string_view word, const int_t value) noexcept
                : user_dictionary_entry{ word }
                , value{ value }
            { }

            [[nodiscard]] error_status operator()(environment& env, void* data) noexcept override
            {
                return env.dstack.push(this->value);
            }

        private:

            int_t value;

        };

        return env.dstack.select(value).and_then(
            [&env](const auto x)
            {
                const auto word = env.tib.read_word();

                if (word.empty())
                    return error_status::out_of_words;

                if (
                    const auto stat = env.dict.emplace_entry(typename_v<user_constant_entry>, word, x);
                    stat != error_status::success
                )
                {
                    return stat;
                }

                env.dstack.pop_unchecked();

                return error_status::success;
            }
        );
    }

    error_status count(data_stack& dstack) noexcept
    {
        return dstack.select(value_of<const char_t*>).and_then(
            [&dstack](const auto c_addr)
            {
                return dstack.replace<1>(reinterpret_cast<int_t>(c_addr + 1), c_addr[0]);
            }
        );
    }

    error_status c_r(data_stack& dstack) noexcept
    {
        return dstack.push('\n');
    }

    error_status create(environment& env) noexcept
    {
        struct user_data_entry : user_dictionary_entry
        {
            using user_dictionary_entry::user_dictionary_entry;

            [[nodiscard]] error_status operator()(environment& env, void* data) noexcept override
            {
                return env.dstack.push(reinterpret_cast<int_t>(data));
            }
        };

        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        return env.dict.emplace_entry(typename_v<user_data_entry>, word);
    }

    void decimal(int_t& base) noexcept
    {
        base = 10;
    }

    error_status depth(data_stack& dstack) noexcept
    {
        return dstack.push(rng::size(dstack));
    }

    error_status drop(data_stack& dstack) noexcept
    {
        return dstack.pop();
    }

    error_status dupe(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([&dstack](const auto x){ return dstack.push(x); });
    }

    error_status emit(environment& env) noexcept
    {
        return env.dstack.select(value).and_then(
            [&env](const auto x)
            {
                if (x < 0 || x > 127)
                    return error_status::out_of_character_range;

                env.term.write(static_cast<char>(x));
                env.dstack.pop_unchecked();

                return error_status::success;
            }
        );
    }

    error_status here(environment& env) noexcept
    {
        return env.dstack.push(reinterpret_cast<int_t>(env.dict.here()));
    }

    error_status invert(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& x){ x = ~x; });
    }

    error_status key(environment& env) noexcept
    {
        std::expected<char_t, error_status> c;

        do
        {
            c = env.term.read_char();

            if (!c)
                return c.error();
        }
        while (!std::isprint(*c));

        return env.dstack.push(*c);
    }

    error_status l_shift(data_stack& dstack) noexcept
    {
        return dstack.select(value, value_of<uint_t>).and_then(
            [&dstack](auto& x, const auto u)
            {
                x <<= u;
                dstack.pop_unchecked();
            }
        );
    }

    // KORREKTUR: Ergebnis muss eine Vorzeichenbehaftete Ganzzahl doppelter Genauigkeit sein (dint_t)
    error_status m_star(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const dint_t n1, const dint_t n2)
            {
                return dstack.replace<2>(static_cast<int_t>(n1 * n2));
            }
        );
    }

    error_status max(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                return dstack.replace<2>(std::max(n1, n2));
            }
        );
    }

    error_status min(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                return dstack.replace<2>(std::min(n1, n2));
            }
        );
    }

    error_status mod(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                if (n2 == 0)
                    return error_status::division_by_zero;

                return dstack.replace<2>(n1 % n2);
            }
        );
    }

    error_status negate(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n = -n; });
    }

    error_status or_(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2){ return dstack.replace<2>(x1 | x2); }
        );
    }

    error_status over(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto){ return dstack.push(x1); }
        );
    }

    error_status rote(data_stack& dstack) noexcept
    {
        return dstack.select(3_cuz * value).and_then(
            [](auto& x1, auto& x2, auto& x3)
            {
                std::swap(x1, x2);
                std::swap(x2, x3);
            }
        );
    }

    error_status r_shift(data_stack& dstack) noexcept
    {
        return dstack.select(value, value_of<uint_t>).and_then(
            [&dstack](auto& x, const auto u)
            {
                x >>= u;
                dstack.pop_unchecked();
            }
        );
    }

    error_status s_to_d(data_stack& dstack) noexcept
    {
        return dstack.push(0);
    }

    error_status space(environment& env) noexcept
    {
        env.term.write(' ');

        return error_status::success;
    }

    error_status spaces(environment& env) noexcept
    {
        return env.dstack.select(value).and_then(
            [&env](const auto n)
            {
                env.term.write_n(' ', n);
                env.dstack.pop_unchecked();
            }
        );
    }

    error_status state(environment& env) noexcept
    {
        return env.dstack.push(reinterpret_cast<int_t>(&env.state));
    }

    error_status swap(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [](auto& x1, auto& x2){ std::swap(x1, x2); }
        );
    }

    error_status type(environment& env) noexcept
    {
        return env.dstack.select(value_of<uint_t>, value_of<const char_t*>).and_then(
            [&env](const auto u, const auto c_addr)
            {
                env.term.write({ c_addr, u });
                env.dstack.pop_n_unchecked(2);
            }
        );
    }

    error_status u_dot(environment& env) noexcept
    {
        return env.dstack.select(value_of<uint_t>).and_then(
            [&env](auto u)
            {
                static constexpr auto buffer_size = std::numeric_limits<uint_t>::digits10 + 1;
                static char buffer[buffer_size];

                const auto out = std::format_to(buffer, "{}", u);

                env.term.write({ buffer, static_cast<std::size_t>(out - buffer) });
            }
        );
    }

    error_status u_less_than(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value_of<uint_t>).and_then(
            [&dstack](const auto u1, const auto u2)
            {
                return dstack.replace<2>(u1 < u2);
            }
        );
    }

    error_status variable_(environment& env) noexcept
    {
        class user_variable_entry : public user_dictionary_entry
        {

        public:

            using user_dictionary_entry::user_dictionary_entry;

            [[nodiscard]] error_status operator()(environment& env, void* data) noexcept override
            {
                return env.dstack.push(reinterpret_cast<int_t>(&this->value));
            }

        private:

            int_t value{};

        };

        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        return env.dict.emplace_entry(typename_v<user_variable_entry>, word);
    }

    error_status word(environment& env) noexcept
    {
        return env.dstack.select(value_of<uint_t>).and_then(
            [&env](const char char_)
            {
                const auto word = env.tib.read_until(char_);

                if (word.size() > 256)
                    return error_status::out_of_memory;

                static char_t string_storage[257];

                string_storage[0] = static_cast<char>(word.size());
                std::memcpy(string_storage + 1, word.data(), word.size());

                return env.dstack.replace<1>(reinterpret_cast<int_t>(string_storage));
            }
        );
    }

    error_status xor_(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2)
            {
                return dstack.replace<2>(x1 ^ x2);
            }
        );
    }

    struct shared_core_word_data
    {
        int_t* base;
    };

    std::expected<shared_core_word_data, error_status> load_core_words(dictionary& dict)
    {
        auto base = dict.allot<int_t>();

        if (!base)
            return std::unexpected(base.error());

        const auto stat = dict.emplace_entries(
            template_v<procedure>, "!"           , store              ,
            //template_v<procedure>, "#"           , sharp              ,
            //template_v<procedure>, "#>"          , number_sign_greater,
            //template_v<procedure>, "#S"          , sharp_s            ,
            //template_v<procedure>, "'"           , tick               ,
            //template_v<procedure>, "("           , paren              ,
            template_v<procedure>, "*"           , times              ,
            template_v<procedure>, "*/"          , times_divide       ,
            template_v<procedure>, "*/MOD"       , times_divide_mod   ,
            template_v<procedure>, "+"           , plus               ,
            template_v<procedure>, "+!"          , plus_store         ,
            //template_v<procedure>, "+LOOP"       , plus_loop          ,
            template_v<procedure>, ","           , comma              ,
            template_v<procedure>, "-"           , minus              ,
            template_v<procedure>, "."           , dot                ,
            //template_v<procedure>, ".\""         , dot_quote          ,
            template_v<procedure>, "/"           , divide             ,
            template_v<procedure>, "/MOD"        , divide_mod         ,
            template_v<procedure>, "0<"          , zero_less          ,
            template_v<procedure>, "0="          , zero_equals        ,
            template_v<procedure>, "1+"          , one_plus           ,
            template_v<procedure>, "1-"          , one_minus          ,
            template_v<procedure>, "2!"          , two_store          ,
            template_v<procedure>, "2*"          , two_star           ,
            template_v<procedure>, "2/"          , two_slash          ,
            template_v<procedure>, "2@"          , two_fetch          ,
            template_v<procedure>, "2DUP"        , two_dupe           ,
            template_v<procedure>, "2DROP"       , two_drop           ,
            template_v<procedure>, "2OVER"       , two_over           ,
            template_v<procedure>, "2SWAP"       , two_swap           ,
            template_v<procedure>, ":"           , colon              ,
            template_v<procedure>, ";"           , semicolon          ,
            template_v<procedure>, "<"           , less_than          ,
            //template_v<procedure>, "<#"          , less_number_sign   ,
            template_v<procedure>, "="           , equals             ,
            template_v<procedure>, ">"           , greater_than       ,
            //template_v<procedure>, ">BODY"       , to_body            ,
            //template_v<procedure>, ">IN"         , to_in              ,
            //template_v<procedure>, ">NUMBER"     , to_number          ,
            //template_v<procedure>, ">R"          , to_r               ,
            template_v<procedure>, "?DUP"        , question_dupe      ,
            template_v<procedure>, "@"           , fetch              ,
            //template_v<procedure>, "ABORT"       , abort_             ,
            //template_v<procedure>, "ABORT\""     , abort_quote        ,
            template_v<procedure>, "ABS"         , abs                ,
            //template_v<procedure>, "ACCEPT"      , accept             ,
            //template_v<procedure>, "ALIGN"       , align_             ,
            template_v<procedure>, "ALIGNED"     , aligned            ,
            template_v<procedure>, "ALLOT"       , allot              ,
            template_v<procedure>, "AND"         , and_               ,
            template_v<variable >, "BASE"        , **base             ,
            //template_v<procedure>, "BEGIN"       , begin_             ,
            template_v<procedure>, "BL"          , b_l                ,
            //template_v<procedure>, "C!"          , c_store            ,
            //template_v<procedure>, "C,"          , c_comma            ,
            //template_v<procedure>, "C@"          , c_fetch            ,
            template_v<procedure>, "CELL+"       , cell_plus          ,
            template_v<procedure>, "CELLS"       , cells              ,
            template_v<procedure>, "CHAR"        , char_              ,
            template_v<procedure>, "CHAR+"       , char_plus          ,
            template_v<procedure>, "CHARS"       , chars              ,
            template_v<procedure>, "CONSTANT"    , constant_          ,
            template_v<procedure>, "COUNT"       , count              ,
            template_v<procedure>, "CR"          , c_r                ,
            template_v<procedure>, "CREATE"      , create             ,
            template_v<closure  >, "DECIMAL"     , decimal, **base    ,
            template_v<procedure>, "DEPTH"       , depth              ,
            //template_v<procedure>, "DO"          , do_                ,
            //template_v<procedure>, "DOES>"       , does               ,
            template_v<procedure>, "DROP"        , drop               ,
            template_v<procedure>, "DUP"         , dupe               ,
            //template_v<procedure>, "ELSE"        , else_              ,
            template_v<procedure>, "EMIT"        , emit               ,
            //template_v<procedure>, "ENVIRONMENT?", environment_query  ,
            //template_v<procedure>, "EVALUATE"    , evaluate           ,
            //template_v<procedure>, "EXECUTE"     , execute            ,
            //template_v<procedure>, "EXIT"        , exit_              ,
            //template_v<procedure>, "FILL"        , fill               ,
            //template_v<procedure>, "FIND"        , find               ,
            //template_v<procedure>, "FM/MOD"      , f_m_slash_mod      ,
            template_v<procedure>, "HERE"        , here               ,
            //template_v<procedure>, "HOLD"        , hold               ,
            //template_v<procedure>, "I"           , i                  ,
            //template_v<procedure>, "IF"          , if_                ,
            //template_v<procedure>, "IMMEDIATE"   , immediate          ,
            template_v<procedure>, "INVERT"      , invert             ,
            //template_v<procedure>, "J"           , j                  ,
            template_v<procedure>, "KEY"         , key                ,
            //template_v<procedure>, "LEAVE"       , leave              ,
            //template_v<procedure>, "LITERAL"     , literal            ,
            //template_v<procedure>, "LOOP"        , loop               ,
            template_v<procedure>, "LSHIFT"      , l_shift            ,
            template_v<procedure>, "M*"          , m_star             ,
            template_v<procedure>, "MAX"         , max                ,
            template_v<procedure>, "MIN"         , min                ,
            template_v<procedure>, "MOD"         , mod                ,
            //template_v<procedure>, "MOVE"        , move               ,
            template_v<procedure>, "NEGATE"      , negate             ,
            template_v<procedure>, "OR"          , or_                ,
            template_v<procedure>, "OVER"        , over               ,
            //template_v<procedure>, "POSTPONE"    , postpone           ,
            //template_v<procedure>, "QUIT"        , quit               ,
            //template_v<procedure>, "R>"          , r_from             ,
            //template_v<procedure>, "R@"          , r_fetch            ,
            //template_v<procedure>, "RECURSE"     , recurse            ,
            //template_v<procedure>, "REPEAT"      , repeat             ,
            template_v<procedure>, "ROT"         , rote               ,
            template_v<procedure>, "RSHIFT"      , r_shift            ,
            //template_v<procedure>, "S\""         , s_quote            ,
            template_v<procedure>, "S>D"         , s_to_d             ,
            //template_v<procedure>, "SIGN"        , sign               ,
            //template_v<procedure>, "SM/REM"      , s_m_slash_rem      ,
            //template_v<procedure>, "SOURCE"      , source             ,
            template_v<procedure>, "SPACE"       , space              ,
            template_v<procedure>, "SPACES"      , spaces             ,
            template_v<procedure>, "STATE"       , state              ,
            template_v<procedure>, "SWAP"        , swap               ,
            //template_v<procedure>, "THEN"        , then               ,
            template_v<procedure>, "TYPE"        , type               ,
            template_v<procedure>, "U."          , u_dot              ,
            template_v<procedure>, "U<"          , u_less_than        ,
            //template_v<procedure>, "UM*"         , u_m_star           ,
            //template_v<procedure>, "UM/MOD"      , u_m_slash_mod      ,
            //template_v<procedure>, "UNLOOP"      , unloop             ,
            //template_v<procedure>, "UNTIL"       , until              ,
            template_v<procedure>, "VARIABLE"    , variable_          ,
            //template_v<procedure>, "WHILE"       , while_             ,
            template_v<procedure>, "WORD"        , word               ,
            template_v<procedure>, "XOR"         , xor_
            //template_v<procedure>, "["           , left_bracket       ,
            //template_v<procedure>, "[']"         , bracket_tick       ,
            //template_v<procedure>, "[CHAR]"      , bracket_char       ,
            //template_v<procedure>, "]"           , right_bracket
        );

        if (stat != error_status::success)
            return std::unexpected(stat);

        return shared_core_word_data{ *base };
    }

}
