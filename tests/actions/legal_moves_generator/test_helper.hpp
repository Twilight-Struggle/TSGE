#pragma once

#include <gtest/gtest.h>

#include <array>
#include <memory>
#include <vector>

#include "tsge/actions/command.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/card.hpp"

// テスト用DummyCardクラス
class DummyCard : public Card {
 public:
  DummyCard(int ops)
      : Card(CardEnum::DUMMY, "DummyCard", ops, Side::NEUTRAL, WarPeriod::DUMMY,
             false) {}
  DummyCard(int ops, Side side)
      : Card(CardEnum::DUMMY, "DummyCard", ops, side, WarPeriod::DUMMY, false) {
  }

  [[nodiscard]]
  std::vector<CommandPtr> event(Side /*side*/,
                                const Board& /*board*/) const override {
    // 空実装
    return {};
  }

  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    // テスト用：常にイベント実行可能とする
    return true;
  }
};

// テスト用MockEventCardクラス（canEventとgetSideを制御可能）
class MockEventCard : public Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  MockEventCard(CardEnum id, Side side, bool can_event)
      : Card(id, "MockEventCard", 3, side, WarPeriod::EARLY_WAR, false),
        can_event_{can_event} {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side /*side*/,
                                const Board& /*board*/) const override {
    // 空実装
    return {};
  }

  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    return can_event_;
  }

 private:
  const bool can_event_;
};

// テスト用のカードプール作成関数
static const std::array<std::unique_ptr<Card>, 111>& createTestCardPool() {
  static std::array<std::unique_ptr<Card>, 111> pool{};
  static bool initialized = false;

  if (!initialized) {
    // DummyカードをOps0として設定（スコアカード相当）
    pool[static_cast<size_t>(CardEnum::DUMMY)] = std::make_unique<DummyCard>(0);
    // その他の既存カード
    pool[static_cast<size_t>(CardEnum::CHINA_CARD)] =
        std::make_unique<DummyCard>(4);
    pool[static_cast<size_t>(CardEnum::DUCK_AND_COVER)] =
        std::make_unique<DummyCard>(3);
    pool[static_cast<size_t>(CardEnum::FIDEL)] = std::make_unique<DummyCard>(2);
    pool[static_cast<size_t>(CardEnum::NUCLEAR_TEST_BAN)] =
        std::make_unique<DummyCard>(4);
    initialized = true;
  }

  return pool;
}

// テスト用ヘルパー関数
class TestHelper {
 public:
  static void clearAllOpponentInfluence(Board& board, Side side) {
    Side opponent = getOpponentSide(side);
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < board.getWorldMap().getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      board.getWorldMap().getCountry(country_enum).clearInfluence(opponent);
    }
  }

  static void clearSuperPowerInfluence(Board& board, Side side) {
    auto country_enum =
        side == Side::USSR ? CountryEnum::USSR : CountryEnum::USA;
    board.getWorldMap().getCountry(country_enum).clearInfluence(side);
  }

  static void setupBoardWithInfluence(Board& board) {
    // まず全ての国の影響力をクリア（初期配置をリセット）
    clearAllOpponentInfluence(board, Side::USSR);
    clearAllOpponentInfluence(board, Side::USA);

    // テスト用の影響力を設定
    // 日本にUSA影響力3を設定
    board.getWorldMap()
        .getCountry(CountryEnum::JAPAN)
        .addInfluence(Side::USA, 3);
    // 西ドイツにUSA影響力2を設定
    board.getWorldMap()
        .getCountry(CountryEnum::WEST_GERMANY)
        .addInfluence(Side::USA, 2);
    // イランにUSA影響力1を設定
    board.getWorldMap()
        .getCountry(CountryEnum::IRAN)
        .addInfluence(Side::USA, 1);
    // アンゴラにUSA影響力1を設定
    board.getWorldMap()
        .getCountry(CountryEnum::ANGOLA)
        .addInfluence(Side::USA, 1);
  }

  static void addCardsToHand(Board& board, Side side,
                             const std::vector<CardEnum>& cards) {
    board.clearHand(side);
    for (CardEnum card : cards) {
      board.addCardToHand(side, card);
    }
  }

  static void setSpaceTrackPosition(Board& board, Side side, int position) {
    // SpaceTrackの位置を設定するため、advanceSpaceTrackを使用
    int current_position = board.getSpaceTrack().getSpaceTrackPosition(side);
    if (position > current_position) {
      board.getSpaceTrack().advanceSpaceTrack(side,
                                              position - current_position);
    }
  }

  static void setSpaceTrackTried(Board& board, Side side, int tried) {
    // SpaceTrackの試行回数を設定するため、spaceTriedを使用
    for (int i = 0; i < tried; ++i) {
      board.getSpaceTrack().spaceTried(side);
    }
  }
};
