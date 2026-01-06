#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <cstdint>
#include <utility>

import plapper;

using namespace plapper;

static environment test_env()
{
    auto dict = dictionary::of_size(65536);
    auto dstack = data_stack::of_size(64);
    auto rstack = return_stack::of_size(64);

    REQUIRE(dict);
    REQUIRE(dstack);
    REQUIRE(rstack);

    environment env{ std::move(*dict), std::move(*dstack), std::move(*rstack) };

    core_words_t core_words{ env.dict };
    REQUIRE(env.dict.load(core_words) == error_status::success);

    return env;
}

TEST_CASE("plus", "[plus][words]" )
{
    SECTION("successful execution")
    {
        auto[param1, param2, result] = GENERATE(table<int_t, int_t, int_t>({
            {  0      ,  5,          5 },
            {  5      ,  0,          5 },
            {  0      , -5,         -5 },
            { -5      ,  0,         -5 },
            {  1      ,  2,          3 },
            {  1      , -2,         -1 },
            { -1      ,  2,          1 },
            { -1      , -2,         -3 },
            { -1      ,  1,          0 },
            { mid_uint,  1, mid_uint+1 }
        }));

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(plus(env, nullptr) == error_status::success);
        REQUIRE(env.dstack == data_stack::containing(result));
    }

    SECTION("insufficient arguments")
    {
        static constexpr int_t param1{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1) == error_status::success);
        REQUIRE(plus(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack == data_stack::containing(param1));
    }
}

TEST_CASE("minus", "[minus][words]" )
{
    SECTION("successful execution")
    {
        auto[param1, param2, result] = GENERATE(table<int_t, int_t, int_t>({
            {  0        ,  5,       -5 },
            {  5        ,  0,        5 },
            {  0        , -5,        5 },
            { -5        ,  0,       -5 },
            {  1        ,  2,       -1 },
            {  1        , -2,        3 },
            { -1        ,  2,       -3 },
            { -1        , -2,        1 },
            {  0        ,  1,       -1 },
            { mid_uint+1,  1, mid_uint }
        }));

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(minus(env, nullptr) == error_status::success);
        REQUIRE(env.dstack == data_stack::containing(result));
    }

    SECTION("insufficient arguments")
    {
        static constexpr int_t param1{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1) == error_status::success);
        REQUIRE(minus(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack == data_stack::containing(param1));
    }
}

TEST_CASE("times", "[times][words]" )
{
    SECTION("successful execution")
    {
        auto[param1, param2, result] = GENERATE(table<int_t, int_t, int_t>({
            {  0                            ,  0,          0 },
            {  0                            ,  1,          0 },
            {  1                            ,  0,          0 },
            {  1                            ,  2,          2 },
            {  2                            ,  1,          2 },
            {  3                            ,  3,          9 },
            { -3                            ,  3,         -9 },
            {  3                            , -3,         -9 },
            { -3                            , -3,          9 },
            { (mid_uint+1) >> 1             ,  2, mid_uint+1 },
            { (mid_uint+1) >> 2             ,  4, mid_uint+1 },
            { (mid_uint+1) >> 1 | mid_uint+1,  2, mid_uint+1 }
        }));

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(times(env, nullptr) == error_status::success);
        REQUIRE(env.dstack == data_stack::containing(result));
    }

    SECTION("insufficient arguments")
    {
        static constexpr int_t param1{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1) == error_status::success);
        REQUIRE(times(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack == data_stack::containing(param1));
    }
}

TEST_CASE("divide", "[divide][words]" )
{
    SECTION("successful execution")
    {
        environment env = test_env();

        REQUIRE(env.dstack.push(2, 2) == error_status::success);
        REQUIRE(divide(env, nullptr) == error_status::success);
        REQUIRE(env.dstack == data_stack::containing(2 / 2));
    }

    SECTION("insufficient arguments")
    {
        static constexpr int_t param1{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1) == error_status::success);
        REQUIRE(divide(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack == data_stack::containing(param1));
    }

    SECTION("division by zero")
    {
        static constexpr int_t param1{};
        static constexpr int_t param2{ 0 };

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(divide(env, nullptr) == error_status::division_by_zero);
        REQUIRE(env.dstack == data_stack::containing(param1, param2));
    }
}

