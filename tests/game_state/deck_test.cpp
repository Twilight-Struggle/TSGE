// どこで: tests/game_state
// 何を: Deckクラスのアクセサとカード管理の挙動を検証するテスト群
// なぜ: デッキ状態の整合性とアクセサのカバレッジを確保するため

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
            std::make_unique<DummyCard>(CardEnum::DUMMY, WarPeriod::EARLY_WAR);
      } else if (i < 60) {
        cardpool_[i] =
            std::make_unique<DummyCard>(CardEnum::DUMMY, WarPeriod::MID_WAR);
      } else if (i < 90) {
        cardpool_[i] =
            std::make_unique<DummyCard>(CardEnum::DUMMY, WarPeriod::LATE_WAR);
      } else {
        cardpool_[i] =
            std::make_unique<DummyCard>(CardEnum::DUMMY, WarPeriod::DUMMY);
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
  deck.getDiscardPile().push_back(CardEnum::DUMMY);
  deck.getDiscardPile().push_back(CardEnum::DUMMY);
  deck.getDiscardPile().push_back(CardEnum::DUMMY);

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

  // EARLY_WARカードが追加されているはず（中国カードを除く29枚）
  EXPECT_EQ(deck_size_before + 29, deck.getDeck().size());
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

  EXPECT_EQ(29, deck.getDeck().size());

  // const版のアクセスも確認
  const Board& const_board = board;
  const auto& const_deck = const_board.getDeck();
  EXPECT_EQ(29, const_deck.getDeck().size());
}

TEST_F(DeckTest, DiscardPileAccessorsConsistency) {
  Randomizer randomizer;
  Deck deck(randomizer, cardpool_);

  // 非constアクセサを通じて捨て札にカードを積み上げる
  auto& discard_mutable = deck.getDiscardPile();
  discard_mutable.push_back(CardEnum::DUMMY);
  discard_mutable.push_back(CardEnum::DUMMY);

  // constアクセサが同じメモリ領域を参照し、内容も一致することを検証
  const Deck& const_deck = deck;
  const auto& discard_const = const_deck.getDiscardPile();
  EXPECT_EQ(2, discard_const.size());
  EXPECT_EQ(static_cast<const void*>(&discard_mutable),
            static_cast<const void*>(&discard_const));
  EXPECT_EQ(discard_mutable[0], discard_const[0]);
  EXPECT_EQ(discard_mutable[1], discard_const[1]);
}

TEST_F(DeckTest, RemovedCardsAccessorsConsistency) {
  Randomizer randomizer;
  Deck deck(randomizer, cardpool_);

  // 非constアクセサを通じて除外カードを記録する
  auto& removed_mutable = deck.getRemovedCards();
  removed_mutable.push_back(CardEnum::DUMMY);

  // constアクセサが同じコンテナを参照し、内容が共有されていることを確認
  const Deck& const_deck = deck;
  const auto& removed_const = const_deck.getRemovedCards();
  EXPECT_EQ(1, removed_const.size());
  EXPECT_EQ(static_cast<const void*>(&removed_mutable),
            static_cast<const void*>(&removed_const));
  EXPECT_EQ(removed_mutable.front(), removed_const.front());
}
