#include <iostream>
#include <vector>
#include <sdlpp/core/core.hh>
#include <sdlpp/system/misc.hh>

void demonstrate_url_helpers() {
    std::cout << "=== URL Helper Functions ===\n\n";
    
    // Test various URLs for protocol detection
    std::vector<std::string> test_urls = {
        "https://www.libsdl.org",
        "http://example.com",
        "www.example.com",
        "example.com",
        "file:///home/user/document.pdf",
        "/home/user/document.pdf",
        "mailto:support@example.com",
        "steam://run/730",
        "ftp://files.example.com/download.zip"
    };
    
    std::cout << "Protocol detection:\n";
    for (const auto& url : test_urls) {
        bool has_proto = sdlpp::url::has_protocol(url);
        std::cout << "  \"" << url << "\" -> " 
                  << (has_proto ? "has protocol" : "no protocol") << "\n";
    }
    
    std::cout << "\nEnsuring HTTPS protocol:\n";
    std::vector<std::string> web_urls = {
        "example.com",
        "www.example.com",
        "https://secure.example.com",
        "http://old.example.com",
        "ftp://files.example.com"
    };
    
    for (const auto& url : web_urls) {
        std::string with_protocol = sdlpp::url::ensure_protocol(url);
        std::cout << "  \"" << url << "\" -> \"" << with_protocol << "\"\n";
    }
}

void demonstrate_mailto_creation() {
    std::cout << "\n=== Creating Mailto URLs ===\n\n";
    
    // Simple email
    std::string simple = sdlpp::url::make_mailto("support@example.com");
    std::cout << "Simple email:\n  " << simple << "\n\n";
    
    // Email with subject
    std::string with_subject = sdlpp::url::make_mailto(
        "bugs@example.com",
        "Bug Report: Application Crash"
    );
    std::cout << "With subject:\n  " << with_subject << "\n\n";
    
    // Email with body
    std::string with_body = sdlpp::url::make_mailto(
        "feedback@example.com",
        "",
        "I wanted to share some feedback about your application..."
    );
    std::cout << "With body:\n  " << with_body << "\n\n";
    
    // Complete email
    std::string complete = sdlpp::url::make_mailto(
        "support@example.com",
        "Help Request",
        "I need help with the following issue:\n\n1. Problem description\n2. Steps to reproduce"
    );
    std::cout << "Complete email:\n  " << complete << "\n\n";
    
    std::cout << "Note: The subject and body are not URL-encoded by this function.\n";
    std::cout << "You may need to encode them for special characters.\n";
}

void demonstrate_file_urls() {
    std::cout << "\n=== Creating File URLs ===\n\n";
    
    std::vector<std::string> paths = {
        "/home/user/documents/report.pdf",
        "/tmp/test-file.txt",
        "C:\\Users\\Username\\Desktop\\file.doc",
        "D:\\Projects\\SDL\\README.md",
        "relative/path/to/file.txt",
        "\\\\network\\share\\file.txt"
    };
    
    std::cout << "Converting paths to file:// URLs:\n";
    for (const auto& path : paths) {
        std::string file_url = sdlpp::url::make_file_url(path);
        std::cout << "  \"" << path << "\"\n";
        std::cout << "  -> \"" << file_url << "\"\n\n";
    }
}

