// config.h
#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>


namespace SE {

class Config {
public:
    static Config& get() {
        static Config instance;
        return instance;
    }

    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif

    bool stringToBool(const std::string& str) {
        std::istringstream iss(str);
        bool result;
        iss >> std::boolalpha >> result; // Use boolalpha to parse "true"/"false"
        return result;
    }

    
    // Public accessors (read-only outside class)
    auto window() const { return window_; }
    const int max_frames_in_flight() const { return max_frames_in_flight_; }
    const float max_frame_time() const { return max_frame_time_; }
    const std::string& asset_path() const { return asset_path_; }
    const std::string& model_path() const { return model_path_; }
    const std::string& texture_path() const { return texture_path_; }
    const std::string& shader_path() const { return shader_path_; }
    const bool& print_extensions_to_console() const { return print_extensions_to_console_; }
    const bool& print_device_info() const { return print_device_info_; }
    
    // Load config from file
    void load_from_file(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Config file not found, using defaults\n";
            return;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            // Skip empty lines or comments
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;
            
            // Trim section headers (e.g., [Renderer]) for simplicity
            if (line[0] == '[') continue;
            
            // Parse key=value
            size_t delim = line.find('=');
            if (delim == std::string::npos) continue;
            
            std::string key = line.substr(0, delim);
            std::string value = line.substr(delim + 1);
            
            // Assign values (basic type conversion)
            if (key == "name") window_.name = value;
            else if (key == "width") window_.width = std::stoul(value);
            else if (key == "height") window_.height = std::stoul(value);
            else if (key == "max_frames_in_flight") max_frames_in_flight_ = std::stoi(value);
            else if (key == "max_frame_time") max_frame_time_ = std::stoi(value);
            else if (key == "asset_path") asset_path_ = value;
            else if (key == "model_path") model_path_ = value;
            else if (key == "texture_path") texture_path_ = value;
            else if (key == "shader_path") shader_path_ = value;
            else if (key == "print_extensions_to_console") print_extensions_to_console_ = stringToBool(value);
            else if (key == "print_device_info") print_device_info_ = stringToBool(value);
            
        }
        
        file.close();
    }

private:
    // Private constructor with defaults

    struct window
    {
        uint32_t width;
        uint32_t height;
        std::string name;
    } window_;
    
    Config() 
        : window_{ 1920, 1080 }
        , max_frames_in_flight_(2)
        , max_frame_time_(2)
        , asset_path_("assets/")
        , model_path_("models/")
        , texture_path_("textures/")
        , shader_path_("shaders/")
        , print_extensions_to_console_(false)
        , print_device_info_(false)
    {
        // Load config at construction (could move to main if preferred)
        load_from_file("config/config.ini");
    }
    
    // Member variables
    
    int max_frames_in_flight_;
    float max_frame_time_;
    std::string asset_path_;
    std::string model_path_;
    std::string texture_path_;
    std::string shader_path_;
    bool print_extensions_to_console_;
    bool print_device_info_;
};
}
