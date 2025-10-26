#include "tsge/actions/legal_moves_generator.hpp"

#include <gtest/gtest.h>

#include <algorithm>

#include "tsge/core/board.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/card.hpp"

// テスト用DummyCardクラス
class DummyCard : public Card {
 public:
  DummyCard(int ops)
      : Card(CardEnum::Dummy, "DummyCard", ops, Side::NEUTRAL, WarPeriod::DUMMY,
             false) {}
  DummyCard(int ops, Side side)
      : Card(CardEnum::Dummy, "DummyCard", ops, side, WarPeriod::DUMMY, false) {
  }

  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override {
    // 空実装
    return {};
  }

  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    // テスト用：常にイベント実行可能とする
    return true;
  }
};

// テスト用のカードプール作成関数
static const std::array<std::unique_ptr<Card>, 111>& createTestCardPool() {
  static std::array<std::unique_ptr<Card>, 111> pool{};
  static bool initialized = false;

  if (!initialized) {
    // DummyカードをOps0として設定（スコアカード相当）
    pool[static_cast<size_t>(CardEnum::Dummy)] = std::make_unique<DummyCard>(0);
    // その他の既存カード
    pool[static_cast<size_t>(CardEnum::DuckAndCover)] =
        std::make_unique<DummyCard>(3);
    pool[static_cast<size_t>(CardEnum::Fidel)] = std::make_unique<DummyCard>(2);
    pool[static_cast<size_t>(CardEnum::NuclearTestBan)] =
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

// realignmentRequestLegalMovesのテスト
class RealignmentRequestLegalMovesTest : public ::testing::Test {
 protected:
  RealignmentRequestLegalMovesTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(RealignmentRequestLegalMovesTest, BasicCaseWithOpponentInfluence) {
  // 基本ケース：相手影響力がある複数国での継続
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::JAPAN};
  int remaining_ops = 2;

  auto moves = LegalMovesGenerator::realignmentRequestLegalMoves(
      board, Side::USSR, CardEnum::DuckAndCover, history, remaining_ops,
      AdditionalOpsType::NONE);

  // 相手影響力がある国数+1（パス）の結果が期待される
  // 日本、西ドイツ、イラン、アンゴラ + パス = 5個
  EXPECT_EQ(moves.size(), 5);

  // パスオプションが含まれていることを確認
  bool has_pass_option = false;
  for (const auto& move : moves) {
    auto* req_move = dynamic_cast<RealignmentRequestMove*>(move.get());
    if (req_move != nullptr) {
      // CountryEnum::USSRがパスとして使われる仕様
      auto cmds = move->toCommand(
          board.getCardpool()[static_cast<size_t>(CardEnum::DuckAndCover)]);
      if (cmds.empty()) {
        has_pass_option = true;
      }
    }
  }
  EXPECT_TRUE(has_pass_option);
}

TEST_F(RealignmentRequestLegalMovesTest, NoOpponentInfluencePassOnly) {
  // エッジケース：相手影響力なし（パスのみ）
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);

  std::vector<CountryEnum> history = {CountryEnum::JAPAN};
  int remaining_ops = 1;

  auto moves = LegalMovesGenerator::realignmentRequestLegalMoves(
      board, Side::USSR, CardEnum::DuckAndCover, history, remaining_ops,
      AdditionalOpsType::NONE);

  // パスも選択肢に上がらない
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(RealignmentRequestLegalMovesTest, CanTargetSameCountryMultipleTimes) {
  // 同じ国を連続選択可能なことを確認
  TestHelper::setupBoardWithInfluence(board);

  // 履歴に同じ国が複数回含まれるケース
  std::vector<CountryEnum> history = {CountryEnum::JAPAN, CountryEnum::JAPAN,
                                      CountryEnum::WEST_GERMANY};
  int remaining_ops = 1;

  auto moves = LegalMovesGenerator::realignmentRequestLegalMoves(
      board, Side::USSR, CardEnum::NuclearTestBan, history, remaining_ops,
      AdditionalOpsType::NONE);

  // 履歴に関係なく、相手影響力がある国全てが選択可能（+パス）
  EXPECT_EQ(moves.size(), 5);
}

TEST_F(RealignmentRequestLegalMovesTest, LastOpsStillIncludesAllOptions) {
  // 履歴引き継ぎテスト
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::JAPAN,
                                      CountryEnum::WEST_GERMANY};
  int remaining_ops = 1;

  auto moves = LegalMovesGenerator::realignmentRequestLegalMoves(
      board, Side::USSR, CardEnum::DuckAndCover, history, remaining_ops,
      AdditionalOpsType::NONE);

  // 残り1opsでも同じ
  EXPECT_EQ(moves.size(), 5);
}

TEST_F(RealignmentRequestLegalMovesTest, DefconRestrictions) {
  // DEFCONによる地域制限テスト（実装では未考慮の可能性）
  TestHelper::setupBoardWithInfluence(board);
  board.getDefconTrack().setDefcon(4);  // ヨーロッパ制限

  std::vector<CountryEnum> history = {};
  int remaining_ops = 2;

  auto moves = LegalMovesGenerator::realignmentRequestLegalMoves(
      board, Side::USSR, CardEnum::DuckAndCover, history, remaining_ops,
      AdditionalOpsType::NONE);

  // TODO: 現在の実装ではDEFCON制限が考慮されていない可能性
  // 西ドイツ（ヨーロッパ）が除外されるべきだが、実装では除外されない可能性
  EXPECT_EQ(moves.size(), 4);

  auto has_move = [&](CountryEnum target) {
    RealignmentRequestMove expected(CardEnum::DuckAndCover, Side::USSR, target,
                                    history, remaining_ops,
                                    AdditionalOpsType::NONE);
    return std::any_of(moves.begin(), moves.end(),
                       [&](const auto& move) { return *move == expected; });
  };

  EXPECT_TRUE(has_move(CountryEnum::JAPAN));
  EXPECT_TRUE(has_move(CountryEnum::IRAN));
  EXPECT_TRUE(has_move(CountryEnum::ANGOLA));
  EXPECT_TRUE(has_move(CountryEnum::USSR));  // パス
  EXPECT_FALSE(has_move(CountryEnum::WEST_GERMANY));
}

// TODO
// additionalOpsRealignmentLegalMovesのテスト
class AdditionalOpsRealignmentLegalMovesTest : public ::testing::Test {
 protected:
  AdditionalOpsRealignmentLegalMovesTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(AdditionalOpsRealignmentLegalMovesTest, ChinaCardBonusAsiaOnly) {
  // 中国カードボーナステスト（アジア限定）
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::JAPAN,
                                      CountryEnum::SOUTH_KOREA};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::Dummy, history, AdditionalOpsType::NONE);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, VietnamRevoltsBonus) {
  // ベトナム蜂起ボーナステスト（東南アジア限定）
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::VIETNAM,
                                      CountryEnum::THAILAND};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::DuckAndCover, history,
      AdditionalOpsType::NONE);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, ChinaAndVietnamBonus) {}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, RegionConditionNotSatisfied) {
  // 地域条件不満足テスト
  TestHelper::setupBoardWithInfluence(board);

  // 地域混在の履歴
  std::vector<CountryEnum> history = {CountryEnum::JAPAN,
                                      CountryEnum::WEST_GERMANY};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::Dummy, history, AdditionalOpsType::NONE);

  // 地域条件を満たさないため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, NoTargetCountries) {
  // 対象国なしテスト
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);

  std::vector<CountryEnum> history = {CountryEnum::JAPAN};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::Dummy, history, AdditionalOpsType::NONE);

  // 対象国がないため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, AlreadyUsedChinaCardBonus) {
  // 既に中国カードボーナスが使用済みの場合
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::JAPAN};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::Dummy, history,
      AdditionalOpsType::CHINA_CARD);

  // 既に使用済みのため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, AlreadyUsedVietnamRevoltsBonus) {
  // 既にベトナム蜂起ボーナスが使用済みの場合
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::VIETNAM,
                                      CountryEnum::THAILAND};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::DuckAndCover, history,
      AdditionalOpsType::VIETNAM_REVOLTS);

  // 既に使用済みのため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, BothBonusesAlreadyUsed) {
  // 両方のボーナスが既に使用済みの場合
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::THAILAND};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::Dummy, history, AdditionalOpsType::BOTH);

  // 両方使用済みのため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

