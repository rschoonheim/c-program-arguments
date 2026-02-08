#ifndef PROGRAM_ARGUMENTS_PROGRAM_ARGUMENTS_H
#define PROGRAM_ARGUMENTS_PROGRAM_ARGUMENTS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Argument types supported by the parser
 */
typedef enum {
    ARG_TYPE_FLAG,      // Boolean flag (--verbose, -v)
    ARG_TYPE_STRING,    // String value (--output file.txt)
    ARG_TYPE_INT,       // Integer value (--count 10)
    ARG_TYPE_FLOAT      // Float value (--threshold 0.5)
} arg_type_t;

/**
 * Union to hold different argument value types
 */
typedef union {
    bool flag;
    char *string;
    int integer;
    float floating;
} arg_value_t;

/**
 * Argument definition structure
 */
typedef struct arg_def {
    const char *short_name;  // Short form (e.g., "-v")
    const char *long_name;   // Long form (e.g., "--verbose")
    const char *description; // Help text
    arg_type_t type;         // Argument type
    bool required;           // Whether argument is required
    arg_value_t default_value; // Default value if not provided
} arg_def_t;

/**
 * Parsed argument result
 */
typedef struct {
    const arg_def_t *definition;
    arg_value_t value;
    bool is_set;
} arg_result_t;

/**
 * Argument parser context
 */
typedef struct arg_parser {
    arg_def_t *definitions;
    size_t definition_count;
    size_t definition_capacity;
    arg_result_t *results;
    char **positional_args;
    size_t positional_count;
    size_t positional_capacity;
} arg_parser_t;

/**
 * Initialize argument parser
 * Returns NULL on failure
 */
arg_parser_t *arg_parser_create(void);

/**
 * Add a flag argument (boolean)
 * @param parser The parser instance
 * @param short_name Short form (e.g., "-v"), can be NULL
 * @param long_name Long form (e.g., "--verbose"), required
 * @param description Help text for this argument
 * @param default_value Default value if not provided
 * @return 0 on success, -1 on error
 */
int arg_parser_add_flag(arg_parser_t *parser, const char *short_name,
                        const char *long_name, const char *description,
                        bool default_value);

/**
 * Add a string argument
 * @param parser The parser instance
 * @param short_name Short form (e.g., "-o"), can be NULL
 * @param long_name Long form (e.g., "--output"), required
 * @param description Help text for this argument
 * @param required Whether this argument must be provided
 * @param default_value Default value if not provided, can be NULL
 * @return 0 on success, -1 on error
 */
int arg_parser_add_string(arg_parser_t *parser, const char *short_name,
                          const char *long_name, const char *description,
                          bool required, const char *default_value);

/**
 * Add an integer argument
 * @param parser The parser instance
 * @param short_name Short form (e.g., "-n"), can be NULL
 * @param long_name Long form (e.g., "--count"), required
 * @param description Help text for this argument
 * @param required Whether this argument must be provided
 * @param default_value Default value if not provided
 * @return 0 on success, -1 on error
 */
int arg_parser_add_int(arg_parser_t *parser, const char *short_name,
                       const char *long_name, const char *description,
                       bool required, int default_value);

/**
 * Add a float argument
 * @param parser The parser instance
 * @param short_name Short form (e.g., "-t"), can be NULL
 * @param long_name Long form (e.g., "--threshold"), required
 * @param description Help text for this argument
 * @param required Whether this argument must be provided
 * @param default_value Default value if not provided
 * @return 0 on success, -1 on error
 */
int arg_parser_add_float(arg_parser_t *parser, const char *short_name,
                         const char *long_name, const char *description,
                         bool required, float default_value);

/**
 * Parse command line arguments
 * @param parser The parser instance
 * @param argc Argument count from main()
 * @param argv Argument vector from main()
 * @return 0 on success, -1 on error
 */
int arg_parser_parse(arg_parser_t *parser, int argc, char **argv);

/**
 * Get parsed argument result by long name
 * @param parser The parser instance
 * @param long_name The long name of the argument (e.g., "--verbose")
 * @return Pointer to result, or NULL if not found
 */
arg_result_t *arg_parser_get(arg_parser_t *parser, const char *long_name);

/**
 * Get flag value (convenience function)
 * @param parser The parser instance
 * @param long_name The long name of the argument
 * @return The flag value, or false if not found
 */
bool arg_parser_get_flag(arg_parser_t *parser, const char *long_name);

/**
 * Get string value (convenience function)
 * @param parser The parser instance
 * @param long_name The long name of the argument
 * @return The string value, or NULL if not found
 */
const char *arg_parser_get_string(arg_parser_t *parser, const char *long_name);

/**
 * Get integer value (convenience function)
 * @param parser The parser instance
 * @param long_name The long name of the argument
 * @return The integer value, or 0 if not found
 */
int arg_parser_get_int(arg_parser_t *parser, const char *long_name);

/**
 * Get float value (convenience function)
 * @param parser The parser instance
 * @param long_name The long name of the argument
 * @return The float value, or 0.0f if not found
 */
float arg_parser_get_float(arg_parser_t *parser, const char *long_name);

/**
 * Check if an argument was explicitly set by the user
 * @param parser The parser instance
 * @param long_name The long name of the argument
 * @return true if set, false otherwise
 */
bool arg_parser_is_set(arg_parser_t *parser, const char *long_name);

/**
 * Get positional arguments (non-option arguments)
 * @param parser The parser instance
 * @param count Output parameter for the number of positional arguments
 * @return Array of positional argument strings, or NULL if none
 */
char **arg_parser_get_positional(arg_parser_t *parser, size_t *count);

/**
 * Print usage/help message to stdout
 * @param parser The parser instance
 * @param program_name Name of the program (typically argv[0])
 */
void arg_parser_print_help(arg_parser_t *parser, const char *program_name);

/**
 * Free parser resources
 * @param parser The parser instance to destroy
 */
void arg_parser_destroy(arg_parser_t *parser);

#endif //PROGRAM_ARGUMENTS_PROGRAM_ARGUMENTS_H