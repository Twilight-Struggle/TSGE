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
    src/core/game.cpp
    src/core/phase_machine.cpp
    src/actions/command.cpp
    src/actions/move.cpp
    src/actions/legal_moves_generator.cpp
    src/game_state/world_map.cpp
    src/game_state/cards.cpp
    src/game_state/trackers.cpp
    src/players/policies.cpp
    src/utils/randomizer.cpp
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

# format
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.hpp)
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
    )
endif()

# カバレッジオプションを追加（デフォルトではOFF）
option(COVERAGE "Enable coverage reporting" OFF)

if(COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
else()
    # カバレッジを無効にする場合は、--coverageフラグを確実に除去
    string(REPLACE "--coverage" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "--coverage" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
endif()

# カバレッジターゲット追加
if(COVERAGE)
    find_program(LCOV lcov REQUIRED)
    find_program(GENHTML genhtml REQUIRED)
    
    add_custom_target(coverage
        # ベースラインを取得
        COMMAND ${LCOV} --directory . --zerocounters
        # テスト実行
        COMMAND ${CMAKE_CTEST_COMMAND}
        # カバレッジ情報収集
        COMMAND ${LCOV} --directory . --capture --output-file coverage.info
        # システムヘッダを除外
        COMMAND ${LCOV} --remove coverage.info '/usr/*' --output-file coverage.info
        COMMAND ${LCOV} --remove coverage.info '*/tests/*' --output-file coverage.info
        # HTMLレポート生成
        COMMAND ${GENHTML} coverage.info --output-directory coverage_report
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# テストの設定
include(GoogleTest)
enable_testing()

function(add_test_with_path TEST_NAME TEST_PATH)
    add_executable(${TEST_NAME} ${TEST_PATH})
    target_link_libraries(${TEST_NAME}
        PRIVATE
            GTest::gtest_main
            ts_core
    )
    target_compile_definitions(${TEST_NAME} PRIVATE TEST=1)
    
    # カバレッジが有効な場合のみcoverageフラグを追加
    if(COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${TEST_NAME} PRIVATE --coverage)
            target_link_options(${TEST_NAME} PRIVATE --coverage)
        endif()
    endif()
    
    gtest_discover_tests(${TEST_NAME})
endfunction()

add_test_with_path(command_test tests/actions/command_test.cpp)
add_test_with_path(world_map_test tests/game_state/world_map_test.cpp)
add_test_with_path(trackers_test tests/game_state/trackers_test.cpp)
add_test_with_path(cards_test tests/game_state/cards_test.cpp)