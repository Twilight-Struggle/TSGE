#pragma once
// File: tests/game_state/basic_event_cards_test/place_cards_test.hpp
// Summary: 基本イベントカードPlace系テスト用の共通フィクスチャを提供する。
// Reason: WarPeriod別のテスト分割後も共通初期化を再利用し、重複を避けるため。

#include <gtest/gtest.h>

#include <array>
#include <memory>

#include "tsge/core/board.hpp"
#include "tsge/game_state/cards.hpp"

class PlaceCardsTest : public ::testing::Test {
 protected:
  PlaceCardsTest() : board(defaultCardPool()) {}

  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }

  Board board;
};
