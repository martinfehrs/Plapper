module;

#include <algorithm>
#include <istream>

export module plapper:input_buffer;

namespace rng = std::ranges;

namespace plapper
{
    
    export class input_buffer
    {

    public:

        static constexpr char delimiters[]{ ' ', '\t', '\n' };

        void clear() {
            this->buffer.clear();
            this->position = 0;
        }

        void reset(std::span<const char*> words)
        {
            this->clear();

            if (words.empty())
                return;

            this->buffer += words.front();

            for (std::size_t i = 1; i < words.size(); ++i)
            {
                this->buffer.push_back(' ');
                this->buffer += words[i];
            }
        }

        void reset(const int argc, const char** argv)
        {
            this->reset({ argv + 1, static_cast<std::size_t>(argc - 1) });
        }

        void reset(std::istream& is)
        {
            this->clear();
            std::getline(is, this->buffer);
        }

        [[nodiscard]] std::string_view read_until(const char delimiter) noexcept
        {
            if (this->position == rng::size(this->buffer))
                return {};

            while (rng::contains(delimiters, this->buffer[this->position]))
                this->position++;

            const auto word_offset = this->position;

            if (
                const auto delim_pos = this->buffer.find(delimiter, this->position);
                delim_pos != std::string_view::npos)
            {
                this->position = delim_pos;
            }
            else
            {
                this->position = rng::size(this->buffer);
            }

            const auto word_length = this->position - word_offset;

            return std::string_view{ this->buffer.data() + word_offset, word_length };
        }

        [[nodiscard]] std::string_view read_word() noexcept
        {
            if (this->position == rng::size(this->buffer))
                return {};

            while (rng::contains(delimiters, this->buffer[this->position]))
                this->position++;

            const auto word_offset = this->position;

            if (
                const auto delim_pos = this->buffer.find_first_of(this->delimiters, this->position);
                delim_pos != std::string_view::npos)
            {
                this->position = delim_pos;
            }
            else
            {
                this->position = rng::size(this->buffer);
            }

            const auto word_length = this->position - word_offset;

            return std::string_view{ this->buffer.data() + word_offset, word_length };
        }

    private:

        std::string buffer;
        std::size_t position;

    };

}