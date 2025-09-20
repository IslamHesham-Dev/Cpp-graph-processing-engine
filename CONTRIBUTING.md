# Contributing to Graph Engine

Thank you for your interest in contributing to Graph Engine! This document provides guidelines and information for contributors.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Contributing Guidelines](#contributing-guidelines)
- [Code Style](#code-style)
- [Testing](#testing)
- [Documentation](#documentation)
- [Pull Request Process](#pull-request-process)
- [Issue Guidelines](#issue-guidelines)

## Code of Conduct

This project adheres to a code of conduct. By participating, you are expected to uphold this code. Please report unacceptable behavior to the maintainers.

## Getting Started

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16+
- Git
- Basic understanding of graph algorithms and data structures

### Development Dependencies

- **Google Test**: For unit testing
- **Google Benchmark**: For performance testing
- **Doxygen**: For documentation generation
- **clang-format**: For code formatting
- **Valgrind**: For memory leak detection (Linux/macOS)

## Development Setup

### 1. Fork and Clone

```bash
# Fork the repository on GitHub, then clone your fork
git clone https://github.com/yourusername/graph-engine.git
cd graph-engine

# Add upstream remote
git remote add upstream https://github.com/originalowner/graph-engine.git
```

### 2. Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
sudo apt-get install libgtest-dev libbenchmark-dev doxygen
sudo apt-get install clang-format valgrind
```

**macOS:**
```bash
# Install Xcode command line tools
xcode-select --install

# Install dependencies via Homebrew
brew install cmake gtest benchmark doxygen
brew install clang-format
```

**Windows:**
```bash
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install packages
.\vcpkg install gtest benchmark
```

### 3. Build the Project

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_BENCHMARKS=ON
make -j$(nproc)
```

### 4. Run Tests

```bash
# Run all tests
make test

# Run specific test
./tests/unit/test_graph

# Run with verbose output
ctest --verbose
```

### 5. Run Benchmarks

```bash
# Run all benchmarks
make benchmark

# Run specific benchmark
./tests/benchmark/benchmark_graph
```

## Contributing Guidelines

### Types of Contributions

We welcome several types of contributions:

1. **Bug Fixes**: Fix existing issues
2. **New Features**: Add new algorithms or functionality
3. **Performance Improvements**: Optimize existing code
4. **Documentation**: Improve or add documentation
5. **Tests**: Add or improve test coverage
6. **Examples**: Add new example applications

### Before You Start

1. **Check Existing Issues**: Look for existing issues or discussions
2. **Create an Issue**: For significant changes, create an issue first
3. **Discuss**: Engage with maintainers and other contributors
4. **Plan**: Break down large changes into smaller, manageable pieces

### Development Workflow

1. **Create a Branch**: Create a feature branch from `main`
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make Changes**: Implement your changes with tests
3. **Test**: Ensure all tests pass
4. **Format**: Format your code with clang-format
5. **Document**: Update documentation as needed
6. **Commit**: Write clear, descriptive commit messages
7. **Push**: Push your branch to your fork
8. **Pull Request**: Create a pull request

## Code Style

### General Guidelines

- Follow the Google C++ Style Guide
- Use meaningful variable and function names
- Write self-documenting code
- Add comments for complex algorithms
- Maximum line length: 100 characters

### Formatting

Use clang-format with the provided configuration:

```bash
# Format a single file
clang-format -i src/your_file.cpp

# Format all source files
find src include tests examples -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

### Naming Conventions

- **Classes**: PascalCase (`Graph`, `ShortestPathResult`)
- **Functions**: snake_case (`add_vertex`, `find_shortest_path`)
- **Variables**: snake_case (`num_vertices`, `edge_weight`)
- **Constants**: UPPER_CASE (`MAX_VERTICES`, `DEFAULT_WEIGHT`)
- **Namespaces**: snake_case (`graph_engine`, `algorithms`)

### Code Organization

```cpp
// Header file structure
#pragma once

#include <system_headers>
#include <third_party_headers>
#include "local_headers"

namespace graph_engine {
namespace algorithms {

// Forward declarations
class YourClass;

// Class definition
class YourClass {
public:
    // Public interface
private:
    // Private members
};

// Inline implementations
template<typename T>
void YourClass::method() {
    // Implementation
}

} // namespace algorithms
} // namespace graph_engine
```

### Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief Brief description of the function
 * 
 * Detailed description of what the function does,
 * including any important implementation details.
 * 
 * @tparam T Template parameter description
 * @param param1 Parameter description
 * @param param2 Parameter description
 * @return Return value description
 * @throws ExceptionType When this exception is thrown
 * 
 * @note Important notes about usage
 * @warning Warnings about potential issues
 * @see Related functions or classes
 * 
 * @example
 * ```cpp
 * auto result = your_function(1, 2.0);
 * ```
 */
template<typename T>
ReturnType your_function(T param1, double param2);
```

## Testing

### Test Requirements

- **Unit Tests**: All new code must have unit tests
- **Test Coverage**: Maintain coverage above 90%
- **Edge Cases**: Test edge cases and error conditions
- **Performance Tests**: Add benchmarks for performance-critical code

### Test Structure

```cpp
#include <gtest/gtest.h>
#include "graph_engine/your_header.hpp"

class YourClassTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }
    
    void TearDown() override {
        // Test cleanup
    }
    
    // Test fixtures
};

TEST_F(YourClassTest, BasicFunctionality) {
    // Test basic functionality
    EXPECT_EQ(expected, actual);
}

TEST_F(YourClassTest, EdgeCases) {
    // Test edge cases
    EXPECT_THROW(function_call(), ExceptionType);
}

TEST_F(YourClassTest, Performance) {
    // Performance tests
    auto start = std::chrono::high_resolution_clock::now();
    // ... code to test ...
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    EXPECT_LT(duration.count(), max_allowed_time);
}
```

### Benchmark Structure

```cpp
#include <benchmark/benchmark.h>

static void BM_YourAlgorithm(benchmark::State& state) {
    // Setup
    int size = state.range(0);
    auto graph = create_test_graph(size);
    
    for (auto _ : state) {
        // Code to benchmark
        auto result = your_algorithm(graph);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(size);
}

BENCHMARK(BM_YourAlgorithm)
    ->Args({100, 1000, 10000})
    ->Complexity(benchmark::oNLogN);
```

### Running Tests

```bash
# Run all tests
make test

# Run specific test suite
./tests/unit/test_graph

# Run with coverage
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON
make coverage

# Run with memory checking
valgrind --leak-check=full ./tests/unit/test_graph
```

## Documentation

### Documentation Requirements

- **API Documentation**: All public interfaces must be documented
- **Algorithm Documentation**: Explain algorithm choices and complexity
- **Example Code**: Provide usage examples
- **README Updates**: Update README for new features

### Documentation Types

1. **Header Documentation**: Doxygen comments in header files
2. **Implementation Comments**: Inline comments for complex logic
3. **README Updates**: Feature descriptions and examples
4. **Algorithm Documentation**: Complexity analysis and usage guidelines

### Building Documentation

```bash
# Generate documentation
make docs

# View documentation
open docs/html/index.html
```

## Pull Request Process

### Before Submitting

1. **Self-Review**: Review your own code
2. **Test**: Ensure all tests pass
3. **Format**: Format code with clang-format
4. **Document**: Update documentation
5. **Rebase**: Rebase on latest main branch

### Pull Request Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Performance improvement
- [ ] Documentation update
- [ ] Test addition/improvement

## Testing
- [ ] Unit tests added/updated
- [ ] All tests pass
- [ ] Benchmarks added (if applicable)

## Documentation
- [ ] API documentation updated
- [ ] README updated (if applicable)
- [ ] Algorithm documentation updated (if applicable)

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] No merge conflicts
- [ ] Breaking changes documented
```

### Review Process

1. **Automated Checks**: CI/CD pipeline runs tests and checks
2. **Code Review**: Maintainers review code quality and correctness
3. **Testing**: Additional testing may be requested
4. **Approval**: At least one maintainer approval required
5. **Merge**: Maintainer merges the pull request

## Issue Guidelines

### Bug Reports

When reporting bugs, please include:

1. **Description**: Clear description of the bug
2. **Reproduction**: Steps to reproduce the issue
3. **Expected Behavior**: What should happen
4. **Actual Behavior**: What actually happens
5. **Environment**: OS, compiler, version information
6. **Code Sample**: Minimal code to reproduce the issue

### Feature Requests

When requesting features, please include:

1. **Description**: Clear description of the feature
2. **Use Case**: Why this feature is needed
3. **Proposed Solution**: How you think it should work
4. **Alternatives**: Other solutions you've considered
5. **Additional Context**: Any other relevant information

### Issue Labels

- `bug`: Something isn't working
- `enhancement`: New feature or request
- `documentation`: Improvements or additions to documentation
- `good first issue`: Good for newcomers
- `help wanted`: Extra attention is needed
- `performance`: Performance-related issues
- `question`: Further information is requested

## Getting Help

### Communication Channels

- **GitHub Issues**: For bug reports and feature requests
- **GitHub Discussions**: For questions and general discussion
- **Email**: For private or sensitive matters

### Resources

- **Documentation**: [Full API Documentation](docs/)
- **Examples**: [Example Applications](examples/)
- **Tests**: [Test Suite](tests/)
- **Benchmarks**: [Performance Tests](tests/benchmark/)

## Recognition

Contributors will be recognized in:

- **README**: Listed as contributors
- **Release Notes**: Mentioned in relevant releases
- **Documentation**: Credited in relevant sections

## License

By contributing to Graph Engine, you agree that your contributions will be licensed under the same license as the project (MIT License).

## Questions?

If you have any questions about contributing, please:

1. Check existing issues and discussions
2. Create a new issue with the `question` label
3. Contact the maintainers directly

Thank you for contributing to Graph Engine!
