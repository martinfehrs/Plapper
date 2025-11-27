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
            auto dict = plapper::dictionary::of_size(65536);

            if (!dict)
                return std::unexpected(dict.error());

            auto dstack = plapper::data_stack::of_size(64);

            if (!dstack)
                return std::unexpected(dstack.error());

            auto rstack = plapper::return_stack::of_size(64);

            if (!rstack)
                return std::unexpected(rstack.error());

            core_words_t core_words{ *dict };

            if (auto stat = dict->load(core_words); stat != error_status::success)
                return std::unexpected(stat);

            if ((settings.additional_modules & modules::core_extension) == modules::core_extension)
            {
                core_extension_words_t core_extension_words{ core_words };

                if (auto stat = dict->load(core_extension_words); stat != error_status::success)
                    return std::unexpected(stat);
            }

            if ((settings.additional_modules & modules::programming_tools) == modules::programming_tools)
            {
                if (auto stat = dict->load(programming_tool_words); stat != error_status::success)
                    return std::unexpected(stat);
            }

            return interpreter{ std::move(*dict), std::move(*dstack), std::move(*rstack) };
        }

        int run(const int argc, const char** argv)
        {
            this->tib.reset(argc, argv);

            int pos = 0;

            while (this->running)
            {
                if (this->instruction_ptr)
                {
                    this->instruction_ptr++;

                    if (const auto stat = (**this->instruction_ptr)(*this, *this->instruction_ptr + 1);
                        stat != plapper::error_status::success)
                    {
                        std::println("{}", error_message_for(stat));
                        this->dstack.clear();
                        this->instruction_ptr = nullptr;
                    }
                }
                else
                {
                    std::string_view word = this->tib.read_word();

                    while (word.empty())
                    {
                        if (!this->tob.empty())
                            this->tob.write("\n");

                        std::print("> ");
                        this->tib.reset(std::cin);
                        word = this->tib.read_word();
                        pos = 0;
                    }

                    if (auto entry = this->dict.find(word); entry != nullptr)
                    {
                        auto stat = entry->xt(*this, entry->data());

                        if (stat != error_status::success)
                        {
                            std::println("{}", error_message_for(stat));
                            this->dstack.clear();
                        }

                        if (this->state == yes)
                        {
                            if (!this->dict.append(&entry->xt))
                            {
                                std::println("{}", error_message_for(error_status::out_of_memory));
                                this->dstack.clear();
                            }
                        }
                        else if (
                            auto stat = entry->xt(*this, entry->data()); stat != error_status::success
                        )
                        {
                            std::println("{}", error_message_for(stat));
                            this->dstack.clear();
                        }
                    }
                    else
                    {
                        int_t value;

                        if(
                            const auto[ptr, ec] = std::from_chars(
                                word.begin(), word.end(), value, 10
                            );
                            ec != std::errc{} || ptr != word.end())
                        {
                            this->tib.clear();

                            if (!this->tob.empty())
                                this->tob.write("\n");

                            std::println("unknown word");
                            this->dstack.clear();
                        }
                        else if (this->state == yes)
                        {
                            static execution_token_t literal_ptr = literal_;

                            if (!this->dict.append(&literal_ptr))
                            {
                                std::println("{}", error_message_for(error_status::out_of_memory));
                                this->dstack.clear();
                            }

                            if (!this->dict.append(value))
                            {
                                std::println("{}", error_message_for(error_status::out_of_memory));
                                this->dstack.clear();
                            }
                        }
                        else if (const auto stat = this->dstack.push(value); stat != error_status::success)
                        {
                            std::println("{}", error_message_for(stat));
                            this->dstack.clear();
                        }
                    }

                    pos++;
                }
            }

            return 0;
        }

    private:

        explicit interpreter(dictionary dict, data_stack dstack, return_stack rstack) noexcept
            : environment{ std::move(dict), std::move(dstack), std::move(rstack) }
        { }

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