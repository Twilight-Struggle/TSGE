#include <gtest/gtest.h>

#include <map>

#include "tsge/core/board.hpp"
#include "tsge/game_state/card.hpp"

class DummyCard : public Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  DummyCard(CardEnum id, WarPeriod warPeriod)
      : Card(id, "DummyCard", 3, Side::NEUTRAL, warPeriod, false) {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override {
    return {};
  }

  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    return true;
  }
};

class BoardMCTSTest : public ::testing::Test {
 protected:
  BoardMCTSTest() : board_(cardpool_) {}

  void SetUp() override {
    // テスト用のダミーカードを作成
    for (int i = 0; i < 111; ++i) {
      cardpool_[i] = std::make_unique<DummyCard>(static_cast<CardEnum>(i),
                                                 WarPeriod::EARLY_WAR);
    }
  }

  std::array<std::unique_ptr<Card>, 111> cardpool_;
  Board board_;
};

TEST_F(BoardMCTSTest, CopyForMCTS_HidesOpponentHand) {
  // 両プレイヤーに手札を配る
  board_.addCardToHand(Side::USSR, CardEnum::DUCK_AND_COVER);
  board_.addCardToHand(Side::USSR, CardEnum::FIDEL);
  board_.addCardToHand(Side::USA, CardEnum::NUCLEAR_TEST_BAN);
  board_.addCardToHand(Side::USA, CardEnum::DUCK_AND_COVER);

  // USSRの視点でコピーを作成
  Board ussr_copy = board_.copyForMCTS(Side::USSR);

  // USSRの手札は見える
  EXPECT_EQ(ussr_copy.getPlayerHand(Side::USSR).size(), 2);
  EXPECT_EQ(ussr_copy.getPlayerHand(Side::USSR)[0], CardEnum::DUCK_AND_COVER);
  EXPECT_EQ(ussr_copy.getPlayerHand(Side::USSR)[1], CardEnum::FIDEL);

  // USAの手札は隠蔽される（枚数は保持、内容はDummy）
  EXPECT_EQ(ussr_copy.getPlayerHand(Side::USA).size(), 2);
  EXPECT_EQ(ussr_copy.getPlayerHand(Side::USA)[0], CardEnum::DUMMY);
  EXPECT_EQ(ussr_copy.getPlayerHand(Side::USA)[1], CardEnum::DUMMY);

  // USAの視点でコピーを作成
  Board usa_copy = board_.copyForMCTS(Side::USA);

  // USAの手札は見える
  EXPECT_EQ(usa_copy.getPlayerHand(Side::USA).size(), 2);
  EXPECT_EQ(usa_copy.getPlayerHand(Side::USA)[0], CardEnum::NUCLEAR_TEST_BAN);
  EXPECT_EQ(usa_copy.getPlayerHand(Side::USA)[1], CardEnum::DUCK_AND_COVER);

  // USSRの手札は隠蔽される
  EXPECT_EQ(usa_copy.getPlayerHand(Side::USSR).size(), 2);
  EXPECT_EQ(usa_copy.getPlayerHand(Side::USSR)[0], CardEnum::DUMMY);
  EXPECT_EQ(usa_copy.getPlayerHand(Side::USSR)[1], CardEnum::DUMMY);
}

TEST_F(BoardMCTSTest, CopyForMCTS_HandlesEmptyHands) {
  // 空の手札でのテスト
  Board ussr_copy = board_.copyForMCTS(Side::USSR);

  EXPECT_EQ(ussr_copy.getPlayerHand(Side::USSR).size(), 0);
  EXPECT_EQ(ussr_copy.getPlayerHand(Side::USA).size(), 0);
}

TEST_F(BoardMCTSTest, CopyForMCTS_HidesHeadlineCards) {
  // ヘッドラインカードを設定
  board_.setHeadlineCard(Side::USSR, CardEnum::DUCK_AND_COVER);
  board_.setHeadlineCard(Side::USA, CardEnum::FIDEL);

  // USSRの視点でコピー（宇宙開発トラック4の優位性なし）
  Board ussr_copy = board_.copyForMCTS(Side::USSR);

  // 自分のヘッドラインカードは見える
  EXPECT_EQ(ussr_copy.getHeadlineCard(Side::USSR), CardEnum::DUCK_AND_COVER);

  // 相手のヘッドラインカードは隠蔽される
  EXPECT_EQ(ussr_copy.getHeadlineCard(Side::USA), CardEnum::DUMMY);
}

TEST_F(BoardMCTSTest, CopyForMCTS_SpaceTrackVisibility) {
  // ヘッドラインカードを設定
  board_.setHeadlineCard(Side::USSR, CardEnum::DUCK_AND_COVER);
  board_.setHeadlineCard(Side::USA, CardEnum::FIDEL);

  // USSRに宇宙開発トラック4の優位性を与える
  auto& space_track = board_.getSpaceTrack();
  space_track.advanceSpaceTrack(Side::USSR, 4);  // USSRを宇宙開発トラック4に

  // USSRの視点でコピー（宇宙開発トラック4の優位性あり）
  Board ussr_copy = board_.copyForMCTS(Side::USSR);

  // 宇宙開発トラック4の優位性があるため、相手のヘッドラインカードも見える
  EXPECT_EQ(ussr_copy.getHeadlineCard(Side::USSR), CardEnum::DUCK_AND_COVER);
  EXPECT_EQ(ussr_copy.getHeadlineCard(Side::USA), CardEnum::FIDEL);
}

