/**
 * @file argument_parser.h
 * @brief Command-line argument parsing and validation utilities
 * @author Komondor Team
 * @date 2025
 */

#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <string>
#include <stdexcept>
#include <cstdlib>
#include <cerrno>
#include <climits>

/**
 * @brief Exception class for argument parsing errors
 */
class ArgumentParseException : public std::runtime_error {
public:
    explicit ArgumentParseException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Utility class for parsing and validating command-line arguments
 */
class ArgumentParser {
public:
    /**
     * @brief Safely parse an integer from a string
     * @param str String to parse
     * @param arg_name Name of the argument (for error messages)
     * @return Parsed integer value
     * @throws ArgumentParseException if parsing fails
     */
    static int ParseInt(const std::string& str, const std::string& arg_name = "argument");
    
    /**
     * @brief Safely parse a double from a string
     * @param str String to parse
     * @param arg_name Name of the argument (for error messages)
     * @return Parsed double value
     * @throws ArgumentParseException if parsing fails
     */
    static double ParseDouble(const std::string& str, const std::string& arg_name = "argument");
    
    /**
     * @brief Parse an integer with validation (range check)
     * @param str String to parse
     * @param min Minimum allowed value
     * @param max Maximum allowed value
     * @param arg_name Name of the argument (for error messages)
     * @return Parsed integer value
     * @throws ArgumentParseException if parsing fails or value is out of range
     */
    static int ParseIntRange(const std::string& str, int min, int max, const std::string& arg_name = "argument");
    
    /**
     * @brief Parse a double with validation (range check)
     * @param str String to parse
     * @param min Minimum allowed value
     * @param max Maximum allowed value
     * @param arg_name Name of the argument (for error messages)
     * @return Parsed double value
     * @throws ArgumentParseException if parsing fails or value is out of range
     */
    static double ParseDoubleRange(const std::string& str, double min, double max, const std::string& arg_name = "argument");
    
    /**
     * @brief Validate that a file exists and is readable
     * @param file_path Path to the file
     * @param arg_name Name of the argument (for error messages)
     * @throws ArgumentParseException if file doesn't exist or is not readable
     */
    static void ValidateFileExists(const std::string& file_path, const std::string& arg_name = "file");
    
    /**
     * @brief Validate that argc matches expected count
     * @param argc Actual argument count
     * @param expected Expected argument count
     * @param usage_message Usage message to display on error
     * @throws ArgumentParseException if count doesn't match
     */
    static void ValidateArgCount(int argc, int expected, const std::string& usage_message = "");
    
private:
    ArgumentParser() = delete;  // Static class, no instances
    ArgumentParser(const ArgumentParser&) = delete;
    ArgumentParser& operator=(const ArgumentParser&) = delete;
};

// Inline implementations
inline int ArgumentParser::ParseInt(const std::string& str, const std::string& arg_name) {
    if (str.empty()) {
        throw ArgumentParseException("Empty string for argument: " + arg_name);
    }
    
    char* endptr = nullptr;
    errno = 0;
    long value = std::strtol(str.c_str(), &endptr, 10);
    
    if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
        throw ArgumentParseException("Integer overflow for argument: " + arg_name);
    }
    if (endptr == str.c_str() || *endptr != '\0') {
        throw ArgumentParseException("Invalid integer format for argument: " + arg_name + " (value: " + str + ")");
    }
    
    return static_cast<int>(value);
}

inline double ArgumentParser::ParseDouble(const std::string& str, const std::string& arg_name) {
    if (str.empty()) {
        throw ArgumentParseException("Empty string for argument: " + arg_name);
    }
    
    char* endptr = nullptr;
    errno = 0;
    double value = std::strtod(str.c_str(), &endptr);
    
    if (errno == ERANGE) {
        throw ArgumentParseException("Double overflow/underflow for argument: " + arg_name);
    }
    if (endptr == str.c_str() || *endptr != '\0') {
        throw ArgumentParseException("Invalid double format for argument: " + arg_name + " (value: " + str + ")");
    }
    
    return value;
}

inline int ArgumentParser::ParseIntRange(const std::string& str, int min, int max, const std::string& arg_name) {
    int value = ParseInt(str, arg_name);
    if (value < min || value > max) {
        throw ArgumentParseException("Argument " + arg_name + " out of range [" + 
                                    std::to_string(min) + ", " + std::to_string(max) + "]: " + str);
    }
    return value;
}

inline double ArgumentParser::ParseDoubleRange(const std::string& str, double min, double max, const std::string& arg_name) {
    double value = ParseDouble(str, arg_name);
    if (value < min || value > max) {
        throw ArgumentParseException("Argument " + arg_name + " out of range [" + 
                                    std::to_string(min) + ", " + std::to_string(max) + "]: " + std::to_string(value));
    }
    return value;
}

inline void ArgumentParser::ValidateFileExists(const std::string& file_path, const std::string& arg_name) {
    if (file_path.empty()) {
        throw ArgumentParseException("Empty file path for argument: " + arg_name);
    }
    
    FILE* test_file = fopen(file_path.c_str(), "r");
    if (test_file == nullptr) {
        throw ArgumentParseException("File not found or not readable: " + file_path + " (argument: " + arg_name + ")");
    }
    fclose(test_file);
}

inline void ArgumentParser::ValidateArgCount(int argc, int expected, const std::string& usage_message) {
    if (argc != expected) {
        std::string error = "Invalid number of arguments. Expected " + std::to_string(expected) + 
                           ", got " + std::to_string(argc);
        if (!usage_message.empty()) {
            error += "\n" + usage_message;
        }
        throw ArgumentParseException(error);
    }
}

#endif // ARGUMENT_PARSER_H

