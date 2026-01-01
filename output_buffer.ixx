module;

#include <cstdio>
#include <string>
#include <string_view>

export module plapper:output_buffer;

namespace plapper
{

    export class output_buffer
    {

    public:

        void flush() noexcept
        {
            std::puts(this->data.c_str());
            this->data.clear();
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return this->data.empty();
        }

        void write(char c)
        {
            this->data.push_back(c);

            if (c == '\n')
                this->flush();
        }

        void write(char c, std::size_t n)
        {
            this->data.reserve(n);

            for (std::size_t i = 0; i < n; ++i) {
                this->data.push_back(c);

                if (c == '\n')
                    this->flush();
            }
        }

        void write(std::string_view text)
        {
            write_impl(text);
        }

        void write(std::u8string_view text)
        {
            write_impl(text);
        }

    private:

        template <typename Char>
        void write_impl(const std::basic_string_view<Char> text)
        {
            const auto last_newline_pos = text.rfind('\n');

            if (last_newline_pos == std::string::npos)
            {
                std::ranges::copy(text, std::back_inserter(this->data));
                return;
            }

            std::ranges::copy(text.substr(0, last_newline_pos), std::back_inserter(this->data));
            this->flush();
            std::ranges::copy(text.substr(last_newline_pos + 1), std::back_inserter(this->data));
        }

        std::string data;

    };

}