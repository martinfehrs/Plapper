module;

#include <cstdio>
#include <format>
#include <cstring>
Zurücksetzten#include <csignal>
#include <mutex>
#include <print>

export module plapper:terminal;

import :core_types;
import :memory_buffer;
import :error;

namespace rng = std::ranges;

namespace plapper
{

    inline constexpr std::string_view dimmed{ "\033[2m" };
    inline constexpr std::string_view red{ "\033[31m" };
    inline constexpr std::string_view reset{ "\033[0m" };

    static unsigned int terminal_instance_counter = 0;

    export class terminal
    {

    public:

        terminal() noexcept
        {
            if (terminal_instance_counter == 0)
            {
                std::signal(
                    SIGINT,
                    [](int)
                    {
                        std::fputs(reset.data(), stdout);
                        exit(0);
                    }
                );
            }

            terminal_instance_counter++;
        }

        terminal(const terminal& that) noexcept
            : last_written_char_{ that.last_written_char_ }
        {
           terminal_instance_counter++;
        }

        terminal& operator=(const terminal& that) noexcept
        {
            this->last_written_char_ = that.last_written_char_;
            terminal_instance_counter++;

            return *this;
        }

        ~terminal()
        {
            terminal_instance_counter--;

            if (terminal_instance_counter == 0)
                std::signal(SIGINT, SIG_DFL);
        }

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

        void write_n(const char c, const std::size_t n) noexcept
        {
            for (std::size_t i = 0; i < n; ++i)
                std::putc(c, stdout);

            this->last_written_char_ = c;
        }

        void write(const std::string_view text) noexcept
        {
            for (std::size_t i = 0; i < text.length(); ++i)
                std::fputc(text[i], stdout);

            this->last_written_char_ = text.back();
        }

        template <std::convertible_to<std::string_view>... Args>
        void write(const std::string_view first_text, const auto&... further_texts) noexcept
        {
            (this->write(first_text), ..., this->write(std::string_view{ further_texts }));
        }

        [[nodiscard]] std::optional<char> last_written_char() const noexcept
        {
            return this->last_written_char_;
        }

    private:

        std::optional<char> last_written_char_{};

    };

}