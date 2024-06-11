cmake_minimum_required(VERSION 3.20)

project(PyClassifiers
  VERSION 1.0.2
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
find_library(BayesNet NAMES libBayesNet BayesNet libBayesNet.a PATHS ${PyClassifiers_SOURCE_DIR}/../lib/lib REQUIRED)
find_path(Bayesnet_INCLUDE_DIRS REQUIRED NAMES bayesnet PATHS ${PyClassifiers_SOURCE_DIR}/../lib/include)
message(STATUS "BayesNet=${BayesNet}")
message(STATUS "Bayesnet_INCLUDE_DIRS=${Bayesnet_INCLUDE_DIRS}")


# Subdirectories
# --------------
add_subdirectory(pyclfs)

# Testing
# -------
if (ENABLE_TESTING)
  MESSAGE("Testing enabled")
  add_git_submodule(lib/catch2)
  add_git_submodule(lib/mdlp)
  add_subdirectory(lib/Files)
  include(CTest)
  add_subdirectory(tests)
endif (ENABLE_TESTING)

# Installation
# ------------
install(TARGETS PyClassifiers
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        CONFIGURATIONS Release)
install(DIRECTORY pyclfs/ DESTINATION include/pyclassifiers FILES_MATCHING CONFIGURATIONS Release PATTERN "*.h" PATTERN "*.hpp")
install(FILES ${Bayesnet_INCLUDE_DIRS}/bayesnet/config.h DESTINATION include/pyclassifiers CONFIGURATIONS Release)