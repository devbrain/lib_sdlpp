#include <doctest/doctest.h>
#include <sdlpp/system/process.hh>
#include <sdlpp/core/core.hh>
#include <chrono>
#include <thread>
#if 0
// Cross-platform command selection
#ifdef _WIN32
    static const std::vector<std::string> ECHO_CMD = {"cmd", "/c", "echo", "Hello World"};
    static const std::vector<std::string> SLEEP_CMD = {"timeout", "/t", "1"};
    static const std::vector<std::string> CAT_CMD = {"type", "con"};
    static const std::vector<std::string> FALSE_CMD = {"cmd", "/c", "exit", "1"};
    static const std::vector<std::string> TRUE_CMD = {"cmd", "/c", "exit", "0"};
    static const std::vector<std::string> ENV_CMD = {"cmd", "/c", "echo", "%TEST_VAR%"};
#else
    static const std::vector<std::string> ECHO_CMD = {"echo", "Hello World"};
    static const std::vector<std::string> SLEEP_CMD = {"sleep", "1"};
    static const std::vector<std::string> CAT_CMD = {"cat"};
    static const std::vector<std::string> FALSE_CMD = {"false"};
    static const std::vector<std::string> TRUE_CMD = {"true"};
    static const std::vector<std::string> ENV_CMD = {"sh", "-c", "echo $TEST_VAR"};
#endif

