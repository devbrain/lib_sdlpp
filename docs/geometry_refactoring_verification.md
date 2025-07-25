# Geometry Refactoring Verification Strategy

## Overview

This document outlines how to ensure all SDL++ components are properly updated to use the new geometry concept-based system.

## Verification Checklist

### 1. Compile-Time Verification

#### Test 1: Build without Built-in Types
```bash
# Create a test that includes all headers with SDLPP_NO_BUILTIN_GEOMETRY defined
cat > test_all_headers_no_builtin.cc << 'EOF'
#define SDLPP_NO_BUILTIN_GEOMETRY

// Include all SDL++ headers
#include <sdlpp/core/sdl.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/video/texture.hh>
#include <sdlpp/video/display.hh>
#include <sdlpp/input/mouse.hh>
#include <sdlpp/input/keyboard.hh>
#include <sdlpp/input/touch.hh>
// ... include all headers

int main() { return 0; }
EOF

# This should compile successfully
g++ -std=c++20 -I include test_all_headers_no_builtin.cc
```

#### Test 2: Verify No Direct SDL Type Usage
```bash
# Search for direct SDL geometry type usage (should only be in conversion functions)
grep -r "SDL_Point\|SDL_Rect\|SDL_FPoint\|SDL_FRect" include/sdlpp/ \
    --exclude="*concepts.hh" \
    --exclude="*sdl.hh"
```

#### Test 3: Concept Satisfaction Tests
Create a test file that verifies all geometry-using functions work with custom types:

```cpp
// test_custom_geometry_integration.cc
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>

struct MyPoint { using value_type = int; int x, y; };
struct MySize { using value_type = int; int width, height; };
struct MyRect { using value_type = int; int x, y, w, h; };

void test_window() {
    // These should all compile
    auto window = sdlpp::window::create("Test", MySize{800, 600});
    window->set_position(MyPoint{100, 100});
    auto pos = window->get_position<MyPoint>();
    auto size = window->get_size<MySize>();
}

void test_renderer() {
    // Renderer methods should accept custom types
    sdlpp::renderer r;
    r.draw_point(MyPoint{10, 20});
    r.draw_rect(MyRect{0, 0, 100, 100});
}
```

### 2. Runtime Verification

#### Test Suite 1: Cross-Type Compatibility
```cpp
// Verify different geometry types can interact
void test_mixed_types() {
    sdlpp::point_f fp{10.5f, 20.5f};
    glm::vec2 gp{30.0f, 40.0f};
    
    // Should work with any point-like types
    auto dist = sdlpp::distance(fp, gp);
    
    // Algorithms should work across types
    auto mid = sdlpp::lerp(fp, gp, 0.5);
}
```

#### Test Suite 2: SDL Conversion Functions
```cpp
// Verify SDL conversion helpers work
void test_sdl_conversions() {
    MyPoint p{10, 20};
    MyRect r{0, 0, 100, 100};
    
    // These conversions should be available globally
    SDL_Point sp = sdlpp::to_sdl_point(p);
    SDL_Rect sr = sdlpp::to_sdl_rect(r);
}
```

### 3. Component Update Tracking

Create a tracking table for each component:

| Component | Header | Status | Tests | Notes |
|-----------|--------|--------|-------|-------|
| Window | video/window.hh | ✓ Updated | ✓ Pass | Uses concepts |
| Renderer | video/renderer.hh | ⚠️ Partial | ✗ Need update | Some methods still use SDL types |
| Surface | video/surface.hh | ✗ Not started | - | - |
| Display | video/display.hh | ✗ Not started | - | Still uses concrete types |
| Mouse | input/mouse.hh | ✓ Updated | ✓ Pass | - |
| Keyboard | input/keyboard.hh | ⚠️ Partial | - | Uses SDL_Rect directly |
| Touch | input/touch.hh | ✗ Not started | - | Uses concrete point<float> |

### 4. Automated Verification Script

