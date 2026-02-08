#include "../includes/program_arguments.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 8

/**
 * Initialize argument parser
 */
arg_parser_t *arg_parser_create(void) {
    arg_parser_t *parser = (arg_parser_t *)malloc(sizeof(arg_parser_t));
    if (!parser) {
        return NULL;
    }

    parser->definitions = (arg_def_t *)malloc(INITIAL_CAPACITY * sizeof(arg_def_t));
    if (!parser->definitions) {
        free(parser);
        return NULL;
    }

    parser->definition_count = 0;
    parser->definition_capacity = INITIAL_CAPACITY;
    parser->results = NULL;
    parser->positional_args = NULL;
    parser->positional_count = 0;
    parser->positional_capacity = 0;

    return parser;
}

/**
 * Helper function to resize definitions array
 */
static int resize_definitions(arg_parser_t *parser) {
    size_t new_capacity = parser->definition_capacity * 2;
    arg_def_t *new_defs = (arg_def_t *)realloc(parser->definitions,
                                                new_capacity * sizeof(arg_def_t));
    if (!new_defs) {
        return -1;
    }
    parser->definitions = new_defs;
    parser->definition_capacity = new_capacity;
    return 0;
}

/**
 * Helper function to add an argument definition
 */
static int add_argument(arg_parser_t *parser, const char *short_name,
                       const char *long_name, const char *description,
                       arg_type_t type, bool required, arg_value_t default_value) {
    if (!parser || !long_name) {
        return -1;
    }

    if (parser->definition_count >= parser->definition_capacity) {
        if (resize_definitions(parser) != 0) {
            return -1;
        }
    }

    arg_def_t *def = &parser->definitions[parser->definition_count];
    def->short_name = short_name;
    def->long_name = long_name;
    def->description = description;
    def->type = type;
    def->required = required;
    def->default_value = default_value;
    def->validator = NULL;

    parser->definition_count++;
    return 0;
}

/**
 * Add a flag argument (boolean)
 */
int arg_parser_add_flag(arg_parser_t *parser, const char *short_name,
                        const char *long_name, const char *description,
                        bool default_value) {
    arg_value_t value;
    value.flag = default_value;
    return add_argument(parser, short_name, long_name, description,
                       ARG_TYPE_FLAG, false, value);
}

/**
 * Add a string argument
 */
int arg_parser_add_string(arg_parser_t *parser, const char *short_name,
                          const char *long_name, const char *description,
                          bool required, const char *default_value) {
    arg_value_t value;
    value.string = default_value ? strdup(default_value) : NULL;
    return add_argument(parser, short_name, long_name, description,
                       ARG_TYPE_STRING, required, value);
}

/**
 * Add an integer argument
 */
int arg_parser_add_int(arg_parser_t *parser, const char *short_name,
                       const char *long_name, const char *description,
                       bool required, int default_value) {
    arg_value_t value;
    value.integer = default_value;
    return add_argument(parser, short_name, long_name, description,
                       ARG_TYPE_INT, required, value);
}

/**
 * Add a float argument
 */
int arg_parser_add_float(arg_parser_t *parser, const char *short_name,
                         const char *long_name, const char *description,
                         bool required, float default_value) {
    arg_value_t value;
    value.floating = default_value;
    return add_argument(parser, short_name, long_name, description,
                       ARG_TYPE_FLOAT, required, value);
}

/**
 * Set validator for an argument
 */
int arg_parser_set_validator(arg_parser_t *parser, const char *long_name,
                             arg_validator_fn validator) {
    if (!parser || !long_name) {
        return -1;
    }

    for (size_t i = 0; i < parser->definition_count; i++) {
        if (parser->definitions[i].long_name &&
            strcmp(parser->definitions[i].long_name, long_name) == 0) {
            parser->definitions[i].validator = validator;
            return 0;
        }
    }
    return -1;
}

/**
 * Helper function to find argument definition by name
 */
static arg_def_t *find_definition(arg_parser_t *parser, const char *name) {
    for (size_t i = 0; i < parser->definition_count; i++) {
        if ((parser->definitions[i].long_name && strcmp(parser->definitions[i].long_name, name) == 0) ||
            (parser->definitions[i].short_name && strcmp(parser->definitions[i].short_name, name) == 0)) {
            return &parser->definitions[i];
        }
    }
    return NULL;
}

/**
 * Helper function to validate a result (runs once)
 */
