---
title: Persistent Storage
sidebar_label: Storage
---

# Persistent Storage

SDL++ provides a cross-platform persistent storage API for saving user preferences, game saves, and other application data.

## Storage Containers

### Opening Storage

```cpp
#include <sdlpp/io/storage.hh>

// Open user storage (platform-specific location)
auto storage = sdlpp::storage::open_user_storage("MyCompany", "MyApp");
if (!storage) {
    std::cerr << "Failed to open storage: " << storage.error() << std::endl;
    return;
}

// Storage is automatically saved and closed when destroyed
```

### Storage Paths

```cpp
// Get storage path (for debugging)
std::filesystem::path storage_path = storage->get_path();
std::cout << "Storage location: " << storage_path << std::endl;

// Platform examples:
// Windows: %APPDATA%/MyCompany/MyApp/
// macOS: ~/Library/Application Support/MyCompany/MyApp/
// Linux: ~/.local/share/MyCompany/MyApp/
```

## Reading and Writing Values

### Basic Types

```cpp
// Write values
storage->set_bool("first_run", false);
storage->set_int("high_score", 10000);
storage->set_float("volume", 0.75f);
storage->set_string("player_name", "Alice");

// Read values with defaults
bool first_run = storage->get_bool("first_run", true);
int high_score = storage->get_int("high_score", 0);
float volume = storage->get_float("volume", 1.0f);
std::string name = storage->get_string("player_name", "Player");
```

### Optional Values

```cpp
// Read without defaults (returns optional)
auto score_opt = storage->try_get_int("high_score");
if (score_opt) {
    std::cout << "High score: " << *score_opt << std::endl;
} else {
    std::cout << "No high score set" << std::endl;
}

// Check if key exists
if (storage->has_key("player_name")) {
    auto name = storage->get_string("player_name");
}
```

### Binary Data

```cpp
// Write binary data
std::vector<Uint8> save_data = serialize_game_state();
storage->set_binary("save_game", save_data);

// Read binary data
auto loaded_data = storage->get_binary("save_game");
if (loaded_data) {
    deserialize_game_state(*loaded_data);
}
```

## Structured Data

### JSON-like Storage

```cpp
// Create nested structures
storage->begin_group("graphics");
storage->set_int("resolution_width", 1920);
storage->set_int("resolution_height", 1080);
storage->set_bool("fullscreen", false);
storage->set_string("renderer", "vulkan");
storage->end_group();

// Read from groups
storage->begin_group("graphics");
int width = storage->get_int("resolution_width", 1280);
int height = storage->get_int("resolution_height", 720);
storage->end_group();
```

### Array Storage

```cpp
// Store arrays
std::vector<int> unlocked_levels = {1, 2, 3, 5, 8};
storage->set_int_array("unlocked_levels", unlocked_levels);

// Read arrays
auto levels = storage->get_int_array("unlocked_levels");
for (int level : levels) {
    std::cout << "Unlocked: Level " << level << std::endl;
}

// String arrays
std::vector<std::string> achievements = {
    "first_victory", "speed_run", "no_damage"
};
storage->set_string_array("achievements", achievements);
```

## Enumeration and Management

### Listing Keys

```cpp
// Get all keys
auto keys = storage->get_keys();
for (const auto& key : keys) {
    std::cout << "Key: " << key << std::endl;
}

// Get keys in group
storage->begin_group("player_stats");
auto stat_keys = storage->get_keys();
storage->end_group();
```

### Deleting Data

```cpp
// Delete single key
storage->remove("old_setting");

// Delete entire group
storage->remove_group("temporary_data");

// Clear all storage
storage->clear();

// Check storage size
size_t bytes_used = storage->get_size();
std::cout << "Storage using " << bytes_used << " bytes" << std::endl;
```

## Transactions

### Atomic Operations

```cpp
// Begin transaction for atomic updates
storage->begin_transaction();

try {
    storage->set_int("level", new_level);
    storage->set_int("experience", new_exp);
    storage->set_string("checkpoint", checkpoint_id);
    
    // Commit all changes atomically
    storage->commit();
} catch (...) {
    // Rollback on error
    storage->rollback();
    throw;
}
```

### Batch Operations

```cpp
// Defer writes for performance
storage->begin_batch();

for (const auto& [key, value] : large_dataset) {
    storage->set_string(key, value);
}

// Write all at once
storage->end_batch();
```

## Common Patterns

### Settings Manager

