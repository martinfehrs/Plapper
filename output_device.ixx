module;

#include <cstdio>
#include <format>
#include <cstring>

export module plapper:output_device;

import :core_types;

namespace rng = std::ranges;

namespace plapper
{

    export class output_device
    {

    public:

        void write(const char c) noexcept
        {
            std::putc(c, stdout);

            this->last_written_char_ = c;
        }

        void write(const char c, const std::size_t n) noexcept
        {
            for (std::size_t i = 0; i < n; ++i)
                std::putc(c, stdout);

            this->last_written_char_ = c;
        }

        void write(const char* text, const std::size_t length) noexcept
        {
            for (std::size_t i = 0; i < length; ++i)
                std::fputc(text[i], stdout);

            this->last_written_char_ = text[length - 1];
        }

        void write(const char* text) noexcept
        {
            this->write(text, std::strlen(text));
        }

        [[nodiscard]] char last_written_char() const noexcept
        {
            return this->last_written_char_;
        }

    private:

        char last_written_char_{};

    };

}