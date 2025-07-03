# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PyClassifiers is a C++ library that provides wrappers for Python machine learning classifiers. It enables C++ applications to use Python-based ML algorithms (scikit-learn, XGBoost, custom implementations) through a unified interface.

## Essential Commands

### Build System
```bash
# Setup build configurations
make debug          # Configure debug build with testing and coverage
make release         # Configure release build

# Build targets
make buildd          # Build debug version
make buildr          # Build release version

# Testing
make test            # Run all unit tests
make test opt="-s"   # Run tests with verbose output
make test opt="-c='Test Name'" # Run specific test section

# Coverage
make coverage        # Run tests and generate coverage report

# Installation
sudo make install    # Install library to system (requires release build)

# Utilities
make clean           # Clean test artifacts
make help            # Show all available targets
```

### Dependencies
- Requires Conan package manager (`pip install conan`)
- Miniconda installation required for Python classifiers
- Boost library (preferably system package: `sudo dnf install boost-devel`)

## Architecture

### Core Components

**PyWrap** (`pyclfs/PyWrap.h`): Singleton managing Python interpreter lifecycle and thread-safe Python/C++ communication.

**PyClassifier** (`pyclfs/PyClassifier.h`): Abstract base class inheriting from `bayesnet::BaseClassifier`. All Python classifier wrappers extend this class.

**Individual Classifiers**: Each classifier (STree, ODTE, SVC, RandomForest, XGBoost, AdaBoostPy) wraps specific Python modules with consistent C++ interface.

### Data Flow
- Uses PyTorch tensors for efficient C++/Python data exchange
- JSON-based hyperparameter configuration
- Automatic memory management for Python objects

## Key Directories

- `pyclfs/` - Core library source code
- `tests/` - Catch2 unit tests with ARFF test datasets
- `build_debug/` - Debug build artifacts
- `build_release/` - Release build artifacts
- `cmake/modules/` - Custom CMake modules

## Development Patterns

### Adding New Classifiers
1. Inherit from `PyClassifier` base class
2. Implement required virtual methods: `fit()`, `predict()`, `predict_proba()`
3. Use `PyWrap::getInstance()` for Python interpreter access
4. Handle hyperparameters via JSON configuration
5. Add corresponding unit tests in `tests/TestPythonClassifiers.cc`

### Python Integration
- All Python interactions go through PyWrap singleton
- Use RAII pattern for Python object management
- Convert data using PyTorch tensors (discrete/continuous data support)
- Handle Python exceptions and convert to C++ exceptions

### Testing
- Catch2 framework with parameterized tests using GENERATE()
- Test data in ARFF format located in `tests/data/`
- Performance benchmarks validate expected accuracy scores
- Coverage reports generated with gcovr

## Important Files

- `pyclfs/PyWrap.h` - Python interpreter management
- `pyclfs/PyClassifier.h` - Base classifier interface
- `CMakeLists.txt` - Main build configuration
- `Makefile` - Build automation and common tasks
- `conanfile.py` - Package dependencies
- `tests/TestPythonClassifiers.cc` - Main test suite

## Technical Requirements

- C++17 standard compliance
- Python 3.11+ required
- Boost library with Python and NumPy support
- PyTorch for tensor operations
- Thread-safe design for concurrent usage