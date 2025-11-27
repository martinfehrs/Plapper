module;

#include <algorithm>
#include <cstring>
#include <format>

export module plapper:core_words;

import :core_types;
import :dictionary;
import :environment;
import :core_module;

namespace rng = std::ranges;

namespace plapper
{

    export error_status store(environment& env, void*) noexcept
    {
        const auto a_addr_ptr = env.dstack.top().as<int_t*>();

        if (!a_addr_ptr)
            return error_status::stack_underflow;

        const auto x_ptr = env.dstack.nth(1);

        if (!x_ptr)
            return error_status::stack_underflow;

        **a_addr_ptr = *x_ptr;

        env.dstack.unchecked_pop_n(2);

        return error_status::success;
    }

    export error_status times_divide(environment& env, void*) noexcept
    {
        return env.dstack.for_args<3>(
            [&env](const dint_t n1, const dint_t n2, const dint_t n3)
            {
                return env.dstack.replace<3>(static_cast<int_t>(n1 * n2 / n3));
            }
        );
    }

    export error_status times(environment& env, void*) noexcept
    {
        return env.dstack.for_args<2>(
            [&env](const auto n1, const auto n2)
            {
                return env.dstack.replace<2>(n1 * n2);
            }
        );
    }

    export error_status plus(environment& env, void*) noexcept
    {
        const auto n_range = env.dstack.top_n<2>();

        if (!n_range)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(n_range[0] + n_range[1]);
    }

    export error_status plus_store(environment& env, void*) noexcept
    {
        const auto range = env.dstack.top_n<2>();

        if (!range)
            return error_status::stack_underflow;

        *reinterpret_cast<int_t*>(range[1]) += range[0];
        env.dstack.unchecked_pop_n(2);

        return error_status::success;
    }

    export error_status comma(environment& env, void*) noexcept
    {
        auto x_ptr = env.dstack.top();

        if (!x_ptr)
            return error_status::stack_underflow;

        if (!env.dict.append(*x_ptr))
            return error_status::out_of_memory;

        env.dstack.unchecked_pop();

        return error_status::success;
    }

    export error_status minus(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(srange[0] - srange[1]);
    }

    export error_status dot(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as_const();

        if (!sptr)
            return error_status::stack_underflow;

        env.tob.write(std::format("{}", *sptr));
        env.dstack.unchecked_pop();

        return error_status::success;
    }

    export error_status divide(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        if (srange[1] == 0)
            return error_status::division_by_zero;

        return env.dstack.replace<2>(srange[0] / srange[1]);
    }

    export error_status divide_mod(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        if (srange[1] == 0)
            return error_status::division_by_zero;

        const auto[quot, reminder] = std::div(srange[0], srange[1]);

        return env.dstack.replace<2>(reminder, quot);
    }

    export error_status zero_less(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as_const();

        if (!sptr)
            return error_status::stack_underflow;

        return env.dstack.replace<1>(*sptr < 0 ? yes : no);
    }

    export error_status zero_equals(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as_const();

        if (!sptr)
            return error_status::stack_underflow;

        return env.dstack.replace<1>(*sptr == 0 ? yes : no);
    }

    export error_status one_plus(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr += 1;

        return error_status::success;
    }

    export error_status one_minus(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr -= 1;

        return error_status::success;
    }

    export error_status two_store(environment& env, void*) noexcept
    {
        const auto a_addr_ptr = env.dstack.top().as<int_t*>();

        if (!a_addr_ptr)
            return error_status::stack_underflow;

        const auto x_range = env.dstack.get_n<2>(1);

        if (!x_range)
            return error_status::stack_underflow;

        rng::copy(x_range, *a_addr_ptr);

        env.dstack.unchecked_pop_n(3);

        return error_status::success;
    }

    export error_status two_star(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr <<= 1;

        return error_status::success;
    }

    export error_status two_slash(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr >>= 1;

        return error_status::success;
    }

    export error_status two_fetch(environment& env, void*) noexcept
    {
        const auto a_addr_ptr = env.dstack.top().as<int_t*>();

        if (!a_addr_ptr)
            return error_status::stack_underflow;

        const auto a_addr = *a_addr_ptr;

        return env.dstack.replace<1>(a_addr[0], a_addr[1]);
    }

    export error_status two_drop(environment& env, void*) noexcept
    {
        return env.dstack.pop_n(2);
    }

