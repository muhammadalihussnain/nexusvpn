cmake_minimum_required(VERSION 3.10)
project(NexusVPN VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(ENABLE_COVERAGE "Enable coverage" OFF)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library as OBJECT library
add_library(nexusvpn_objects OBJECT
    src/core/tun_interface.cpp
)

# Main executable
add_executable(nexusvpn
    src/main.cpp
    $<TARGET_OBJECTS:nexusvpn_objects>
)

target_link_libraries(nexusvpn pthread)

# Tests
if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    
    add_executable(tun_tests
        tests/unit/test_tun_interface.cpp
        $<TARGET_OBJECTS:nexusvpn_objects>
    )
    
    target_link_libraries(tun_tests
        GTest::GTest
        GTest::Main
        pthread
    )
    
    enable_testing()
    add_test(NAME TUNInterfaceTest COMMAND tun_tests)
endif()

# Coverage
if(ENABLE_COVERAGE)
    target_compile_options(nexusvpn_objects PRIVATE --coverage -O0 -g)
    target_link_options(nexusvpn PRIVATE --coverage)
    if(BUILD_TESTS)
        target_link_options(tun_tests PRIVATE --coverage)
    endif()
endif()

message(STATUS "Build configured successfully!")
