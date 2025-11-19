// どこで: tests/actions/command_test.cpp
// 何を: Command適用ロジックとRequestCommandの契約を検証する単体テスト
// なぜ: 各種コマンドの仕様を退行から守り、入力要求の基本挙動を固定するため

#include "tsge/actions/command.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <random>

#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/game_state/card.hpp"
#include "tsge/game_state/cards.hpp"

// RequestCommandのlegalMoves戻り値を追跡するための簡易Move実装
class StubMove final : public Move {
 public:
  StubMove(Side side, int identifier)
      : Move(CardEnum::DUMMY, side), identifier_(identifier) {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(const std::unique_ptr<Card>& /*card*/,
                                    const Board& /*board*/) const override {
    return {};
  }

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    const auto* other_cast = dynamic_cast<const StubMove*>(&other);
    return other_cast != nullptr && identifier_ == other_cast->identifier_ &&
           getSide() == other_cast->getSide();
  }

 private:
  int identifier_;
};

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

class CommandTest : public ::testing::Test {
 protected:
  CommandTest() : board(defaultCardPool()) {}

  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    if (!pool[0]) {
      // Create a dummy card for testing
      pool[0] = std::make_unique<DummyCard>();
      pool[static_cast<std::size_t>(CardEnum::CHINA_CARD)] =
          std::make_unique<ChinaCard>();
    }
    return pool;
  }

  Board board;
};