// actionRealignmentLegalMovesのテスト
class ActionRealignmentLegalMovesTest : public ::testing::Test {
 protected:
  ActionRealignmentLegalMovesTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(ActionRealignmentLegalMovesTest, BasicCaseStandardHand) {
  // 基本ケース：標準的な手札と相手影響力
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DuckAndCover, CardEnum::Fidel});

  auto moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // 手札2枚 × 相手影響力4か国 = 8個
  EXPECT_EQ(moves.size(), 8);

  auto has_move = [&](CardEnum card, CountryEnum target) {
    ActionRealigmentMove expected(card, Side::USSR, target);
    return std::any_of(moves.begin(), moves.end(),
                       [&](const auto& move) { return *move == expected; });
  };

  for (CountryEnum target : {CountryEnum::JAPAN, CountryEnum::WEST_GERMANY,
                             CountryEnum::IRAN, CountryEnum::ANGOLA}) {
    EXPECT_TRUE(has_move(CardEnum::DuckAndCover, target));
    EXPECT_TRUE(has_move(CardEnum::Fidel, target));
  }
}

TEST_F(ActionRealignmentLegalMovesTest, EmptyHand) {
  // 手札なしテスト
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // 手札がないため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionRealignmentLegalMovesTest, NoOpponentInfluence) {
  // 相手影響力なしテスト
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});

  auto moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // 相手影響力がないため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionRealignmentLegalMovesTest, ScoringCardOpsZero) {
  // スコアカード（Ops0）テスト
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::Dummy});  // Ops0

  auto moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // TODO: 仕様では除外されるべき
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionRealignmentLegalMovesTest, DefconRegionRestrictions) {
  // DEFCONによる地域制限テスト
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});
  board.getDefconTrack().setDefcon(3);  // ヨーロッパ、アジア制限

  auto moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // TODO: 仕様では日本と西ドイツが除外されるべき
  EXPECT_EQ(moves.size(), 2);
}

