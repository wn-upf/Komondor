/**
 * @file file_manager.h
 * @brief File I/O operations manager
 * @author Komondor Team
 * @date 2025
 */

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <iterator>

#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
    #include <sys/stat.h>
    #ifndef mode_t
        typedef unsigned short mode_t;
    #endif
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

/**
 * @brief Utility class for file operations
 * 
 * Provides a safe, RAII-based interface for file I/O operations.
 * All methods are static for convenience.
 */
class FileManager {
public:
    /**
     * @brief Check if a file exists
     * @param file_path Path to the file
     * @return true if file exists, false otherwise
     */
    static bool FileExists(const std::string& file_path);
    
    /**
     * @brief Check if a directory exists
     * @param dir_path Path to the directory
     * @return true if directory exists, false otherwise
     */
    static bool DirectoryExists(const std::string& dir_path);
    
    /**
     * @brief Create a directory if it doesn't exist
     * @param dir_path Path to the directory
     * @param mode Permissions mode (default: 0755)
     * @return true if directory exists or was created, false on error
     */
    static bool CreateDirectoryIfNotExists(const std::string& dir_path, mode_t mode = 0755);
    
    /**
     * @brief Remove a file if it exists
     * @param file_path Path to the file
     * @return true if file was removed or didn't exist, false on error
     */
    static bool RemoveFileIfExists(const std::string& file_path);
    
    /**
     * @brief Get file size in bytes
     * @param file_path Path to the file
     * @return File size in bytes, or -1 on error
     */
    static long GetFileSize(const std::string& file_path);
    
    /**
     * @brief Read entire file into a string
     * @param file_path Path to the file
     * @return File contents as string
     * @throws std::runtime_error if file cannot be read
     */
    static std::string ReadFile(const std::string& file_path);
    
    /**
     * @brief Write string to file
     * @param file_path Path to the file
     * @param content Content to write
     * @param append If true, append to file; if false, overwrite
     * @return true on success, false on error
     */
    static bool WriteFile(const std::string& file_path, const std::string& content, bool append = false);
    
    /**
     * @brief Open a file for reading/writing
     * @param file_path Path to the file
     * @param mode File mode ("r", "w", "a", etc.)
     * @return FILE* pointer, or nullptr on error
     * @note Caller is responsible for closing the file with fclose()
     */
    static FILE* OpenFile(const std::string& file_path, const char* mode);
    
    /**
     * @brief Safely close a file
     * @param file File pointer to close
     * @return true on success, false on error
     */
    static bool CloseFile(FILE* file);
    
private:
    FileManager() = delete;  // Static class, no instances
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;
};

// Inline implementations
inline bool FileManager::FileExists(const std::string& file_path) {
    struct stat buffer;
    return (stat(file_path.c_str(), &buffer) == 0);
}

inline bool FileManager::DirectoryExists(const std::string& dir_path) {
    struct stat buffer;
    if (stat(dir_path.c_str(), &buffer) != 0) {
        return false;
    }
    return S_ISDIR(buffer.st_mode);
}

inline bool FileManager::CreateDirectoryIfNotExists(const std::string& dir_path, mode_t mode) {
    if (DirectoryExists(dir_path)) {
        return true;
    }
    
    // Try to create directory
    #ifdef _WIN32
        return (_mkdir(dir_path.c_str()) == 0);
    #else
        return (mkdir(dir_path.c_str(), mode) == 0);
    #endif
}

inline bool FileManager::RemoveFileIfExists(const std::string& file_path) {
    if (!FileExists(file_path)) {
        return true;  // File doesn't exist, consider it successful
    }
    return (remove(file_path.c_str()) == 0);
}

inline long FileManager::GetFileSize(const std::string& file_path) {
    struct stat buffer;
    if (stat(file_path.c_str(), &buffer) != 0) {
        return -1;
    }
    return buffer.st_size;
}

inline std::string FileManager::ReadFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for reading: " + file_path);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
}

inline bool FileManager::WriteFile(const std::string& file_path, const std::string& content, bool append) {
    std::ofstream file(file_path, append ? std::ios::app : std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    return file.good();
}

inline FILE* FileManager::OpenFile(const std::string& file_path, const char* mode) {
    return fopen(file_path.c_str(), mode);
}

inline bool FileManager::CloseFile(FILE* file) {
    if (file == nullptr) {
        return true;  // Already closed or null, consider it successful
    }
    return (fclose(file) == 0);
}

#endif // FILE_MANAGER_H

