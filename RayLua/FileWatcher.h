#pragma once
#include <string>
#include <filesystem>
#include <chrono>

class FileWatcher {
private:
    std::string filepath;
    std::filesystem::file_time_type last_write_time;
    
public:
    FileWatcher(const std::string& file) : filepath(file) {
        if (std::filesystem::exists(filepath)) {
            last_write_time = std::filesystem::last_write_time(filepath);
        }
    }
    
    bool HasChanged() {
        if (!std::filesystem::exists(filepath)) return false;
        
        auto current_time = std::filesystem::last_write_time(filepath);
        if (current_time != last_write_time) {
            last_write_time = current_time;
            return true;
        }
        return false;
    }
};
