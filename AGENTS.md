# AGENTS.md - Starsector Nano Development Guide

## Build Commands
- **Build**: `mkdir build && cd build && cmake .. && cmake --build .`
- **Run**: `./starsector-nano.exe` (Windows) or `./starsector-nano` (Linux)
- **Clean**: `rm -rf build/` or delete build directory
- **IDE Build**: Use CLion/VS Code with CMake integration

## Code Style Guidelines
- **Language**: C (C++17 standard for CMake)
- **Naming**: snake_case for functions/variables, PascalCase for structs
- **Function Prefix**: Module name prefix (e.g., `ShipAPI_Init`, `Vector2f_add`)
- **Headers**: Include guards with `#ifndef MODULE_H` / `#define MODULE_H`
- **Imports**: System headers first, then project headers
- **Error Handling**: Return NULL/error codes, minimal exception usage
- **Memory**: Manual allocation with `new()` and proper cleanup
- **Formatting**: 4-space indentation, braces on same line

## Project Structure
- `src/` - Source files (.cpp/.c)
- `include/` - Header files (.h)
- `assets/` - Game resources (JSON/CSV/images)
- `easyx/` - EasyX graphics library
- `json-3.11.3/` - JSON parsing library

## Testing
- No formal test framework - manual testing via gameplay
- Test new features by running game and verifying behavior
- Check memory leaks with tools like Valgrind

## Dependencies
- EasyX graphics library (Windows only)
- nlohmann/json for JSON parsing
- Standard C/C++ libraries