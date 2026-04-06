module;

#include <cstdio>
#include <expected>
#include <format>
#include <cstring>
#include <csignal>
#include <mutex>
#include <termios.h>

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

    static auto terminal_instance_counter = 0U;
    static std::optional<char> last_written_char_ = std::nullopt;
    std::mutex terminal_mutex;
    static auto previous_signal_handler = SIG_DFL;

    [[nodiscard]] static bool is_invisible(const std::string_view text) noexcept
    {
        return text.starts_with("\033[") && text.ends_with('m');
    }

    export class terminal
    {

    public:

        terminal() noexcept
        {
            std::lock_guard guard{ terminal_mutex };

            if (terminal_instance_counter == 0)
            {
                previous_signal_handler = std::signal(
                    SIGINT,
                    [](const int signal)
                    {
                        std::fputs(reset.data(), stdout);
                        previous_signal_handler(signal);
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

        ~terminal() noexcept
        {
            std::lock_guard guard{ terminal_mutex };

            terminal_instance_counter--;

            if (terminal_instance_counter == 0)
                std::signal(SIGINT, previous_signal_handler);
        }

        [[nodiscard]] static std::expected<char_t, error_status> read_char() noexcept
        {
            std::lock_guard guard{ terminal_mutex };

            termios saved_state{};

            tcgetattr(STDIN_FILENO, &saved_state);

            termios new_state = saved_state;

            new_state.c_lflag &= ~(ECHO | ICANON);
            new_state.c_cc[VMIN] = 1;

            tcsetattr(STDIN_FILENO, TCSANOW, &new_state);

            const auto c = std::getchar();

            if (c == EOF)
                return std::unexpected(error_status::input_error);

            tcsetattr(STDIN_FILENO, TCSANOW, &saved_state);

            return static_cast<char_t>(c);
        }

        static error_status read_line(memory_buffer<char_t>& buffer) noexcept
        {
            std::lock_guard guard{ terminal_mutex };

            auto c = std::getc(stdin);

            last_written_char_ = '\n';

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

            if (is_invisible(text))
                return;

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