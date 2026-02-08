# Program Arguments Library

![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)
![License](https://img.shields.io/badge/license-MIT-blue)

A type-safe command-line argument parser for C programs.

## Features

- **Type-safe argument parsing** with dedicated functions for each type
- Support for multiple argument types:
  - Boolean flags (`--verbose`, `-v`)
  - String values (`--output file.txt`)
  - Integer values (`--count 10`)
  - Float values (`--threshold 0.5`)
- Short and long argument names (`-v` / `--verbose`)
- Required and optional arguments
- Default values
- **Argument validation with custom validators**
- Validation caching (runs once per argument access)
- Positional arguments
- Automatic help message generation
- Memory-safe with proper cleanup

## Usage

### Basic Example

```c
#include "program_arguments.h"

int main(int argc, char *argv[]) {
    // Create parser
    arg_parser_t *parser = arg_parser_create();
    
    // Add arguments using type-specific functions
    arg_parser_add_flag(parser, "-v", "--verbose", 
                       "Enable verbose output", false);
    
    arg_parser_add_string(parser, "-o", "--output",
                         "Output file", false, "output.txt");
    
    arg_parser_add_int(parser, "-n", "--count",
                      "Number of iterations", false, 10);
    
    // Parse command line
    if (arg_parser_parse(parser, argc, argv) != 0) {
        arg_parser_destroy(parser);
        return 1;
    }
    
    // Get values using type-safe getters
    bool verbose = arg_parser_get_flag(parser, "--verbose");
    const char *output = arg_parser_get_string(parser, "--output");
    int count = arg_parser_get_int(parser, "--count");
    
    printf("Verbose: %s\n", verbose ? "yes" : "no");
    printf("Output: %s\n", output);
    printf("Count: %d\n", count);
    
    // Clean up
    arg_parser_destroy(parser);
    return 0;
}
```

### API Reference

#### Creating Parser

```c
arg_parser_t *arg_parser_create(void);
```

#### Adding Arguments

```c
// Add boolean flag
int arg_parser_add_flag(arg_parser_t *parser, 
                       const char *short_name,
                       const char *long_name,
                       const char *description,
                       bool default_value);

// Add string argument
int arg_parser_add_string(arg_parser_t *parser,
                         const char *short_name,
                         const char *long_name,
                         const char *description,
                         bool required,
                         const char *default_value);

// Add integer argument
int arg_parser_add_int(arg_parser_t *parser,
                      const char *short_name,
                      const char *long_name,
                      const char *description,
                      bool required,
                      int default_value);

// Add float argument
int arg_parser_add_float(arg_parser_t *parser,
                        const char *short_name,
                        const char *long_name,
                        const char *description,
                        bool required,
                        float default_value);
```

#### Setting Validators

```c
// Validator function type
typedef bool (*arg_validator_fn)(arg_value_t value, arg_type_t type, 
                                  char *error_msg, size_t error_msg_size);

// Set a validator for an argument
int arg_parser_set_validator(arg_parser_t *parser,
                             const char *long_name,
                             arg_validator_fn validator);
```

**Example validator:**

```c
// Validate that a number is in range
bool validate_count(arg_value_t value, arg_type_t type, 
                   char *error_msg, size_t error_msg_size) {
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

// Set the validator
arg_parser_set_validator(parser, "--count", validate_count);
```

**Validation behavior:**
- Validators run once when the argument is first accessed
- Results are cached for subsequent accesses
- Invalid arguments return default values
- Error messages are printed to stderr

#### Parsing

```c
int arg_parser_parse(arg_parser_t *parser, int argc, char **argv);
```

#### Getting Values

```c
// Type-safe getters
bool arg_parser_get_flag(arg_parser_t *parser, const char *long_name);
const char *arg_parser_get_string(arg_parser_t *parser, const char *long_name);
int arg_parser_get_int(arg_parser_t *parser, const char *long_name);
float arg_parser_get_float(arg_parser_t *parser, const char *long_name);

// Check if argument was set
bool arg_parser_is_set(arg_parser_t *parser, const char *long_name);

// Get positional arguments
char **arg_parser_get_positional(arg_parser_t *parser, size_t *count);
```

#### Help and Cleanup

```c
void arg_parser_print_help(arg_parser_t *parser, const char *program_name);
void arg_parser_destroy(arg_parser_t *parser);
```

## Building

```bash
cmake -B cmake-build-debug -S .
cmake --build cmake-build-debug
```

## Running Example

```bash
# Show help
./cmake-build-debug/example --help

# Run with required argument
./cmake-build-debug/example -i input.txt

# Run with all options
./cmake-build-debug/example -v --input data.txt -o result.txt -n 20 -t 0.75

# With positional arguments
./cmake-build-debug/example --input file.txt extra1 extra2
```

## License

MIT

