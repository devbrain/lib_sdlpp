#include <sdlpp/io/filesystem.hh>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

void print_path_info(const sdlpp::path_info& info, const fs::path& path) {
    std::cout << "  Path: " << path << "\n";
    std::cout << "  Type: ";
    switch (info.type) {
        case sdlpp::path_type::none:
            std::cout << "None\n";
            break;
        case sdlpp::path_type::file:
            std::cout << "File (size: " << info.size << " bytes)\n";
            break;
        case sdlpp::path_type::directory:
            std::cout << "Directory\n";
            break;
        case sdlpp::path_type::other:
            std::cout << "Other\n";
            break;
    }
    
    auto format_time = [](std::int64_t time) -> std::string {
        if (time <= 0) return "N/A";
        auto time_point = std::chrono::system_clock::from_time_t(time);
        auto tt = std::chrono::system_clock::to_time_t(time_point);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    };
    
    std::cout << "  Created: " << format_time(info.create_time) << "\n";
    std::cout << "  Modified: " << format_time(info.modify_time) << "\n";
    std::cout << "  Accessed: " << format_time(info.access_time) << "\n";
}

int main() {
    std::cout << "SDL++ Filesystem Example\n";
    std::cout << "=======================\n\n";

    // 1. Get various system paths
    std::cout << "System Paths:\n";
    std::cout << "-------------\n";
    
    if (auto base_path = sdlpp::filesystem::get_base_path()) {
        std::cout << "Base path: " << *base_path << "\n";
    } else {
        std::cout << "Failed to get base path: " << base_path.error() << "\n";
    }
    
    if (auto pref_path = sdlpp::filesystem::get_pref_path("ExampleOrg", "FilesystemExample")) {
        std::cout << "Preferences path: " << *pref_path << "\n";
    } else {
        std::cout << "Failed to get preferences path: " << pref_path.error() << "\n";
    }
    
    if (auto current_dir = sdlpp::filesystem::get_current_directory()) {
        std::cout << "Current directory: " << *current_dir << "\n";
    } else {
        std::cout << "Failed to get current directory: " << current_dir.error() << "\n";
    }
    
    // 2. Get user folders
    std::cout << "\nUser Folders:\n";
    std::cout << "-------------\n";
    
    const std::pair<sdlpp::folder_type, const char*> folders[] = {
        {sdlpp::folder_type::home, "Home"},
        {sdlpp::folder_type::desktop, "Desktop"},
        {sdlpp::folder_type::documents, "Documents"},
        {sdlpp::folder_type::downloads, "Downloads"},
        {sdlpp::folder_type::music, "Music"},
        {sdlpp::folder_type::pictures, "Pictures"},
        {sdlpp::folder_type::videos, "Videos"},
    };
    
    for (const auto& [type, name] : folders) {
        if (auto folder = sdlpp::filesystem::get_user_folder(type)) {
            std::cout << name << ": " << *folder << "\n";
        } else {
            std::cout << name << ": " << folder.error() << "\n";
        }
    }
    
    // 3. Create a test directory structure
    std::cout << "\nCreating Test Directory Structure:\n";
    std::cout << "---------------------------------\n";
    
    auto pref_path_result = sdlpp::filesystem::get_pref_path("ExampleOrg", "FilesystemExample");
    if (!pref_path_result) {
        std::cerr << "Failed to get preferences path\n";
        return 1;
    }
    
    auto test_dir = *pref_path_result / "test_dir";
    
    if (auto result = sdlpp::filesystem::create_directory(test_dir)) {
        std::cout << "Created directory: " << test_dir << "\n";
    } else {
        std::cout << "Failed to create directory: " << result.error() << "\n";
    }
    
    // 4. Create some test files
    std::cout << "\nCreating Test Files:\n";
    std::cout << "-------------------\n";
    
    std::vector<fs::path> test_files = {
        test_dir / "readme.txt",
        test_dir / "data.json",
        test_dir / "config.ini",
        test_dir / "log.txt"
    };
    
    for (const auto& file : test_files) {
        std::ofstream ofs(file);
        if (ofs) {
            ofs << "Test content for " << file.filename() << "\n";
            std::cout << "Created file: " << file.filename() << "\n";
        }
    }
    
    // 5. Test file operations
    std::cout << "\nFile Operations:\n";
    std::cout << "---------------\n";
    
    auto src_file = test_dir / "readme.txt";
    auto copy_file = test_dir / "readme_copy.txt";
    
    if (auto result = sdlpp::filesystem::copy_file(src_file, copy_file)) {
        std::cout << "Copied file: " << src_file.filename() << " -> " << copy_file.filename() << "\n";
    } else {
        std::cout << "Failed to copy file: " << result.error() << "\n";
    }
    
    auto rename_src = test_dir / "log.txt";
    auto rename_dst = test_dir / "old_log.txt";
    
    if (auto result = sdlpp::filesystem::rename_path(rename_src, rename_dst)) {
        std::cout << "Renamed file: " << rename_src.filename() << " -> " << rename_dst.filename() << "\n";
    } else {
        std::cout << "Failed to rename file: " << result.error() << "\n";
    }
    
    // 6. Get path information
    std::cout << "\nPath Information:\n";
    std::cout << "----------------\n";
    
    if (auto info = sdlpp::filesystem::get_path_info(test_dir)) {
        print_path_info(*info, test_dir);
    }
    
    std::cout << "\n";
    
    if (auto info = sdlpp::filesystem::get_path_info(copy_file)) {
        print_path_info(*info, copy_file);
    }
    
    // 7. Directory enumeration
    std::cout << "\nDirectory Enumeration:\n";
    std::cout << "---------------------\n";
    
    std::cout << "All files in test directory:\n";
    auto enum_result = sdlpp::filesystem::enumerate_directory(
        test_dir,
        [](const std::string& name) {
            std::cout << "  - " << name << "\n";
            return SDL_ENUM_CONTINUE; // Required by SDL API
        }
    );
    
    if (!enum_result) {
        std::cout << "Failed to enumerate directory: " << enum_result.error() << "\n";
    }
    
    // 8. Glob pattern matching
    std::cout << "\nGlob Pattern Matching:\n";
    std::cout << "---------------------\n";
    
    std::cout << "Files matching '*.txt':\n";
    if (auto glob_result = sdlpp::filesystem::glob_directory(test_dir, "*.txt")) {
        auto files = glob_result->to_vector();
        for (const auto& file : files) {
            std::cout << "  - " << file << "\n";
        }
        std::cout << "Total matches: " << glob_result->size() << "\n";
    } else {
        std::cout << "Failed to glob directory: " << glob_result.error() << "\n";
    }
    
    // 9. Cleanup
    std::cout << "\nCleanup:\n";
    std::cout << "--------\n";
    
    // Remove individual files first
    for (const auto& entry : fs::directory_iterator(test_dir)) {
        if (entry.is_regular_file()) {
            if (auto result = sdlpp::filesystem::remove_path(entry.path())) {
                std::cout << "Removed file: " << entry.path().filename() << "\n";
            }
        }
    }
    
    // Remove the directory
    if (auto result = sdlpp::filesystem::remove_path(test_dir)) {
        std::cout << "Removed directory: " << test_dir.filename() << "\n";
    } else {
        std::cout << "Failed to remove directory: " << result.error() << "\n";
    }
    
    std::cout << "\nFilesystem example completed!\n";
    
    return 0;
}