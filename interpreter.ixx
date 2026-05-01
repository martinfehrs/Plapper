module;

#include <print>
#include <iostream>

export module plapper:interpreter;

import :environment;
import :core_words;
import :core_extension_words;
import :programming_tools_words;
import :settings;
import :dictionary;
import :error;

namespace plapper
{

    export class interpreter : environment
    {

    public:

        static expected<interpreter> from_settings(const settings& settings) noexcept
        {
            auto dict = plapper::dictionary::of_capacity(settings.dict_capacity);

            if (!dict)
                return unexpected(dict.error());

            auto dstack = data_stack::of_capacity(settings.dstack_capacity);

            if (!dstack)
                return unexpected(dstack.error());

            auto rstack = return_stack::of_capacity(settings.rstack_capacity);

            if (!rstack)
                return unexpected(rstack.error());

            auto tib = input_buffer::of_capacity(settings.tib_capacity);

            if (!tib)
                return unexpected(tib.error());

            auto core_words_data = load_core_words(*dict);

            if (!core_words_data)
                return unexpected(core_words_data.error());

            if ((settings.additional_modules & modules::core_extension) == modules::core_extension)
            {
                if (auto stat = load_core_extension_words(*dict, *core_words_data); stat != error_status::success)
                    return unexpected(stat);
            }

            if ((settings.additional_modules & modules::programming_tools) == modules::programming_tools)
            {
                if (auto stat = load_programming_tool_words(*dict); stat != error_status::success)
                    return unexpected(stat);
            }

            return interpreter{ std::move(*dict), std::move(*dstack), std::move(*rstack), std::move(*tib) };
        }

        void interpret_instructions() noexcept
        {
            do
            {
                this->instruction_ptr++;

                if (const auto stat = (***this->instruction_ptr)(*this, *this->instruction_ptr + 1);
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
            struct literal_t : execution_token
            {
                [[nodiscard]] error_status operator()(environment& env, void*) noexcept override
                {
                    env.instruction_ptr++;

                    if (const auto status = env.dstack.push(reinterpret_cast<int_t>(*env.instruction_ptr));
                        status != error_status::success)
                        return status;

                    return error_status::success;
                }
            };

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

                    if (auto entry = this->dict.find(word); entry != nullptr)
                    {
                        if (this->state == yes  && entry->execution_time != execution_time_t::immediate)
                        {
                            if (auto ret = this->dict.append(&entry))
                                this->handle_error(ret.error());
                        }
                        else if (
                            const auto stat = (*entry)(*this, entry->data());
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
                        static literal_t literal;
                        static auto literal_ptr = &literal;

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

    };

}