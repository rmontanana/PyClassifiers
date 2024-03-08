cmake_minimum_required(VERSION 3.20)

project(PyClassifiers
  VERSION 1.0.1
  DESCRIPTION "Python Classifiers Wrapper."
  HOMEPAGE_URL "https://github.com/rmontanana/pyclassifiers"
  LANGUAGES CXX
)

if (CODE_COVERAGE AND NOT ENABLE_TESTING)
  MESSAGE(FATAL_ERROR "Code coverage requires testing enabled")
endif (CODE_COVERAGE AND NOT ENABLE_TESTING)

find_package(Torch REQUIRED)

if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif ()

# Global CMake variables
# ----------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED            ON)
set(CMAKE_CXX_EXTENSIONS                  OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS          ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Options
# -------
option(ENABLE_TESTING "Unit testing build"                        OFF)
option(CODE_COVERAGE "Collect coverage from test library"         OFF)
option(INSTALL_GTEST "Enable installation of googletest."         OFF)

# Boost Library
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.66.0 REQUIRED COMPONENTS python3 numpy3) 
if(Boost_FOUND)
  message("Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}")
  include_directories(${Boost_INCLUDE_DIRS}) 
endif()

# Python
find_package(Python3 3.11...3.11.9 COMPONENTS Interpreter Development REQUIRED)
message("Python3_LIBRARIES=${Python3_LIBRARIES}")

# CMakes modules
# --------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
include(AddGitSubmodule)

if (CODE_COVERAGE)
  enable_testing()
  include(CodeCoverage)
  MESSAGE("Code coverage enabled")
  set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0 -g")
  SET(GCC_COVERAGE_LINK_FLAGS " ${GCC_COVERAGE_LINK_FLAGS} -lgcov --coverage")
endif (CODE_COVERAGE)

if (ENABLE_CLANG_TIDY)
  include(StaticAnalyzers) # clang-tidy
endif (ENABLE_CLANG_TIDY)

# External libraries - dependencies of PyClassifiers
# --------------------------------------------------
find_library(BayesNet NAMES libBayesNet BayesNet)

# Subdirectories
# --------------
file(GLOB PyClassifiers_SOURCES CONFIGURE_DEPENDS ${PyClassifiers_SOURCE_DIR}/src/*.cc ${PyClassifiers_SOURCE_DIR}/src/*.hpp)
add_subdirectory(src)

# Testing
# -------
if (ENABLE_TESTING)
  MESSAGE("Testing enabled")
  include(CTest)
  add_subdirectory(tests)
endif (ENABLE_TESTING)

# Installation
# ------------
install(TARGETS PyClassifiers
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        CONFIGURATIONS Release)
install(DIRECTORY src/ DESTINATION include/pyclassifiers FILES_MATCHING CONFIGURATIONS Release PATTERN "*.h" PATTERN "*.hpp")
install(FILES /usr/local/include/bayesnet/config.h DESTINATION include/pyclassifiers CONFIGURATIONS Release)