#pragma once
// File: tests/game_state/basic_event_cards_test/basic_event_card_test_base.hpp
// Summary:
// 基本イベントカードテストの共通フィクスチャをテンプレートとして提供する。
// Reason:
// 重複する初期化ロジックを集約し、ファイルサイズとメンテナンスコストを削減するため。

#include <gtest/gtest.h>

#include <array>
#include <memory>

#include "tsge/core/board.hpp"
#include "tsge/game_state/cards.hpp"

template <typename CardType>
class BasicEventCardTestBase : public ::testing::Test {
 protected:
  BasicEventCardTestBase() : board(defaultCardPool()), sut() {}

  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }

  Board board;
  CardType sut;
};
