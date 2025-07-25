#include <doctest/doctest.h>
#include <sdlpp/ui/dialog.hh>
#include <sdlpp/core/core.hh>

TEST_SUITE("dialog") {
    // Note: File dialogs are asynchronous and require user interaction,
    // so we can only test the API construction and basic validation
    
    TEST_CASE("dialog_file_filter construction") {
        sdlpp::dialog_file_filter filter{"Image files", "*.png;*.jpg"};
        
        auto sdl_filter = filter.to_sdl();
        CHECK(sdl_filter.name != nullptr);
        CHECK(sdl_filter.pattern != nullptr);
        CHECK(std::string(sdl_filter.name) == "Image files");
        CHECK(std::string(sdl_filter.pattern) == "*.png;*.jpg");
    }
    
    TEST_CASE("dialog_result") {
        SUBCASE("accepted result") {
            sdlpp::dialog_result result;
            result.accepted = true;
            result.paths = {"/path/to/file1.txt", "/path/to/file2.txt"};
            
            CHECK_FALSE(result.cancelled());
            CHECK(result.get_path().has_value());
            CHECK(result.get_path().value() == "/path/to/file1.txt");
            CHECK(result.paths.size() == 2);
        }
        
        SUBCASE("cancelled result") {
            sdlpp::dialog_result result;
            result.accepted = false;
            
            CHECK(result.cancelled());
            CHECK_FALSE(result.get_path().has_value());
            CHECK(result.paths.empty());
        }
    }
    
    TEST_CASE("file_dialog_builder") {
        sdlpp::file_dialog_builder builder;
        
        // Test builder pattern
        builder.set_type(sdlpp::file_dialog_type::save_file)
               .set_title("Save Document")
               .set_accept_label("Save")
               .set_cancel_label("Don't Save")
               .set_default_location("/home/user/documents")
               .set_default_name("untitled.txt")
               .add_filter("Text files", "*.txt")
               .add_filter("All files", "*.*")
               .allow_multiple(false);
        
        // Test adding filter object
        sdlpp::dialog_file_filter filter{"Markdown files", "*.md"};
        builder.add_filter(filter);
        
        // Can't test show() without user interaction
    }
    
    TEST_CASE("dialog types") {
        // Just verify the enum values are accessible
        auto open_type = sdlpp::file_dialog_type::open_file;
        auto save_type = sdlpp::file_dialog_type::save_file;
        auto folder_type = sdlpp::file_dialog_type::open_folder;
        
        CHECK(static_cast<int>(open_type) == SDL_FILEDIALOG_OPENFILE);
        CHECK(static_cast<int>(save_type) == SDL_FILEDIALOG_SAVEFILE);
        CHECK(static_cast<int>(folder_type) == SDL_FILEDIALOG_OPENFOLDER);
    }
}