static bool validate_result(arg_result_t *result) {
    if (!result) {
        return false;
    }

    // If validation already attempted, return cached result
    if (result->validation_attempted) {
        return result->is_valid;
    }

    result->validation_attempted = true;

    // If no validator is set, consider it valid
    if (!result->definition->validator) {
        result->is_valid = true;
        return true;
    }

    // Run the validator
    result->is_valid = result->definition->validator(
        result->value,
        result->definition->type,
        result->validation_error,
        sizeof(result->validation_error)
    );

    // If validation failed, print error
    if (!result->is_valid && result->validation_error[0] != '\0') {
        fprintf(stderr, "Validation error for %s: %s\n",
                result->definition->long_name, result->validation_error);
    }

    return result->is_valid;
}

/**
 * Helper function to add positional argument
 */
static int add_positional_arg(arg_parser_t *parser, const char *arg) {
    if (parser->positional_count >= parser->positional_capacity) {
        size_t new_capacity = parser->positional_capacity == 0 ?
                              INITIAL_CAPACITY : parser->positional_capacity * 2;
        char **new_args = (char **)realloc(parser->positional_args,
                                           new_capacity * sizeof(char *));
        if (!new_args) {
            return -1;
        }
        parser->positional_args = new_args;
        parser->positional_capacity = new_capacity;
    }

    parser->positional_args[parser->positional_count] = strdup(arg);
    if (!parser->positional_args[parser->positional_count]) {
        return -1;
    }
    parser->positional_count++;
    return 0;
}

/**
 * Parse command line arguments
 */
int arg_parser_parse(arg_parser_t *parser, int argc, char **argv) {
    if (!parser) {
        return -1;
    }

    // Allocate results array
    parser->results = (arg_result_t *)calloc(parser->definition_count, sizeof(arg_result_t));
    if (!parser->results) {
        return -1;
    }

    // Initialize results with default values
    for (size_t i = 0; i < parser->definition_count; i++) {
        parser->results[i].definition = &parser->definitions[i];
        parser->results[i].value = parser->definitions[i].default_value;
        parser->results[i].is_set = false;
        parser->results[i].validation_attempted = false;
        parser->results[i].is_valid = false;
        parser->results[i].validation_error[0] = '\0';
    }

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        // Check if it's an option
        if (arg[0] == '-') {
            const arg_def_t *def = find_definition(parser, arg);
            if (!def) {
                fprintf(stderr, "Unknown argument: %s\n", arg);
                return -1;
            }

            // Find corresponding result
            arg_result_t *result = NULL;
            for (size_t j = 0; j < parser->definition_count; j++) {
                if (parser->results[j].definition == def) {
                    result = &parser->results[j];
                    break;
                }
            }

            if (!result) {
                return -1;
            }

            // Parse value based on type
            if (def->type == ARG_TYPE_FLAG) {
                result->value.flag = true;
                result->is_set = true;
            } else {
                // Need next argument for value
                if (i + 1 >= argc) {
                    fprintf(stderr, "Missing value for argument: %s\n", arg);
                    return -1;
                }
                i++;
                const char *value = argv[i];

                switch (def->type) {
                    case ARG_TYPE_STRING:
                        result->value.string = strdup(value);
                        if (!result->value.string) {
                            return -1;
                        }
                        break;
                    case ARG_TYPE_INT:
                        result->value.integer = atoi(value);
                        break;
                    case ARG_TYPE_FLOAT:
                        result->value.floating = (float)atof(value);
                        break;
                    default:
                        break;
                }
                result->is_set = true;
            }
        } else {
            // Positional argument
            if (add_positional_arg(parser, arg) != 0) {
                return -1;
            }
        }
    }

    // Check for required arguments
    for (size_t i = 0; i < parser->definition_count; i++) {
        if (parser->definitions[i].required && !parser->results[i].is_set) {
            fprintf(stderr, "Required argument missing: %s\n",
                    parser->definitions[i].long_name);
            return -1;
        }
    }

    return 0;
}

/**
 * Get parsed argument result by long name
 */
arg_result_t *arg_parser_get(arg_parser_t *parser, const char *long_name) {
    if (!parser || !long_name) {
        return NULL;
    }

    for (size_t i = 0; i < parser->definition_count; i++) {
        if (parser->definitions[i].long_name &&
            strcmp(parser->definitions[i].long_name, long_name) == 0) {
            arg_result_t *result = &parser->results[i];

            // Run validation if not already done
            if (!validate_result(result)) {
                return NULL;
            }

            return result;
        }
    }
    return NULL;
}