    export error_status two_dupe(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.push(srange[0], srange[1]);
    }

    export error_status two_over(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<4>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.push(srange[0], srange[1]);
    }

    export error_status two_swap(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<4>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.replace<4>(srange[2], srange[3], srange[0], srange[1]);
    }

    error_status colon_rt(environment& env, void* data) noexcept
    {
        if (env.instruction_ptr)
        {
            if (const auto status = env.rstack.push(env.instruction_ptr); status != error_status::success)
                return status;
        }

        env.instruction_ptr = static_cast<execution_token_t**>(data) - 1;

        return error_status::success;
    }

    export error_status colon(environment& env, void*) noexcept
    {
        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        if (!env.dict.create(static_cast<std::string>(word), colon_rt, false))
            return error_status::out_of_memory;

        env.state = yes;

        return error_status::success;
    }

    error_status semicolon_rt(environment& env, void*) noexcept
    {
        if (const auto ptr = env.rstack.top())
        {
            env.instruction_ptr = *ptr;
            env.rstack.unchecked_pop();
        }
        else
            env.instruction_ptr = nullptr;

        return error_status::success;
    };

    export error_status semicolon(environment& env, void*) noexcept
    {
        env.state = no;

        static execution_token_t semicolon_rt_ptr = semicolon_rt;

        if (!env.dict.append(&semicolon_rt_ptr))
            return error_status::out_of_memory;

        return error_status::success;
    }

    export error_status less_than(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(srange[0] < srange[1] ? yes : no);
    }

    export error_status equals(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(srange[0] == srange[1] ? yes : no);
    }

    export error_status greater_than(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(srange[0] > srange[1] ? yes : no);
    }

    export error_status question_dupe(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as_const();

        if (!sptr)
            return error_status::stack_underflow;

        if (*sptr)
            return env.dstack.push(*sptr);

        return error_status::success;
    }

    export error_status fetch(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as<int_t*>();

        if (!sptr)
            return error_status::stack_underflow;

        return env.dstack.replace<1>(**sptr);
    }

    export error_status abs(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr = std::abs(*sptr);

        return error_status::success;
    }

    export error_status aligned(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        auto offset = *sptr % cell_size;

        if (offset == 0)
            return error_status::success;

        offset = cell_size - offset;

        *sptr += offset;

        return error_status::success;
    }

    export error_status allot(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as_const();

        if (!sptr)
            return error_status::stack_underflow;

        if (const auto* mem = env.dict.allot<byte_t>(*sptr); !mem)
            return error_status::out_of_memory;

        env.dstack.unchecked_pop();

        return error_status::success;
    }

    export error_status and_(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(srange[0] & srange[1]);
    }

    export error_status base(environment& env, void* data) noexcept
    {
        return env.dstack.push(reinterpret_cast<int_t>(&env.base));
    }

    export error_status b_l(environment& env, void*) noexcept
    {
        return env.dstack.push(' ');
    }

    export error_status cell_plus(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr += cell_size;

        return error_status::success;
    }

    export error_status cells(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr = cell_size;

        return error_status::success;
    }

    export error_status char_(environment& env, void*) noexcept
    {
        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        return env.dstack.push(word[0]);
    }

    export error_status char_plus(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr += char_size;

        return error_status::success;
    }

    export error_status chars(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr = char_size;

        return error_status::success;
    }

    error_status constant__(environment& env, void* data) noexcept
    {
        return env.dstack.push(*static_cast<int_t*>(data));
    }

    export error_status constant_(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as_const();

        if (!sptr)
            return error_status::stack_underflow;

        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        const auto data = env.dict.create(
            static_cast<std::string>(word),
            constant__,
            *sptr);

        if (!data)
            return error_status::out_of_memory;

        env.dstack.unchecked_pop();

        return error_status::success;
    }

    export error_status count(environment& env, void*) noexcept
    {
        const auto c_addr_1_ptr = env.dstack.top().as<const char_t*>();

        if (!c_addr_1_ptr)
            return error_status::stack_underflow;

        const auto c_addr_1 = *c_addr_1_ptr;
        const int_t u = c_addr_1[0];
        const char_t* c_addr_2 = c_addr_1 + 1;

        return env.dstack.replace<1>(reinterpret_cast<int_t>(c_addr_2), u);
    }

