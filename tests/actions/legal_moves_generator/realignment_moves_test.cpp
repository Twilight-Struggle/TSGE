#include "test_helper.hpp"
#include "tsge/actions/legal_moves_generator.hpp"

// realignmentRequestLegalMovesのテスト
class RealignmentRequestLegalMovesTest : public ::testing::Test {
 protected:
  RealignmentRequestLegalMovesTest() : board(createTestCardPool()) {}

  void SetUp() override { board.giveChinaCardTo(Side::USA, false); }

  Board board;
};

TEST_F(RealignmentRequestLegalMovesTest, BasicCaseWithOpponentInfluence) {
  // 基本ケース：相手影響力がある複数国での継続
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::JAPAN};
  int remaining_ops = 2;

  auto moves = LegalMovesGenerator::realignmentRequestLegalMoves(
      board, Side::USSR, CardEnum::DUCK_AND_COVER, history, remaining_ops,
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
          board.getCardpool()[static_cast<size_t>(CardEnum::DUCK_AND_COVER)],
          board);
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
      board, Side::USSR, CardEnum::DUCK_AND_COVER, history, remaining_ops,
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
      board, Side::USSR, CardEnum::NUCLEAR_TEST_BAN, history, remaining_ops,
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
      board, Side::USSR, CardEnum::DUCK_AND_COVER, history, remaining_ops,
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
      board, Side::USSR, CardEnum::DUCK_AND_COVER, history, remaining_ops,
      AdditionalOpsType::NONE);

  // TODO: 現在の実装ではDEFCON制限が考慮されていない可能性
  // 西ドイツ（ヨーロッパ）が除外されるべきだが、実装では除外されない可能性
  EXPECT_EQ(moves.size(), 4);

  auto has_move = [&](CountryEnum target) {
    RealignmentRequestMove expected(CardEnum::DUCK_AND_COVER, Side::USSR,
                                    target, history, remaining_ops,
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

  void SetUp() override { board.giveChinaCardTo(Side::USA, false); }

  Board board;
};

TEST_F(AdditionalOpsRealignmentLegalMovesTest, ChinaCardBonusAsiaOnly) {
  // 中国カードボーナステスト（アジア限定）
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {
      CountryEnum::JAPAN, CountryEnum::SOUTH_KOREA, CountryEnum::SOUTH_KOREA,
      CountryEnum::SOUTH_KOREA};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::CHINA_CARD, history,
      AdditionalOpsType::NONE);

  ASSERT_EQ(moves.size(), 2);

  RealignmentRequestMove expected_bonus(CardEnum::CHINA_CARD, Side::USSR,
                                        CountryEnum::JAPAN, history, 1,
                                        AdditionalOpsType::CHINA_CARD);
  const bool has_bonus_move =
      std::any_of(moves.begin(), moves.end(),
                  [&expected_bonus](const std::shared_ptr<Move>& move) {
                    return move != nullptr && *move == expected_bonus;
                  });
  EXPECT_TRUE(has_bonus_move);

  RealignmentRequestMove expected_pass(CardEnum::CHINA_CARD, Side::USSR,
                                       CountryEnum::USSR, history, 1,
                                       AdditionalOpsType::NONE);
  const bool has_pass_move =
      std::any_of(moves.begin(), moves.end(),
                  [&expected_pass](const std::shared_ptr<Move>& move) {
                    return move != nullptr && *move == expected_pass;
                  });
  EXPECT_TRUE(has_pass_move);
}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, VietnamRevoltsBonus) {
  // ベトナム蜂起ボーナステスト（東南アジア限定）
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::VIETNAM,
                                      CountryEnum::THAILAND};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::DUCK_AND_COVER, history,
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
      board, Side::USSR, CardEnum::CHINA_CARD, history,
      AdditionalOpsType::NONE);

  // 地域条件を満たさないため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, NoTargetCountries) {
  // 対象国なしテスト
  TestHelper::clearAllOpponentInfluence(board, Side::USSR);

  std::vector<CountryEnum> history = {CountryEnum::JAPAN};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::CHINA_CARD, history,
      AdditionalOpsType::NONE);

  // 対象国がないため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, AlreadyUsedChinaCardBonus) {
  // 既に中国カードボーナスが使用済みの場合
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::JAPAN};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::CHINA_CARD, history,
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
      board, Side::USSR, CardEnum::DUCK_AND_COVER, history,
      AdditionalOpsType::VIETNAM_REVOLTS);

  // 既に使用済みのため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(AdditionalOpsRealignmentLegalMovesTest, BothBonusesAlreadyUsed) {
  // 両方のボーナスが既に使用済みの場合
  TestHelper::setupBoardWithInfluence(board);

  std::vector<CountryEnum> history = {CountryEnum::THAILAND};

  auto moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board, Side::USSR, CardEnum::CHINA_CARD, history,
      AdditionalOpsType::BOTH);

  // 両方使用済みのため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

