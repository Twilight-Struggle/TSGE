cmake_minimum_required(VERSION 3.10)
set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "" FORCE)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(TS)

# ビルドタイプのデフォルト設定
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

option(ENABLE_TESTING "Enable unit tests" ON)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

# C++20を使用する
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # GNU拡張を無効化

# ビルドタイプ別の設定
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -fno-omit-frame-pointer)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native)
endif()

# サニタイザー設定
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

add_library(ts_core
    src/core/board.cpp
    src/core/game.cpp
    src/core/phase_machine.cpp
    src/actions/command.cpp
    src/actions/move.cpp
    src/actions/legal_moves_generator.cpp
    src/game_state/world_map.cpp
    src/game_state/world_map_constants.cpp
    src/game_state/cards.cpp
    src/game_state/deck.cpp
    src/players/tsnnmcts.cpp
    src/players/policies.cpp
    src/utils/randomizer.cpp
)

# インクルードディレクトリの設定
target_include_directories(ts_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# テスト有効時のみTESTマクロを定義
if(ENABLE_TESTING)
    target_compile_definitions(ts_core PUBLIC TEST=1)
endif()

# format
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.hpp)
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
    )
endif()

# カバレッジ設定
if(ENABLE_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "Coverage is only supported with Clang")
    endif()

    # カバレッジフラグの設定
    target_compile_options(ts_core PRIVATE
        -fprofile-instr-generate
        -fcoverage-mapping
        -mllvm -enable-name-compression=false
    )
    target_link_options(ts_core PRIVATE -fprofile-instr-generate)
    
    if(ENABLE_TESTING)
        find_program(LLVM_COV llvm-cov REQUIRED)
        find_program(LLVM_PROFDATA llvm-profdata REQUIRED)
        
        add_custom_target(coverage
            COMMAND rm -rf *.profraw default.profdata coverage_report
            COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/test_%p.profraw ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR}
            COMMAND ${LLVM_PROFDATA} merge -sparse *.profraw -o default.profdata
            
            # テキストレポート
            COMMAND ${LLVM_COV} report
                --instr-profile=default.profdata
                -object $<TARGET_FILE:trackers_test>
                -object $<TARGET_FILE:country_test>
                -object $<TARGET_FILE:board_test>
                -object $<TARGET_FILE:board_mcts_test>
                -object $<TARGET_FILE:command_test>
                -object $<TARGET_FILE:legal_moves_generator_test>
                -object $<TARGET_FILE:move_test>
                -object $<TARGET_FILE:phase_machine_test>
                -object $<TARGET_FILE:world_map_test>
                -object $<TARGET_FILE:cards_test>
                -object $<TARGET_FILE:deck_test>
                -object $<TARGET_FILE:tsnnmcts_policy_test>
                
            # HTMLレポート
            COMMAND ${LLVM_COV} show
                $<TARGET_FILE:country_test>
                -instr-profile=default.profdata
                -format=html
                -output-dir=coverage_report
                -show-line-counts-or-regions
                -show-instantiations
                -show-expansions
                -object $<TARGET_FILE:trackers_test>
                -object $<TARGET_FILE:board_test>
                -object $<TARGET_FILE:board_mcts_test>
                -object $<TARGET_FILE:command_test>
                -object $<TARGET_FILE:legal_moves_generator_test>
                -object $<TARGET_FILE:move_test>
                -object $<TARGET_FILE:phase_machine_test>
                -object $<TARGET_FILE:world_map_test>
                -object $<TARGET_FILE:cards_test>
                -object $<TARGET_FILE:deck_test>
                -object $<TARGET_FILE:tsnnmcts_policy_test>

            # カバレッジサマリーを表示
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in: ${CMAKE_BINARY_DIR}/coverage_report/index.html"

            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report with llvm-cov..."
            DEPENDS board_test board_mcts_test command_test legal_moves_generator_test move_test phase_machine_test world_map_test country_test trackers_test cards_test deck_test
        )
    endif()
endif()


# テストの設定
if(ENABLE_TESTING)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.external/googletest/CMakeLists.txt")
        add_subdirectory(.external/googletest)
    else()
        include(FetchContent)
        # WSL2向けの設定
        set(FETCHCONTENT_QUIET FALSE)
        set(FETCHCONTENT_TIMEOUT 60)  # タイムアウトを延長
        # HTTPSを使用（WSL2でより安定）
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
            URL_HASH SHA256=7b42b4d6ed48810c5362c265a17faebe90dc2373c885e5216439d37927f02926
            DOWNLOAD_NO_PROGRESS FALSE  # 進捗を表示
        )
        FetchContent_MakeAvailable(googletest)
    endif()

    include(GoogleTest)
    enable_testing()

    function(add_test_with_path TEST_NAME TEST_PATH)
        add_executable(${TEST_NAME} ${TEST_PATH})
        target_link_libraries(${TEST_NAME}
            PRIVATE
                GTest::gtest_main
                ts_core
        )
        if(ENABLE_COVERAGE)
            target_compile_options(${TEST_NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
            target_link_options(${TEST_NAME} PRIVATE -fprofile-instr-generate)
        endif()
        set(_gtest_discover_props)
        if(ENABLE_COVERAGE)
            # テスト実行時のプロファイルファイル出力先を設定
            list(APPEND _gtest_discover_props
                PROPERTIES
                    ENVIRONMENT "LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/${TEST_NAME}_%p.profraw"
            )
        endif()
        gtest_discover_tests(${TEST_NAME} ${_gtest_discover_props})
    endfunction()

    add_test_with_path(board_test tests/core/board_test.cpp)
    add_test_with_path(board_mcts_test tests/core/board_mcts_test.cpp)
    add_test_with_path(command_test tests/actions/command_test.cpp)
    add_test_with_path(legal_moves_generator_test tests/actions/legal_moves_generator_test.cpp)
    add_test_with_path(move_test tests/actions/move_test.cpp)
    add_test_with_path(phase_machine_test tests/core/phase_machine_test.cpp)
    add_test_with_path(world_map_test tests/game_state/world_map_test.cpp)
    add_test_with_path(country_test tests/game_state/country_test.cpp)
    add_test_with_path(trackers_test tests/game_state/trackers_test.cpp)
    add_test_with_path(cards_test tests/game_state/cards_test.cpp)
    add_test_with_path(deck_test tests/game_state/deck_test.cpp)
    add_test_with_path(tsnnmcts_policy_test tests/players/tsnnmcts_policy_test.cpp)
endif()
