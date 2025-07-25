//
// Example: SDL++ Properties System Usage
//

#include <iostream>
#include <thread>
#include "sdlpp/core/core.hh"
#include "sdlpp/config/properties.hh"
#include "sdlpp/core/error.hh"

using namespace sdlpp;

// Example 1: Basic property usage
void example_basic_properties() {
    std::cout << "\n=== Basic Properties Example ===" << std::endl;
    
    auto props = properties::create();
    if (!props) {
        std::cerr << "Failed to create properties: " << props.error() << std::endl;
        return;
    }
    
    // Set various property types
    (void)props->set("player.name", "Hero");
    (void)props->set("player.level", 10);
    (void)props->set("player.health", 100.0f);
    (void)props->set("player.alive", true);
    (void)props->set("player.inventory.size", 20);
    
    // Read properties
    std::cout << "Player: " << props->get_string("player.name") << std::endl;
    std::cout << "Level: " << props->get_number("player.level") << std::endl;
    std::cout << "Health: " << props->get_float("player.health") << std::endl;
    std::cout << "Alive: " << (props->get_boolean("player.alive") ? "Yes" : "No") << std::endl;
    std::cout << "Inventory size: " << props->get_number("player.inventory.size") << std::endl;
}

// Example 2: Property builder pattern
void example_property_builder() {
    std::cout << "\n=== Property Builder Example ===" << std::endl;
    
    auto game_config = property_builder()
        .add("game.title", "My Awesome Game")
        .add("game.version", "1.0.0")
        .add("graphics.resolution.width", 1920)
        .add("graphics.resolution.height", 1080)
        .add("graphics.fullscreen", false)
        .add("graphics.vsync", true)
        .add("audio.master_volume", 0.8f)
        .add("audio.music_volume", 0.6f)
        .add("audio.sfx_volume", 0.7f)
        .add("controls.mouse_sensitivity", 1.5f)
        .build();
    
    if (!game_config) {
        std::cerr << "Failed to build config: " << game_config.error() << std::endl;
        return;
    }
    
    std::cout << "Game: " << game_config->get_string("game.title") 
              << " v" << game_config->get_string("game.version") << std::endl;
    std::cout << "Resolution: " << game_config->get_number("graphics.resolution.width") 
              << "x" << game_config->get_number("graphics.resolution.height") << std::endl;
    std::cout << "Fullscreen: " << (game_config->get_boolean("graphics.fullscreen") ? "Yes" : "No") << std::endl;
}

// Example 3: Type-safe property accessors
void example_property_accessors() {
    std::cout << "\n=== Property Accessors Example ===" << std::endl;
    
    auto props = properties::create();
    if (!props) return;
    
    // Create type-safe accessors
    property_accessor<std::string> username(*props, "user.name", "Guest");
    property_accessor<int> highscore(*props, "user.highscore", 0);
    property_accessor<float> playtime(*props, "user.playtime", 0.0f);
    property_accessor<bool> premium(*props, "user.premium", false);
    
    // Use like variables
    username = "PlayerOne";
    highscore = 15000;
    playtime = 45.5f;
    premium = true;
    
    std::cout << "User: " << static_cast<std::string>(username) << std::endl;
    std::cout << "High Score: " << static_cast<int>(highscore) << std::endl;
    std::cout << "Play Time: " << static_cast<float>(playtime) << " hours" << std::endl;
    std::cout << "Premium: " << (static_cast<bool>(premium) ? "Yes" : "No") << std::endl;
    
    // Check existence
    if (username.exists()) {
        std::cout << "Username is set" << std::endl;
    }
}

// Example 4: Managed resources with cleanup
class Resource {
    std::string name;
public:
    explicit Resource(const std::string& n) : name(n) {
        std::cout << "Resource '" << name << "' created" << std::endl;
    }
    ~Resource() {
        std::cout << "Resource '" << name << "' destroyed" << std::endl;
    }
    const std::string& get_name() const { return name; }
};

void example_managed_resources() {
    std::cout << "\n=== Managed Resources Example ===" << std::endl;
    
    auto props = properties::create();
    if (!props) return;
    
    // Create a resource and let properties manage it
    auto* resource = new Resource("Texture");
    
    (void)props->set_pointer_with_cleanup("game.texture", resource,
        []([[maybe_unused]] void* userdata, void* value) {
            std::cout << "Cleanup callback called" << std::endl;
            delete static_cast<Resource*>(value);
        });
    
    // Use the resource
    auto* retrieved = static_cast<Resource*>(props->get_pointer("game.texture"));
    if (retrieved) {
        std::cout << "Using resource: " << retrieved->get_name() << std::endl;
    }
    
    // Clear triggers cleanup
    std::cout << "Clearing property..." << std::endl;
    (void)props->clear("game.texture");
}

