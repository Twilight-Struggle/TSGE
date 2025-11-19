#include "tsge/core/board.hpp"

#include <gtest/gtest.h>

#include "tsge/game_state/card.hpp"

class DummyCard : public Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  DummyCard(CardEnum id, WarPeriod warPeriod)
      : Card(id, "DummyCard", 3, Side::NEUTRAL, warPeriod, false) {}

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

class BoardDrawTest : public ::testing::Test {
 protected:
  void SetUp() override {
    cardpool_ = std::make_unique<std::array<std::unique_ptr<Card>, 111>>();
    // テスト用のダミーカードを作成
    for (int i = 0; i < 50; ++i) {
      (*cardpool_)[i] =
          std::make_unique<DummyCard>(CardEnum::DUMMY, WarPeriod::EARLY_WAR);
    }
    board_ = std::make_unique<Board>(*cardpool_);

    // Early War カードをデッキに追加
    board_->getDeck().addEarlyWarCards();
  }

  std::unique_ptr<std::array<std::unique_ptr<Card>, 111>> cardpool_;
  std::unique_ptr<Board> board_;
};

TEST_F(BoardDrawTest, CalculateDrawCountTurn1EmptyHands) {
  // ターン1では6アクションラウンド + 2 = 8枚必要
  auto [ussr_draw, usa_draw] = board_->calculateDrawCount(1);

  EXPECT_EQ(ussr_draw, 8);
  EXPECT_EQ(usa_draw, 8);
}

TEST_F(BoardDrawTest, CalculateDrawCountWithExistingCards) {
  // 手札にカードを追加
  board_->addCardToHand(Side::USSR, CardEnum::DUMMY);
  board_->addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board_->addCardToHand(Side::USA, CardEnum::FIDEL);

  // ターン1では8枚必要、USSR:2枚持ち、USA:1枚持ち
  auto [ussr_draw, usa_draw] = board_->calculateDrawCount(1);

  EXPECT_EQ(ussr_draw, 6);
  EXPECT_EQ(usa_draw, 7);
}

TEST_F(BoardDrawTest, CalculateDrawCountAlreadyEnoughCards) {
  // 既に十分な手札を持っている場合
  for (int i = 0; i < 10; ++i) {
    board_->addCardToHand(Side::USSR, CardEnum::DUMMY);
    board_->addCardToHand(Side::USA, CardEnum::DUCK_AND_COVER);
  }

  auto [ussr_draw, usa_draw] = board_->calculateDrawCount(1);

  EXPECT_EQ(ussr_draw, 0);
  EXPECT_EQ(usa_draw, 0);
}

TEST_F(BoardDrawTest, DrawCardsNormalCase) {
  const size_t initial_deck_size = board_->getDeck().getDeck().size();

  // 各プレイヤーが3枚ずつ引く
  board_->drawCardsForPlayers(3, 3);

  EXPECT_EQ(board_->getPlayerHand(Side::USSR).size(), 3);
  EXPECT_EQ(board_->getPlayerHand(Side::USA).size(), 3);
  EXPECT_EQ(board_->getDeck().getDeck().size(), initial_deck_size - 6);
}

TEST_F(BoardDrawTest, DrawCardsExactlyDeckSize) {
  // 捨て札にカードを追加（reshuffleのため）
  auto& discard_pile = board_->getDeck().getDiscardPile();
  for (int i = 0; i < 10; ++i) {
    discard_pile.push_back(CardEnum::DUMMY);
  }

  // デッキを特定の枚数にする
  auto& deck = board_->getDeck().getDeck();
  const int target_size = 8;
  while (deck.size() > target_size) {
    deck.pop_back();
  }

  // 合計でデッキと同じ枚数を引く
  board_->drawCardsForPlayers(4, 4);

  EXPECT_EQ(board_->getPlayerHand(Side::USSR).size(), 4);
  EXPECT_EQ(board_->getPlayerHand(Side::USA).size(), 4);
  // reshuffleが実行されているはず
  EXPECT_EQ(board_->getDeck().getDiscardPile().size(), 0);
}

TEST_F(BoardDrawTest, DrawCardsMoreThanDeckEvenCase) {
  // 捨て札にカードを追加（reshuffleのため）
  auto& discard_pile = board_->getDeck().getDiscardPile();
  for (int i = 0; i < 20; ++i) {
    discard_pile.push_back(CardEnum::DUMMY);
  }

  // デッキを偶数枚にする
  auto& deck = board_->getDeck().getDeck();
  while (deck.size() > 6) {
    deck.pop_back();
  }

  // 10枚ずつ引く（合計20枚、デッキは6枚）
  board_->drawCardsForPlayers(10, 10);

  EXPECT_EQ(board_->getPlayerHand(Side::USSR).size(), 10);
  EXPECT_EQ(board_->getPlayerHand(Side::USA).size(), 10);
}

TEST_F(BoardDrawTest, DrawCardsMoreThanDeckOddCase) {
  // 捨て札にカードを追加（reshuffleのため）
  auto& discard_pile = board_->getDeck().getDiscardPile();
  for (int i = 0; i < 20; ++i) {
    discard_pile.push_back(CardEnum::DUMMY);
  }

  // デッキを奇数枚にする
  auto& deck = board_->getDeck().getDeck();
  while (deck.size() > 7) {
    deck.pop_back();
  }

  // 10枚ずつ引く（合計20枚、デッキは7枚）
  board_->drawCardsForPlayers(10, 10);

  EXPECT_EQ(board_->getPlayerHand(Side::USSR).size(), 10);
  EXPECT_EQ(board_->getPlayerHand(Side::USA).size(), 10);
}

