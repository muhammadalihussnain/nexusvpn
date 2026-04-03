cmake_minimum_required(VERSION 3.16)
project(NexusVPN VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Main executable
add_executable(nexusvpn
    src/main.cpp
    src/core/tun_interface.cpp
)

# Testing
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    
    add_executable(tun_tests
        tests/unit/test_tun_interface.cpp
    )
    
    target_link_libraries(tun_tests
        nexusvpn
        GTest::GTest
        GTest::Main
        pthread
    )
    
    enable_testing()
    add_test(NAME TUNInterfaceTest COMMAND tun_tests)
endif()

# Coverage
option(ENABLE_COVERAGE "Enable coverage" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(nexusvpn PRIVATE --coverage -O0 -g)
    target_link_options(nexusvpn PRIVATE --coverage)
    target_compile_options(tun_tests PRIVATE --coverage -O0 -g)
    target_link_options(tun_tests PRIVATE --coverage)
endif()
