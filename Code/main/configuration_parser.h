/**
 * @file configuration_parser.h
 * @brief Configuration file parsing utilities
 * @author Komondor Team
 * @date 2025
 */

#ifndef CONFIGURATION_PARSER_H
#define CONFIGURATION_PARSER_H

#include <string>
#include <map>
#include <fstream>
#include <stdexcept>
#include <cstdio>
#include "argument_parser.h"

/**
 * @brief Exception class for configuration parsing errors
 */
class ConfigurationParseException : public std::runtime_error {
public:
    explicit ConfigurationParseException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Utility class for parsing configuration files
 * 
 * Supports key-value pair configuration files with comments (lines starting with #)
 */
class ConfigurationParser {
public:
    /**
     * @brief Parse a configuration file into a map of key-value pairs
     * @param file_path Path to the configuration file
     * @param delimiter Delimiter between key and value (default: "=")
     * @return Map of configuration parameters (key -> value)
     * @throws ConfigurationParseException if file cannot be opened or parsed
     */
    static std::map<std::string, std::string> ParseFile(const std::string& file_path, 
                                                          const std::string& delimiter = "=");
    
    /**
     * @brief Get an integer value from configuration map
     * @param config Configuration map
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @return Parsed integer value or default
     * @throws ConfigurationParseException if key exists but cannot be parsed
     */
    static int GetInt(const std::map<std::string, std::string>& config, 
                      const std::string& key, 
                      int default_value = 0);
    
    /**
     * @brief Get a double value from configuration map
     * @param config Configuration map
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @return Parsed double value or default
     * @throws ConfigurationParseException if key exists but cannot be parsed
     */
    static double GetDouble(const std::map<std::string, std::string>& config, 
                            const std::string& key, 
                            double default_value = 0.0);
    
    /**
     * @brief Get a string value from configuration map
     * @param config Configuration map
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @return String value or default
     */
    static std::string GetString(const std::map<std::string, std::string>& config, 
                                  const std::string& key, 
                                  const std::string& default_value = "");

private:
    ConfigurationParser() = delete;  // Static class, no instances
    ConfigurationParser(const ConfigurationParser&) = delete;
    ConfigurationParser& operator=(const ConfigurationParser&) = delete;
    
    /**
     * @brief Trim whitespace from both ends of a string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string Trim(const std::string& str);
};

// Inline implementations
inline std::map<std::string, std::string> ConfigurationParser::ParseFile(
    const std::string& file_path, 
    const std::string& delimiter) {
    
    std::map<std::string, std::string> config;
    
    FILE* file = fopen(file_path.c_str(), "r");
    if (!file) {
        throw ConfigurationParseException("Cannot open configuration file: " + file_path);
    }
    
    char line[1024];
    int line_number = 0;
    
    while (fgets(line, sizeof(line), file)) {
        ++line_number;
        std::string line_str(line);
        
        // Remove trailing newline
        if (!line_str.empty() && line_str.back() == '\n') {
            line_str.pop_back();
        }
        if (!line_str.empty() && line_str.back() == '\r') {
            line_str.pop_back();
        }
        
        // Trim whitespace
        line_str = Trim(line_str);
        
        // Skip empty lines and comments
        if (line_str.empty() || line_str[0] == '#') {
            continue;
        }
        
        // Find delimiter
        size_t pos = line_str.find(delimiter);
        if (pos == std::string::npos) {
            fclose(file);
            throw ConfigurationParseException("Invalid configuration format at line " + 
                                             std::to_string(line_number) + ": missing delimiter '" + 
                                             delimiter + "'");
        }
        
        // Extract key and value
        std::string key = Trim(line_str.substr(0, pos));
        std::string value = Trim(line_str.substr(pos + delimiter.length()));
        
        if (key.empty()) {
            fclose(file);
            throw ConfigurationParseException("Empty key at line " + std::to_string(line_number));
        }
        
        // Store in map (overwrites if duplicate key)
        config[key] = value;
    }
    
    fclose(file);
    return config;
}

inline int ConfigurationParser::GetInt(const std::map<std::string, std::string>& config, 
                                        const std::string& key, 
                                        int default_value) {
    auto it = config.find(key);
    if (it == config.end()) {
        return default_value;
    }
    
    try {
        return ArgumentParser::ParseInt(it->second, key);
    } catch (const ArgumentParseException& e) {
        throw ConfigurationParseException("Error parsing integer for key '" + key + "': " + e.what());
    }
}

inline double ConfigurationParser::GetDouble(const std::map<std::string, std::string>& config, 
                                              const std::string& key, 
                                              double default_value) {
    auto it = config.find(key);
    if (it == config.end()) {
        return default_value;
    }
    
    try {
        return ArgumentParser::ParseDouble(it->second, key);
    } catch (const ArgumentParseException& e) {
        throw ConfigurationParseException("Error parsing double for key '" + key + "': " + e.what());
    }
}

inline std::string ConfigurationParser::GetString(const std::map<std::string, std::string>& config, 
                                                    const std::string& key, 
                                                    const std::string& default_value) {
    auto it = config.find(key);
    if (it == config.end()) {
        return default_value;
    }
    return it->second;
}

inline std::string ConfigurationParser::Trim(const std::string& str) {
    if (str.empty()) {
        return str;
    }
    
    size_t start = 0;
    while (start < str.length() && (str[start] == ' ' || str[start] == '\t')) {
        ++start;
    }
    
    if (start == str.length()) {
        return "";
    }
    
    size_t end = str.length() - 1;
    while (end > start && (str[end] == ' ' || str[end] == '\t' || str[end] == '\r' || str[end] == '\n')) {
        --end;
    }
    
    return str.substr(start, end - start + 1);
}

#endif // CONFIGURATION_PARSER_H