TEST_CASE("mod", "[mod][words]")
{
    SECTION("successful execution")
    {
        static constexpr int_t param1{ 2 };
        static constexpr int_t param2{ 2 };

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(mod(env, nullptr) == error_status::success);
        REQUIRE(env.dstack == data_stack::containing(param1 % param2));
    }

    SECTION("insufficient arguments")
    {
        static constexpr int_t param1{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1) == error_status::success);
        REQUIRE(mod(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack == data_stack::containing(param1));
    }

    SECTION("division by zero")
    {
        static constexpr int_t param1{};
        static constexpr int_t param2{ 0 };

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(mod(env, nullptr) == error_status::division_by_zero);
        REQUIRE(env.dstack == data_stack::containing(param1, param2));
    }
}

TEST_CASE("swap", "[swap][words]")
{
    SECTION("successful execution")
    {
        static constexpr int_t param1{ 1 };
        static constexpr int_t param2{ 2 };

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(swap(env, nullptr) == error_status::success);
        REQUIRE(env.dstack == data_stack::containing(param2, param1));
    }

    SECTION("insufficient arguments")
    {
        static constexpr int_t param1{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1) == error_status::success);
        REQUIRE(swap(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack == data_stack::containing(param1));
    }
}

TEST_CASE("drop", "[drop][words]")
{
    SECTION("successful execution")
    {
        static constexpr int_t param1{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1) == error_status::success);
        REQUIRE(drop(env, nullptr) == error_status::success);
        REQUIRE(env.dstack.empty());
    }

    SECTION("insufficient arguments")
    {
        environment env{ .istack{} };

        REQUIRE(drop(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack.empty());
    }
}

TEST_CASE("equals", "[equals][words]")
{
    SECTION("successful exection")
    {
        const int_t param1 = GENERATE(-1, 0, 1);
        const int_t param2 = GENERATE(-1, 0, 1);

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(equals(env, nullptr) == error_status::success);
        REQUIRE(env.dstack == data_stack::containing(param1 == param2 ? yes : no));
    }

    SECTION("insufficient arguments")
    {
        static constexpr int_t param1{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1) == error_status::success);
        REQUIRE(equals(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack == data_stack::containing(param1));
    }
}

TEST_CASE("less-than", "[less-than][words]")
{
    SECTION("successful execution")
    {
        const int_t param1 = GENERATE(-1, 0, 1);
        const int_t param2 = GENERATE(-1, 0, 1);

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(less_than(env, nullptr) == error_status::success);
        REQUIRE(env.dstack == data_stack::containing(param1 < param2 ? yes : no));
    }

    SECTION("insufficient arguments")
    {
        static constexpr int_t param1{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1) == error_status::success);
        REQUIRE(less_than(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack == data_stack::containing(param1));
    }
}

TEST_CASE("rotate", "[rotate][words]")
{
    SECTION("successful execution")
    {
        static constexpr int_t param1{ 1 };
        static constexpr int_t param2{ 2 };
        static constexpr int_t param3{ 3 };

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2, param3) == error_status::success);
        REQUIRE(rote(env, nullptr) == error_status::success);
        REQUIRE(env.dstack == data_stack::containing(param2, param3, param1));
    }

    SECTION("insufficient arguments")
    {
        static constexpr int_t param1{};
        static constexpr int_t param2{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(rote(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack == data_stack::containing(param1, param2));
    }
}

TEST_CASE("over", "[over][words]")
{
    SECTION("successful execution")
    {
        static constexpr int_t param1{ 1 };
        static constexpr int_t param2{ 2 };

        environment env = test_env();

        REQUIRE(env.dstack.push(param1, param2) == error_status::success);
        REQUIRE(over(env, nullptr) == error_status::success);
        REQUIRE(env.dstack == data_stack::containing(param1, param2, param1));
    }

    SECTION("insufficient arguments")
    {
        static constexpr int_t param1{};

        environment env = test_env();

        REQUIRE(env.dstack.push(param1) == error_status::success);
        REQUIRE(over(env, nullptr) == error_status::stack_underflow);
        REQUIRE(env.dstack == data_stack::containing(param1));
    }
}