```cpp
class settings_manager {
    sdlpp::storage storage;
    
    // Setting definitions
    struct setting_def {
        std::string key;
        std::string group;
        std::variant<bool, int, float, std::string> default_value;
    };
    
    std::vector<setting_def> definitions = {
        {"master_volume", "audio", 1.0f},
        {"sfx_volume", "audio", 0.8f},
        {"music_volume", "audio", 0.6f},
        {"fullscreen", "video", false},
        {"vsync", "video", true},
        {"resolution", "video", std::string("1920x1080")},
        {"difficulty", "gameplay", 1},
        {"show_tutorials", "gameplay", true}
    };
    
public:
    settings_manager(const std::string& app_name) 
        : storage(sdlpp::storage::open_user_storage("", app_name).value()) {
        
        // Initialize defaults
        for (const auto& def : definitions) {
            if (!storage.has_key(make_key(def))) {
                set_default(def);
            }
        }
    }
    
    template<typename T>
    T get(const std::string& group, const std::string& key) {
        storage.begin_group(group);
        T value;
        
        if constexpr (std::is_same_v<T, bool>) {
            value = storage.get_bool(key);
        } else if constexpr (std::is_same_v<T, int>) {
            value = storage.get_int(key);
        } else if constexpr (std::is_same_v<T, float>) {
            value = storage.get_float(key);
        } else if constexpr (std::is_same_v<T, std::string>) {
            value = storage.get_string(key);
        }
        
        storage.end_group();
        return value;
    }
    
    template<typename T>
    void set(const std::string& group, const std::string& key, T value) {
        storage.begin_group(group);
        
        if constexpr (std::is_same_v<T, bool>) {
            storage.set_bool(key, value);
        } else if constexpr (std::is_same_v<T, int>) {
            storage.set_int(key, value);
        } else if constexpr (std::is_same_v<T, float>) {
            storage.set_float(key, value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            storage.set_string(key, value);
        }
        
        storage.end_group();
        
        // Notify listeners
        notify_change(group, key);
    }
    
    void reset_to_defaults() {
        storage.clear();
        for (const auto& def : definitions) {
            set_default(def);
        }
    }
    
private:
    std::string make_key(const setting_def& def) {
        return def.group + "/" + def.key;
    }
    
    void set_default(const setting_def& def) {
        std::visit([&](auto&& value) {
            set(def.group, def.key, value);
        }, def.default_value);
    }
    
    void notify_change(const std::string& group, const std::string& key) {
        // Implement observer pattern for settings changes
    }
};
```

### Save Game System

```cpp
class save_game_system {
    static constexpr int MAX_SAVE_SLOTS = 3;
    
    struct save_metadata {
        std::string character_name;
        int level;
        int play_time_seconds;
        std::chrono::system_clock::time_point timestamp;
        std::string location;
    };
    
    sdlpp::storage storage;
    
public:
    save_game_system(const std::string& game_name)
        : storage(sdlpp::storage::open_user_storage("", game_name + "_saves").value()) {}
    
    std::vector<std::optional<save_metadata>> get_save_slots() {
        std::vector<std::optional<save_metadata>> slots(MAX_SAVE_SLOTS);
        
        for (int i = 0; i < MAX_SAVE_SLOTS; ++i) {
            std::string slot = "slot_" + std::to_string(i);
            storage.begin_group(slot);
            
            if (storage.has_key("character_name")) {
                save_metadata meta;
                meta.character_name = storage.get_string("character_name");
                meta.level = storage.get_int("level");
                meta.play_time_seconds = storage.get_int("play_time");
                meta.location = storage.get_string("location");
                
                auto timestamp_ms = storage.get_int64("timestamp");
                meta.timestamp = std::chrono::system_clock::time_point(
                    std::chrono::milliseconds(timestamp_ms)
                );
                
                slots[i] = meta;
            }
            
            storage.end_group();
        }
        
        return slots;
    }
    
    bool save_game(int slot, const game_state& state) {
        if (slot < 0 || slot >= MAX_SAVE_SLOTS) return false;
        
        std::string slot_key = "slot_" + std::to_string(slot);
        
        storage.begin_transaction();
        storage.begin_group(slot_key);
        
        try {
            // Save metadata
            storage.set_string("character_name", state.player.name);
            storage.set_int("level", state.player.level);
            storage.set_int("play_time", state.play_time_seconds);
            storage.set_string("location", state.current_location);
            
            auto now = std::chrono::system_clock::now();
            auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count();
            storage.set_int64("timestamp", timestamp_ms);
            
            // Save game data
            auto save_data = serialize_state(state);
            storage.set_binary("game_data", save_data);
            
            storage.end_group();
            storage.commit();
            
            return true;
        } catch (...) {
            storage.rollback();
            return false;
        }
    }
    
    std::optional<game_state> load_game(int slot) {
        if (slot < 0 || slot >= MAX_SAVE_SLOTS) return std::nullopt;
        
        std::string slot_key = "slot_" + std::to_string(slot);
        storage.begin_group(slot_key);
        
        auto save_data = storage.get_binary("game_data");
        storage.end_group();
        
        if (!save_data) return std::nullopt;
        
        return deserialize_state(*save_data);
    }
    
    bool delete_save(int slot) {
        if (slot < 0 || slot >= MAX_SAVE_SLOTS) return false;
        
        std::string slot_key = "slot_" + std::to_string(slot);
        storage.remove_group(slot_key);
        
        return true;
    }
};
```

