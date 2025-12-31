#include <sdlpp/io/filesystem.hh>

#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <set>

namespace fs = std::filesystem;

TEST_SUITE("filesystem") {
    TEST_CASE("get_base_path") {
        auto result = sdlpp::filesystem::get_base_path();
        CHECK(result.has_value());
        if (result) {
            CHECK(fs::exists(*result));
            CHECK(fs::is_directory(*result));
        }
    }

    TEST_CASE("get_pref_path") {
        auto result = sdlpp::filesystem::get_pref_path("TestOrg", "TestApp");
        CHECK(result.has_value());
        if (result) {
            CHECK(fs::exists(*result));
            CHECK(fs::is_directory(*result));
            
            // Cleanup
            std::error_code ec;
            fs::remove_all(*result, ec);
            if (ec) {
                MESSAGE("Failed to remove pref path: ", ec.message());
            }
        }
    }

    TEST_CASE("get_user_folder") {
        SUBCASE("home folder") {
            auto result = sdlpp::filesystem::get_user_folder(sdlpp::folder_type::home);
            CHECK(result.has_value());
            if (result) {
                CHECK(fs::exists(*result));
                CHECK(fs::is_directory(*result));
            }
        }

        SUBCASE("documents folder") {
            auto result = sdlpp::filesystem::get_user_folder(sdlpp::folder_type::documents);
            // Documents folder may not exist in CI environments
            if (!result) {
                MESSAGE("Documents folder not available: ", result.error());
            } else {
                CHECK(fs::exists(*result));
                CHECK(fs::is_directory(*result));
            }
        }

        SUBCASE("downloads folder") {
            auto result = sdlpp::filesystem::get_user_folder(sdlpp::folder_type::downloads);
            // Downloads folder may not exist in CI environments
            if (!result) {
                MESSAGE("Downloads folder not available: ", result.error());
            } else {
                CHECK(fs::exists(*result));
                CHECK(fs::is_directory(*result));
            }
        }
    }

    TEST_CASE("get_current_directory") {
        auto result = sdlpp::filesystem::get_current_directory();
        CHECK(result.has_value());
        if (result) {
            CHECK(fs::exists(*result));
            CHECK(fs::is_directory(*result));
            
            // Compare with std::filesystem current_path
            CHECK(fs::equivalent(*result, fs::current_path()));
        }
    }

    TEST_CASE("directory operations") {
        auto temp_dir = fs::temp_directory_path() / "sdlpp_test";
        fs::remove_all(temp_dir);

        SUBCASE("create_directory") {
            auto result = sdlpp::filesystem::create_directory(temp_dir);
            CHECK(result.has_value());
            CHECK(fs::exists(temp_dir));
            CHECK(fs::is_directory(temp_dir));

            // Create nested directory
            auto nested = temp_dir / "nested" / "deep";
            result = sdlpp::filesystem::create_directory(nested);
            CHECK(result.has_value());
            CHECK(fs::exists(nested));
            CHECK(fs::is_directory(nested));
        }

        SUBCASE("remove_path") {
            fs::create_directories(temp_dir);
            auto file_path = temp_dir / "test.txt";
            std::ofstream{file_path} << "test content";

            // Remove file
            auto result = sdlpp::filesystem::remove_path(file_path);
            CHECK(result.has_value());
            CHECK(!fs::exists(file_path));

            // Remove empty directory
            result = sdlpp::filesystem::remove_path(temp_dir);
            CHECK(result.has_value());
            CHECK(!fs::exists(temp_dir));
        }

        SUBCASE("rename_path") {
            fs::create_directories(temp_dir);
            auto old_path = temp_dir / "old.txt";
            auto new_path = temp_dir / "new.txt";
            
            std::ofstream{old_path} << "test content";

            auto result = sdlpp::filesystem::rename_path(old_path, new_path);
            CHECK(result.has_value());
            CHECK(!fs::exists(old_path));
            CHECK(fs::exists(new_path));
        }

        SUBCASE("copy_file") {
            fs::create_directories(temp_dir);
            auto src_path = temp_dir / "source.txt";
            auto dst_path = temp_dir / "destination.txt";
            
            std::string content = "test content for copying";
            std::ofstream{src_path} << content;

            auto result = sdlpp::filesystem::copy_file(src_path, dst_path);
            CHECK(result.has_value());
            CHECK(fs::exists(src_path));
            CHECK(fs::exists(dst_path));
            
            // Verify content
            std::ifstream dst_file{dst_path};
            std::string dst_content;
            std::getline(dst_file, dst_content);
            CHECK(dst_content == content);
        }

        // Cleanup
        fs::remove_all(temp_dir);
    }

    TEST_CASE("get_path_info") {
        auto temp_dir = fs::temp_directory_path() / "sdlpp_test_info";
        fs::remove_all(temp_dir);
        fs::create_directories(temp_dir);

        SUBCASE("file info") {
            auto file_path = temp_dir / "test_file.txt";
            std::string content = "Hello, filesystem!";
            std::ofstream{file_path} << content;

            auto result = sdlpp::filesystem::get_path_info(file_path);
            CHECK(result.has_value());
            if (result) {
                CHECK(result->type == sdlpp::path_type::file);
                CHECK(result->size == content.size());
                CHECK(result->create_time > 0);
                CHECK(result->modify_time > 0);
                CHECK(result->access_time > 0);
            }
        }

        SUBCASE("directory info") {
            auto result = sdlpp::filesystem::get_path_info(temp_dir);
            CHECK(result.has_value());
            if (result) {
                CHECK(result->type == sdlpp::path_type::directory);
                CHECK(result->create_time > 0);
                CHECK(result->modify_time > 0);
                CHECK(result->access_time > 0);
            }
        }

        SUBCASE("non-existent path") {
            auto result = sdlpp::filesystem::get_path_info(temp_dir / "non_existent");
            CHECK(!result.has_value());
        }

        // Cleanup
        fs::remove_all(temp_dir);
    }

    TEST_CASE("glob_directory") {
        auto temp_dir = fs::temp_directory_path() / "sdlpp_glob_test";
        fs::remove_all(temp_dir);
        fs::create_directories(temp_dir);

        // Create test files
        std::vector<fs::path> test_files = {
            temp_dir / "test1.txt",
            temp_dir / "test2.txt",
            temp_dir / "test3.log",
            temp_dir / "readme.md",
            temp_dir / "data.json",
            temp_dir / "TEST4.TXT"
        };

        for (const auto& file : test_files) {
            std::ofstream{file} << "test";
        }

        // Create subdirectory
        fs::create_directories(temp_dir / "subdir");

        SUBCASE("glob all files") {
            auto result = sdlpp::filesystem::glob_directory(temp_dir);
            CHECK(result.has_value());
            if (result) {
                auto paths = result->to_vector();
                CHECK(paths.size() >= test_files.size());
            }
        }

        SUBCASE("glob txt files") {
            auto result = sdlpp::filesystem::glob_directory(temp_dir, "*.txt");
            CHECK(result.has_value());
            if (result) {
                auto paths = result->to_vector();
                CHECK(paths.size() == 2);  // test1.txt and test2.txt
            }
        }

        SUBCASE("glob with case insensitive") {
            auto result = sdlpp::filesystem::glob_directory(
                temp_dir, 
                "*.txt", 
                sdlpp::filesystem::glob_flags::case_insensitive
            );
            CHECK(result.has_value());
            if (result) {
                auto paths = result->to_vector();
                CHECK(paths.size() == 3);  // test1.txt, test2.txt, and TEST4.TXT
            }
        }

        SUBCASE("glob with pattern") {
            auto result = sdlpp::filesystem::glob_directory(temp_dir, "test*");
            CHECK(result.has_value());
            if (result) {
                auto paths = result->to_vector();
                CHECK(paths.size() == 3);  // test1.txt, test2.txt, test3.log
            }
        }

        SUBCASE("empty glob result") {
            auto result = sdlpp::filesystem::glob_directory(temp_dir, "*.nonexistent");
            CHECK(result.has_value());
            if (result) {
                CHECK(result->empty());
                CHECK(result->size() == 0);
            }
        }

        // Cleanup
        fs::remove_all(temp_dir);
    }

    TEST_CASE("enumerate_directory") {
        auto temp_dir = fs::temp_directory_path() / "sdlpp_enum_test";
        fs::remove_all(temp_dir);
        fs::create_directories(temp_dir);

        // Create test files
        std::set<std::string> expected_files = {
            "file1.txt",
            "file2.log",
            "file3.dat"
        };

        for (const auto& name : expected_files) {
            std::ofstream{temp_dir / name} << "test";
        }

        // Create subdirectory
        fs::create_directories(temp_dir / "subdir");
        expected_files.insert("subdir");

        SUBCASE("enumerate all entries") {
            std::set<std::string> found_files;
            
            auto result = sdlpp::filesystem::enumerate_directory(
                temp_dir,
                [&found_files](const std::string& name) {
                    found_files.insert(name);
                    return SDL_ENUM_CONTINUE;
                }
            );

            CHECK(result.has_value());
            CHECK(found_files == expected_files);
        }

        SUBCASE("enumerate with early stop") {
            int count = 0;
            
            auto result = sdlpp::filesystem::enumerate_directory(
                temp_dir,
                [&count]([[maybe_unused]] const std::string& name) {
                    ++count;
                    return count >= 2 ? SDL_ENUM_SUCCESS : SDL_ENUM_CONTINUE;
                }
            );

            CHECK(result.has_value());
            CHECK(count == 2);
        }

        SUBCASE("enumerate with filter") {
            std::vector<std::string> txt_files;
            
            auto result = sdlpp::filesystem::enumerate_directory(
                temp_dir,
                [&txt_files](const std::string& name) {
                    if (name.ends_with(".txt")) {
                        txt_files.push_back(name);
                    }
                    return SDL_ENUM_CONTINUE;
                }
            );

            CHECK(result.has_value());
            CHECK(txt_files.size() == 1);
            CHECK(txt_files[0] == "file1.txt");
        }

        // Cleanup
        fs::remove_all(temp_dir);
    }

    TEST_CASE("glob_flags operations") {
        using flags = sdlpp::filesystem::glob_flags;
        
        auto combined = flags::none | flags::case_insensitive;
        CHECK(static_cast<std::uint32_t>(combined) == SDL_GLOB_CASEINSENSITIVE);
        
        auto both = flags::case_insensitive | flags::case_insensitive;
        CHECK(static_cast<std::uint32_t>(both) == SDL_GLOB_CASEINSENSITIVE);
        
        auto masked = combined & flags::case_insensitive;
        CHECK(masked == flags::case_insensitive);
        
        auto none = flags::none & flags::case_insensitive;
        CHECK(none == flags::none);
    }
}