/**
 * Get flag value (convenience function)
 */
bool arg_parser_get_flag(arg_parser_t *parser, const char *long_name) {
    arg_result_t *result = arg_parser_get(parser, long_name);
    if (!result || result->definition->type != ARG_TYPE_FLAG) {
        return false;
    }
    return result->value.flag;
}

/**
 * Get string value (convenience function)
 */
const char *arg_parser_get_string(arg_parser_t *parser, const char *long_name) {
    arg_result_t *result = arg_parser_get(parser, long_name);
    if (!result || result->definition->type != ARG_TYPE_STRING) {
        return NULL;
    }
    return result->value.string;
}

/**
 * Get integer value (convenience function)
 */
int arg_parser_get_int(arg_parser_t *parser, const char *long_name) {
    arg_result_t *result = arg_parser_get(parser, long_name);
    if (!result || result->definition->type != ARG_TYPE_INT) {
        // Return default value on validation failure
        arg_def_t *def = find_definition(parser, long_name);
        if (def && def->type == ARG_TYPE_INT) {
            return def->default_value.integer;
        }
        return 0;
    }
    return result->value.integer;
}

/**
 * Get float value (convenience function)
 */
float arg_parser_get_float(arg_parser_t *parser, const char *long_name) {
    arg_result_t *result = arg_parser_get(parser, long_name);
    if (!result || result->definition->type != ARG_TYPE_FLOAT) {
        // Return default value on validation failure
        arg_def_t *def = find_definition(parser, long_name);
        if (def && def->type == ARG_TYPE_FLOAT) {
            return def->default_value.floating;
        }
        return 0.0f;
    }
    return result->value.floating;
}

/**
 * Check if an argument was explicitly set by the user
 */
bool arg_parser_is_set(arg_parser_t *parser, const char *long_name) {
    const arg_result_t *result = arg_parser_get(parser, long_name);
    if (!result) {
        return false;
    }
    return result->is_set;
}

/**
 * Get positional arguments (non-option arguments)
 */
char **arg_parser_get_positional(const arg_parser_t *parser, size_t *count) {
    if (!parser || !count) {
        return NULL;
    }
    *count = parser->positional_count;
    return parser->positional_args;
}

/**
 * Print usage/help message to stdout
 */
void arg_parser_print_help(arg_parser_t *parser, const char *program_name) {
    if (!parser) {
        return;
    }

    printf("Usage: %s [OPTIONS]...\n\n", program_name ? program_name : "program");
    printf("Options:\n");

    for (size_t i = 0; i < parser->definition_count; i++) {
        const arg_def_t *def = &parser->definitions[i];

        printf("  ");
        if (def->short_name) {
            printf("%s", def->short_name);
            if (def->long_name) {
                printf(", ");
            }
        }
        if (def->long_name) {
            printf("%s", def->long_name);
        }

        // Print value placeholder for non-flag arguments
        if (def->type != ARG_TYPE_FLAG) {
            switch (def->type) {
                case ARG_TYPE_STRING:
                    printf(" <string>");
                    break;
                case ARG_TYPE_INT:
                    printf(" <int>");
                    break;
                case ARG_TYPE_FLOAT:
                    printf(" <float>");
                    break;
                default:
                    break;
            }
        }

        printf("\n");

        if (def->description) {
            printf("      %s", def->description);
            if (def->required) {
                printf(" (required)");
            }
            printf("\n");
        }
    }
}

/**
 * Free parser resources
 */
void arg_parser_destroy(arg_parser_t *parser) {
    if (!parser) {
        return;
    }

    // Free string default values
    for (size_t i = 0; i < parser->definition_count; i++) {
        if (parser->definitions[i].type == ARG_TYPE_STRING &&
            parser->definitions[i].default_value.string) {
            free(parser->definitions[i].default_value.string);
        }
    }

    // Free parsed string values
    if (parser->results) {
        for (size_t i = 0; i < parser->definition_count; i++) {
            if (parser->results[i].definition->type == ARG_TYPE_STRING &&
                parser->results[i].is_set &&
                parser->results[i].value.string) {
                free(parser->results[i].value.string);
            }
        }
        free(parser->results);
    }

    // Free positional arguments
    if (parser->positional_args) {
        for (size_t i = 0; i < parser->positional_count; i++) {
            free(parser->positional_args[i]);
        }
        free(parser->positional_args);
    }

    free(parser->definitions);
    free(parser);
}
