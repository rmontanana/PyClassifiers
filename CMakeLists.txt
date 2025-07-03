cmake_minimum_required(VERSION 3.20)

project(PyClassifiers
  VERSION 1.0.3
  DESCRIPTION "Python Classifiers Wrapper."
  HOMEPAGE_URL "https://github.com/rmontanana/pyclassifiers"
  LANGUAGES CXX
)

cmake_policy(SET CMP0135 NEW)

# Global CMake variables
# ----------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED            ON)
set(CMAKE_CXX_EXTENSIONS                  OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS          ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  MESSAGE("Debug mode")
else(CMAKE_BUILD_TYPE STREQUAL "Debug")
  MESSAGE("Release mode")
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Options
# -------
option(ENABLE_TESTING "Unit testing build"                        OFF)

# External libraries
# ------------------
find_package(Torch REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(bayesnet CONFIG REQUIRED)

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
find_package(Python3 3.11 COMPONENTS Interpreter Development REQUIRED)
message("Python3_LIBRARIES=${Python3_LIBRARIES}")

# Add the library
# ---------------
add_subdirectory(pyclfs)

# Testing
# -------
if (ENABLE_TESTING)
  MESSAGE("Testing enabled")
  find_package(Catch2 CONFIG REQUIRED)
  find_package(arff-files CONFIG REQUIRED)
  set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0 -g")
  if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-default-inline")
  endif()
  ## Configure test data path
  cmake_path(SET TEST_DATA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
  configure_file(tests/config/SourceData.h.in "${CMAKE_BINARY_DIR}/configured_files/include/SourceData.h")
  enable_testing()
  include(CTest)
  add_subdirectory(tests)
endif (ENABLE_TESTING)

# Installation
# ------------
install(TARGETS PyClassifiers
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
)
install(DIRECTORY pyclfs/ DESTINATION include/pyclassifiers FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