// Example 5: Thread-safe property access
void example_thread_safety() {
    std::cout << "\n=== Thread Safety Example ===" << std::endl;
    
    auto props = properties::create();
    if (!props) return;
    
    (void)props->set_number("counter", 0);
    
    // Simulate concurrent access
    auto increment = [&props]() {
        for (int i = 0; i < 5; ++i) {
            properties::lock_guard lock(*props);
            auto current = props->get_number("counter");
            (void)props->set_number("counter", current + 1);
            std::cout << "Thread incremented to: " << current + 1 << std::endl;
        }
    };
    
    std::thread t1(increment);
    std::thread t2(increment);
    
    t1.join();
    t2.join();
    
    std::cout << "Final counter value: " << props->get_number("counter") << std::endl;
}

// Example 6: Property enumeration
void example_enumeration() {
    std::cout << "\n=== Property Enumeration Example ===" << std::endl;
    
    auto props = properties::create();
    if (!props) return;
    
    // Add some properties
    (void)props->set("config.app_name", "MyApp");
    (void)props->set("config.version", "2.0");
    (void)props->set("config.debug", true);
    (void)props->set("config.max_users", 100);
    (void)props->set("config.timeout", 30.0f);
    
    // Enumerate all properties
    std::cout << "All properties:" << std::endl;
    props->enumerate([&props](const std::string& name) {
        auto type = props->get_type(name);
        std::cout << "  " << name << " (";
        
        switch (type) {
            case static_cast<SDL_PropertyType>(1): // STRING
                std::cout << "string): " << props->get_string(name);
                break;
            case static_cast<SDL_PropertyType>(2): // NUMBER
                std::cout << "number): " << props->get_number(name);
                break;
            case static_cast<SDL_PropertyType>(3): // FLOAT
                std::cout << "float): " << props->get_float(name);
                break;
            case static_cast<SDL_PropertyType>(4): // BOOLEAN
                std::cout << "boolean): " << (props->get_boolean(name) ? "true" : "false");
                break;
            default:
                std::cout << "unknown)";
        }
        std::cout << std::endl;
    });
    
    // Get all property names
    auto names = props->get_names();
    std::cout << "\nTotal properties: " << names.size() << std::endl;
}

// Example 7: Global properties
void example_global_properties() {
    std::cout << "\n=== Global Properties Example ===" << std::endl;
    
    // Access global properties (shared across application)
    auto global = properties::get_global();
    
    // Set some global config
    (void)global.set_string("app.vendor", "My Company");
    (void)global.set_string("app.copyright", "2025");
    (void)global.set_number("app.build", 12345);
    
    std::cout << "App vendor: " << global.get_string("app.vendor") << std::endl;
    std::cout << "Copyright: " << global.get_string("app.copyright") << std::endl;
    std::cout << "Build number: " << global.get_number("app.build") << std::endl;
    
    // Clean up global properties we added
    (void)global.clear("app.vendor");
    (void)global.clear("app.copyright");
    (void)global.clear("app.build");
}

// Example 8: Using properties for game state
struct GameState {
    properties props;
    
    GameState() {
        auto result = properties::create();
        if (result) {
            props = std::move(*result);
        }
    }
    
    void save_checkpoint() {
        // In a real game, you'd serialize these to disk
        std::cout << "\nSaving checkpoint:" << std::endl;
        props.enumerate([this](const std::string& name) {
            std::cout << "  Saving: " << name << std::endl;
        });
    }
    
    void load_checkpoint() {
        // In a real game, you'd deserialize from disk
        std::cout << "\nLoading checkpoint..." << std::endl;
    }
};

void example_game_state() {
    std::cout << "\n=== Game State Example ===" << std::endl;
    
    GameState state;
    
    // Set game state
    (void)state.props.set("level.current", 5);
    (void)state.props.set("level.name", "Crystal Caverns");
    (void)state.props.set("player.position.x", 128.5f);
    (void)state.props.set("player.position.y", 256.0f);
    (void)state.props.set("player.health", 75.0f);
    (void)state.props.set("player.mana", 50.0f);
    (void)state.props.set("inventory.gold", 1500);
    (void)state.props.set("flags.boss_defeated", true);
    (void)state.props.set("flags.secret_found", false);
    
    // Use type-safe accessors for frequently accessed properties
    property_accessor<float> player_x(state.props, "player.position.x");
    property_accessor<float> player_y(state.props, "player.position.y");
    
    // Update position
    player_x = player_x + 10.0f;
    player_y = player_y - 5.0f;
    
    std::cout << "Player position: (" << static_cast<float>(player_x) 
              << ", " << static_cast<float>(player_y) << ")" << std::endl;
    
    // Save checkpoint
    state.save_checkpoint();
}

int main() {
    try {
        // Initialize SDL (try without any subsystems first)
        sdlpp::init init(sdlpp::init_flags::none);
        
        if (!init.is_initialized()) {
            std::cerr << "Warning: Failed to initialize SDL" << std::endl;
            std::cerr << "Continuing without SDL initialization..." << std::endl;
            // Properties can work without SDL_Init in SDL3
        }
    
    std::cout << "=== SDL++ Properties System Examples ===" << std::endl;
    
    // Run examples
    example_basic_properties();
    example_property_builder();
    example_property_accessors();
    example_managed_resources();
    example_thread_safety();
    example_enumeration();
    example_global_properties();
    example_game_state();
    
    std::cout << "\n✅ All property examples completed!" << std::endl;
    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}