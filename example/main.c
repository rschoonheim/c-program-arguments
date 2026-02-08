#include "program_arguments.h"
#include <stdio.h>
#include <string.h>

// Validation function for count (must be between 1 and 100)
bool validate_count(arg_value_t value, arg_type_t type, char *error_msg, size_t error_msg_size) {
    if (type != ARG_TYPE_INT) {
        return false;
    }

    if (value.integer < 1 || value.integer > 100) {
        snprintf(error_msg, error_msg_size,
                "Count must be between 1 and 100, got %d", value.integer);
        return false;
    }
    return true;
}

// Validation function for threshold (must be between 0.0 and 1.0)
bool validate_threshold(arg_value_t value, arg_type_t type, char *error_msg, size_t error_msg_size) {
    if (type != ARG_TYPE_FLOAT) {
        return false;
    }

    if (value.floating < 0.0f || value.floating > 1.0f) {
        snprintf(error_msg, error_msg_size,
                "Threshold must be between 0.0 and 1.0, got %.2f", value.floating);
        return false;
    }
    return true;
}

// Validation function for output file (must end with .txt)
bool validate_output_file(arg_value_t value, arg_type_t type, char *error_msg, size_t error_msg_size) {
    if (type != ARG_TYPE_STRING || !value.string) {
        return false;
    }

    size_t len = strlen(value.string);
    if (len < 4 || strcmp(value.string + len - 4, ".txt") != 0) {
        snprintf(error_msg, error_msg_size,
                "Output file must have .txt extension, got '%s'", value.string);
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    // Create argument parser
    arg_parser_t *parser = arg_parser_create();
    if (!parser) {
        fprintf(stderr, "Failed to create argument parser\n");
        return 1;
    }

    // Add various types of arguments
    arg_parser_add_flag(parser, "-v", "--verbose",
                       "Enable verbose output", false);

    arg_parser_add_flag(parser, "-h", "--help",
                       "Display this help message", false);

    arg_parser_add_string(parser, "-o", "--output",
                         "Output file path", false, "output.txt");

    arg_parser_add_string(parser, "-i", "--input",
                         "Input file path (required)", true, NULL);

    arg_parser_add_int(parser, "-n", "--count",
                      "Number of iterations", false, 10);

    arg_parser_add_float(parser, "-t", "--threshold",
                        "Threshold value", false, 0.5f);

    // Set up validators for arguments
    arg_parser_set_validator(parser, "--count", validate_count);
    arg_parser_set_validator(parser, "--threshold", validate_threshold);
    arg_parser_set_validator(parser, "--output", validate_output_file);

    // Check for help flag first (before parsing errors)
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            arg_parser_print_help(parser, argv[0]);
            arg_parser_destroy(parser);
            return 0;
        }
    }

    // Parse arguments
    if (arg_parser_parse(parser, argc, argv) != 0) {
        fprintf(stderr, "\nUse --help for usage information\n");
        arg_parser_destroy(parser);
        return 1;
    }

    // Check if help was requested
    if (arg_parser_get_flag(parser, "--help")) {
        arg_parser_print_help(parser, argv[0]);
        arg_parser_destroy(parser);
        return 0;
    }

    // Get and display parsed values
    bool verbose = arg_parser_get_flag(parser, "--verbose");
    const char *input = arg_parser_get_string(parser, "--input");
    const char *output = arg_parser_get_string(parser, "--output");
    int count = arg_parser_get_int(parser, "--count");
    float threshold = arg_parser_get_float(parser, "--threshold");

    printf("=== Program Arguments Example ===\n");
    printf("Verbose mode: %s\n", verbose ? "enabled" : "disabled");
    printf("Input file: %s\n", input);
    printf("Output file: %s%s\n", output,
           arg_parser_is_set(parser, "--output") ? "" : " (default)");
    printf("Count: %d%s\n", count,
           arg_parser_is_set(parser, "--count") ? "" : " (default)");
    printf("Threshold: %.2f%s\n", threshold,
           arg_parser_is_set(parser, "--threshold") ? "" : " (default)");

    // Display positional arguments if any
    size_t positional_count;
    char **positional_args = arg_parser_get_positional(parser, &positional_count);
    if (positional_count > 0) {
        printf("\nPositional arguments:\n");
        for (size_t i = 0; i < positional_count; i++) {
            printf("  [%zu] %s\n", i, positional_args[i]);
        }
    }

    if (verbose) {
        printf("\n=== Verbose Details ===\n");
        printf("Processing %d iterations with threshold %.2f\n", count, threshold);
        printf("Reading from: %s\n", input);
        printf("Writing to: %s\n", output);
    }

    // Clean up
    arg_parser_destroy(parser);
    return 0;
}