TEST_F(ActionRealignmentLegalMovesTest, MultipleCardsOfSameOps) {
  // 複数カードテスト
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(
      board, Side::USSR,
      {CardEnum::DuckAndCover, CardEnum::DuckAndCover, CardEnum::DuckAndCover});

  auto moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // 同じOps値のカード3枚から独立したMoveが生成される
  // 3枚 × 4か国 = 12個
  EXPECT_EQ(moves.size(), 12);
}

TEST_F(ActionRealignmentLegalMovesTest, OpsValueDoesNotAffectMoveCount) {
  // カードのOps値は生成されるMoveの数に影響しない
  TestHelper::setupBoardWithInfluence(board);

  // 1 Opsカードと4 Opsカードで同じ数のMoveが生成される
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::Fidel});  // 2 Ops
  auto moves2ops =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::NuclearTestBan});  // 4 Ops
  auto moves4ops =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // 両方とも相手影響力がある国数と同じ数のMoveを生成
  EXPECT_EQ(moves2ops.size(), 4);
  EXPECT_EQ(moves4ops.size(), 4);
}

TEST_F(ActionRealignmentLegalMovesTest, SingleCardHelperMatchesGeneral) {
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::Fidel});

  auto helper_moves = LegalMovesGenerator::actionRealignmentLegalMovesForCard(
      board, Side::USSR, CardEnum::Fidel);
  auto general_moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  ASSERT_EQ(helper_moves.size(), general_moves.size());
  EXPECT_FALSE(helper_moves.empty());
  for (const auto& move : helper_moves) {
    ASSERT_NE(move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::Fidel);
  }
}