```python
#!/usr/bin/env python3
# verify_geometry_update.py

import os
import re
import subprocess

def check_header(filepath):
    """Check if a header is properly updated"""
    issues = []
    
    with open(filepath, 'r') as f:
        content = f.read()
        
    # Check 1: Should not include old geometry.hh
    if '#include <sdlpp/utility/geometry.hh>' in content and 'geometry_concepts' not in filepath:
        issues.append("Still includes old geometry.hh")
    
    # Check 2: Should not use concrete types in public API
    if re.search(r'\bsdlpp::(point|rect|size)\b(?!_like)', content):
        issues.append("Uses concrete geometry types in API")
    
    # Check 3: Should use concepts for template parameters
    if 'template' in content and not re.search(r'(point_like|size_like|rect_like)', content):
        if any(word in content for word in ['point', 'rect', 'size', 'position']):
            issues.append("May need concept constraints")
    
    # Check 4: Direct SDL type usage
    sdl_types = re.findall(r'\b(SDL_Point|SDL_Rect|SDL_FPoint|SDL_FRect)\b', content)
    if sdl_types and 'to_sdl' not in content:
        issues.append(f"Direct SDL type usage: {set(sdl_types)}")
    
    return issues

def verify_all_headers():
    """Verify all headers are updated"""
    include_dir = 'include/sdlpp'
    results = {}
    
    for root, dirs, files in os.walk(include_dir):
        for file in files:
            if file.endswith('.hh'):
                filepath = os.path.join(root, file)
                issues = check_header(filepath)
                if issues:
                    results[filepath] = issues
    
    return results

def compile_test_no_builtin():
    """Test compilation without built-in types"""
    test_code = '''
#define SDLPP_NO_BUILTIN_GEOMETRY
#include <sdlpp/utility/geometry.hh>
int main() { return 0; }
'''
    
    with open('/tmp/test_no_builtin.cc', 'w') as f:
        f.write(test_code)
    
    result = subprocess.run(
        ['g++', '-std=c++20', '-I', 'include', '/tmp/test_no_builtin.cc'],
        capture_output=True
    )
    
    return result.returncode == 0

if __name__ == '__main__':
    print("=== Geometry Update Verification ===\n")
    
    # Test 1: Check headers
    print("1. Checking headers for issues...")
    issues = verify_all_headers()
    if issues:
        print(f"Found issues in {len(issues)} files:")
        for file, problems in issues.items():
            print(f"\n{file}:")
            for p in problems:
                print(f"  - {p}")
    else:
        print("All headers look good!")
    
    # Test 2: Compile without built-in types
    print("\n2. Testing compilation without built-in types...")
    if compile_test_no_builtin():
        print("✓ Compiles successfully without built-in types")
    else:
        print("✗ Compilation failed without built-in types")
    
    # Summary
    total_files = sum(1 for root, dirs, files in os.walk('include/sdlpp') 
                     for f in files if f.endswith('.hh'))
    updated_files = total_files - len(issues)
    
    print(f"\n=== Summary ===")
    print(f"Total headers: {total_files}")
    print(f"Updated: {updated_files}")
    print(f"Need attention: {len(issues)}")
    print(f"Progress: {updated_files/total_files*100:.1f}%")
```

### 5. Integration Test Suite

Create a comprehensive test that uses all components with custom geometry:

```cpp
// test/integration/test_geometry_complete.cc
#define SDLPP_NO_BUILTIN_GEOMETRY
#include <sdlpp/sdlpp.hh>  // Include everything

// Custom geometry types
namespace custom {
    struct point { using value_type = float; float x, y; };
    struct size { using value_type = int; int width, height; };
    struct rect { using value_type = double; double x, y, w, h; };
}

void test_complete_integration() {
    // Initialize SDL
    sdlpp::init(sdlpp::init_flags::video);
    
    // Create window with custom size
    auto window = sdlpp::window::create("Test", custom::size{800, 600});
    
    // Set position with custom point
    window->set_position(custom::point{100.0f, 100.0f});
    
    // Create renderer
    auto renderer = sdlpp::renderer::create(*window);
    
    // Draw with custom types
    renderer->draw_rect(custom::rect{10, 10, 50, 50});
    
    // Mouse position as custom type
    auto mouse_pos = sdlpp::mouse::get_position<custom::point>();
    
    // All components work together
    assert(true);
}
```

### 6. Continuous Verification

Add these checks to CI/CD:

```yaml
# .github/workflows/geometry-verification.yml
name: Geometry System Verification

on: [push, pull_request]

jobs:
  verify-geometry:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    
    - name: Verify headers updated
      run: python3 verify_geometry_update.py
    
    - name: Test without built-in types
      run: |
        g++ -std=c++20 -DSDLPP_NO_BUILTIN_GEOMETRY \
            -I include test/integration/test_geometry_complete.cc
    
    - name: Test with GLM
      run: |
        g++ -std=c++20 -I include -I /usr/include/glm \
            test/integration/test_geometry_glm.cc
    
    - name: Check for direct SDL usage
      run: |
        ! grep -r "SDL_\(Point\|Rect\|FPoint\|FRect\)" include/sdlpp/ \
            --exclude="*concepts.hh" \
            --exclude="*sdl.hh" \
            --exclude-dir="detail"
```

## Success Criteria

The geometry refactoring is complete when:

1. ✓ All headers compile with `SDLPP_NO_BUILTIN_GEOMETRY`
2. ✓ No public APIs use concrete geometry types
3. ✓ All geometry parameters use concepts
4. ✓ Integration tests pass with custom types
5. ✓ Examples work with external libraries (GLM, Eigen)
6. ✓ No performance regressions
7. ✓ Documentation is updated
8. ✓ Migration guide is complete

## Red Flags to Watch For

1. **Implicit Conversions**: Watch for code that assumes specific geometry types
2. **Member Access**: Code that accesses `.x`, `.y` directly without concepts
3. **SDL Type Leakage**: SDL types appearing in public APIs
4. **Missing Constraints**: Template functions without concept constraints
5. **Broken Examples**: Example code that no longer compiles

## Rollback Plan

If issues are discovered:

1. The old geometry system is preserved in `geometry_legacy.hh`
2. Can temporarily add `using` declarations to maintain compatibility
3. Can phase the migration over multiple releases
4. Each component can be reverted independently