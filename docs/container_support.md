# Container Support in String Utilities

The `sdlpp::detail::append_to_stream` and `sdlpp::detail::build_message` functions now have comprehensive support for STL containers, with both automatic formatting and customizable formatter options.

## Automatic Container Detection

The system automatically detects and formats the following container types:

### Sequential Containers (use `[]` brackets)
- `std::vector`
- `std::array`
- `std::list`
- `std::deque`
- `std::forward_list`

### Associative Containers (use `{}` braces)
- `std::set` / `std::multiset`
- `std::unordered_set` / `std::unordered_multiset`

### Map Containers (use `{}` braces with key: value pairs)
- `std::map` / `std::multimap`
- `std::unordered_map` / `std::unordered_multimap`

## Basic Usage

```cpp
#include <sdlpp/log.hh>
#include <vector>
#include <set>
#include <map>

// Sequential containers
std::vector<int> vec = {1, 2, 3, 4, 5};
SDLPP_LOG_APP("Vector:", vec);
// Output: Vector: [1, 2, 3, 4, 5]

std::array<double, 3> arr = {1.1, 2.2, 3.3};
SDLPP_LOG_APP("Array:", arr);
// Output: Array: [1.1, 2.2, 3.3]

// Set containers
std::set<int> s = {5, 2, 8, 1};
SDLPP_LOG_APP("Set:", s);
// Output: Set: {1, 2, 5, 8}

// Map containers
std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}};
SDLPP_LOG_APP("Scores:", scores);
// Output: Scores: {Alice: 95, Bob: 87}
```

## Container Formatter

For more control over container output, use the `container()` formatter:

### Basic Limiting

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Limit to first N items
SDLPP_LOG_APP("Limited:", container(vec, 5));
// Output: Limited: [1, 2, 3, 4, 5, ...]

// Show all items (default)
SDLPP_LOG_APP("All:", container(vec));
// Output: All: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
```

### Custom Formatting Options

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Using lambda to configure options
auto custom = container(vec, [](auto& fmt) {
    fmt.max_items = 3;         // Show only 3 items
    fmt.show_indices = true;   // Show indices
    fmt.prefix = "{";          // Custom prefix
    fmt.suffix = "}";          // Custom suffix
    fmt.delimiter = "; ";      // Custom delimiter
    fmt.ellipsis = "...more";  // Custom ellipsis
});

SDLPP_LOG_APP("Custom:", custom);
// Output: Custom: {[0]: 1; [1]: 2; [2]: 3; ...more}
```

### Starting from Specific Index

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

auto from_middle = container(vec, [](auto& fmt) {
    fmt.start_index = 5;  // Start from index 5
    fmt.max_items = 3;    // Show 3 items
});

SDLPP_LOG_APP("Middle:", from_middle);
// Output: Middle: [6, 7, 8, ...]
```

### Multiline Format

```cpp
std::map<std::string, double> data = {
    {"temperature", 23.5},
    {"humidity", 65.2},
    {"pressure", 1013.25}
};

auto multiline = container(data, [](auto& fmt) {
    fmt.multiline = true;
    fmt.indent = "    ";
    fmt.prefix = "Sensors {\n";
    fmt.suffix = "}";
});

SDLPP_LOG_APP("\n", multiline);
// Output:
// Sensors {
//     temperature: 23.5, 
//     humidity: 65.2, 
//     pressure: 1013.25
// }
```

## Container Formatter Options

The `container_format` struct provides the following customization options:

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `max_items` | `size_t` | unlimited | Maximum number of items to display |
| `start_index` | `size_t` | 0 | Starting index (0-based) |
| `prefix` | `string_view` | "[" | Container opening delimiter |
| `suffix` | `string_view` | "]" | Container closing delimiter |
| `delimiter` | `string_view` | ", " | Separator between items |
| `ellipsis` | `string_view` | "..." | Text shown when truncated |
| `show_indices` | `bool` | false | Show item indices |
| `multiline` | `bool` | false | Use multiline format |
| `indent` | `string_view` | "  " | Indentation for multiline |

## Nested Containers

Containers can be nested arbitrarily deep:

```cpp
// Matrix (vector of vectors)
std::vector<std::vector<int>> matrix = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};
SDLPP_LOG_APP("Matrix:", matrix);
// Output: Matrix: [[1, 2, 3], [4, 5, 6], [7, 8, 9]]

// Map with vector values
std::map<std::string, std::vector<int>> grouped = {
    {"evens", {2, 4, 6}},
    {"odds", {1, 3, 5}}
};
SDLPP_LOG_APP("Groups:", grouped);
// Output: Groups: {evens: [2, 4, 6], odds: [1, 3, 5]}
```

## Containers with Special Types

Containers work seamlessly with all special types:

```cpp
// Container of optionals
std::vector<std::optional<int>> opts = {42, std::nullopt, 100};
SDLPP_LOG_APP("Optionals:", opts);
// Output: Optionals: [42, nullopt, 100]

// Container of pairs
std::vector<std::pair<int, std::string>> pairs = {
    {1, "first"}, {2, "second"}
};
SDLPP_LOG_APP("Pairs:", pairs);
// Output: Pairs: [(1, first), (2, second)]

// Container of filesystem paths
std::vector<std::filesystem::path> paths = {
    "/home/user", "/tmp", "/var/log"
};
SDLPP_LOG_APP("Paths:", paths);
// Output: Paths: [/home/user, /tmp, /var/log]

// Container with formatters
std::vector<int> hex_values = {255, 128, 64};
// Note: Need to manually format each element
```

## Performance Considerations

- The automatic container detection uses compile-time concepts with zero runtime overhead
- The `container()` formatter creates a lightweight wrapper object
- Large containers are efficiently handled with the `max_items` limit
- No unnecessary copies are made - containers are passed by reference

## Integration with Error Messages

```cpp
std::vector<int> invalid_indices = {-1, 1000, 2000};
sdlpp::set_error("Invalid indices:", invalid_indices, "out of range");

std::map<std::string, std::string> config_errors = {
    {"host", "invalid hostname"},
    {"port", "must be positive"}
};
sdlpp::set_error("Configuration errors:", container(config_errors, 2));
```

## Implementation Details

The container support is implemented using C++20 concepts:

- `has_begin_end` - Detects types with begin(), end(), and size()
- `container_like` - Filters out strings and tuples
- `map_like` - Detects map types with key_type and mapped_type
- `set_like` - Detects set types with key_type but no mapped_type

The system automatically chooses the appropriate formatting:
- Sequential containers → `[item1, item2, ...]`
- Sets → `{item1, item2, ...}`
- Maps → `{key1: value1, key2: value2, ...}`