#include "tsge/actions/command.hpp"

#include <gtest/gtest.h>

#include "tsge/core/board.hpp"
#include "tsge/game_state/card.hpp"

// Dummy card class for testing
class DummyCard : public Card {
 public:
  DummyCard()
      : Card(CardEnum::Dummy, "Dummy", 3, Side::NEUTRAL, WarPeriod::DUMMY,
             false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override {
    return {};
  }
  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    return true;
  }
};

class CommandTest : public ::testing::Test {
 protected:
  CommandTest() : board(defaultCardPool()) {}

  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    if (!pool[0]) {
      // Create a dummy card for testing
      pool[0] = std::make_unique<DummyCard>();
    }
    return pool;
  }

  Board board;
};

TEST_F(CommandTest, PlaceTest) {
  // 単一国への影響力配置テスト
  auto& north_korea = board.getWorldMap().getCountry(CountryEnum::NORTH_KOREA);
  EXPECT_EQ(north_korea.getInfluence(Side::USA), 0);  // 初期状態確認

  ActionPlaceInfluenceCommand action_single_country(
      Side::USA, board.getCardpool()[0], {{CountryEnum::NORTH_KOREA, 2}});
  action_single_country.apply(board);
  EXPECT_EQ(north_korea.getInfluence(Side::USA),
            2);  // 影響力が正しく配置されたか確認

  // 複数国への同時配置テスト
  auto& south_korea = board.getWorldMap().getCountry(CountryEnum::SOUTH_KOREA);
  auto& japan = board.getWorldMap().getCountry(CountryEnum::JAPAN);
  EXPECT_EQ(south_korea.getInfluence(Side::USSR), 0);  // 初期状態確認
  EXPECT_EQ(japan.getInfluence(Side::USSR), 0);

  ActionPlaceInfluenceCommand action_multiple_countries(
      Side::USSR, board.getCardpool()[0],
      {{CountryEnum::SOUTH_KOREA, 3}, {CountryEnum::JAPAN, 2}});
  action_multiple_countries.apply(board);
  EXPECT_EQ(south_korea.getInfluence(Side::USSR),
            3);  // 両国に正しく配置されたか確認
  EXPECT_EQ(japan.getInfluence(Side::USSR), 2);

  // 累積的な影響力配置テスト（同じ国に追加配置）
  ActionPlaceInfluenceCommand action_cumulative(
      Side::USSR, board.getCardpool()[0], {{CountryEnum::JAPAN, 1}});
  action_cumulative.apply(board);
  EXPECT_EQ(japan.getInfluence(Side::USSR), 3);  // 2 + 1 = 3になっているか確認

  // 他の国には影響がないことを確認
  auto& angola = board.getWorldMap().getCountry(CountryEnum::ANGOLA);
  EXPECT_EQ(angola.getInfluence(Side::USA), 0);
  EXPECT_EQ(angola.getInfluence(Side::USSR), 0);
}

TEST_F(CommandTest, RealigmentTest) {
  // 相手が置いている国には影響力排除判定ができる
  ActionRealigmentCommand action_can_realigment_ussr(
      Side::USSR, board.getCardpool()[0], CountryEnum::SOUTH_KOREA);
  action_can_realigment_ussr.apply(board);
  ActionRealigmentCommand action_can_realigment_usa(
      Side::USA, board.getCardpool()[0], CountryEnum::NORTH_KOREA);
  action_can_realigment_usa.apply(board);
}

TEST_F(CommandTest, RealignmentWithUSAControlledAdjacentCountryTest) {
  // JAPANに対するRealignmentで、隣接するUSAがUSA支配状態であることを利用
  // USAはstability=100でUSA支配状態のため、usa_dice += 1が実行される
  ActionRealigmentCommand action_realigment_japan(
      Side::USSR, board.getCardpool()[0], CountryEnum::JAPAN);
  action_realigment_japan.apply(board);
}

TEST_F(CommandTest, CoupTest) {
  // 相手が置いてない国にはクーデターできない
  ActionCoupCommand action_cant_coup_ussr(Side::USSR, board.getCardpool()[0],
                                          CountryEnum::ANGOLA);
  action_cant_coup_ussr.apply(board);
  ActionCoupCommand action_cant_coup_usa(Side::USA, board.getCardpool()[0],
                                         CountryEnum::AFGHANISTAN);
  action_cant_coup_usa.apply(board);
  // 相手が置いている国にはクーデターできる
  ActionCoupCommand action_can_coup_ussr(Side::USSR, board.getCardpool()[0],
                                         CountryEnum::SOUTH_KOREA);
  action_can_coup_ussr.apply(board);
  ActionCoupCommand action_can_coup_usa(Side::USA, board.getCardpool()[0],
                                        CountryEnum::NORTH_KOREA);
  action_can_coup_usa.apply(board);
}

TEST_F(CommandTest, ChangeDefconCommandTest) {
  // 初期状態チェック
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 5);

  // Defcon変更テスト
  ChangeDefconCommand change_defcon(-2);
  change_defcon.apply(board);
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 3);

  // NORAD効果トリガーテスト（Defconを2に変更）
  board.getDefconTrack().setDefcon(3);
  ChangeDefconCommand trigger_norad(-1);
  trigger_norad.apply(board);
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 2);

  // 範囲制限テスト（changeDefconはclampする）
  ChangeDefconCommand exceed_max(10);
  exceed_max.apply(board);
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 5);  // 5でクランプされる

  ChangeDefconCommand exceed_min(-10);
  exceed_min.apply(board);
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 1);  // 1でクランプされる
}

