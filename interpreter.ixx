module;

#include <expected>
#include <print>
#include <iostream>

export module plapper:interpreter;

import :environment;
import :core_words;
import :core_extension_words;
import :programming_tools_words;
import :settings;
import :dictionary;

namespace plapper
{

    export class interpreter : environment
    {

    public:

        static std::expected<interpreter, error_status> from_settings(const settings& settings) noexcept
        {
            auto dict = plapper::dictionary::of_capacity(65536);

            if (!dict)
                return std::unexpected(dict.error());

            auto dstack = data_stack::of_capacity(64);

            if (!dstack)
                return std::unexpected(dstack.error());

            auto rstack = return_stack::of_capacity(64);

            if (!rstack)
                return std::unexpected(rstack.error());

            auto tib = input_buffer::of_capacity(64);

            if (!tib)
                return std::unexpected(tib.error());

            auto core_words = core_words_t::with_dict(*dict);

            if (!core_words)
                return std::unexpected(core_words.error());

            if (auto stat = dict->load(*core_words); stat != error_status::success)
                return std::unexpected(stat);

            if ((settings.additional_modules & modules::core_extension) == modules::core_extension)
            {
                core_extension_words_t core_extension_words{ *core_words };

                if (auto stat = dict->load(core_extension_words); stat != error_status::success)
                    return std::unexpected(stat);
            }

            if ((settings.additional_modules & modules::programming_tools) == modules::programming_tools)
            {
                if (auto stat = dict->load(programming_tool_words); stat != error_status::success)
                    return std::unexpected(stat);
            }

            return interpreter{ std::move(*dict), std::move(*dstack), std::move(*rstack), std::move(*tib) };
        }

        void interpret_instructions() noexcept
        {
            do
            {
                this->instruction_ptr++;

                if (const auto stat = (**this->instruction_ptr)(*this, *this->instruction_ptr + 1);
                    stat != error_status::success)
                {
                    this->handle_error(stat);
                    this->instruction_ptr = nullptr;
                }
            }
            while (this->running && this->instruction_ptr);
        }

        [[nodiscard]] std::string_view ask_for_input() noexcept
        {
            auto word = this->tib.read_word();

            while (word.empty())
            {
                if (auto last_char = this->term.last_written_char(); last_char && *last_char != '\n')
                    this->term.write('\n');

                this->term.write(reset, dimmed, "> ");

                if (const auto stat = this->tib.refill_from(this->term); stat != error_status::success)
                {
                    this->handle_error(stat);
                    continue;
                }

                this->term.write(reset);
                word = this->tib.read_word();
            }

            return word;
        }

        [[nodiscard]] static std::optional<int_t> word_as_number(const std::string_view word) noexcept
        {
            int_t value;

            const auto[ptr, ec] = std::from_chars(
                word.begin(), word.end(), value, 10
            );

            if (ec != std::errc{} || ptr != word.end())
                return std::nullopt;

            return value;
        }

        int run(const int argc, const char** argv) noexcept
        {
            if (const auto stat = this->tib.refill_from(argc, argv); stat != error_status::success)
                this->handle_error(stat);

            while (this->running)
            {
                if (this->instruction_ptr)
                {
                    this->interpret_instructions();
                }
                else
                {
                    const auto word = this->ask_for_input();

                    if (auto const entry = this->dict.find(word); entry != nullptr)
                    {
                        if (this->state == yes  && !entry->immediate)
                        {
                            if (auto ret = this->dict.append(&entry->xt))
                                this->handle_error(ret.error());
                        }
                        else if (
                            const auto stat = entry->xt(*this, entry->data());
                            stat != error_status::success
                        )
                        {
                            this->handle_error(stat);
                        }
                    }
                    else if (const auto number = this->word_as_number(word); !number)
                    {
                        this->handle_error(error_status::unknown_word);
                    }
                    else if (this->state == yes)
                    {
                        static execution_token_t literal_ptr = literal_;

                        if (auto ret = this->dict.append(&literal_ptr); !ret)
                        {
                            this->handle_error(ret.error());
                            continue;
                        }

                        if (auto ret = this->dict.append(*number); !ret)
                            this->handle_error(ret.error());
                    }
                    else if (const auto stat = this->dstack.push(*number); stat != error_status::success)
                    {
                        this->handle_error(stat);
                    }
                }
            }

            return 0;
        }

    private:

        explicit interpreter(dictionary dict, data_stack dstack, return_stack rstack, input_buffer tib) noexcept
            : environment{ std::move(dict), std::move(dstack), std::move(rstack), std::move(tib) }
        { }

        void handle_error(const error_status status) noexcept
        {
            this->term.write(red, error_message_for(status));
            this->dstack.clear();
            this->tib.clear();
        }

        static error_status literal_(environment& env, void*) noexcept
        {
            env.instruction_ptr++;

            if (const auto status = env.dstack.push(reinterpret_cast<int_t>(*env.instruction_ptr));
                status != error_status::success)
                return status;

            return error_status::success;
        }

    };

}