module;

#include <cstdio>
#include <format>
#include <cstring>
#include <csignal>
#include <mutex>

export module plapper:terminal;

import :core_types;
import :memory_buffer;
import :error;

namespace rng = std::ranges;

namespace plapper
{

    constexpr std::string_view dimmed{ "\033[2m" };
    constexpr std::string_view red{ "\033[31m" };
    constexpr std::string_view reset{ "\033[0m" };

    static unsigned int terminal_instance_counter = 0;
    static std::optional<char> last_written_char_ = std::nullopt;
    static std::mutex terminal_mutex;

    export class terminal
    {

    public:

        terminal() noexcept
        {
            std::lock_guard guard{ terminal_mutex };

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

        terminal(const terminal&) noexcept
        {
            std::lock_guard guard{ terminal_mutex };

            terminal_instance_counter++;
        }

        terminal& operator=(const terminal&) noexcept
        {
            std::lock_guard guard(terminal_mutex);

            terminal_instance_counter++;

            return *this;
        }

        ~terminal()
        {
            std::lock_guard guard{ terminal_mutex };

            terminal_instance_counter--;

            if (terminal_instance_counter == 0)
                std::signal(SIGINT, SIG_DFL);
        }

        static error_status read_line(memory_buffer<char_t>& buffer) noexcept
        {
            std::lock_guard guard{ terminal_mutex };

            last_written_char_ = std::nullopt;

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

        static void write(const char c) noexcept
        {
            std::lock_guard guard{ terminal_mutex };

            std::putc(c, stdout);

            last_written_char_ = c;
        }

        static void write_n(const char c, const std::size_t n) noexcept
        {
            std::lock_guard guard{ terminal_mutex };

            for (std::size_t i = 0; i < n; ++i)
                std::putc(c, stdout);

            last_written_char_ = c;
        }

        static void write(const std::string_view text) noexcept
        {
            std::lock_guard guard{ terminal_mutex };

            for (std::size_t i = 0; i < text.length(); ++i)
                std::fputc(text[i], stdout);

            last_written_char_ = text.back();
        }

        template <std::convertible_to<std::string_view>... Args>
        static void write(const std::string_view first_text, const Args&... further_texts) noexcept
        {
            std::lock_guard guard{ terminal_mutex };

            (write(first_text), ..., write(std::string_view{ further_texts }));
        }

        [[nodiscard]] static std::optional<char> last_written_char() noexcept
        {
            return last_written_char_;
        }

    };

}