TEST_F(BoardDrawTest, DrawCardsMoreThanDeckRedistributesAfterCap) {
  // デッキ不足時に片側の要求枚数が少ないケース
  auto& discard_pile = board_->getDeck().getDiscardPile();
  for (int i = 0; i < 20; ++i) {
    discard_pile.push_back(CardEnum::DUMMY);
  }

  auto& deck = board_->getDeck().getDeck();
  while (deck.size() > 5) {
    deck.pop_back();
  }

  board_->drawCardsForPlayers(1, 6);

  EXPECT_EQ(board_->getPlayerHand(Side::USSR).size(), 1);
  EXPECT_EQ(board_->getPlayerHand(Side::USA).size(), 6);
  EXPECT_EQ(board_->getDeck().getDiscardPile().size(), 0);
}

TEST_F(BoardDrawTest, DrawCardsZeroCase) {
  const size_t initial_deck_size = board_->getDeck().getDeck().size();

  // 0枚引く
  board_->drawCardsForPlayers(0, 0);

  EXPECT_EQ(board_->getPlayerHand(Side::USSR).size(), 0);
  EXPECT_EQ(board_->getPlayerHand(Side::USA).size(), 0);
  EXPECT_EQ(board_->getDeck().getDeck().size(), initial_deck_size);
}

TEST_F(BoardDrawTest, DrawCardsAsymmetricCase) {
  const size_t initial_deck_size = board_->getDeck().getDeck().size();

  // 非対称に引く
  board_->drawCardsForPlayers(5, 2);

  EXPECT_EQ(board_->getPlayerHand(Side::USSR).size(), 5);
  EXPECT_EQ(board_->getPlayerHand(Side::USA).size(), 2);
  EXPECT_EQ(board_->getDeck().getDeck().size(), initial_deck_size - 7);
}

TEST_F(BoardDrawTest, DrawCardsMoreThanDeckButUsaOnly) {
  // 捨て札にカードを追加（reshuffleのため）
  auto& discard_pile = board_->getDeck().getDiscardPile();
  for (int i = 0; i < 20; ++i) {
    discard_pile.push_back(CardEnum::DUMMY);
  }

  // デッキを奇数枚にする
  auto& deck = board_->getDeck().getDeck();
  while (deck.size() > 7) {
    deck.pop_back();
  }

  // 10枚ずつ引く（合計20枚、デッキは7枚）
  board_->drawCardsForPlayers(0, 8);

  EXPECT_EQ(board_->getPlayerHand(Side::USSR).size(), 0);
  EXPECT_EQ(board_->getPlayerHand(Side::USA).size(), 8);
  EXPECT_EQ(board_->getDeck().getDiscardPile().size(), 0);
}

// ヘッドラインカード管理のテスト
TEST_F(BoardDrawTest, HeadlineCardManagement) {
  // 初期状態はDummy
  EXPECT_EQ(board_->getHeadlineCard(Side::USSR), CardEnum::DUMMY);
  EXPECT_EQ(board_->getHeadlineCard(Side::USA), CardEnum::DUMMY);

  // ヘッドラインカードの設定
  board_->setHeadlineCard(Side::USSR, CardEnum::DUCK_AND_COVER);
  board_->setHeadlineCard(Side::USA, CardEnum::FIDEL);

  EXPECT_EQ(board_->getHeadlineCard(Side::USSR), CardEnum::DUCK_AND_COVER);
  EXPECT_EQ(board_->getHeadlineCard(Side::USA), CardEnum::FIDEL);

  // ヘッドラインカードのクリア
  board_->clearHeadlineCards();

  EXPECT_EQ(board_->getHeadlineCard(Side::USSR), CardEnum::DUMMY);
  EXPECT_EQ(board_->getHeadlineCard(Side::USA), CardEnum::DUMMY);
}

TEST_F(BoardDrawTest, HeadlineCardVisibility) {
  // 自分のカードは常に見える
  EXPECT_TRUE(board_->isHeadlineCardVisible(Side::USSR, Side::USSR));
  EXPECT_TRUE(board_->isHeadlineCardVisible(Side::USA, Side::USA));

  // 宇宙開発トラック4の優位性がない場合、相手のカードは見えない
  EXPECT_FALSE(board_->isHeadlineCardVisible(Side::USSR, Side::USA));
  EXPECT_FALSE(board_->isHeadlineCardVisible(Side::USA, Side::USSR));

  // USSRが宇宙開発トラック4に到達
  board_->getSpaceTrack().advanceSpaceTrack(Side::USSR, 4);

  // USSRからUSAのカードが見える
  EXPECT_TRUE(board_->isHeadlineCardVisible(Side::USSR, Side::USA));
  // USAからUSSRのカードは見えない
  EXPECT_FALSE(board_->isHeadlineCardVisible(Side::USA, Side::USSR));

  // USAも宇宙開発トラック4に到達（優位性がなくなる）
  board_->getSpaceTrack().advanceSpaceTrack(Side::USA, 4);

  // お互いに相手のカードが見えない
  EXPECT_FALSE(board_->isHeadlineCardVisible(Side::USSR, Side::USA));
  EXPECT_FALSE(board_->isHeadlineCardVisible(Side::USA, Side::USSR));
}
