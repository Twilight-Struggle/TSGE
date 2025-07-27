#include "tsge/game_state/deck.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "tsge/core/board.hpp"
#include "tsge/game_state/card.hpp"
#include "tsge/utils/randomizer.hpp"

class DummyCard : public Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  DummyCard(CardEnum id, WarPeriod warPeriod)
      : Card(id, "MockCard", 3, Side::NEUTRAL, warPeriod, false) {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override {
    return {};
  }

  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    return true;
  }
};

class DeckTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // 簡単なカードプールを作成（実際のゲームより少ない）
    for (int i = 0; i < 111; ++i) {
      if (i < 30) {
        cardpool_[i] =
            std::make_unique<DummyCard>(CardEnum::Dummy, WarPeriod::EARLY_WAR);
      } else if (i < 60) {
        cardpool_[i] =
            std::make_unique<DummyCard>(CardEnum::Dummy, WarPeriod::MID_WAR);
      } else if (i < 90) {
        cardpool_[i] =
            std::make_unique<DummyCard>(CardEnum::Dummy, WarPeriod::LATE_WAR);
      } else {
        cardpool_[i] =
            std::make_unique<DummyCard>(CardEnum::Dummy, WarPeriod::DUMMY);
      }
    }
  }

  std::array<std::unique_ptr<Card>, 111> cardpool_;
};

TEST_F(DeckTest, ReshuffleFromDiscardEmptyDiscard) {
  Randomizer randomizer;
  Deck deck(randomizer, cardpool_);

  // 捨て札が空の場合、デッキは変化しないはず
  auto deck_size_before = deck.getDeck().size();
  deck.reshuffleFromDiscard();
  EXPECT_EQ(deck_size_before, deck.getDeck().size());
}

TEST_F(DeckTest, ReshuffleFromDiscardWithCards) {
  Randomizer randomizer;
  Deck deck(randomizer, cardpool_);

  // 捨て札にカードを追加
  deck.getDiscardPile().push_back(CardEnum::Dummy);
  deck.getDiscardPile().push_back(CardEnum::Dummy);
  deck.getDiscardPile().push_back(CardEnum::Dummy);

  // 0のはず
  auto deck_size_before = deck.getDeck().size();
  auto discard_size_before = deck.getDiscardPile().size();

  deck.reshuffleFromDiscard();

  EXPECT_EQ(0, deck.getDiscardPile().size());
  EXPECT_EQ(deck_size_before + discard_size_before, deck.getDeck().size());
}

TEST_F(DeckTest, AddEarlyWarCards) {
  Randomizer randomizer;
  Deck deck(randomizer, cardpool_);

  auto deck_size_before = deck.getDeck().size();
  deck.addEarlyWarCards();

  // EARLY_WARカードが追加されているはず（30枚）
  EXPECT_EQ(deck_size_before + 30, deck.getDeck().size());
}

TEST_F(DeckTest, AddMidWarCards) {
  Randomizer randomizer;
  Deck deck(randomizer, cardpool_);

  // まずEARLY_WARカードを追加
  deck.addEarlyWarCards();
  auto deck_size_after_early = deck.getDeck().size();

  deck.addMidWarCards();

  // MID_WARカードが追加されているはず（30枚）
  EXPECT_EQ(deck_size_after_early + 30, deck.getDeck().size());
}

TEST_F(DeckTest, AddLateWarCards) {
  Randomizer randomizer;
  Deck deck(randomizer, cardpool_);

  // まずEARLY_WARとMID_WARカードを追加
  deck.addEarlyWarCards();
  deck.addMidWarCards();
  auto deck_size_after_mid = deck.getDeck().size();

  deck.addLateWarCards();

  // LATE_WARカードが追加されているはず（30枚）
  EXPECT_EQ(deck_size_after_mid + 30, deck.getDeck().size());
}

TEST_F(DeckTest, BoardIntegration) {
  Board board(cardpool_);

  // Boardからデッキにアクセスできることを確認
  auto& deck = board.getDeck();
  deck.addEarlyWarCards();

  EXPECT_EQ(30, deck.getDeck().size());

  // const版のアクセスも確認
  const Board& const_board = board;
  const auto& const_deck = const_board.getDeck();
  EXPECT_EQ(30, const_deck.getDeck().size());
}