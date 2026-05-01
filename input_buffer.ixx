module;

#include <algorithm>
#include <cstring>
#include <istream>
#include <ranges>

export module plapper:input_buffer;

import :memory_buffer;
import :terminal;
import :error;

namespace rng = std::ranges;

namespace plapper
{
    export class input_buffer
    {

    public:

        using buffer_type = memory_buffer<char>;

        static constexpr char delimiters[]{ ' ', '\t', '\n' };

        input_buffer(const input_buffer&) = delete;
        input_buffer(input_buffer&& that) noexcept = default;
        input_buffer& operator=(const input_buffer&) = delete;
        input_buffer& operator=(input_buffer&& that) noexcept = default;

        [[nodiscard]] static expected<input_buffer> of_capacity(const std::size_t capacity) noexcept
        {
            auto buffer = buffer_type::of_capacity(capacity);

            if (!buffer)
                return unexpected(buffer.error());

            return input_buffer{ std::move(*buffer) };
        }

        void clear()
        {
            this->buffer.clear();
            this->position = rng::begin(this->buffer);
        }

        error_status refill_from(const std::span<const char*> words) noexcept
        {
            this->clear();

            if (words.empty())
                return error_status::success;

            const auto to_string_view = [](const auto word){ return std::string_view{ word }; };
            const auto to_size = [](const auto word){ return rng::size(word); };
            const auto word_views = words | rng::views::transform(to_string_view);
            const auto word_sizes = word_views | rng::views::transform(to_size);
            const auto number_of_separators = rng::size(words) - 1;
            const auto total_size = rng::fold_left(word_sizes, number_of_separators, std::plus{});

            if (const auto stat = this->buffer.resize(total_size); stat != error_status::success)
                return stat;

            auto combined_words = word_views | rng::views::join_with(' ');

            rng::copy(combined_words, this->buffer.data());

            return error_status::success;
        }

        error_status refill_from(const int argc, const char** argv) noexcept
        {
            return this->refill_from({ argv + 1, static_cast<std::size_t>(argc - 1) });
        }

        error_status refill_from(terminal& term) noexcept
        {
            this->clear();
            return term.read_line(this->buffer);
        }

        [[nodiscard]] std::string_view read_until(const char delimiter) noexcept
        {
            if (this->position == rng::end(this->buffer))
                return {};

            const auto is_delimiter = [](const char c){ return rng::contains(delimiters, c); };

            const auto word_begin = rng::find_if_not(
                rng::subrange{ this->position, rng::end(this->buffer) }, is_delimiter
            );

            if (word_begin == rng::end(this->buffer))
                return {};

            const auto word_end = rng::find(
                rng::subrange{ word_begin, rng::end(this->buffer) },
                delimiter
            );

            this->position = word_end;

            return std::string_view{ word_begin, word_end };
        }

        [[nodiscard]] std::string_view read_word() noexcept
        {
            if (this->position == rng::end(this->buffer))
                return {};

            const auto is_delimiter = [](const char c){ return rng::contains(delimiters, c); };

            const auto word_begin = rng::find_if_not(
                rng::subrange{ this->position, rng::end(this->buffer) }, is_delimiter
            );

            if (word_begin == rng::end(this->buffer))
                return {};

            const auto word_end = rng::find_if(
                rng::subrange{ word_begin, rng::end(this->buffer) },
                is_delimiter
            );

            this->position = word_end;

            return std::string_view{ word_begin, word_end };
        }

    private:

        explicit input_buffer(buffer_type&& buffer) noexcept
            : buffer{ std::move(buffer) }
            , position{ rng::begin(this->buffer) }
        { }

        buffer_type buffer;
        buffer_type::iterator position;

    };

}