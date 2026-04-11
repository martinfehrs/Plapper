module;

#include <algorithm>
#include <cstring>
#include <expected>
#include <format>

export module plapper:core_words;

import :core_types;
import :dictionary;
import :environment;
import :core_module;

namespace rng = std::ranges;

namespace plapper
{

    export error_status store(data_stack& dstack) noexcept
    {
        return dstack.select(value, value_of<int_t*>).and_then(
            [&dstack](const auto x, const auto a_addr)
            {
                *a_addr = x;
                dstack.pop_n_unchecked(2);
            }
        );
    }

    export error_status times_divide(data_stack& dstack) noexcept
    {
        return dstack.select(3_cuz * value).and_then(
            [&dstack](const dint_t n1, const dint_t n2, const dint_t n3)
            {
                return dstack.replace<3>(static_cast<int_t>(n1 * n2 / n3));
            }
        );
    }

    export error_status times_divide_mod(data_stack& dstack) noexcept
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

    export error_status times(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2) { return dstack.replace<2>(n1 * n2); }
        );
    }

    export error_status plus(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2) { return dstack.replace<2>(n1 + n2); }
        );
    }

    export error_status plus_store(data_stack& dstack) noexcept
    {
        return dstack.select(value_of<int_t*>, value).and_then(
            [&dstack](const auto a_addr, const auto n)
            {
                *a_addr += n;
                dstack.pop_n_unchecked(2);
            }
        );
    }

    export error_status comma(environment& env) noexcept
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

    export error_status minus(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2) { return dstack.replace<2>(n1 - n2); }
        );
    }

    export error_status dot(environment& env) noexcept
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

    export error_status divide(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                return n2 == 0 ? error_status::division_by_zero
                               : dstack.replace<2>(n1 / n2);
            }
        );
    }

    export error_status divide_mod(data_stack& dstack) noexcept
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

    export error_status zero_less(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& x){ x = x < 0 ? yes : no; });
    }

    export error_status zero_equals(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& x){ x = x == 0 ? yes : no; });
    }

    export error_status one_plus(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n += 1; });
    }

    export error_status one_minus(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n -= 1; });
    }

    export error_status two_store(data_stack& dstack) noexcept
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

    export error_status two_star(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& x){ x <<= 1; });
    }

    export error_status two_slash(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& x){ x >>= 1; });
    }

    export error_status two_fetch(data_stack& dstack) noexcept
    {
        return dstack.select(value_of<int_t*>).and_then(
            [&dstack](const auto a_addr)
            {
                return dstack.replace<1>(a_addr[0], a_addr[1]);
            }
        );
    }

    export error_status two_drop(data_stack& dstack) noexcept
    {
        return dstack.pop_n(2);
    }

    export error_status two_dupe(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2){ return dstack.push(x1, x2); }
        );
    }

    export error_status two_over(data_stack& dstack) noexcept
    {
        return dstack.select(4_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2, const auto, const auto)
            {
                return dstack.push(x1, x2);
            }
        );
    }

    export error_status two_swap(data_stack& dstack) noexcept
    {
        return dstack.select(4_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2, const auto x3, const auto x4)
            {
                return dstack.replace<4>(x3, x4, x1, x2);
            }
        );
    }

    export error_status colon(environment& env) noexcept
    {
        struct colon_rt_t final : execution_token
        {
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

        static colon_rt_t colon_rt{};

        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        if (!env.dict.create(static_cast<std::string>(word), &colon_rt, false))
            return error_status::out_of_memory;

        env.state = yes;

        return error_status::success;
    }

    export error_status semicolon(environment& env) noexcept
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

    export error_status less_than(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                return dstack.replace<2>(n1 < n2 ? yes : no);
            }
        );
    }

    export error_status equals(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2)
            {
                return dstack.replace<2>(x1 == x2 ? yes : no);
            }
        );
    }

    export error_status greater_than(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                return dstack.replace<2>(n1 > n2 ? yes : no);
            }
        );
    }

    export error_status question_dupe(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then(
            [&dstack](const auto x){ return x ? dstack.push(x) : error_status::success; }
        );
    }

    export error_status fetch(data_stack& dstack) noexcept
    {
        return dstack.select(value_of<int_t*>).and_then(
            [&dstack](const auto a_addr){ return dstack.replace<1>(*a_addr); }
        );
    }

    export error_status abs(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n = std::abs(n); });
    }

    export error_status aligned(data_stack& dstack) noexcept
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

    export error_status allot(environment& env) noexcept
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

    export error_status and_(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2){ return dstack.replace<2>(x1 & x2); }
        );
    }

    export error_status b_l(data_stack& dstack) noexcept
    {
        return dstack.push(' ');
    }

    export error_status cell_plus(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& a_addr){ a_addr += cell_size; });
    }

    export error_status cells(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n *= cell_size; });
    }

    export error_status char_(environment& env) noexcept
    {
        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        return env.dstack.push(word[0]);
    }

    export error_status char_plus(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& c_addr){ c_addr += char_size; });
    }

    export error_status chars(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n *= char_size; });
    }

    export error_status constant_(environment& env) noexcept
    {
        class user_constant_t : public execution_token
        {

        public:

            explicit user_constant_t(const int_t value) noexcept
                : value{ value }
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

                const auto exec_token = env.dict.create<user_constant_t>(x);

                if (!exec_token)
                    return exec_token.error();

                const auto data = env.dict.create(static_cast<std::string>(word), *exec_token);

                if (!data)
                    return error_status::out_of_memory;

                env.dstack.pop_unchecked();

                return error_status::success;
            }
        );
    }

    export error_status count(data_stack& dstack) noexcept
    {
        return dstack.select(value_of<const char_t*>).and_then(
            [&dstack](const auto c_addr)
            {
                return dstack.replace<1>(reinterpret_cast<int_t>(c_addr + 1), c_addr[0]);
            }
        );
    }

    export error_status c_r(data_stack& dstack) noexcept
    {
        return dstack.push('\n');
    }

    export error_status create(environment& env) noexcept
    {
        struct user_data_t : execution_token
        {
            [[nodiscard]] error_status operator()(environment& env, void* data) noexcept override
            {
                return env.dstack.push(reinterpret_cast<int_t>(data));
            }
        };

        static user_data_t user_data;

        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        if (!env.dict.create(static_cast<std::string>(word), &user_data))
            return error_status::out_of_memory;

        return error_status::success;
    }

    export void decimal(int_t& base) noexcept
    {
        base = 10;
    }

    export error_status depth(data_stack& dstack) noexcept
    {
        return dstack.push(rng::size(dstack));
    }

    export error_status drop(data_stack& dstack) noexcept
    {
        return dstack.pop();
    }

    export error_status dupe(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([&dstack](const auto x){ return dstack.push(x); });
    }

    export error_status emit(environment& env) noexcept
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

    export error_status here(environment& env) noexcept
    {
        return env.dstack.push(reinterpret_cast<int_t>(env.dict.here()));
    }

    export error_status invert(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& x){ x = ~x; });
    }

    export error_status key(environment& env) noexcept
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

    export error_status l_shift(data_stack& dstack) noexcept
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
    export error_status m_star(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const dint_t n1, const dint_t n2)
            {
                return dstack.replace<2>(static_cast<int_t>(n1 * n2));
            }
        );
    }

    export error_status max(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                return dstack.replace<2>(std::max(n1, n2));
            }
        );
    }

    export error_status min(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto n1, const auto n2)
            {
                return dstack.replace<2>(std::min(n1, n2));
            }
        );
    }

    export error_status mod(data_stack& dstack) noexcept
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

    export error_status negate(data_stack& dstack) noexcept
    {
        return dstack.select(value).and_then([](auto& n){ n = -n; });
    }

    export error_status or_(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2){ return dstack.replace<2>(x1 | x2); }
        );
    }

    export error_status over(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto){ return dstack.push(x1); }
        );
    }

    export error_status rote(data_stack& dstack) noexcept
    {
        return dstack.select(3_cuz * value).and_then(
            [](auto& x1, auto& x2, auto& x3)
            {
                std::swap(x1, x2);
                std::swap(x2, x3);
            }
        );
    }

    export error_status r_shift(data_stack& dstack) noexcept
    {
        return dstack.select(value, value_of<uint_t>).and_then(
            [&dstack](auto& x, const auto u)
            {
                x >>= u;
                dstack.pop_unchecked();
            }
        );
    }

    export error_status s_to_d(data_stack& dstack) noexcept
    {
        return dstack.push(0);
    }

    export error_status space(environment& env) noexcept
    {
        env.term.write(' ');

        return error_status::success;
    }

    export error_status spaces(environment& env) noexcept
    {
        return env.dstack.select(value).and_then(
            [&env](const auto n)
            {
                env.term.write_n(' ', n);
                env.dstack.pop_unchecked();
            }
        );
    }

    export error_status state(environment& env) noexcept
    {
        return env.dstack.push(reinterpret_cast<int_t>(&env.state));
    }

    export error_status swap(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [](auto& x1, auto& x2){ std::swap(x1, x2); }
        );
    }

    export error_status type(environment& env) noexcept
    {
        return env.dstack.select(value_of<uint_t>, value_of<const char_t*>).and_then(
            [&env](const auto u, const auto c_addr)
            {
                env.term.write({ c_addr, u });
                env.dstack.pop_n_unchecked(2);
            }
        );
    }

    export error_status u_dot(environment& env) noexcept
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

    export error_status u_less_than(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value_of<uint_t>).and_then(
            [&dstack](const auto u1, const auto u2)
            {
                return dstack.replace<2>(u1 < u2);
            }
        );
    }

    export error_status variable_(environment& env) noexcept
    {
        class user_variable_t : public execution_token
        {

        public:

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

        auto exec_token = env.dict.create<user_variable_t>();

        if (!exec_token)
            return exec_token.error();

        if (!env.dict.create(static_cast<std::string>(word), *exec_token))
            return error_status::out_of_memory;

        return error_status::success;
    }

    export error_status word(environment& env) noexcept
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

    export error_status xor_(data_stack& dstack) noexcept
    {
        return dstack.select(2_cuz * value).and_then(
            [&dstack](const auto x1, const auto x2)
            {
                return dstack.replace<2>(x1 ^ x2);
            }
        );
    }

    export class core_words_t
    {

        static auto& create_entries(int_t& base) noexcept
        {

            static module_entry entries_[]{
                { "!"            , procedure      { store                   }, false },
                //{ "#"            , procedure      { /*sharp*/               }, false },
                //{ "#>"           , procedure      { /*number_sign_greater*/ }, false },
                //{ "#S"           , procedure      { /*sharp_s*/             }, false },
                //{ "'"            , procedure      { /*tick*/                }, false },
                //{ "("            , procedure      { /*paren*/               }, true  },
                { "*"            , procedure      { times                   }, false },
                { "*/"           , procedure      { times_divide            }, false },
                { "*/MOD"        , procedure      { times_divide_mod        }, false },
                { "+"            , procedure      { plus                    }, false },
                { "+!"           , procedure      { plus_store              }, false },
                //{ "+LOOP"        , procedure      { /*plus_loop*/           }, false },
                { ","            , procedure      { comma                   }, false },
                { "-"            , procedure      { minus                   }, false },
                { "."            , procedure      { dot                     }, false },
                //{ ".\""          , procedure      { /*dot_quote*/           }, false },
                { "/"            , procedure      { divide                  }, false },
                { "/MOD"         , procedure      { divide_mod              }, false },
                { "0<"           , procedure      { zero_less               }, false },
                { "0="           , procedure      { zero_equals             }, false },
                { "1+"           , procedure      { one_plus                }, false },
                { "1-"           , procedure      { one_minus               }, false },
                { "2!"           , procedure      { two_store               }, false },
                { "2*"           , procedure      { two_star                }, false },
                { "2/"           , procedure      { two_slash               }, false },
                { "2@"           , procedure      { two_fetch               }, false },
                { "2DUP"         , procedure      { two_dupe                }, false },
                { "2DROP"        , procedure      { two_drop                }, false },
                { "2OVER"        , procedure      { two_over                }, false },
                { "2SWAP"        , procedure      { two_swap                }, false },
                { ":"            , procedure      { colon                   }, false },
                { ";"            , procedure      { semicolon               }, true  },
                { "<"            , procedure      { less_than               }, false },
                //{ "<#"           , procedure      { /*less_number_sign*/    }, false },
                { "="            , procedure      { equals                  }, false },
                { ">"            , procedure      { greater_than            }, false },
                //{ ">BODY"        , procedure      { /*to_body*/             }, false },
                //{ ">IN"          , procedure      { /*to_in*/               }, false },
                //{ ">NUMBER"      , procedure      { /*to_number*/           }, false },
                //{ ">R"           , procedure      { /*to_r*/                }, false },
                { "?DUP"         , procedure      { question_dupe           }, false },
                { "@"            , procedure      { fetch                   }, false },
                //{ "ABORT"        , procedure      { /*abort_*/              }, false },
                //{ "ABORT\""      , procedure      { /*abort_quote*/         }, false },
                { "ABS"          , procedure      { abs                     }, false },
                //{ "ACCEPT"       , procedure      { /*accept*/              }, false },
                //{ "ALIGN"        , procedure      { /*align_*/              }, false },
                { "ALIGNED"      , procedure      { aligned                 }, false },
                { "ALLOT"        , procedure      { allot                   }, false },
                { "AND"          , procedure      { and_                    }, false },
                { "BASE"         , variable       { base                 }, false },
                //{ "BEGIN"        , procedure      { /*begin_*/              }, false },
                { "BL"           , procedure      { b_l                     }, false },
                //{ "C!"           , procedure      { /*c_store*/             }, false },
                //{ "C,"           , procedure      { /*c_comma*/             }, false },
                //{ "C@"           , procedure      { /*c_fetch*/             }, false },
                { "CELL+"        , procedure      { cell_plus               }, false },
                { "CELLS"        , procedure      { cells                   }, false },
                { "CHAR"         , procedure      { char_                   }, false },
                { "CHAR+"        , procedure      { char_plus               }, false },
                { "CHARS"        , procedure      { chars                   }, false },
                { "CONSTANT"     , procedure      { constant_               }, false },
                { "COUNT"        , procedure      { count                   }, false },
                { "CR"           , procedure      { c_r                     }, false },
                { "CREATE"       , procedure      { create                  }, false },
                { "DECIMAL"      , closure        { decimal, base        }, false },
                { "DEPTH"        , procedure      { depth                   }, false },
                //{ "DO"           , procedure      { /*do_*/                 }, false },
                //{ "DOES>"        , procedure      { /*does*/                }, false },
                { "DROP"         , procedure      { drop                    }, false },
                { "DUP"          , procedure      { dupe                    }, false },
                //{ "ELSE"         , procedure      { /*else_*/               }, false },
                { "EMIT"         , procedure      { emit                    }, false },
                //{ "ENVIRONMENT?" , procedure      { /*environment_query*/   }, false },
                //{ "EVALUATE"     , procedure      { /*evaluate*/            }, false },
                //{ "EXECUTE"      , procedure      { /*execute*/             }, false },
                //{ "EXIT"         , procedure      { /*exit_*/               }, false },
                //{ "FILL"         , procedure      { /*fill*/                }, false },
                //{ "FIND"         , procedure      { /*find*/                }, false },
                //{ "FM/MOD"       , procedure      { /*f_m_slash_mod*/       }, false },
                { "HERE"         , procedure      { here                    }, false },
                //{ "HOLD"         , procedure      { /*hold*/                }, false },
                //{ "I"            , procedure      { /*i*/                   }, false },
                //{ "IF"           , procedure      { /*if_*/                 }, false },
                //{ "IMMEDIATE"    , procedure      { /*immediate*/           }, false },
                { "INVERT"       , procedure      { invert                  }, false },
                //{ "J"            , procedure      { /*j*/                   }, false },
                { "KEY"          , procedure      { key                     }, false },
                //{ "LEAVE"        , procedure      { /*leave*/               }, false },
                //{ "LITERAL"      , procedure      { /*literal*/             }, false },
                //{ "LOOP"         , procedure      { /*loop*/                }, false },
                { "LSHIFT"       , procedure      { l_shift                 }, false },
                { "M*"           , procedure      { m_star                  }, false },
                { "MAX"          , procedure      { max                     }, false },
                { "MIN"          , procedure      { min                     }, false },
                { "MOD"          , procedure      { mod                     }, false },
                //{ "MOVE"         , procedure      { /*move*/                }, false },
                { "NEGATE"       , procedure      { negate                  }, false },
                { "OR"           , procedure      { or_                     }, false },
                { "OVER"         , procedure      { over                    }, false },
                //{ "POSTPONE"     , procedure      { /*postpone*/            }, false },
                //{ "QUIT"         , procedure      { /*quit*/                }, false },
                //{ "R>"           , procedure      { /*r_from*/              }, false },
                //{ "R@"           , procedure      { /*r_fetch*/             }, false },
                //{ "RECURSE"      , procedure      { /*recurse*/             }, false },
                //{ "REPEAT"       , procedure      { /*repeat*/              }, false },
                { "ROT"          , procedure      { rote                    }, false },
                { "RSHIFT"       , procedure      { r_shift                 }, false },
                //{ "S\""          , procedure      { /*s_quote*/             }, false },
                { "S>D"          , procedure      { s_to_d                  }, false },
                //{ "SIGN"         , procedure      { /*sign*/                }, false },
                //{ "SM/REM"       , procedure      { /*s_m_slash_rem*/       }, false },
                //{ "SOURCE"       , procedure      { /*source*/              }, false },
                { "SPACE"        , procedure      { space                   }, false },
                { "SPACES"       , procedure      { spaces                  }, false },
                { "STATE"        , procedure      { state                   }, false },
                { "SWAP"         , procedure      { swap                    }, false },
                //{ "THEN"         , procedure      { /*then*/                }, false },
                { "TYPE"         , procedure      { type                    }, false },
                { "U."           , procedure      { u_dot                   }, false },
                { "U<"           , procedure      { u_less_than             }, false },
                //{ "UM*"          , procedure      { /*u_m_star*/            }, false },
                //{ "UM/MOD"       , procedure      { /*u_m_slash_mod*/       }, false },
                //{ "UNLOOP"       , procedure      { /*unloop*/              }, false },
                //{ "UNTIL"        , procedure      { /*until*/               }, false },
                { "VARIABLE"     , procedure      { variable_               }, false },
                //{ "WHILE"        , procedure      { /*while_*/              }, false },
                { "WORD"         , procedure      { word                    }, false },
                { "XOR"          , procedure      { xor_                    }, false },
                //{ "["            , procedure      { /*left_bracket*/        }, true  },
                //{ "[']"          , procedure      { /*bracket_tick*/        }, false },
                //{ "[CHAR]"       , procedure      { /*bracket_char*/        }, false },
                //{ "]"            , procedure      { /*right_bracket*/       }, false },
            };
            return entries_;
        }

        int_t* base_addr_;
        std::span<module_entry> entries;

    public:

        static std::expected<core_words_t, error_status> with_dict(dictionary& dict)
        {
            auto base_addr = dict.append<int_t>(10);

            if (!base_addr)
                return std::unexpected(base_addr.error());

            return core_words_t{ **base_addr };
        }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return this->base_addr_;
        }

        [[nodiscard]] int_t& base() const noexcept
        {
            return *this->base_addr_;
        }

        [[nodiscard]] auto begin() const noexcept
        {
            return this->entries.begin();
        }

        [[nodiscard]] auto end() const noexcept
        {
            return this->entries.end();
        }

    private:

        explicit core_words_t(int_t& base) noexcept
            : base_addr_{ std::addressof(base) }
            , entries{ this->create_entries(base) }
        { }

    };

}