### User Profiles

```cpp
class user_profile_manager {
    sdlpp::storage storage;
    std::string current_profile;
    
public:
    user_profile_manager()
        : storage(sdlpp::storage::open_user_storage("", "profiles").value()) {
        
        // Load last active profile
        current_profile = storage.get_string("last_profile", "default");
        ensure_profile_exists(current_profile);
    }
    
    std::vector<std::string> list_profiles() {
        storage.begin_group("profiles");
        auto groups = storage.get_groups();
        storage.end_group();
        return groups;
    }
    
    bool create_profile(const std::string& name) {
        if (profile_exists(name)) return false;
        
        storage.begin_group("profiles/" + name);
        storage.set_string("created", current_timestamp());
        storage.set_int("play_time", 0);
        storage.end_group();
        
        return true;
    }
    
    bool switch_profile(const std::string& name) {
        if (!profile_exists(name)) return false;
        
        current_profile = name;
        storage.set_string("last_profile", name);
        
        return true;
    }
    
    template<typename T>
    T get_profile_data(const std::string& key) {
        storage.begin_group("profiles/" + current_profile);
        T value;
        
        if constexpr (std::is_same_v<T, int>) {
            value = storage.get_int(key);
        } else if constexpr (std::is_same_v<T, std::string>) {
            value = storage.get_string(key);
        }
        // ... other types
        
        storage.end_group();
        return value;
    }
    
    template<typename T>
    void set_profile_data(const std::string& key, T value) {
        storage.begin_group("profiles/" + current_profile);
        
        if constexpr (std::is_same_v<T, int>) {
            storage.set_int(key, value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            storage.set_string(key, value);
        }
        // ... other types
        
        storage.end_group();
    }
    
private:
    bool profile_exists(const std::string& name) {
        storage.begin_group("profiles");
        auto groups = storage.get_groups();
        storage.end_group();
        
        return std::find(groups.begin(), groups.end(), name) != groups.end();
    }
    
    void ensure_profile_exists(const std::string& name) {
        if (!profile_exists(name)) {
            create_profile(name);
        }
    }
    
    std::string current_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};
```

## Migration and Versioning

```cpp
class storage_migrator {
    sdlpp::storage& storage;
    
public:
    explicit storage_migrator(sdlpp::storage& s) : storage(s) {}
    
    void migrate() {
        int version = storage.get_int("schema_version", 0);
        
        while (version < CURRENT_VERSION) {
            switch (version) {
                case 0:
                    migrate_v0_to_v1();
                    break;
                case 1:
                    migrate_v1_to_v2();
                    break;
                // ... more migrations
            }
            version++;
        }
        
        storage.set_int("schema_version", CURRENT_VERSION);
    }
    
private:
    static constexpr int CURRENT_VERSION = 2;
    
    void migrate_v0_to_v1() {
        // Migrate from flat structure to groups
        if (storage.has_key("volume")) {
            float volume = storage.get_float("volume");
            storage.remove("volume");
            
            storage.begin_group("audio");
            storage.set_float("master_volume", volume);
            storage.end_group();
        }
    }
    
    void migrate_v1_to_v2() {
        // Add new fields with defaults
        storage.begin_group("graphics");
        if (!storage.has_key("anti_aliasing")) {
            storage.set_string("anti_aliasing", "MSAA_4x");
        }
        storage.end_group();
    }
};
```

## Best Practices

1. **Use Groups**: Organize related settings into groups
2. **Provide Defaults**: Always have sensible default values
3. **Validate Input**: Check values before storing
4. **Version Your Schema**: Plan for future migrations
5. **Handle Errors**: Storage operations can fail