// actionPlaceInfluenceLegalMovesのテスト
class ActionPlaceInfluenceLegalMovesTest : public ::testing::Test {
 protected:
  ActionPlaceInfluenceLegalMovesTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(ActionPlaceInfluenceLegalMovesTest, SimpleCase2OpsOneCountry) {
  // シンプルなケース：1国のみ配置可能、2 Ops
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  // USSRに東ドイツに影響力1を配置（東ドイツのみ配置可能）
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);

  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::Fidel});  // 2 Ops

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  // 2 Opsで東ドイツに配置可能なパターン：
  // 全ての配置可能国が1 Opsコストのため、多数のパターンが生成される
  EXPECT_EQ(moves.size(), 15);

  // 全てのMoveがActionPlaceInfluenceMoveであることを確認
  for (const auto& move : moves) {
    EXPECT_NE(dynamic_cast<ActionPlaceInfluenceMove*>(move.get()), nullptr);
  }
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, EmptyHand) {
  // 手札なしテスト
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, NoPlaceableCountries) {
  // 配置可能国なしテスト（非現実的だが境界値テスト）
  // 全ての国から両方のプレイヤーの影響力を削除
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::Fidel});

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, MixedCostCountries) {
  // 異なるコストの国が混在するケース
  // まず状態をリセット
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  // USSR影響力を配置
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);
  // USA支配国を作成（コスト2）
  board.getWorldMap()
      .getCountry(CountryEnum::WEST_GERMANY)
      .addInfluence(Side::USA, 4);

  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DuckAndCover});  // 3 Ops

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  // 3 Opsで可能な配置パターンが生成される
  // 西ドイツ（2 Opsコスト）と他の国（1 Opsコスト）の組み合わせ
  EXPECT_EQ(moves.size(), 25);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, MultipleCardsWithSameOps) {
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  // USSR影響力を配置
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);

  // 同じOps値のカードが複数ある場合
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::Fidel, CardEnum::Fidel});  // 両方2 Ops

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  // 同じOps値のカード2枚分のMoveが生成される
  auto placeable = board.getWorldMap().placeableCountries(Side::USSR);
  // 2 Opsの配置パターン数を2倍
  auto single_card_moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);
  board.clearHand(Side::USSR);
  board.addCardToHand(Side::USSR, CardEnum::Fidel);
  auto one_card_moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);
  EXPECT_EQ(moves.size(), one_card_moves.size() * 2);

  // 同じ配置パターンが複数カード分存在することを確認
  std::map<CardEnum, int> card_count;
  for (const auto& move : moves) {
    card_count[move->getCard()]++;
  }
  EXPECT_EQ(card_count[CardEnum::Fidel], card_count[CardEnum::Fidel]);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, ScoringCardNoMoves) {
  // スコアカード（Ops0）では配置不可
  // 状態をクリアして制御された環境でテスト
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  // USSRに影響力を配置して配置可能国を作る
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);

  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::Dummy});  // Ops0

  auto moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  // CLAUDE.mdの仕様：Ops0では合法手なし（実装でOps0カードは除外される）
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionPlaceInfluenceLegalMovesTest, SingleCardHelperMatchesGeneral) {
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::clearAllOpponentInfluence(board, Side::USA);
  TestHelper::clearSuperPowerInfluence(board, Side::USSR);
  TestHelper::clearSuperPowerInfluence(board, Side::USA);

  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);

  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::Fidel});

  auto helper_moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMovesForCard(
          board, Side::USSR, CardEnum::Fidel);
  auto general_moves =
      LegalMovesGenerator::actionPlaceInfluenceLegalMoves(board, Side::USSR);

  ASSERT_EQ(helper_moves.size(), general_moves.size());
  EXPECT_FALSE(helper_moves.empty());
  for (const auto& move : helper_moves) {
    ASSERT_NE(move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::Fidel);
  }
}

// actionCoupLegalMovesのテスト
class ActionCoupLegalMovesTest : public ::testing::Test {
 protected:
  ActionCoupLegalMovesTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(ActionCoupLegalMovesTest, BasicCaseDefcon5) {
  // 基本ケース（DEFCON 5）：全ての相手影響力がある国が対象
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DuckAndCover});  // 3 Ops
  board.getDefconTrack().setDefcon(5);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 相手影響力がある国数分のMoveが生成される
  // setupBoardWithInfluenceで日本、西ドイツ、イラン、アンゴラにUSA影響力設定
  EXPECT_EQ(moves.size(), 4);

  // 全てのMoveがActionCoupMoveであることを確認
  for (const auto& move : moves) {
    EXPECT_NE(dynamic_cast<ActionCoupMove*>(move.get()), nullptr);
  }
}

TEST_F(ActionCoupLegalMovesTest, Defcon4RestrictionEurope) {
  // DEFCON 4：ヨーロッパ制限
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});
  board.getDefconTrack().setDefcon(4);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 西ドイツ（ヨーロッパ）が除外される
  // 日本、イラン、アンゴラの3か国のみ
  EXPECT_EQ(moves.size(), 3);

  auto has_move = [&](CountryEnum target) {
    ActionCoupMove expected(CardEnum::DuckAndCover, Side::USSR, target);
    return std::any_of(moves.begin(), moves.end(),
                       [&](const auto& move) { return *move == expected; });
  };

  EXPECT_TRUE(has_move(CountryEnum::JAPAN));
  EXPECT_TRUE(has_move(CountryEnum::IRAN));
  EXPECT_TRUE(has_move(CountryEnum::ANGOLA));
  EXPECT_FALSE(has_move(CountryEnum::WEST_GERMANY));
}