TEST_F(CommandTest, ChangeVPCommandTest) {
  // 初期状態チェック
  EXPECT_EQ(board.getVp(), 0);

  // VP変更テスト（USSR側に+5VP）
  ChangeVpCommand change_vp(Side::USSR, 5);
  change_vp.apply(board);
  EXPECT_EQ(board.getVp(), 5);

  // 負のVP変更テスト（USA側に+3VP、実際は-3VP）
  ChangeVpCommand change_vp_negative(Side::USA, 3);
  change_vp_negative.apply(board);
  EXPECT_EQ(board.getVp(), 2);  // 5 + (-3) = 2

  // 大きなVP変更テスト（ゲーム終了条件チェック）
  ChangeVpCommand large_vp(Side::USSR, 25);
  large_vp.apply(board);
  EXPECT_EQ(board.getVp(), 27);  // ゲーム終了条件に達してGAME_ENDがpushされる

  // stateにUSSR_WIN_ENDが積まれていることを確認
  auto& states = board.getStates();
  EXPECT_FALSE(states.empty());
  EXPECT_TRUE(std::holds_alternative<StateType>(states.back()));
  EXPECT_EQ(std::get<StateType>(states.back()), StateType::USSR_WIN_END);
}

TEST_F(CommandTest, ChangeVpCommandUSAWinTest) {
  // 初期状態チェック
  EXPECT_EQ(board.getVp(), 0);

  // VP <= -20でUSA勝利テスト
  ChangeVpCommand usa_win_vp(Side::USA, 21);
  usa_win_vp.apply(board);
  EXPECT_EQ(board.getVp(), -21);  // VP=-21でUSA勝利条件に達する

  // stateにUSA_WIN_ENDが積まれていることを確認
  auto& states = board.getStates();
  EXPECT_FALSE(states.empty());
  EXPECT_TRUE(std::holds_alternative<StateType>(states.back()));
  EXPECT_EQ(std::get<StateType>(states.back()), StateType::USA_WIN_END);
}

TEST_F(CommandTest, GameEndTriggerTest) {
  auto& states = board.getStates();

  // 初期状態：statesが空
  EXPECT_TRUE(states.empty());

  // DEFCON 1でゲーム終了トリガー（NEUTRALの場合はelse節でUSSR勝利）
  ChangeDefconCommand end_by_defcon(-4);  // 5 → 1
  end_by_defcon.apply(board);

  // 終了StateTypeがstackにpushされることを確認（NEUTRALの場合USSR勝利）
  EXPECT_FALSE(states.empty());
  EXPECT_TRUE(std::holds_alternative<StateType>(states.back()));
  // getCurrentArPlayer()がNEUTRALなのでelse節が実行されUSSR勝利
  EXPECT_EQ(std::get<StateType>(states.back()), StateType::USSR_WIN_END);

  // statesをリセット
  states.clear();

  // VP 20でゲーム終了トリガー
  ChangeVpCommand end_by_vp(Side::USSR, 20);
  end_by_vp.apply(board);

  // 終了StateTypeがstackにpushされることを確認（VP 20でUSSR勝利）
  EXPECT_FALSE(states.empty());
  EXPECT_TRUE(std::holds_alternative<StateType>(states.back()));
  EXPECT_EQ(std::get<StateType>(states.back()), StateType::USSR_WIN_END);
}

TEST_F(CommandTest, DefconOneWithArPlayerTest) {
  auto& states = board.getStates();

  // USSR ARで DEFCON 1 に到達した場合
  board.setCurrentArPlayer(Side::USSR);
  board.getDefconTrack().setDefcon(2);  // 開始時DEFCON 2

  ChangeDefconCommand ussr_triggers_defcon_one(-1);
  ussr_triggers_defcon_one.apply(board);

  // USSR敗北 = USA勝利
  EXPECT_FALSE(states.empty());
  EXPECT_TRUE(std::holds_alternative<StateType>(states.back()));
  EXPECT_EQ(std::get<StateType>(states.back()), StateType::USA_WIN_END);

  // statesをリセット
  states.clear();
  board.getDefconTrack().setDefcon(2);  // DEFCON 2にリセット

  // USA ARで DEFCON 1 に到達した場合
  board.setCurrentArPlayer(Side::USA);

  ChangeDefconCommand usa_triggers_defcon_one(-1);
  usa_triggers_defcon_one.apply(board);

  // USA敗北 = USSR勝利
  EXPECT_FALSE(states.empty());
  EXPECT_TRUE(std::holds_alternative<StateType>(states.back()));
  EXPECT_EQ(std::get<StateType>(states.back()), StateType::USSR_WIN_END);
}

TEST_F(CommandTest, BoardArPlayerManagementTest) {
  // 初期状態はNEUTRAL
  EXPECT_EQ(board.getCurrentArPlayer(), Side::NEUTRAL);

  // ARプレイヤーの設定
  board.setCurrentArPlayer(Side::USSR);
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USSR);

  board.setCurrentArPlayer(Side::USA);
  EXPECT_EQ(board.getCurrentArPlayer(), Side::USA);

  // NEUTRALに戻す
  board.setCurrentArPlayer(Side::NEUTRAL);
  EXPECT_EQ(board.getCurrentArPlayer(), Side::NEUTRAL);
}