    export error_status c_r(environment& env, void*) noexcept
    {
        return env.dstack.push('\n');
    }

    error_status create_rt(environment& env, void* data) noexcept
    {
        return env.dstack.push(reinterpret_cast<int_t>(data));
    }

    export error_status create(environment& env, void*) noexcept
    {
        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        if (!env.dict.create(static_cast<std::string>(word), create_rt))
            return error_status::out_of_memory;

        return error_status::success;
    }

    export error_status decimal(environment& env, void*) noexcept
    {
        env.base = 10;

        return error_status::success;
    }

    export error_status depth(environment& env, void*) noexcept
    {
        return env.dstack.push(rng::size(env.dstack));
    }

    export error_status drop(environment& env, void*) noexcept
    {
        return env.dstack.pop();
    }

    export error_status dupe(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as_const();

        if (!sptr)
            return error_status::stack_underflow;

        return env.dstack.push(*sptr);
    }

    export error_status emit(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as_const();

        if (!sptr)
            return error_status::stack_underflow;

        if (*sptr < 0 || *sptr > 127)
            return error_status::out_of_character_range;

        env.tob.write(static_cast<char>(*sptr));
        env.dstack.unchecked_pop();

        return error_status::success;
    }

    export error_status here(environment& env, void*) noexcept
    {
        return env.dstack.push(reinterpret_cast<int_t>(env.dict.here()));
    }

    export error_status invert(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr = ~*sptr;

        return error_status::success;
    }

    export error_status l_shift(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr <<= 1;

        return error_status::success;
    }

    export error_status m_star(environment& env, void*) noexcept
    {
        const auto n_range = env.dstack.top_n<2>();

        if (!n_range)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(static_cast<dint_t>(n_range[0]) * static_cast<dint_t>(n_range[1]));
    }

    export error_status max(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(std::max(srange[0], srange[1]));
    }

    export error_status min(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(std::min(srange[0], srange[1]));
    }

    export error_status mod(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        if (srange[1] == 0)
            return error_status::division_by_zero;

        return env.dstack.replace<2>(srange[0] % srange[1]);
    }

    export error_status negate(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr = -*sptr;

        return error_status::success;
    }

    export error_status or_(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(srange[0] | srange[1]);
    }

    export error_status over(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.push(srange[0]);
    }

    export error_status rote(environment& env, void*) noexcept
    {
        auto srange = env.dstack.top_n<3>();

        if (!srange)
            return error_status::stack_underflow;

        rng::rotate(srange, rng::next(rng::begin(srange)));

        return error_status::success;
    }

    export error_status r_shift(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top();

        if (!sptr)
            return error_status::stack_underflow;

        *sptr >>= 1;

        return error_status::success;
    }

    export error_status s_to_d(environment& env, void*) noexcept
    {
        return env.dstack.push(0);
    }

    export error_status space(environment& env, void*) noexcept
    {
        env.tob.write(' ');

        return error_status::success;
    }

    export error_status spaces(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as_const();

        if (!sptr)
            return error_status::stack_underflow;

        env.tob.write(' ', *sptr);
        env.dstack.unchecked_pop();

        return error_status::success;
    }

    export error_status state(environment& env, void*) noexcept
    {
        return env.dstack.push(reinterpret_cast<int_t>(&env.state));
    }

    export error_status swap(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        std::swap(srange[0], srange[1]);

        return error_status::success;
    }

    export error_status type(environment& env, void*) noexcept
    {
        const auto u_ptr = env.dstack.top().as<std::size_t>();

        if (!u_ptr)
            return error_status::stack_underflow;

        const auto c_addr_ptr = env.dstack.nth(1).as<const char*>();

        if (!c_addr_ptr)
            return error_status::stack_underflow;

        env.tob.write({ *c_addr_ptr, *u_ptr });

        env.dstack.unchecked_pop_n(2);

        return error_status::success;
    }

    export error_status u_dot(environment& env, void*) noexcept
    {
        const auto sptr = env.dstack.top().as_const();

        if (!sptr)
            return error_status::stack_underflow;

        env.tob.write(std::format("{}", static_cast<uint_t>(*sptr)));

        return error_status::success;
    }

    export error_status u_less_than(environment& env, void*) noexcept
    {
        const auto srange = env.dstack.top_n<2>();

        if (!srange)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(static_cast<uint_t>(srange[0]) < static_cast<uint_t>(srange[1]));
    }