// actionRealignmentLegalMovesのテスト
class ActionRealignmentLegalMovesTest : public ::testing::Test {
 protected:
  ActionRealignmentLegalMovesTest() : board(createTestCardPool()) {}

  void SetUp() override { board.giveChinaCardTo(Side::USSR, false); }

  Board board;
};

TEST_F(ActionRealignmentLegalMovesTest, BasicCaseStandardHand) {
  // 基本ケース：標準的な手札と相手影響力
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::DUCK_AND_COVER, CardEnum::FIDEL});

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
    EXPECT_TRUE(has_move(CardEnum::DUCK_AND_COVER, target));
    EXPECT_TRUE(has_move(CardEnum::FIDEL, target));
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
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});

  auto moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // 相手影響力がないため、結果は空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionRealignmentLegalMovesTest, ScoringCardOpsZero) {
  // スコアカード（Ops0）テスト
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUMMY});  // Ops0

  auto moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // TODO: 仕様では除外されるべき
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(ActionRealignmentLegalMovesTest, DefconRegionRestrictions) {
  // DEFCONによる地域制限テスト
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::DUCK_AND_COVER});
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
      {CardEnum::DUCK_AND_COVER, CardEnum::DUCK_AND_COVER,
       CardEnum::DUCK_AND_COVER});

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
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::FIDEL});  // 2 Ops
  auto moves2ops =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  TestHelper::addCardsToHand(board, Side::USSR,
                             {CardEnum::NUCLEAR_TEST_BAN});  // 4 Ops
  auto moves4ops =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // 両方とも相手影響力がある国数と同じ数のMoveを生成
  EXPECT_EQ(moves2ops.size(), 4);
  EXPECT_EQ(moves4ops.size(), 4);
}

TEST_F(ActionRealignmentLegalMovesTest, ChinaCardAddsLegalMovesWhenFaceUp) {
  board.giveChinaCardTo(Side::USSR, true);
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::FIDEL});

  auto moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  // 手札1枚 + 中国カードで 2 枚分のムーブが生成される
  EXPECT_EQ(moves.size(), 8);
  const bool has_china_card_move =
      std::any_of(moves.begin(), moves.end(), [](const auto& move) {
        return move != nullptr && move->getCard() == CardEnum::CHINA_CARD;
      });
  EXPECT_TRUE(has_china_card_move);
}

TEST_F(ActionRealignmentLegalMovesTest, SingleCardHelperMatchesGeneral) {
  TestHelper::setupBoardWithInfluence(board);
  TestHelper::addCardsToHand(board, Side::USSR, {CardEnum::FIDEL});

  auto helper_moves = LegalMovesGenerator::actionRealignmentLegalMovesForCard(
      board, Side::USSR, CardEnum::FIDEL);
  auto general_moves =
      LegalMovesGenerator::actionRealignmentLegalMoves(board, Side::USSR);

  ASSERT_EQ(helper_moves.size(), general_moves.size());
  EXPECT_FALSE(helper_moves.empty());
  for (const auto& move : helper_moves) {
    ASSERT_NE(move, nullptr);
    EXPECT_EQ(move->getCard(), CardEnum::FIDEL);
  }
}