void demonstrate_open_url() {
    std::cout << "\n=== Opening URLs ===\n\n";
    
    std::cout << "The open_url function can open various types of URLs:\n\n";
    
    std::cout << "1. Web URLs:\n";
    std::cout << "   sdlpp::open_url(\"https://www.libsdl.org\")\n";
    std::cout << "   Opens in default web browser\n\n";
    
    std::cout << "2. Local files:\n";
    std::cout << "   sdlpp::open_url(\"file:///home/user/document.pdf\")\n";
    std::cout << "   Opens in default application for that file type\n\n";
    
    std::cout << "3. Email links:\n";
    std::cout << "   sdlpp::open_url(\"mailto:user@example.com\")\n";
    std::cout << "   Opens in default email client\n\n";
    
    std::cout << "4. Application protocols:\n";
    std::cout << "   sdlpp::open_url(\"steam://run/730\")\n";
    std::cout << "   Opens in registered application (if installed)\n\n";
    
    // Interactive demo
    std::cout << "Interactive demo (uncomment to test):\n";
    std::cout << "WARNING: This will actually open URLs in external applications!\n\n";
    
    /*
    // Uncomment these lines to actually test opening URLs
    
    std::cout << "Press Enter to open SDL website...";
    std::cin.get();
    
    auto result = sdlpp::open_url("https://www.libsdl.org");
    if (result) {
        std::cout << "Successfully requested to open URL\n";
    } else {
        std::cout << "Failed to open URL: " << result.error() << "\n";
    }
    
    std::cout << "\nPress Enter to open a local file URL...";
    std::cin.get();
    
    // Open current directory
    auto file_url = sdlpp::url::make_file_url(".");
    auto result2 = sdlpp::open_url(file_url);
    if (result2) {
        std::cout << "Successfully requested to open: " << file_url << "\n";
    } else {
        std::cout << "Failed to open file URL: " << result2.error() << "\n";
    }
    */
}

void demonstrate_error_handling() {
    std::cout << "\n=== Error Handling ===\n\n";
    
    std::cout << "The open_url function returns tl::expected<void, std::string>:\n\n";
    
    std::cout << "Example error handling:\n";
    std::cout << R"(
auto result = sdlpp::open_url("https://example.com");
if (result) {
    std::cout << "URL opened successfully\n";
} else {
    std::cerr << "Failed to open URL: " << result.error() << "\n";
    // Handle error appropriately
}
)";
    
    std::cout << "\nNote: Success only means SDL requested the OS to open the URL.\n";
    std::cout << "It doesn't guarantee the URL actually loaded or that the\n";
    std::cout << "application to handle it exists.\n";
}

void demonstrate_platform_notes() {
    std::cout << "\n=== Platform-Specific Behavior ===\n\n";
    
    std::cout << "URL opening behavior varies by platform:\n\n";
    
    std::cout << "Windows:\n";
    std::cout << "  - Uses ShellExecute\n";
    std::cout << "  - Excellent protocol support\n";
    std::cout << "  - May show UAC prompts for some URLs\n\n";
    
    std::cout << "macOS:\n";
    std::cout << "  - Uses NSWorkspace\n";
    std::cout << "  - Seamless integration with system\n";
    std::cout << "  - Respects user's default applications\n\n";
    
    std::cout << "Linux:\n";
    std::cout << "  - Uses xdg-open or similar\n";
    std::cout << "  - Depends on desktop environment\n";
    std::cout << "  - May fail if no DE is running\n\n";
    
    std::cout << "Mobile platforms:\n";
    std::cout << "  - iOS: May switch apps\n";
    std::cout << "  - Android: Uses Intent system\n";
    std::cout << "  - Both may require permissions\n\n";
    
    std::cout << "Web (Emscripten):\n";
    std::cout << "  - Uses window.open()\n";
    std::cout << "  - Subject to popup blocking\n";
    std::cout << "  - May need user interaction first\n";
}

int main() {
    try {
        // Initialize SDL
        sdlpp::init sdl_init(sdlpp::init_flags::video);
        
        std::cout << "SDL++ Misc Example\n";
        std::cout << "==================\n\n";
        
        demonstrate_url_helpers();
        demonstrate_mailto_creation();
        demonstrate_file_urls();
        demonstrate_open_url();
        demonstrate_error_handling();
        demonstrate_platform_notes();
        
        std::cout << "\n=== Summary ===\n";
        std::cout << "The misc module provides URL/URI opening functionality.\n";
        std::cout << "Helper functions make it easy to work with various URL types.\n";
        std::cout << "Always test URL opening on your target platforms.\n";
        std::cout << "Consider that opening URLs may cause focus loss.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}