TEST_F(CommandTest, PlaceTest) {
  // 単一国への影響力配置テスト
  auto& north_korea = board.getWorldMap().getCountry(CountryEnum::NORTH_KOREA);
  EXPECT_EQ(north_korea.getInfluence(Side::USA), 0);  // 初期状態確認

  PlaceInfluenceCommand action_single_country(Side::USA, board.getCardpool()[0],
                                              {{CountryEnum::NORTH_KOREA, 2}});
  action_single_country.apply(board);
  EXPECT_EQ(north_korea.getInfluence(Side::USA),
            2);  // 影響力が正しく配置されたか確認

  // 複数国への同時配置テスト
  auto& south_korea = board.getWorldMap().getCountry(CountryEnum::SOUTH_KOREA);
  auto& japan = board.getWorldMap().getCountry(CountryEnum::JAPAN);
  EXPECT_EQ(south_korea.getInfluence(Side::USSR), 0);  // 初期状態確認
  EXPECT_EQ(japan.getInfluence(Side::USSR), 0);

  PlaceInfluenceCommand action_multiple_countries(
      Side::USSR, board.getCardpool()[0],
      {{CountryEnum::SOUTH_KOREA, 3}, {CountryEnum::JAPAN, 2}});
  action_multiple_countries.apply(board);
  EXPECT_EQ(south_korea.getInfluence(Side::USSR),
            3);  // 両国に正しく配置されたか確認
  EXPECT_EQ(japan.getInfluence(Side::USSR), 2);

  // 累積的な影響力配置テスト（同じ国に追加配置）
  PlaceInfluenceCommand action_cumulative(Side::USSR, board.getCardpool()[0],
                                          {{CountryEnum::JAPAN, 1}});
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

TEST_F(CommandTest, ChinaCardCoupAddsBonusOpsInAsia) {
  board.getMilopsTrack().resetMilopsTrack();

  auto& north_korea = board.getWorldMap().getCountry(CountryEnum::NORTH_KOREA);
  north_korea.clearInfluence(Side::USA);
  north_korea.clearInfluence(Side::USSR);
  north_korea.addInfluence(Side::USA, 2);

  std::mt19937_64 rng(0);
  board.getRandomizer().setRng(&rng);

  ActionCoupCommand china_coup(
      Side::USSR,
      board.getCardpool()[static_cast<size_t>(CardEnum::CHINA_CARD)],
      CountryEnum::NORTH_KOREA);
  china_coup.apply(board);

  EXPECT_EQ(board.getMilopsTrack().getMilops(Side::USSR), 5);

  board.getRandomizer().setRng(nullptr);
}

TEST_F(CommandTest, ChinaCardCoupNoBonusOutsideAsia) {
  board.getMilopsTrack().resetMilopsTrack();

  auto& angola = board.getWorldMap().getCountry(CountryEnum::ANGOLA);
  angola.clearInfluence(Side::USA);
  angola.clearInfluence(Side::USSR);
  angola.addInfluence(Side::USA, 2);

  std::mt19937_64 rng(0);
  board.getRandomizer().setRng(&rng);

  ActionCoupCommand china_coup(
      Side::USSR,
      board.getCardpool()[static_cast<size_t>(CardEnum::CHINA_CARD)],
      CountryEnum::ANGOLA);
  china_coup.apply(board);

  EXPECT_EQ(board.getMilopsTrack().getMilops(Side::USSR), 4);

  board.getRandomizer().setRng(nullptr);
}

TEST_F(CommandTest, FinalizeCardPlayCommandMovesCardToDiscard) {
  board.addCardToHand(Side::USA, CardEnum::DUMMY);
  FinalizeCardPlayCommand finalize(Side::USA, CardEnum::DUMMY, false);

  finalize.apply(board);

  EXPECT_TRUE(board.getPlayerHand(Side::USA).empty());
  const auto& discard = board.getDeck().getDiscardPile();
  ASSERT_FALSE(discard.empty());
  EXPECT_EQ(discard.back(), CardEnum::DUMMY);
  EXPECT_TRUE(board.getDeck().getRemovedCards().empty());
}

TEST_F(CommandTest, FinalizeCardPlayCommandMovesCardToRemoved) {
  board.addCardToHand(Side::USSR, CardEnum::DUMMY);
  FinalizeCardPlayCommand finalize(Side::USSR, CardEnum::DUMMY, true);

  finalize.apply(board);

  EXPECT_TRUE(board.getPlayerHand(Side::USSR).empty());
  const auto& removed = board.getDeck().getRemovedCards();
  ASSERT_FALSE(removed.empty());
  EXPECT_EQ(removed.back(), CardEnum::DUMMY);
  EXPECT_TRUE(board.getDeck().getDiscardPile().empty());
}

TEST_F(CommandTest, FinalizeChinaCardTransfersOwnershipAndAvoidsDiscard) {
  ASSERT_EQ(board.getChinaCardOwner(), Side::USSR);
  FinalizeCardPlayCommand finalize(Side::USSR, CardEnum::CHINA_CARD, false);

  finalize.apply(board);

  EXPECT_EQ(board.getChinaCardOwner(), Side::USA);
  EXPECT_FALSE(board.isChinaCardFaceUp());
  EXPECT_TRUE(board.getDeck().getDiscardPile().empty());
  EXPECT_TRUE(board.getDeck().getRemovedCards().empty());
}

TEST_F(CommandTest, SpaceRaceCommandSuccessAdvancesTrackAndAwardsVp) {
  // ロール最大値以下になるシードを探索し、成功パスを確実に再現する
  auto& space_track = board.getSpaceTrack();
  const int roll_max = space_track.getRollMax(Side::USSR);
  std::uniform_int_distribution<int> dice(1, 6);
  std::mt19937_64 success_rng;
  int success_roll = 0;
  for (std::mt19937_64::result_type seed = 0; seed < 1000; ++seed) {
    std::mt19937_64 candidate(seed);
    auto candidate_copy = candidate;
    const int roll = dice(candidate_copy);
    if (roll <= roll_max) {
      success_rng = candidate;
      success_roll = roll;
      break;
    }
  }
  ASSERT_NE(success_roll, 0);

  // 成功時は進行が+1され、VP変化コマンドがpushされることを検証する
  board.getRandomizer().setRng(&success_rng);
  ActionSpaceRaceCommand action(Side::USSR, board.getCardpool()[0]);
  action.apply(board);
  board.getRandomizer().setRng(nullptr);

  EXPECT_EQ(space_track.getSpaceTrackPosition(Side::USSR), 1);
  auto& states = board.getStates();
  ASSERT_EQ(states.size(), 1);
  ASSERT_TRUE(std::holds_alternative<CommandPtr>(states.back()));
  auto change_vp_ptr = std::get<CommandPtr>(states.back());
  ASSERT_NE(change_vp_ptr, nullptr);
  auto* change_vp_cmd = dynamic_cast<ChangeVpCommand*>(change_vp_ptr.get());
  ASSERT_NE(change_vp_cmd, nullptr);
}

TEST_F(CommandTest, SpaceRaceCommandFailureDoesNotAdvanceTrack) {
  // ロール最大値を超えるシードを探索し、失敗パスを再現する
  auto& space_track = board.getSpaceTrack();
  const int roll_max = space_track.getRollMax(Side::USSR);
  std::uniform_int_distribution<int> dice(1, 6);
  std::mt19937_64 failure_rng;
  int failure_roll = 0;
  for (std::mt19937_64::result_type seed = 0; seed < 1000; ++seed) {
    std::mt19937_64 candidate(seed);
    auto candidate_copy = candidate;
    const int roll = dice(candidate_copy);
    if (roll > roll_max) {
      failure_rng = candidate;
      failure_roll = roll;
      break;
    }
  }
  ASSERT_GT(failure_roll, roll_max);

  // 失敗時は位置維持かつstate未追加で、spaceTried実行による挑戦不可を確認
  board.getRandomizer().setRng(&failure_rng);
  ActionSpaceRaceCommand action(Side::USSR, board.getCardpool()[0]);
  action.apply(board);
  board.getRandomizer().setRng(nullptr);

  EXPECT_EQ(space_track.getSpaceTrackPosition(Side::USSR), 0);
  EXPECT_TRUE(board.getStates().empty());
  EXPECT_FALSE(space_track.canSpaceChallenge(Side::USSR));
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

TEST_F(CommandTest, RequestCommandLegalMovesLambdaIsInvoked) {
  // Arrange: legalMovesラムダが呼ばれた回数と参照を追跡
  int call_count = 0;
  const Board* observed_board = nullptr;
  auto expected_move = std::make_shared<StubMove>(Side::USA, 42);

  RequestCommand command(Side::USA, [this, &call_count, &observed_board,
                                     expected_move](const Board& ref) {
    ++call_count;
    observed_board = &ref;
    return std::vector<std::shared_ptr<Move>>{expected_move};
  });

  // Act: legalMovesを実行し戻り値を取得
  const auto moves = command.legalMoves(board);

  // Assert: ラムダが1回だけ呼ばれ、返されたMoveが期待通り
  ASSERT_EQ(call_count, 1);
  ASSERT_EQ(observed_board, &board);
  ASSERT_EQ(moves.size(), 1U);
  EXPECT_EQ(moves.front(), expected_move);
}

TEST_F(CommandTest, RequestCommandGetSide) {
  // Arrange: USSR側で初期化し、legalMovesは空コレクションを返す
  RequestCommand command(Side::USSR, [](const Board&) {
    return std::vector<std::shared_ptr<Move>>{};
  });

  // Assert: getSide()が注入されたSideをそのまま返す
  EXPECT_EQ(command.getSide(), Side::USSR);
}