TEST_F(BoardMCTSTest, CopyForMCTS_PreservesOtherData) {
  // VP、ARプレイヤーなどの他のデータを設定
  board_.changeVp(5);
  board_.setCurrentArPlayer(Side::USA);

  Board copy = board_.copyForMCTS(Side::USSR);

  // 他のデータは保持される
  EXPECT_EQ(copy.getVp(), 5);
  EXPECT_EQ(copy.getCurrentArPlayer(), Side::USA);
}

TEST_F(BoardMCTSTest, CopyForMCTS_CopiesStatesCorrectly) {
  // statesにStateTypeを追加
  board_.pushState(StateType::TURN_START);

  // statesにCommandPtrを追加
  std::map<CountryEnum, int> targets{{CountryEnum::JAPAN, 3}};
  auto command_ptr = std::make_shared<ActionPlaceInfluenceCommand>(
      Side::USSR, cardpool_[0], targets);
  board_.pushState(command_ptr);

  auto& states = board_.getStates();
  EXPECT_EQ(states.size(), 2);

  // コピーを作成
  Board copy = board_.copyForMCTS(Side::USSR);

  // states_が適切にコピーされていることを確認
  auto& copy_states = copy.getStates();
  EXPECT_EQ(copy_states.size(), 2);

  // StateTypeがコピーされていることを確認
  EXPECT_TRUE(std::holds_alternative<StateType>(copy_states[0]));
  EXPECT_EQ(std::get<StateType>(copy_states[0]), StateType::TURN_START);

  // CommandPtrがコピーされていることを確認
  EXPECT_TRUE(std::holds_alternative<CommandPtr>(copy_states[1]));

  // コピーされたCommandPtrが元のCommandPtrと同じインスタンスを共有していることを確認
  // (shared_ptrのため、shallow copyでは同じインスタンスを共有する)
  auto original_cmd = std::get<CommandPtr>(states[1]);
  auto copy_cmd = std::get<CommandPtr>(copy_states[1]);
  EXPECT_EQ(original_cmd.get(), copy_cmd.get());  // 同じポインタを共有

  // 元のstatesとコピーのstatesが独立していることを確認
  board_.pushState(StateType::HEADLINE_PHASE);
  copy.pushState(StateType::AR_USSR);

  EXPECT_EQ(board_.getStates().size(), 3);
  EXPECT_EQ(copy.getStates().size(), 3);

  // 内容は異なる
  auto& original_states = board_.getStates();
  EXPECT_TRUE(std::holds_alternative<StateType>(original_states[2]));
  EXPECT_EQ(std::get<StateType>(original_states[2]), StateType::HEADLINE_PHASE);

  EXPECT_TRUE(std::holds_alternative<StateType>(copy_states[2]));
  EXPECT_EQ(std::get<StateType>(copy_states[2]), StateType::AR_USSR);
}

TEST_F(BoardMCTSTest, CopyForMCTS_CopiesWorldMapAndDeckStates) {
  // WorldMapの状態を変更
  auto& world_map = board_.getWorldMap();
  world_map.getCountry(CountryEnum::JAPAN).addInfluence(Side::USSR, 5);

  // Deckの状態を変更
  auto& deck = board_.getDeck();
  deck.addEarlyWarCards();

  // コピーを作成
  Board copy = board_.copyForMCTS(Side::USSR);

  // WorldMapの状態がコピーされていることを確認
  auto& copy_world_map = copy.getWorldMap();
  EXPECT_EQ(
      copy_world_map.getCountry(CountryEnum::JAPAN).getInfluence(Side::USSR),
      5);

  // Deckのサイズがコピーされていることを確認
  auto& copy_deck = copy.getDeck();
  EXPECT_EQ(copy_deck.getDeck().size(), deck.getDeck().size());

  // 独立性の確認
  world_map.getCountry(CountryEnum::JAPAN).addInfluence(Side::USSR, 3);
  EXPECT_EQ(world_map.getCountry(CountryEnum::JAPAN).getInfluence(Side::USSR),
            8);
  EXPECT_EQ(
      copy_world_map.getCountry(CountryEnum::JAPAN).getInfluence(Side::USSR),
      5);
}

TEST_F(BoardMCTSTest, CopyForMCTS_HidesDeck) {
  auto& deck = board_.getDeck();
  deck.addEarlyWarCards();
  // USSRの視点でコピーを作成
  Board ussr_copy = board_.copyForMCTS(Side::USSR);

  // Deckは隠蔽される
  EXPECT_EQ(ussr_copy.getDeck().getDeck().size(),
            board_.getDeck().getDeck().size());
  EXPECT_EQ(ussr_copy.getDeck().getDeck()[0], CardEnum::DUMMY);
  EXPECT_EQ(ussr_copy.getDeck().getDeck()[1], CardEnum::DUMMY);
}