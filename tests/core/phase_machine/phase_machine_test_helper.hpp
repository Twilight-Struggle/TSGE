#ifndef TSGE_PHASE_MACHINE_TEST_HELPER_HPP
#define TSGE_PHASE_MACHINE_TEST_HELPER_HPP

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "tsge/actions/command.hpp"
#include "tsge/core/board.hpp"
#include "tsge/core/phase_machine.hpp"
#include "tsge/game_state/card.hpp"

// Dummy card class for testing
class DummyCard : public Card {
 public:
  DummyCard()
      : Card(CardEnum::DUMMY, "Dummy", 3, Side::NEUTRAL, WarPeriod::DUMMY,
             false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side /*side*/,
                                const Board& /*board*/) const override {
    return {};
  }
  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    return true;
  }
};

class HeadlineEventCard final : public Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  HeadlineEventCard(CardEnum id, std::string&& name, int ops, Side allegiance,
                    std::shared_ptr<std::vector<std::pair<Side, CardEnum>>> log,
                    bool removedAfterEvent = false, bool canEventValue = true)
      : Card(id, std::move(name), ops, allegiance, WarPeriod::DUMMY,
             removedAfterEvent),
        executionLog_(std::move(log)),
        canEventEnabled_(canEventValue) {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side side,
                                const Board& /*board*/) const override {
    auto log = executionLog_;
    const auto card_id = id_;
    return {std::make_shared<LambdaCommand>(
        [log, side, card_id](Board&) { log->emplace_back(side, card_id); })};
  }

  [[nodiscard]]
  bool canEvent(const Board& /*board*/) const override {
    return canEventEnabled_;
  }

 private:
  std::shared_ptr<std::vector<std::pair<Side, CardEnum>>> executionLog_;
  bool canEventEnabled_;
};

class WarPeriodTaggedCard final : public Card {
 public:
  WarPeriodTaggedCard(CardEnum id, WarPeriod warPeriod)
      : Card(id, "WarPeriodTagged", 2, Side::NEUTRAL, warPeriod, false) {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side /*unused*/,
                                const Board& /*board*/) const override {
    return {};
  }

  [[nodiscard]]
  bool canEvent(const Board& /*board*/) const override {
    return true;
  }
};

class CardPoolGuard final {
 public:
  CardPoolGuard(std::array<std::unique_ptr<Card>, 111>& pool, CardEnum card,
                std::unique_ptr<Card> replacement)
      : pool_(pool),
        index_(static_cast<std::size_t>(card)),
        original_(std::move(pool_[index_])) {
    pool_[index_] = std::move(replacement);
  }

  CardPoolGuard(const CardPoolGuard&) = delete;
  CardPoolGuard& operator=(const CardPoolGuard&) = delete;

  CardPoolGuard(CardPoolGuard&&) = delete;
  CardPoolGuard& operator=(CardPoolGuard&&) = delete;

  ~CardPoolGuard() { pool_[index_] = std::move(original_); }

 private:
  std::array<std::unique_ptr<Card>, 111>& pool_;
  std::size_t index_;
  std::unique_ptr<Card> original_;
};

class TrackingMove final : public Move {
 public:
  TrackingMove(CardEnum cardEnum, Side side, std::shared_ptr<bool> executed)
      : Move(cardEnum, side), executedFlag_(std::move(executed)) {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(const std::unique_ptr<Card>& card,
                                    const Board& /*board*/) const override {
    auto flag = executedFlag_;
    return {std::make_shared<LambdaCommand>([flag](Board&) { *flag = true; })};
  }

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    if (this->getCard() != other.getCard() ||
        this->getSide() != other.getSide()) {
      return false;
    }
    const auto* other_cast = dynamic_cast<const TrackingMove*>(&other);
    return other_cast != nullptr && other_cast->executedFlag_ == executedFlag_;
  }

 private:
  std::shared_ptr<bool> executedFlag_;
};

class PhaseMachineTest : public ::testing::Test {
 protected:
  PhaseMachineTest() : board(defaultCardPool()) {}

  void SetUp() override { board.giveChinaCardTo(Side::USSR, false); }

  // デッキはデフォルトで空のため、テストごとに明示的に初期化する。
  // TURN_STARTでの配布処理が空デッキにアクセスしないよう、十分な枚数の
  // Dummyカードを積み上げてから検証を実施する。
  void prepareDeckWithDummyCards(int card_count) {
    auto& deck = board.getDeck();
    auto& draw_pile = deck.getDeck();
    draw_pile.clear();
    draw_pile.reserve(card_count);
    for (int i = 0; i < card_count; ++i) {
      draw_pile.push_back(CardEnum::DUMMY);
    }

    // 余計な副作用を避けるため、捨て札と除外山も都度リセットする。
    deck.getDiscardPile().clear();
    deck.getRemovedCards().clear();
  }

  static std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    static bool initialized = false;
    if (!initialized) {
      for (auto& card : pool) {
        if (!card) {
          card = std::make_unique<DummyCard>();
        }
      }
      initialized = true;
    }
    return pool;
  }

  Board board;
};

#endif  // TSGE_PHASE_MACHINE_TEST_HELPER_HPP