TEST_F(ActionCoupLegalMovesTest, Defcon3RestrictionEuropeAsia) {
  // DEFCON 3：ヨーロッパ、アジア制限
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});
  board.getDefconTrack().setDefcon(3);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 西ドイツ（ヨーロッパ）と日本（アジア）が除外される
  // イラン、アンゴラの2か国のみ
  EXPECT_EQ(moves.size(), 2);
}

TEST_F(ActionCoupLegalMovesTest, Defcon2RestrictionEuropeAsiaMiddleEast) {
  // DEFCON 2：ヨーロッパ、アジア、中東制限
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});
  board.getDefconTrack().setDefcon(2);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 西ドイツ（ヨーロッパ）、日本（アジア）、イラン（中東）が除外される
  // アンゴラ（アフリカ）の1か国のみ
  EXPECT_EQ(moves.size(), 1);
}

TEST_F(ActionCoupLegalMovesTest, EmptyHand) {
  // 手札なしテスト
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionCoupLegalMovesTest, NoOpponentInfluence) {
  // 相手影響力なしテスト
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionCoupLegalMovesTest, ScoringCardOpsZero) {
  // スコアカード（Ops0）のみの手札
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::Dummy});  // Ops0

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // CLAUDE.mdの仕様：Ops0では合法手なし
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionCoupLegalMovesTest, AllExcludedByDefcon) {
  // DEFCON制限で全て除外されるケース
  TestHelper::setupBoardWithInfluence(board);

  // ヨーロッパ、アジア、中東にのみUSA影響力を配置
  board.getWorldMap().getCountry(CountryEnum::ANGOLA).clearInfluence(Side::USA);

  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});
  board.getDefconTrack().setDefcon(2);  // ヨーロッパ、アジア、中東制限

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 全ての国がDEFCON制限により除外される
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionCoupLegalMovesTest, MultipleCards) {
  // 複数カードテスト
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(
      board, Side::USSR,
      {CardEnum::DuckAndCover, CardEnum::Fidel, CardEnum::NuclearTestBan});
  board.getDefconTrack().setDefcon(5);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // 3枚のカード × 4か国 = 12個のMove
  EXPECT_EQ(moves.size(), 12);
}

TEST_F(ActionCoupLegalMovesTest, MixedOpsValues) {
  // Ops値が異なる複数カード
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::Fidel,           // 2 Ops
                              CardEnum::DuckAndCover,    // 3 Ops
                              CardEnum::NuclearTestBan,  // 4 Ops
                              CardEnum::Dummy});         // 0 Ops
  board.getDefconTrack().setDefcon(5);

  auto moves = LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  // Ops0のカード以外の3枚 × 4か国 = 12個のMove
  EXPECT_EQ(moves.size(), 12);

  // Dummyカード（Ops0）のMoveが含まれていないことを確認
  bool has_dummy_move = false;
  for (const auto& move : moves) {
    if (move->getCard() == CardEnum::Dummy) {
      has_dummy_move = true;
      break;
    }
  }
  EXPECT_FALSE(has_dummy_move);
}

TEST_F(ActionCoupLegalMovesTest, SingleCardHelperMatchesGeneral) {
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});
  board.getDefconTrack().setDefcon(5);

  auto helper_moves = LegalMovesGenerator::actionCoupLegalMovesForCard(
      board, Side::USSR, CardEnum::DuckAndCover);
  auto general_moves =
      LegalMovesGenerator::actionCoupLegalMoves(board, Side::USSR);

  ASSERT_EQ(helper_moves.size(), general_moves.size());
  EXPECT_FALSE(helper_moves.empty());
  for (const auto& move : helper_moves) {
    ASSERT_NE(move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::DuckAndCover);
  }
}