TEST_SUITE("process") {
    TEST_CASE("construction and destruction") {
        SUBCASE("default construction") {
            sdlpp::process proc;
            CHECK_FALSE(proc.is_valid());
            CHECK_FALSE(static_cast<bool>(proc));
            CHECK(proc.get() == nullptr);
        }
        
        SUBCASE("move construction") {
            auto result = sdlpp::process::create(TRUE_CMD);
            if (result) {
                sdlpp::process proc1 = std::move(result.value());
                CHECK(proc1.is_valid());
                sdlpp::process proc2(std::move(proc1));
                CHECK(proc2.is_valid());
                CHECK_FALSE(proc1.is_valid());
            }
        }
        
        SUBCASE("move assignment") {
            auto result1 = sdlpp::process::create(TRUE_CMD);
            auto result2 = sdlpp::process::create(TRUE_CMD);
            
            if (result1 && result2) {
                sdlpp::process proc1 = std::move(result1.value());
                sdlpp::process proc2 = std::move(result2.value());
                
                CHECK(proc1.is_valid());
                CHECK(proc2.is_valid());
                
                proc2 = std::move(proc1);
                CHECK_FALSE(proc1.is_valid());
                CHECK(proc2.is_valid());
            }
        }
    }
    
    TEST_CASE("basic process creation") {
        SUBCASE("create valid process") {
            auto result = sdlpp::process::create(TRUE_CMD);
            REQUIRE(result.has_value());
            CHECK(result.value().is_valid());
            
            auto status = result.value().wait();
            CHECK(status.has_value());
            CHECK(status.value().exited);
            CHECK(status.value().exit_code == 0);
            CHECK(status.value().success());
        }
        
        SUBCASE("create process that exits with error") {
            auto result = sdlpp::process::create(FALSE_CMD);
            if (result) {
                auto status = result.value().wait();
                CHECK(status.has_value());
                CHECK(status.value().exited);
                CHECK(status.value().exit_code != 0);
                CHECK_FALSE(status.value().success());
            }
        }
        
        SUBCASE("create invalid process") {
            auto result = sdlpp::process::create({"this_command_does_not_exist_12345"});
            CHECK_FALSE(result.has_value());
            CHECK(!result.error().empty());
        }
        
        SUBCASE("empty command") {
            auto result = sdlpp::process::create({});
            CHECK_FALSE(result.has_value());
            CHECK(result.error() == "No command specified");
        }
    }
    
    TEST_CASE("process waiting") {
        SUBCASE("wait blocking") {
            auto result = sdlpp::process::create(TRUE_CMD);
            if (result) {
                auto& proc = result.value();
                auto status = proc.wait(true);
                CHECK(status.has_value());
                CHECK(status.value().exited);
                
                // Waiting again should return the same status
                auto status2 = proc.wait(true);
                CHECK(status2.has_value());
                CHECK(status2.value().exited);
            }
        }
        
        SUBCASE("wait non-blocking") {
            auto result = sdlpp::process::create(SLEEP_CMD);
            if (result) {
                auto& proc = result.value();
                
                // Immediately check - should still be running
                auto status = proc.wait(false);
                if (!status.has_value()) {
                    // Process still running, wait a bit
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    
                    // Now wait for completion
                    status = proc.wait(true);
                    CHECK(status.has_value());
                }
            }
        }
    }
    
    TEST_CASE("process termination") {
        SUBCASE("kill process") {
            auto result = sdlpp::process::create(SLEEP_CMD);
            if (result) {
                auto& proc = result.value();
                
                // Kill the process
                bool killed = proc.kill(false);
                CHECK(killed);
                
                // Wait should show it was terminated
                auto status = proc.wait();
                CHECK(status.has_value());
                // On Unix, it would be signaled; on Windows, it exits with error
            }
        }
        
        SUBCASE("force kill process") {
            auto result = sdlpp::process::create(SLEEP_CMD);
            if (result) {
                auto& proc = result.value();
                
                // Force kill the process
                bool killed = proc.kill(true);
                CHECK(killed);
                
                auto status = proc.wait();
                CHECK(status.has_value());
            }
        }
    }
    
    TEST_CASE("process builder") {
        SUBCASE("basic builder usage") {
            auto result = sdlpp::process_builder()
                .set_command("echo")
                .add_arg("test")
                .spawn();
            
            if (result) {
                auto status = result.value().wait();
                CHECK(status.has_value());
                CHECK(status.value().success());
            }
        }
        
        SUBCASE("stdout redirection") {
            auto result = sdlpp::process_builder()
                .set_command(ECHO_CMD)
                .stdout_to_pipe()
                .spawn();
            
            if (result) {
                auto& proc = result.value();
                
                // Read output
                std::string output;
                auto output_result = proc.read_stdout_all();
                if (output_result) {
                    output = output_result.value();
                }
                
                // Wait for completion
                auto status = proc.wait();
                CHECK(status.has_value());
                CHECK(status.value().success());
                
                // Check output contains expected text
                INFO("Output: '" << output << "'");
                INFO("Output length: " << output.length());
                CHECK(output.find("Hello World") != std::string::npos);
            }
        }
        
        SUBCASE("stdin redirection") {
            auto result = sdlpp::process_builder()
                .set_command(CAT_CMD)
                .stdin_from_pipe()
                .stdout_to_pipe()
                .spawn();
            
            if (result) {
                auto& proc = result.value();
                
                // Write to stdin
                const std::string test_input = "Test input line\n";
                auto write_result = proc.write_stdin(test_input);
                CHECK(write_result.has_value());
                
                // Close stdin to signal EOF
                proc.close_stdin();
                
                // Read output
                std::string output;
                auto output_result = proc.read_stdout_all();
                if (output_result) {
                    output = output_result.value();
                }
                
                // Wait for completion
                auto status = proc.wait();
                CHECK(status.has_value());
                CHECK(status.value().success());
                
                // Output should match input
                CHECK(output == test_input);
            }
        }
        
        SUBCASE("null redirection") {
            auto result = sdlpp::process_builder()
                .set_command(ECHO_CMD)
                .stdout_to_null()
                .spawn();
            
            if (result) {
                auto& proc = result.value();
                
                // Should not have stdout stream
                auto stdout = proc.get_stdout();
                CHECK_FALSE(stdout.has_value());
                
                auto status = proc.wait();
                CHECK(status.has_value());
                CHECK(status.value().success());
            }
        }
        
        SUBCASE("stderr to stdout") {
            // This test would need a command that writes to stderr
            // For now, just verify it compiles and runs
            auto result = sdlpp::process_builder()
                .set_command(TRUE_CMD)
                .stderr_to_stdout()
                .spawn();
            
            if (result) {
                auto status = result.value().wait();
                CHECK(status.has_value());
            }
        }
        
        SUBCASE("environment variables") {
            auto result = sdlpp::process_builder()
                .set_command(ENV_CMD)
                .set_env("TEST_VAR", "custom_value")
                .stdout_to_pipe()
                .spawn();
            
            if (result) {
                auto& proc = result.value();
                
                // Read output
                std::string output;
                auto output_result = proc.read_stdout_all();
                if (output_result) {
                    output = output_result.value();
                }
                
                auto status = proc.wait();
                CHECK(status.has_value());
                CHECK(status.value().success());
                
                // Output should contain the custom value
                CHECK(output.find("custom_value") != std::string::npos);
            }
        }
    }
    
    TEST_CASE("convenience functions") {
        SUBCASE("execute") {
            auto result = sdlpp::execute(TRUE_CMD);
            CHECK(result.has_value());
            CHECK(result.value().success());
            
            auto result2 = sdlpp::execute(FALSE_CMD);
            CHECK(result2.has_value());
            CHECK_FALSE(result2.value().success());
        }
        
        SUBCASE("execute_with_output") {
            auto result = sdlpp::execute_with_output(ECHO_CMD);
            CHECK(result.has_value());
            if (result) {
                auto [status, output] = result.value();
                CHECK(status.success());
                INFO("execute_with_output: '" << output << "'");
                INFO("execute_with_output length: " << output.length());
                CHECK(output.find("Hello World") != std::string::npos);
            }
        }
    }
    
    TEST_CASE("error handling") {
        SUBCASE("operations on invalid process") {
            sdlpp::process proc;
            
            auto status = proc.wait();
            CHECK_FALSE(status.has_value());
            
            bool killed = proc.kill();
            CHECK_FALSE(killed);
            
            auto stdin = proc.get_stdin();
            CHECK_FALSE(stdin.has_value());
            
            auto stdout = proc.get_stdout();
            CHECK_FALSE(stdout.has_value());
            
            auto stderr = proc.get_stderr();
            CHECK_FALSE(stderr.has_value());
        }
    }
    
    TEST_CASE("release") {
        SUBCASE("release ownership") {
            auto result = sdlpp::process::create(TRUE_CMD);
            if (result) {
                auto& proc = result.value();
                CHECK(proc.is_valid());
                
                SDL_Process* handle = proc.release();
                CHECK(handle != nullptr);
                CHECK_FALSE(proc.is_valid());
                CHECK(proc.get() == nullptr);
                
                // Manual cleanup
                SDL_WaitProcess(handle, true, nullptr);
                SDL_DestroyProcess(handle);
            }
        }
    }
}
#endif
