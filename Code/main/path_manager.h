/**
 * @file path_manager.h
 * @brief Path management utilities
 * @author Komondor Team
 * @date 2025
 */

#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H

#include <string>
#include <vector>

/**
 * @brief Utility class for path operations
 * 
 * Provides portable path manipulation and directory management.
 * All methods are static for convenience.
 */
class PathManager {
public:
    /**
     * @brief Get the output directory path
     * @return Output directory path
     */
    static std::string GetOutputDirectory();
    
    /**
     * @brief Get the input directory path
     * @return Input directory path
     */
    static std::string GetInputDirectory();
    
    /**
     * @brief Get the logs directory path
     * @return Logs directory path
     */
    static std::string GetLogsDirectory();
    
    /**
     * @brief Join path components
     * @param base Base path
     * @param part Path part to join
     * @return Joined path
     */
    static std::string Join(const std::string& base, const std::string& part);
    
    /**
     * @brief Join multiple path components
     * @param parts Vector of path parts
     * @return Joined path
     */
    static std::string Join(const std::vector<std::string>& parts);
    
    /**
     * @brief Get the directory part of a path
     * @param file_path Full file path
     * @return Directory path
     */
    static std::string GetDirectory(const std::string& file_path);
    
    /**
     * @brief Get the filename part of a path
     * @param file_path Full file path
     * @return Filename
     */
    static std::string GetFilename(const std::string& file_path);
    
    /**
     * @brief Get the base name (filename without extension) of a path
     * @param file_path Full file path
     * @return Base name
     */
    static std::string GetBasename(const std::string& file_path);
    
    /**
     * @brief Get the extension of a file
     * @param file_path Full file path
     * @return File extension (including dot), or empty string if no extension
     */
    static std::string GetExtension(const std::string& file_path);
    
    /**
     * @brief Normalize a path (remove redundant separators, resolve . and ..)
     * @param path Path to normalize
     * @return Normalized path
     */
    static std::string Normalize(const std::string& path);
    
    /**
     * @brief Check if a path is absolute
     * @param path Path to check
     * @return true if absolute, false if relative
     */
    static bool IsAbsolute(const std::string& path);
    
    /**
     * @brief Make a path relative to a base directory
     * @param path Path to make relative
     * @param base Base directory
     * @return Relative path
     */
    static std::string MakeRelative(const std::string& path, const std::string& base);
    
    /**
     * @brief Create output directory structure if it doesn't exist
     * @return true on success, false on error
     */
    static bool InitializeOutputDirectory();
    
private:
    PathManager() = delete;  // Static class, no instances
    PathManager(const PathManager&) = delete;
    PathManager& operator=(const PathManager&) = delete;
};

// Inline implementations
inline std::string PathManager::GetOutputDirectory() {
    return "../output";
}

inline std::string PathManager::GetInputDirectory() {
    return "input";
}

inline std::string PathManager::GetLogsDirectory() {
    return "output/logs";
}

inline std::string PathManager::Join(const std::string& base, const std::string& part) {
    if (base.empty()) {
        return part;
    }
    if (part.empty()) {
        return base;
    }
    
    // Check if base ends with separator
    bool base_ends_with_sep = (base.back() == '/' || base.back() == '\\');
    // Check if part starts with separator
    bool part_starts_with_sep = (part.front() == '/' || part.front() == '\\');
    
    if (base_ends_with_sep && part_starts_with_sep) {
        return base + part.substr(1);
    } else if (!base_ends_with_sep && !part_starts_with_sep) {
        return base + "/" + part;
    } else {
        return base + part;
    }
}

inline std::string PathManager::Join(const std::vector<std::string>& parts) {
    if (parts.empty()) {
        return "";
    }
    
    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result = Join(result, parts[i]);
    }
    return result;
}

inline std::string PathManager::GetDirectory(const std::string& file_path) {
    size_t last_sep = file_path.find_last_of("/\\");
    if (last_sep == std::string::npos) {
        return ".";
    }
    return file_path.substr(0, last_sep);
}

inline std::string PathManager::GetFilename(const std::string& file_path) {
    size_t last_sep = file_path.find_last_of("/\\");
    if (last_sep == std::string::npos) {
        return file_path;
    }
    return file_path.substr(last_sep + 1);
}

inline std::string PathManager::GetBasename(const std::string& file_path) {
    std::string filename = GetFilename(file_path);
    size_t last_dot = filename.find_last_of('.');
    if (last_dot == std::string::npos) {
        return filename;
    }
    return filename.substr(0, last_dot);
}

inline std::string PathManager::GetExtension(const std::string& file_path) {
    std::string filename = GetFilename(file_path);
    size_t last_dot = filename.find_last_of('.');
    if (last_dot == std::string::npos || last_dot == 0) {
        return "";
    }
    return filename.substr(last_dot);
}

inline bool PathManager::IsAbsolute(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    #ifdef _WIN32
        // Windows: check for drive letter (C:) or UNC path (\\)
        return (path.length() >= 2 && path[1] == ':') || 
               (path.length() >= 2 && path[0] == '\\' && path[1] == '\\');
    #else
        // Unix: check if starts with /
        return path[0] == '/';
    #endif
}

inline std::string PathManager::MakeRelative(const std::string& path, const std::string& base) {
    // Simple implementation - can be enhanced
    if (!IsAbsolute(path) || !IsAbsolute(base)) {
        return path;  // Already relative or can't determine
    }
    
    // For now, return original path if different base
    // A full implementation would resolve .. and . components
    return path;
}

inline bool PathManager::InitializeOutputDirectory() {
    // This will be implemented using FileManager
    // For now, return true (placeholder)
    return true;
}

#endif // PATH_MANAGER_H

