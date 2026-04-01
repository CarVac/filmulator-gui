# Filmulator Modernization Implementation Plan

This plan outlines the steps to modernize the Filmulator codebase, improve code quality, and enable future enhancements.

## User Review Required

> [!IMPORTANT]
> **Qt6 Migration**: This will be a breaking change for the build environment. Ensure all dev environments are updated to support Qt6.
> **Modern C++**: We will target C++20. Ensure your compilers support this.
> **GPGPU Strategy**: Halide is proposed. This introduces a new dependency and build step.

## Proposed Changes Order

### 1. CI/CD Setup
- **Goal**: Replace legacy Travis/Azure pipelines with GitHub Actions.
- **Actions**:
  - Create `.github/workflows/cmake.yml`.
  - Configure jobs for:
    - Ubuntu (GCC/Clang)
    - Windows (MSVC)
    - macOS (Clang)
  - Integrate `vcpkg` caching in GitHub Actions to speed up builds.

### 2. Testing Framework
- **Goal**: Ensure reliability with regression testing.
- **Actions**:
  - Add `Catch2` or `GTest` via `vcpkg`.
  - Create a new `tests/` directory.
  - Create a `tests/CMakeLists.txt` to register tests.
  - Implement unit tests for:
    - `ColorSpaces`
    - `Curves`
    - Basic `ImagePipeline` functionality.

### 3. Code Quality & Tooling
- **Goal**: Maintain code health and prevent regressions.
- **Actions**:
  - **Formatting**: Add `.clang-format` (Google or LLVM style) and run it across the codebase.
  - **Static Analysis**: Add `.clang-tidy` with checks for modern C++, performance, and readability.
  - **Sanitizers**: Configure CMake to easily enable ASan and UBSan (`-fsanitize=address,undefined`) in Debug builds.
  - **Git Hooks**: Add a `.pre-commit-config.yaml` to run formatting and linting before commits.

### 4. Modern C++ & Warnings
- **Goal**: Upgrade to C++20 and enforce high code quality.
- **Actions**:
  - Update `CMakeLists.txt` to set `CXX_STANDARD 20` and `CXX_STANDARD_REQUIRED ON`.
  - Add compiler warning flags:
    - GCC/Clang: `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`
    - MSVC: `/W4 /WX`
  - Fix immediate compilation errors resulting from strict warnings.

### 5. Qt6 Migration
- **Goal**: Future-proof the GUI.
- **Actions**:
  - Update `vcpkg.json` dependencies from `qt5-*` to `qt6-*`.
  - Update `CMakeLists.txt` to find `Qt6` packages.
  - Refactor C++ code:
    - Remove `Qt5` specific macros.
    - Update `QRegExp` to `QRegularExpression`.
    - Update any deprecated Qt5 APIs.
  - Refactor QML:
    - Update imports (e.g., `import QtQuick 2.x` -> `import QtQuick`).
    - Fix any behavior changes in `QtQuick.Controls`.

### 6. Core Pipeline Abstraction
- **Goal**: Make the pipeline modular and testable.
- **Actions**:
  - Create an abstract base class `ProcessingStage`.
    - Virtual method `process(Image& input, Parameters params)`.
  - Refactor `ImagePipeline` to use a list/graph of `ProcessingStage` objects instead of hardcoded function calls.
  - Isolate distinct stages:
    - `WhiteBalance`
    - `Demosaic` (conceptually)
    - `ColorMix`
    - `Filmulation`

### 7. GPGPU with Halide
- **Goal**: Accelerate processing.
- **Setup**:
  - Add `halide` to `vcpkg.json`.
  - Create a proof-of-concept Halide generator for a simple stage (e.g., `ColorMix` or `Blur`).
  - Integrate the Halide-generated object file into the CMake build.

## Verification Plan

### Automated Tests
- Run `ctest` locally to verify new unit tests pass.
- Verify GitHub Actions workflow passes on all platforms.

### Manual Verification
- Build and run the application to ensure GUI works as expected after Qt6 port.
- Open a RAW file and verify the image outputs match (or are very close) to the previous version.
