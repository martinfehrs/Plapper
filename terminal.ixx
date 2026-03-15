module;

#include <cstdio>
#include <format>
#include <cstring>

export module plapper:terminal;

import :core_types;
import :memory_buffer;
import :error;

namespace rng = std::ranges;

namespace plapper
{

    export class terminal
    {

    public:

        error_status read_line(memory_buffer<char_t>& buffer) noexcept
        {
            this->last_written_char_ = std::nullopt;

            auto c = std::getc(stdin);

            while (c != '\n' && c != EOF && buffer.resize(buffer.size() + 1) == error_status::success)
            {
                buffer[buffer.size() - 1] = static_cast<char>(c);
                c = std::getc(stdin);
            }

            if (c != '\n' && c != EOF)
            {
                while (c != '\n' && c != EOF)
                    c = std::getc(stdin);

                return error_status::out_of_memory;
            }

            return error_status::success;
        }

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

        [[nodiscard]] std::optional<char> last_written_char() const noexcept
        {
            return this->last_written_char_;
        }

    private:

        std::optional<char> last_written_char_{};

    };

}