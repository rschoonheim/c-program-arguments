# Argument Validation Guide

## Overview

The program arguments library supports custom validation functions that are executed once when an argument is first accessed. Validation results are cached to avoid repeated validation calls.

## How It Works

1. **Define a validator function** that checks if the argument value is valid
2. **Attach the validator** to an argument using `arg_parser_set_validator()`
3. **Validation runs automatically** when you first access the argument value
4. **Results are cached** - subsequent accesses use the cached validation result
5. **Invalid values return defaults** with an error message printed to stderr

## Validator Function Signature

```c
typedef bool (*arg_validator_fn)(arg_value_t value, arg_type_t type, 
                                  char *error_msg, size_t error_msg_size);
```

**Parameters:**
- `value`: The argument value to validate
- `type`: The type of the argument (ARG_TYPE_INT, ARG_TYPE_FLOAT, etc.)
- `error_msg`: Buffer to write error message (can be NULL)
- `error_msg_size`: Size of the error message buffer

**Returns:**
- `true` if the value is valid
- `false` if the value is invalid

## Example Validators

### Integer Range Validation

```c
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
```

### Float Range Validation

```c
bool validate_threshold(arg_value_t value, arg_type_t type, 
                       char *error_msg, size_t error_msg_size) {
    if (type != ARG_TYPE_FLOAT) {
        return false;
    }
    
    if (value.floating < 0.0f || value.floating > 1.0f) {
        snprintf(error_msg, error_msg_size,
                "Threshold must be between 0.0 and 1.0, got %.2f", 
                value.floating);
        return false;
    }
    return true;
}
```

### String Pattern Validation

```c
bool validate_output_file(arg_value_t value, arg_type_t type, 
                         char *error_msg, size_t error_msg_size) {
    if (type != ARG_TYPE_STRING || !value.string) {
        return false;
    }
    
    size_t len = strlen(value.string);
    if (len < 4 || strcmp(value.string + len - 4, ".txt") != 0) {
        snprintf(error_msg, error_msg_size,
                "Output file must have .txt extension, got '%s'", 
                value.string);
        return false;
    }
    return true;
}
```

## Usage Example

```c
int main(int argc, char *argv[]) {
    arg_parser_t *parser = arg_parser_create();
    
    // Add arguments
    arg_parser_add_int(parser, "-n", "--count", "Count", false, 10);
    arg_parser_add_float(parser, "-t", "--threshold", "Threshold", false, 0.5f);
    arg_parser_add_string(parser, "-o", "--output", "Output file", false, "out.txt");
    
    // Set validators
    arg_parser_set_validator(parser, "--count", validate_count);
    arg_parser_set_validator(parser, "--threshold", validate_threshold);
    arg_parser_set_validator(parser, "--output", validate_output_file);
    
    // Parse arguments
    if (arg_parser_parse(parser, argc, argv) != 0) {
        arg_parser_destroy(parser);
        return 1;
    }
    
    // Access values - validation runs here (only once)
    int count = arg_parser_get_int(parser, "--count");
    float threshold = arg_parser_get_float(parser, "--threshold");
    const char *output = arg_parser_get_string(parser, "--output");
    
    // Subsequent accesses use cached validation results
    int count_again = arg_parser_get_int(parser, "--count");  // No validation
    
    arg_parser_destroy(parser);
    return 0;
}
```

## Validation Behavior

### Valid Arguments
- Validation passes silently
- The actual value is returned
- Subsequent accesses are fast (cached)

### Invalid Arguments
- Error message is printed to stderr
- Default value is returned instead
- Validation result is cached

### No Validator Set
- Arguments without validators always pass validation
- Values are returned as parsed

## Example Output

### Valid Input
```bash
$ ./example -i input.txt -n 50 -t 0.75 -o result.txt
=== Program Arguments Example ===
Verbose mode: disabled
Input file: input.txt
Output file: result.txt
Count: 50
Threshold: 0.75
```

### Invalid Input
```bash
$ ./example -i input.txt -n 150 -t 1.5 -o file.csv
Validation error for --count: Count must be between 1 and 100, got 150
Validation error for --threshold: Threshold must be between 0.0 and 1.0, got 1.50
Validation error for --output: Output file must have .txt extension, got 'file.csv'
=== Program Arguments Example ===
Verbose mode: disabled
Input file: input.txt
Output file: output.txt (default)
Count: 10 (default)
Threshold: 0.50 (default)
```

## Best Practices

1. **Always check the type** in your validator function
2. **Provide clear error messages** that explain what went wrong and what's expected
3. **Return false on type mismatch** to prevent runtime errors
4. **Validate null strings** before accessing string values
5. **Use snprintf** to safely format error messages
6. **Set validators after adding arguments** for better code organization

## Performance

- Validation runs **once per argument** on first access
- Results are **cached** for subsequent accesses
- No performance penalty for accessing validated arguments multiple times
- Validators can be as simple or complex as needed

