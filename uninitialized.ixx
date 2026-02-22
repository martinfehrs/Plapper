export module plapper:uninitialized;

namespace plapper
{
    struct uninitialized_t
    {
        explicit uninitialized_t() noexcept = default;
    };

    inline constexpr uninitialized_t uninitialized;
}