## Example: Complete Game Storage

```cpp
class game_storage_manager {
    struct storage_stats {
        size_t total_bytes = 0;
        size_t settings_bytes = 0;
        size_t saves_bytes = 0;
        size_t cache_bytes = 0;
    };
    
    std::unique_ptr<sdlpp::storage> settings_storage;
    std::unique_ptr<sdlpp::storage> saves_storage;
    std::unique_ptr<sdlpp::storage> cache_storage;
    
public:
    bool initialize(const std::string& game_name) {
        // Open different storage containers
        auto settings = sdlpp::storage::open_user_storage("", game_name + "_settings");
        auto saves = sdlpp::storage::open_user_storage("", game_name + "_saves");
        auto cache = sdlpp::storage::open_user_storage("", game_name + "_cache");
        
        if (!settings || !saves || !cache) {
            return false;
        }
        
        settings_storage = std::make_unique<sdlpp::storage>(std::move(*settings));
        saves_storage = std::make_unique<sdlpp::storage>(std::move(*saves));
        cache_storage = std::make_unique<sdlpp::storage>(std::move(*cache));
        
        // Migrate if needed
        storage_migrator migrator(*settings_storage);
        migrator.migrate();
        
        // Clean old cache
        clean_cache();
        
        return true;
    }
    
    // Settings access
    template<typename T>
    T get_setting(const std::string& group, const std::string& key, T default_value) {
        settings_storage->begin_group(group);
        T value;
        
        if constexpr (std::is_same_v<T, bool>) {
            value = settings_storage->get_bool(key, default_value);
        } else if constexpr (std::is_same_v<T, int>) {
            value = settings_storage->get_int(key, default_value);
        } else if constexpr (std::is_same_v<T, float>) {
            value = settings_storage->get_float(key, default_value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            value = settings_storage->get_string(key, default_value);
        }
        
        settings_storage->end_group();
        return value;
    }
    
    // Cache management
    void cache_data(const std::string& key, const std::vector<Uint8>& data) {
        cache_storage->begin_group("cache_entries");
        cache_storage->set_binary(key, data);
        cache_storage->set_int64(key + "_timestamp", 
            std::chrono::system_clock::now().time_since_epoch().count());
        cache_storage->end_group();
    }
    
    std::optional<std::vector<Uint8>> get_cached_data(const std::string& key,
                                                     std::chrono::hours max_age) {
        cache_storage->begin_group("cache_entries");
        
        auto timestamp = cache_storage->try_get_int64(key + "_timestamp");
        if (!timestamp) {
            cache_storage->end_group();
            return std::nullopt;
        }
        
        auto age = std::chrono::system_clock::now() - 
                   std::chrono::system_clock::time_point(
                       std::chrono::nanoseconds(*timestamp));
                       
        if (age > max_age) {
            // Cache expired
            cache_storage->remove(key);
            cache_storage->remove(key + "_timestamp");
            cache_storage->end_group();
            return std::nullopt;
        }
        
        auto data = cache_storage->get_binary(key);
        cache_storage->end_group();
        
        return data;
    }
    
    // Storage information
    storage_stats get_storage_stats() {
        storage_stats stats;
        stats.settings_bytes = settings_storage->get_size();
        stats.saves_bytes = saves_storage->get_size();
        stats.cache_bytes = cache_storage->get_size();
        stats.total_bytes = stats.settings_bytes + 
                           stats.saves_bytes + 
                           stats.cache_bytes;
        return stats;
    }
    
    void clean_cache() {
        cache_storage->begin_group("cache_entries");
        auto keys = cache_storage->get_keys();
        
        auto now = std::chrono::system_clock::now();
        
        for (const auto& key : keys) {
            if (key.ends_with("_timestamp")) continue;
            
            auto timestamp = cache_storage->try_get_int64(key + "_timestamp");
            if (!timestamp) continue;
            
            auto age = now - std::chrono::system_clock::time_point(
                std::chrono::nanoseconds(*timestamp));
                
            if (age > std::chrono::hours(24 * 7)) {  // 1 week
                cache_storage->remove(key);
                cache_storage->remove(key + "_timestamp");
            }
        }
        
        cache_storage->end_group();
    }
    
    // Export/Import for cloud sync
    std::vector<Uint8> export_settings() {
        return settings_storage->export_all();
    }
    
    bool import_settings(const std::vector<Uint8>& data) {
        return settings_storage->import_all(data);
    }
};
```