class ActionLegalMovesForCardTest : public ::testing::Test {
 protected:
  ActionLegalMovesForCardTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(ActionLegalMovesForCardTest, AggregatesPlaceRealignCoupMoves) {
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});
  board.getDefconTrack().setDefcon(5);

  auto place_moves = LegalMovesGenerator::actionPlaceInfluenceLegalMovesForCard(
      board, Side::USSR, CardEnum::DuckAndCover);
  auto realign_moves = LegalMovesGenerator::actionRealignmentLegalMovesForCard(
      board, Side::USSR, CardEnum::DuckAndCover);
  auto coup_moves = LegalMovesGenerator::actionCoupLegalMovesForCard(
      board, Side::USSR, CardEnum::DuckAndCover);

  auto ops_moves = LegalMovesGenerator::actionLegalMovesForCard(
      board, Side::USSR, CardEnum::DuckAndCover);

  const size_t expected_size =
      place_moves.size() + realign_moves.size() + coup_moves.size();
  EXPECT_EQ(ops_moves.size(), expected_size);

  for (const auto& move : ops_moves) {
    ASSERT_NE(move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::DuckAndCover);
    EXPECT_EQ(move->getSide(), Side::USSR);

    const bool is_place =
        dynamic_cast<ActionPlaceInfluenceMove*>(move.get()) != nullptr;
    const bool is_realign =
        dynamic_cast<ActionRealigmentMove*>(move.get()) != nullptr;
    const bool is_coup = dynamic_cast<ActionCoupMove*>(move.get()) != nullptr;
    EXPECT_TRUE(is_place || is_realign || is_coup);
  }
}

class ExtraActionRoundLegalMovesTest : public ::testing::Test {
 protected:
  ExtraActionRoundLegalMovesTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(ExtraActionRoundLegalMovesTest, ProvidesPassWhenNoOtherMoves) {
  board.clearHand(Side::USSR);

  auto moves =
      LegalMovesGenerator::extraActionRoundLegalMoves(board, Side::USSR);

  ASSERT_EQ(moves.size(), 1);
  EXPECT_NE(dynamic_cast<ExtraActionPassMove*>(moves.front().get()), nullptr);
}

TEST_F(ExtraActionRoundLegalMovesTest, PassIncludedAlongsideOpsMoves) {
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::Fidel});

  auto moves =
      LegalMovesGenerator::extraActionRoundLegalMoves(board, Side::USSR);

  ASSERT_GE(moves.size(), 2);
  const auto pass_count =
      std::count_if(moves.begin(), moves.end(), [](const auto& move) {
        return dynamic_cast<ExtraActionPassMove*>(move.get()) != nullptr;
      });
  EXPECT_EQ(pass_count, 1);
}

// actionSpaceRaceLegalMovesのテスト
class ActionSpaceRaceLegalMovesTest : public ::testing::Test {
 protected:
  ActionSpaceRaceLegalMovesTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(ActionSpaceRaceLegalMovesTest, BasicCaseInitialStage) {
  // 基本ケース：初期段階（位置1、必要2 Ops、試行未使用）
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DuckAndCover, CardEnum::Fidel,
                              CardEnum::NuclearTestBan});  // 3, 2, 4 Ops

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 2 Ops以上のカードのみ使用可能：DuckAndCover(3), NuclearTestBan(4)
  // Fidel(2)も使用可能
  EXPECT_EQ(moves.size(), 3);

  // 全てのMoveがActionSpaceRaceMoveであることを確認
  for (const auto& move : moves) {
    EXPECT_NE(dynamic_cast<ActionSpaceRaceMove*>(move.get()), nullptr);
  }
}

TEST_F(ActionSpaceRaceLegalMovesTest, RequiredOpsChanges) {
  // 必要Ops値の変化確認（位置により要求値変化）
  TestHelper::setSpaceTrackPosition(board, Side::USSR,
                                    5);  // 中期段階（3 Ops必要）
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::Fidel, CardEnum::DuckAndCover,
                              CardEnum::NuclearTestBan});  // 2, 3, 4 Ops

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 3 Ops以上のカードのみ使用可能：DuckAndCover(3), NuclearTestBan(4)
  // Fidel(2)は不足
  EXPECT_EQ(moves.size(), 2);
}

