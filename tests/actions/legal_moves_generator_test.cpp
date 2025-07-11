#include "tsge/actions/legal_moves_generator.hpp"

#include <gtest/gtest.h>

#include "tsge/core/board.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/card.hpp"

// テスト用DummyCardクラス
class DummyCard : public Card {
 public:
  DummyCard(int ops) : Card(0, "DummyCard", ops, Side::NEUTRAL, false) {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override {
    // 空実装
    return {};
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