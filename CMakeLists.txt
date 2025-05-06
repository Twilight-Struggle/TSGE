cmake_minimum_required(VERSION 3.10)
set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "" FORCE)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(TS)

option(PROD "Enable production build (no TEST macros)" OFF)

# C++20を使用する場合
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # GNU拡張を無効化

add_library(ts_core
    src/game.cpp
    src/country.cpp
    src/world_map.cpp
    src/command.cpp
    src/randomizer.cpp
    src/trackers.cpp
    src/cards.cpp
    src/policies.cpp
)

# インクルードディレクトリの設定
target_include_directories(ts_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
if (NOT PROD)
    target_compile_definitions(ts_core PRIVATE TEST=1)
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# テストの設定
include(GoogleTest)
enable_testing()

function(add_test TEST_NAME)
    add_executable(${TEST_NAME} tests/${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME}
        PRIVATE
            GTest::gtest_main
            ts_core
    )
    target_compile_definitions(${TEST_NAME} PRIVATE TEST=1)
    gtest_discover_tests(${TEST_NAME})
endfunction()

add_test(command_test)
add_test(world_map_test)
add_test(trackers_test)
add_test(cards_test)