// C_MAKE_COMPILER_FLAGS = "-std=c99 -Wall -Wextra -pedantic"
#define C_MAKE_IMPLEMENTATION
#include "c_make.h"

C_MAKE_INFO(commands_info, configs_info)
{
    add_info(configs_info, StringLiteral("unity_plugin_api_include_path"), StringLiteral("Path to the PluginAPI folder inside the unity editor. [default: <unset>]"));

    add_default_info(commands_info, configs_info);
}

C_MAKE_ENTRY(command, argument_count, arguments)
{
    (void) argument_count;
    (void) arguments;

    if (strings_are_equal(command, COMMAND_SETUP))
    {
        ConfigValue unity_plugin_api_include_path = config_get("unity_plugin_api_include_path");

        if (!unity_plugin_api_include_path.is_valid ||
            (string_trim(CString(unity_plugin_api_include_path.val)).count == 0))
        {
            c_make_log(LogLevelWarning, "you might want to set unity_plugin_api_include_path to have the IUnity* headers available.\n");
        }
    }
    else if (strings_are_equal(command, COMMAND_BUILD))
    {
        String readme;

        if (!read_entire_file(c_string_path_concat(get_source_path(), "README.md"), &readme))
        {
            set_failed(true);
            return;
        }

        size_t code_index = 0;

        const String code_start = StringLiteral("```c");
        const String code_end = StringLiteral("```");

        size_t index = string_find(readme, code_start);

        while (index < readme.count)
        {
            string_advance(&readme, index);

            bool is_cpp = string_starts_with(readme, StringLiteral("```cpp"));

            string_split_left(&readme, '\n');

            index = string_find(readme, code_end);

            String code = readme;
            code.count = index;

            const char *target_compiler = NULL;
            const char *source_path = NULL;

            Command command = { 0 };

            if (is_cpp)
            {
                target_compiler = get_target_cpp_compiler();
                source_path = c_string_path_concat(get_build_path(), c_string_formated("readme%zu.cpp", code_index));
            }
            else
            {
                target_compiler = get_target_c_compiler();
                source_path = c_string_path_concat(get_build_path(), c_string_formated("readme%zu.c", code_index));
            }

            write_entire_file(source_path, code);

            command_append(&command, target_compiler);
            command_append_command_line(&command, is_cpp ? get_target_cpp_flags() : get_target_c_flags());
            command_append_default_compiler_flags(&command, get_build_type());
            command_append(&command, c_string_concat("-I", get_source_path()));

            ConfigValue unity_plugin_api_include_path = config_get("unity_plugin_api_include_path");

            if (unity_plugin_api_include_path.is_valid &&
                (string_trim(CString(unity_plugin_api_include_path.val)).count > 0))
            {
                command_append(&command, c_string_concat("-I", unity_plugin_api_include_path.val));
            }

            command_append_output_executable(&command, c_string_path_concat(get_build_path(), c_string_formated("readme%zu", code_index)), get_target_platform());
            command_append(&command, source_path);
            command_append_default_linker_flags(&command, get_target_architecture());

            c_make_log(LogLevelInfo, "compile 'readme%zu'\n", code_index);
            command_run_and_reset_and_wait(&command);

            code_index += 1;
            index = string_find(readme, code_start);
        }
    }
    else
    {
        handle_default_commands(command, argument_count, arguments);
    }
}