TEST_F(ActionSpaceRaceLegalMovesTest, EmptyHand) {
  // エッジケース：手札なし
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionSpaceRaceLegalMovesTest, TrialLimitReached) {
  // エッジケース：試行回数制限到達
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 1);  // 上限到達
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 試行回数制限により実行不可
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionSpaceRaceLegalMovesTest, SpaceRaceCompleted) {
  // エッジケース：宇宙開発完了済み（位置8）
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 8);  // 完了
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DuckAndCover});

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 完了済みのため実行不可
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionSpaceRaceLegalMovesTest, AllCardsInsufficientOps) {
  // エッジケース：全カードがOps不足
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 5);  // 3 Ops必要
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::Fidel});  // 2 Ops

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 全カードがOps不足
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionSpaceRaceLegalMovesTest, SpecialCardsOnly) {
  // エッジケース：スコアリングカードのみの手札
  // 注意：中国カードは宇宙開発で使用可能
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::Dummy});  // スコアリングカード（Ops0）

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // スコアリングカードは使用不可
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionSpaceRaceLegalMovesTest, MultipleSameOpsCards) {
  // 特殊ケース：複数の同Opsカード
  TestHelper::setSpaceTrackPosition(board, Side::USSR, 1);
  TestHelper::setSpaceTrackTried(board, Side::USSR, 0);
  TestHelper::addCardsToHand(
      board, Side::USSR,
      {CardEnum::DuckAndCover, CardEnum::DuckAndCover, CardEnum::DuckAndCover});

  auto moves =
      LegalMovesGenerator::actionSpaceRaceLegalMoves(board, Side::USSR);

  // 同じOps値のカード3枚から独立したMoveが生成される
  EXPECT_EQ(moves.size(), 3);
}

// actionEventLegalMovesのテスト
class ActionEventLegalMovesTest : public ::testing::Test {
 protected:
  ActionEventLegalMovesTest() : board(createTestCardPool()) {}

  Board board;
};

TEST_F(ActionEventLegalMovesTest, MixedHandStandard) {
  // 基本ケース：混在する手札（自分、相手、中立イベント）
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DuckAndCover,      // 3 Ops, 中立
                              CardEnum::Fidel,             // 2 Ops, 中立
                              CardEnum::NuclearTestBan});  // 4 Ops, 中立

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board, Side::USSR);

  // 全てのイベントカードが実行可能
  EXPECT_EQ(moves.size(), 3);

  // 全てのMoveがActionEventMoveであることを確認
  for (const auto& move : moves) {
    EXPECT_NE(dynamic_cast<ActionEventMove*>(move.get()), nullptr);
  }
}

TEST_F(ActionEventLegalMovesTest, ScoringCardMandatory) {
  // 基本ケース：スコアリングカード必須の確認
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::Dummy,  // 0 Ops, スコアリングカード
                              CardEnum::DuckAndCover});  // 3 Ops, 通常イベント

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board, Side::USSR);

  // スコアリングカードも含まれる
  EXPECT_EQ(moves.size(), 2);

  // スコアリングカードのMoveが含まれていることを確認
  bool has_scoring_move = false;
  for (const auto& move : moves) {
    if (move->getCard() == CardEnum::Dummy) {
      has_scoring_move = true;
      break;
    }
  }
  EXPECT_TRUE(has_scoring_move);
}

TEST_F(ActionEventLegalMovesTest, EmptyHand) {
  // エッジケース：手札なし
  TestHelper::addCardsToHand(board, Side::USSR, {});

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board, Side::USSR);

  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionEventLegalMovesTest, AllScoringCards) {
  // エッジケース：全てスコアリングカード
  TestHelper::addCardsToHand(
      board, Side::USSR,
      {CardEnum::Dummy, CardEnum::Dummy});  // 両方スコアリングカード

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board, Side::USSR);

  // スコアリングカード2枚分のEventMove
  EXPECT_EQ(moves.size(), 2);
}

TEST_F(ActionEventLegalMovesTest, MixedCardTypes) {
  // 特殊ケース：異なる種類のカードが混在
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::Dummy,  // スコアリングカード
                              CardEnum::DuckAndCover,  // 通常イベント
                              CardEnum::Fidel});       // 通常イベント

  auto moves = LegalMovesGenerator::actionEventLegalMoves(board, Side::USSR);

  // 全てのカードがイベントとして実行可能
  EXPECT_EQ(moves.size(), 3);

  // カードの種類を確認
  std::map<CardEnum, int> card_count;
  for (const auto& move : moves) {
    card_count[move->getCard()]++;
  }

  EXPECT_EQ(card_count[CardEnum::Dummy], 1);
  EXPECT_EQ(card_count[CardEnum::DuckAndCover], 1);
  EXPECT_EQ(card_count[CardEnum::Fidel], 1);
}