    error_status variable_(environment& env, void* data) noexcept
    {
        return env.dstack.push(reinterpret_cast<int_t>(data));
    }

    export error_status variable(environment& env, void*) noexcept
    {
        const auto word = env.tib.read_word();

        if (word.empty())
            return error_status::out_of_words;

        if (!env.dict.create(static_cast<std::string>(word), variable_))
            return error_status::out_of_memory;

        if (!env.dict.allot<int_t>())
            return error_status::out_of_memory;

        return error_status::success;
    }

    export error_status word(environment& env, void*) noexcept
    {
        const auto char_ptr = env.dstack.top().as<const char>();

        if (!char_ptr)
            return error_status::stack_underflow;

        const auto word = env.tib.read_until(*char_ptr);

        static char_t string_storage[256];

        string_storage[0] = word.size();
        std::memcpy(string_storage + 1, word.data(), word.size());

        return env.dstack.replace<1>(reinterpret_cast<int_t>(string_storage));
    }

    export error_status xor_(environment& env, void*) noexcept
    {
        const auto x_range = env.dstack.top_n<2>();

        if (!x_range)
            return error_status::stack_underflow;

        return env.dstack.replace<2>(x_range[0] ^ x_range[1]);
    }

    export class core_words_t
    {

        static const auto& create_entries(int_t* base_addr) noexcept
        {
            static const module_entry entries_[]{
                { "!"            , &store              , false },
                //{ "#"            , &sharp              , false },
                //{ "#>"           , &number_sign_greater, false },
                //{ "#S"           , &sharp_s            , false },
                //{ "'",           , &tick               , false },
                //{ "(",           , &paren              , true  },
                { "*"            , &times              , false },
                { "*/"           , &times_divide       , false },
                //{ "*/MOD"        , &times_divide_mod   , false },
                { "+"            , &plus               , false },
                { "+!"           , &plus_store         , false },
                //{ "+LOOP"        , &plus_loop          , false },
                { ","            , &comma              , false },
                { "-"            , &minus              , false },
                { "."            , &dot                , false },
                //{ ".\""          , &dot_quote          , false },
                { "/"            , &divide             , false },
                { "/MOD"         , &divide_mod         , false },
                { "0<"           , &zero_less          , false },
                { "0="           , &zero_equals        , false },
                { "1+"           , &one_plus           , false },
                { "1-"           , &one_minus          , false },
                { "2!"           , &two_store          , false },
                { "2*"           , &two_star           , false },
                { "2/"           , &two_slash          , false },
                { "2@"           , &two_fetch          , false },
                { "2DUP"         , &two_dupe           , false },
                { "2DROP"        , &two_drop           , false },
                { "2OVER"        , &two_over           , false },
                { "2SWAP"        , &two_swap           , false },
                { ":"            , &colon              , false },
                { ";"            , &semicolon          , true  },
                { "<"            , &less_than          , false },
                //{ "<#"           , &less_number_sign   , false },
                { "="            , &equals             , false },
                { ">"            , &greater_than       , false },
                //{ ">BODY"        , &to_body            , false },
                //{ ">IN"          , &to_in              , false },
                //{ ">NUMBER"      , &to_number          , false },
                //{ ">R"           , &to_r               , false },
                { "?DUP"         , &question_dupe      , false                   },
                { "@"            , &fetch              , false                   },
                //{ "ABORT"        , &abort              , false                   },
                //{ "ABORT\""      , &abort_quote        , false                   },
                { "ABS"          , &abs                , false                   },
                //{ "ACCEPT"       , &accept             , false                   },
                //{ "ALIGN"        , &align              , false                   },
                { "ALIGNED"      , &aligned            , false                   },
                { "ALLOT"        , &allot              , false                   },
                { "AND"          , &and_               , false                   },
                { "BASE"         , &base               , false, base_addr },
                //{ "BEGIN"        , &begin              , false                   },
                { "BL"           , &b_l                , false                   },
                //{ "C!"           , &c_store            , false                   },
                //{ "C,"           , &c_comma            , false                   },
                //{ "C@"           , &c_fetch            , false                   },
                { "CELL+"        , &cell_plus          , false                   },
                { "CELLS"        , &cells              , false                   },
                { "CHAR"         , &char_              , false                   },
                { "CHAR+"        , &char_plus          , false                   },
                { "CHARS"        , &chars              , false                   },
                { "CONSTANT"     , &constant_          , false                   },
                { "COUNT"        , &count              , false                   },
                { "CR"           , &c_r                , false                   },
                { "CREATE"       , &create             , false                   },
                { "DECIMAL"      , &decimal            , false, base_addr },
                { "DEPTH"        , &depth              , false                   },
                //{ "DO"           , &do_                , false },
                //{ "DOES>"        , &does               , false },
                { "DROP"        , &drop                , false },
                { "DUP"         , &dupe                , false },
                //{ "ELSE"          , &else_             , false },
                { "EMIT"        , &emit                , false },
                //{ "ENVIRONMENT?", &environment_query   , false },
                //{ "EVALUATE"    , &evaluate            , false },
                //{ "EXECUTE"     , &execute             , false },
                //{ "EXIT"        , &exit                , false },
                //{ "FILL"        , &fill                , false },
                //{ "FIND"        , &find                , false },
                //{ "FM/MOD"      , &f_m_slash_mod       , false },
                { "HERE"          , &here              , false },
                //{ "HOLD"        , &hold                , false },
                //{ "I"           , &i                   , false },
                //{ "IF"          , &if_                 , false },
                //{ "IMMEDIATE"   , &immediate           , false },
                { "INVERT"      , &invert              , false },
                //{ "J"           , &j                   , false },
                //{ "KEY"         , &k                   , false },
                //{ "LEAVE"       , &leave               , false },
                //{ "LITERAL"     , &literal             , false },
                //{ "LOOP"        , &loop                , false },
                { "LSHIFT"      , &l_shift             , false },
                { "M*"          , &m_star              , false },
                { "MAX"         , &max                 , false },
                { "MIN"         , &min                 , false },
                { "MOD"         , &mod                 , false },
                //module_entry{ "MOVE"        , &move                , false },
                { "NEGATE"      , &negate              , false },
                { "OR"          , &or_                 , false },
                { "OVER"        , &over                , false },
                //{ "POSTPONE"    , &postpone            , false },
                //{ "QUIT"        , &quit                , false },
                //{ "R>"          , &r_from              , false },
                //{ "R@"          , &r_fetch             , false },
                //{ "RECURSE"     , &recurse             , false },
                //{ "REPEAT"      , &repeat              , false },
                { "ROT"         , &rote                , false },
                { "RSHIFT"      , &r_shift             , false },
                //{ "S\""         , &s_quote             , false },
                { "S>D"         , &s_to_d              , false },
                //{ "SIGN"        , &sign                , false },
                //{ "SM/REM"      , &s_m_slash_rem       , false },
                //{ "SOURCE"      , &source              , false },
                { "SPACE"       , &space               , false },
                { "SPACES"      , &spaces              , false },
                { "STATE"       , &state               , false },
                { "SWAP"        , &swap                , false },
                //module_entry{ "THEN"       , &then                 , false },
                { "TYPE"       , &type                 , false },
                { "U."         , &u_dot                , false },
                { "U<"         , &u_less_than          , false },
                //{ "UM*"        , &u_m_star             , false },
                //{ "UM/MOD"     , &u_m_slash_mod        , false },
                //{ "UNLOOP"     , &unloop               , false },
                //{ "UNTIL"      , &until                , false },
                { "VARIABLE"   , &variable             , false },
                //{ "WHILE"      , &while_               , false },
                { "WORD"       , &word                 , false },
                { "XOR"        , &xor_                 , false },
                //{ "["          , &left_bracket         , true  },
                //{ "[']"        , &bracket_tick         , false },
                //{ "[CHAR]"     , &bracket-char         , false },
                //{ "]"          , &right_bracket        , false },
            };

            return entries_;
        }

        int_t* base_addr_;
        std::span<const module_entry> entries;

    public:

        explicit core_words_t(dictionary& dict) noexcept
            : base_addr_{ dict.append<int_t>(10) }
            , entries{ this->create_entries(this->base_addr_) }
        { }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return this->base_addr_;
        }

        [[nodiscard]] int_t* base_addr() const noexcept
        {
            return this->base_addr_;
        }

        [[nodiscard]] auto begin() const noexcept
        {
            return this->entries.begin();
        }

        [[nodiscard]] auto end() const noexcept
        {
            return this->entries.end();
        }

    };

}
