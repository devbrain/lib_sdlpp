#include <sdlpp/core/error.hh>
#include <sdlpp/core/core.hh>

#include <doctest/doctest.h>

#include <thread>
#include <vector>

TEST_SUITE("error") {
    TEST_CASE("get_error and clear_error") {
        // Clear any existing error
        sdlpp::clear_error();
        
        // Initially no error should be set
        CHECK(sdlpp::get_error().empty());
        
        // Set an error using SDL directly
        SDL_SetError("Test error message");
        CHECK(sdlpp::get_error() == "Test error message");
        
        // Clear error
        sdlpp::clear_error();
        CHECK(sdlpp::get_error().empty());
    }
    
    TEST_CASE("set_error with various types") {
        sdlpp::clear_error();
        
        SUBCASE("simple string") {
            (void)sdlpp::set_error("Simple error message");
            CHECK(sdlpp::get_error() == "Simple error message");
        }
        
        SUBCASE("multiple strings") {
            (void)sdlpp::set_error("Error:", "File not found");
            CHECK(sdlpp::get_error() == "Error: File not found");
        }
        
        SUBCASE("mixed types") {
            (void)sdlpp::set_error("Failed to allocate", 1024, "bytes");
            CHECK(sdlpp::get_error() == "Failed to allocate 1024 bytes");
        }
        
        SUBCASE("with numbers") {
            (void)sdlpp::set_error("Position:", 10, 20, "Size:", 800, "x", 600);
            CHECK(sdlpp::get_error() == "Position: 10 20 Size: 800 x 600");
        }
        
        SUBCASE("with boolean") {
            (void)sdlpp::set_error("Success:", true, "Failed:", false);
            CHECK(sdlpp::get_error() == "Success: true Failed: false");
        }
        
        SUBCASE("with nullptr") {
            void* ptr = nullptr;
            (void)sdlpp::set_error("Pointer is", ptr);
            CHECK(sdlpp::get_error() == "Pointer is nullptr");
        }
        
        SUBCASE("with valid pointer") {
            int value = 42;
            int* ptr = &value;
            (void)sdlpp::set_error("Pointer is", ptr);
            std::string error = sdlpp::get_error();
            CHECK(error.starts_with("Pointer is 0x"));
            CHECK(error.find("nullptr") == std::string::npos);
        }
        
        SUBCASE("empty message") {
            (void)sdlpp::set_error();
            CHECK(sdlpp::get_error().empty());
        }
        
        // Clear error after each subcase
        sdlpp::clear_error();
    }
    
    TEST_CASE("convenience error functions") {
        sdlpp::clear_error();
        
        SUBCASE("set_out_of_memory_error") {
            (void)sdlpp::set_out_of_memory_error();
            std::string error = sdlpp::get_error();
            CHECK(!error.empty());
            // SDL's out of memory message may vary, just check it's not empty
        }
        
        SUBCASE("set_unsupported_error") {
            (void)sdlpp::set_unsupported_error();
            std::string error = sdlpp::get_error();
            CHECK(!error.empty());
        }
        
        SUBCASE("set_invalid_param_error") {
            (void)sdlpp::set_invalid_param_error("test_param");
            std::string error = sdlpp::get_error();
            CHECK(!error.empty());
            CHECK(error.find("test_param") != std::string::npos);
        }
        
        sdlpp::clear_error();
    }
    
    TEST_CASE("error_guard") {
        sdlpp::clear_error();
        
        SUBCASE("preserves error state") {
            (void)sdlpp::set_error("Original error");
            
            {
                sdlpp::error_guard guard;
                CHECK(guard.saved_error() == "Original error");
                CHECK(sdlpp::get_error().empty()); // Guard clears error
                
                (void)sdlpp::set_error("New error");
                CHECK(sdlpp::get_error() == "New error");
            } // Guard restores original error
            
            CHECK(sdlpp::get_error() == "Original error");
        }
        
        SUBCASE("handles no initial error") {
            sdlpp::clear_error();
            
            {
                sdlpp::error_guard guard;
                CHECK(guard.saved_error().empty());
                
                (void)sdlpp::set_error("Error inside guard");
                CHECK(sdlpp::get_error() == "Error inside guard");
            } // Guard doesn't set error if there was none
            
            CHECK(sdlpp::get_error().empty());
        }
        
        sdlpp::clear_error();
    }
    
    TEST_CASE("error_scope") {
        SUBCASE("clears on entry and exit") {
            (void)sdlpp::set_error("Existing error");
            
            {
                sdlpp::error_scope scope;
                CHECK(sdlpp::get_error().empty()); // Cleared on entry
                
                (void)sdlpp::set_error("Error in scope");
                CHECK(sdlpp::get_error() == "Error in scope");
            } // Cleared on exit
            
            CHECK(sdlpp::get_error().empty());
        }
        
        SUBCASE("clears only on entry when requested") {
            (void)sdlpp::set_error("Existing error");
            
            {
                sdlpp::error_scope scope(false); // Don't clear on exit
                CHECK(sdlpp::get_error().empty()); // Cleared on entry
                
                (void)sdlpp::set_error("Error in scope");
                CHECK(sdlpp::get_error() == "Error in scope");
            } // Not cleared on exit
            
            CHECK(sdlpp::get_error() == "Error in scope");
        }
        
        sdlpp::clear_error();
    }
    
    TEST_CASE("thread safety") {
        // Initialize SDL to properly handle thread-local storage
        auto init = sdlpp::init(sdlpp::init_flags::none);
        REQUIRE(init);
        
        // SDL maintains per-thread error state
        std::vector<std::thread> threads;
        std::vector<std::string> results(4);
        
        for (int i = 0; i < 4; ++i) {
            threads.emplace_back([i, &results]() {
                sdlpp::clear_error();
                (void)sdlpp::set_error("Thread", i, "error");
                results[static_cast<std::size_t>(i)] = sdlpp::get_error();
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        // Each thread should have its own error message
        for (int i = 0; i < 4; ++i) {
            CHECK(results[static_cast<std::size_t>(i)] == "Thread " + std::to_string(i) + " error");
        }
        
        // Main thread error should be unaffected
        sdlpp::clear_error();
        CHECK(sdlpp::get_error().empty());
    }
    
    TEST_CASE("error return values") {
        // SDL error functions return negative on error
        SUBCASE("set_error returns false") {
            bool result = sdlpp::set_error("Test error");
            CHECK(result == false);
        }
        
        SUBCASE("set_out_of_memory_error returns false") {
            bool result = sdlpp::set_out_of_memory_error();
            CHECK(result == false);
        }
        
        SUBCASE("set_unsupported_error returns false") {
            bool result = sdlpp::set_unsupported_error();
            CHECK(result == false);
        }
        
        SUBCASE("set_invalid_param_error returns false") {
            bool result = sdlpp::set_invalid_param_error("param");
            CHECK(result == false);
        }
        
        sdlpp::clear_error();
    }
    
    TEST_CASE("special characters in error messages") {
        sdlpp::clear_error();
        
        SUBCASE("newlines and tabs") {
            (void)sdlpp::set_error("Line 1\nLine 2\tTabbed");
            CHECK(sdlpp::get_error() == "Line 1\nLine 2\tTabbed");
        }
        
        SUBCASE("quotes and backslashes") {
            (void)sdlpp::set_error("Path: \"C:\\Program Files\\\"");
            CHECK(sdlpp::get_error() == "Path: \"C:\\Program Files\\\"");
        }
        
        SUBCASE("percent signs") {
            // Percent signs should be safe since we use %s format
            (void)sdlpp::set_error("Progress: 50% complete");
            CHECK(sdlpp::get_error() == "Progress: 50% complete");
        }
        
        sdlpp::clear_error();
    }
}