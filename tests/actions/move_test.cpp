#include "tsge/actions/move.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "tsge/actions/command.hpp"
#include "tsge/actions/legal_moves_generator.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/card.hpp"

// テスト用DummyCardクラス
class DummyCard : public Card {
 public:
  DummyCard(int ops, Side side = Side::NEUTRAL, bool removedAfterEvent = false)
      : Card(CardEnum::DUMMY, "DummyCard", ops, side, WarPeriod::DUMMY,
             removedAfterEvent) {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side side,
                                const Board& /*board*/) const override {
    // テスト用：イベントコマンドを1つ返す
    std::vector<CommandPtr> commands;
    commands.emplace_back(std::make_shared<ChangeVpCommand>(side, 1));
    return commands;
  }

  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    return true;
  }
};

// テスト用カードプール作成関数
static const std::array<std::unique_ptr<Card>, 111>& createTestCardPool() {
  static std::array<std::unique_ptr<Card>, 111> pool{};
  static bool initialized = false;
  if (!initialized) {
    pool[static_cast<size_t>(CardEnum::DUMMY)] =
        std::make_unique<DummyCard>(3, Side::NEUTRAL);
    pool[static_cast<size_t>(CardEnum::CHINA_CARD)] =
        std::make_unique<DummyCard>(4, Side::NEUTRAL);
    initialized = true;
  }
  return pool;
}

// テスト用フィクスチャ
class MoveTest : public ::testing::Test {
 public:
  MoveTest() : board_(Board(createTestCardPool())) {}

 protected:
  void SetUp() override {
    // テスト用カードプールの初期化
    initializeTestCardPool();
  }

  void initializeTestCardPool() {
    // DummyCardの設定（Card基底クラスのポインタとして保持）
    dummy_card_neutral_ = std::make_unique<DummyCard>(3, Side::NEUTRAL);
    dummy_card_ussr_ = std::make_unique<DummyCard>(2, Side::USSR);
    dummy_card_usa_ = std::make_unique<DummyCard>(2, Side::USA);
    dummy_card_1_ops_ = std::make_unique<DummyCard>(1, Side::NEUTRAL);
    dummy_card_4_ops_ = std::make_unique<DummyCard>(4, Side::NEUTRAL);
  }

  Board board_;
  std::unique_ptr<Card> dummy_card_neutral_;
  std::unique_ptr<Card> dummy_card_ussr_;
  std::unique_ptr<Card> dummy_card_usa_;
  std::unique_ptr<Card> dummy_card_1_ops_;
  std::unique_ptr<Card> dummy_card_4_ops_;
};

// ActionPlaceInfluenceMoveのテスト
TEST_F(MoveTest, ActionPlaceInfluenceMove_BasicCommand) {
  // 配置対象国の設定
  std::map<CountryEnum, int> targets;
  targets[CountryEnum::FRANCE] = 2;
  targets[CountryEnum::ITALY] = 1;

  ActionPlaceInfluenceMove move(CardEnum::DUMMY, Side::USA, targets);

  // toCommandの実行
  auto commands = move.toCommand(dummy_card_neutral_, board_);

  // コマンド数の確認（PlaceInfluenceCommand + イベントなし）
  ASSERT_EQ(commands.size(), 2);

  // 最初のコマンドがPlaceInfluenceCommandであることを確認
  auto* place_cmd = dynamic_cast<PlaceInfluenceCommand*>(commands[0].get());
  ASSERT_NE(place_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

TEST_F(MoveTest, ActionPlaceInfluenceMove_OpponentEvent) {
  std::map<CountryEnum, int> targets;
  targets[CountryEnum::FRANCE] = 2;

  // USAプレイヤーがUSSRカードを使用
  ActionPlaceInfluenceMove move(CardEnum::DUMMY, Side::USA, targets);
  auto commands = move.toCommand(dummy_card_ussr_, board_);

  // コマンド数の確認（PlaceInfluenceCommand + イベント）
  ASSERT_EQ(commands.size(), 3);

  // 2番目のコマンドがイベントコマンドであることを確認
  auto* vp_cmd = dynamic_cast<ChangeVpCommand*>(commands[1].get());
  ASSERT_NE(vp_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

TEST_F(MoveTest, ActionPlaceInfluenceMove_OwnEvent) {
  std::map<CountryEnum, int> targets;
  targets[CountryEnum::FRANCE] = 2;

  // USAプレイヤーがUSAカードを使用
  ActionPlaceInfluenceMove move(CardEnum::DUMMY, Side::USA, targets);
  auto commands = move.toCommand(dummy_card_usa_, board_);

  // コマンド数の確認（PlaceInfluenceCommandのみ）
  ASSERT_EQ(commands.size(), 2);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

// ActionCoupMoveのテスト
TEST_F(MoveTest, ActionCoupMove_BasicCommand) {
  ActionCoupMove move(CardEnum::DUMMY, Side::USSR, CountryEnum::ITALY);
  auto commands = move.toCommand(dummy_card_neutral_, board_);

  ASSERT_EQ(commands.size(), 2);
  auto* coup_cmd = dynamic_cast<ActionCoupCommand*>(commands[0].get());
  ASSERT_NE(coup_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

TEST_F(MoveTest, ActionCoupMove_OpponentEvent) {
  // USSRプレイヤーがUSAカードを使用
  ActionCoupMove move(CardEnum::DUMMY, Side::USSR, CountryEnum::ITALY);
  auto commands = move.toCommand(dummy_card_usa_, board_);

  ASSERT_EQ(commands.size(), 3);
  auto* vp_cmd = dynamic_cast<ChangeVpCommand*>(commands[1].get());
  ASSERT_NE(vp_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

// ActionSpaceRaceMoveのテスト
TEST_F(MoveTest, ActionSpaceRaceMove_NoOpponentEvent) {
  // USSRプレイヤーがUSAカードを使用
  ActionSpaceRaceMove move(CardEnum::DUMMY, Side::USSR);
  auto commands = move.toCommand(dummy_card_usa_, board_);

  // SpaceRaceでは相手イベントが発動しない
  ASSERT_EQ(commands.size(), 2);
  auto* space_cmd = dynamic_cast<ActionSpaceRaceCommand*>(commands[0].get());
  ASSERT_NE(space_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

TEST_F(MoveTest, ActionSpaceRaceMove_NeutralCard) {
  ActionSpaceRaceMove move(CardEnum::DUMMY, Side::USA);
  auto commands = move.toCommand(dummy_card_neutral_, board_);

  ASSERT_EQ(commands.size(), 2);
  auto* space_cmd = dynamic_cast<ActionSpaceRaceCommand*>(commands[0].get());
  ASSERT_NE(space_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

// ActionRealigmentMoveのテスト
TEST_F(MoveTest, ActionRealigmentMove_MultipleOps) {
  // 3 Opsカード使用
  ActionRealigmentMove move(CardEnum::DUMMY, Side::USA, CountryEnum::FRANCE);
  auto commands = move.toCommand(dummy_card_neutral_, board_);

  // ActionRealigmentCommand + RequestCommand
  ASSERT_EQ(commands.size(), 3);

  auto* realign_cmd = dynamic_cast<ActionRealigmentCommand*>(commands[0].get());
  ASSERT_NE(realign_cmd, nullptr);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

TEST_F(MoveTest, ActionRealigmentMove_SingleOp) {
  // 1 Opsカード使用
  ActionRealigmentMove move(CardEnum::DUMMY, Side::USSR, CountryEnum::ITALY);
  auto commands = move.toCommand(dummy_card_1_ops_, board_);

  // ActionRealigmentCommand + RequestCommand（additionalOps用）
  ASSERT_EQ(commands.size(), 3);

  auto* realign_cmd = dynamic_cast<ActionRealigmentCommand*>(commands[0].get());
  ASSERT_NE(realign_cmd, nullptr);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

TEST_F(MoveTest, ActionRealigmentMove_OpponentEvent) {
  // USAプレイヤーがUSSRカードを使用
  ActionRealigmentMove move(CardEnum::DUMMY, Side::USA, CountryEnum::FRANCE);
  auto commands = move.toCommand(dummy_card_ussr_, board_);

  // ActionRealigmentCommand + RequestCommand + イベント
  ASSERT_EQ(commands.size(), 4);

  auto* vp_cmd = dynamic_cast<ChangeVpCommand*>(commands[2].get());
  ASSERT_NE(vp_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

// RealignmentRequestMoveのテスト
TEST_F(MoveTest, RealignmentRequestMove_Pass) {
  std::vector<CountryEnum> history = {CountryEnum::FRANCE};
  RealignmentRequestMove move(CardEnum::DUMMY, Side::USA, CountryEnum::USSR,
                              history, 2, AdditionalOpsType::NONE);

  auto commands = move.toCommand(dummy_card_neutral_, board_);

  // パスの場合は空のコマンドリスト
  ASSERT_EQ(commands.size(), 0);
}

TEST_F(MoveTest, RealignmentRequestMove_Continue) {
  std::vector<CountryEnum> history = {CountryEnum::FRANCE};
  RealignmentRequestMove move(CardEnum::DUMMY, Side::USSR, CountryEnum::ITALY,
                              history, 2, AdditionalOpsType::NONE);

  auto commands = move.toCommand(dummy_card_neutral_, board_);

  // ActionRealigmentCommand + RequestCommand
  ASSERT_EQ(commands.size(), 2);

  auto* realign_cmd = dynamic_cast<ActionRealigmentCommand*>(commands[0].get());
  ASSERT_NE(realign_cmd, nullptr);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);
}

TEST_F(MoveTest, RealignmentRequestMove_LastOp) {
  std::vector<CountryEnum> history = {CountryEnum::FRANCE, CountryEnum::ITALY};
  RealignmentRequestMove move(CardEnum::DUMMY, Side::USA,
                              CountryEnum::WEST_GERMANY, history, 1,
                              AdditionalOpsType::NONE);

  auto commands = move.toCommand(dummy_card_neutral_, board_);

  // ActionRealigmentCommand + RequestCommand（additionalOps用）
  ASSERT_EQ(commands.size(), 2);
}

TEST_F(MoveTest, RealignmentRequestMove_WithAdditionalOps) {
  std::vector<CountryEnum> history = {CountryEnum::JAPAN};
  // TODO: CardEnumをCHINA_CARDへ
  RealignmentRequestMove move(CardEnum::DUMMY, Side::USSR,
                              CountryEnum::SOUTH_KOREA, history, 1,
                              AdditionalOpsType::CHINA_CARD);

  auto commands = move.toCommand(dummy_card_neutral_, board_);

  // ActionRealigmentCommand + RequestCommand（additionalOps処理）
  ASSERT_EQ(commands.size(), 2);
}

// ActionEventMoveのテスト
TEST_F(MoveTest, ActionEventMove_OwnSideEvent) {
  // USAプレイヤーがUSAカードのイベントを実行
  ActionEventMove move(CardEnum::DUMMY, Side::USA, true);
  auto commands = move.toCommand(dummy_card_usa_, board_);

  // イベントコマンドのみ（追加アクションなし）
  ASSERT_EQ(commands.size(), 2);
  auto* vp_cmd = dynamic_cast<ChangeVpCommand*>(commands[0].get());
  ASSERT_NE(vp_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

TEST_F(MoveTest, ActionEventMove_OpponentSideEvent) {
  // USAプレイヤーがUSSRカードのイベントを実行
  ActionEventMove move(CardEnum::DUMMY, Side::USA, true);
  auto commands = move.toCommand(dummy_card_ussr_, board_);

  // イベントコマンド + RequestCommand（追加アクション用）
  ASSERT_EQ(commands.size(), 3);

  auto* vp_cmd = dynamic_cast<ChangeVpCommand*>(commands[0].get());
  ASSERT_NE(vp_cmd, nullptr);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

TEST_F(MoveTest, ActionEventMove_NeutralEvent) {
  ActionEventMove move(CardEnum::DUMMY, Side::USSR, true);
  auto commands = move.toCommand(dummy_card_neutral_, board_);

  // 中立イベントは追加アクションなし
  ASSERT_EQ(commands.size(), 2);
  auto* vp_cmd = dynamic_cast<ChangeVpCommand*>(commands[0].get());
  ASSERT_NE(vp_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);
}

TEST_F(MoveTest, HeadlineCardSelectMove) {
  HeadlineCardSelectMove move(CardEnum::DUMMY, Side::USA);
  auto commands = move.toCommand(dummy_card_neutral_, board_);

  ASSERT_EQ(commands.size(), 1);
  auto* headline_cmd = dynamic_cast<SetHeadlineCardCommand*>(commands[0].get());
  ASSERT_NE(headline_cmd, nullptr);
}

// RequestCommandのラムダ関数テスト（間接的な検証アプローチ）
TEST_F(MoveTest, ActionRealigmentMove_RequestCommandLambda) {
  ActionRealigmentMove move(CardEnum::DUMMY, Side::USA, CountryEnum::FRANCE);
  auto commands = move.toCommand(dummy_card_4_ops_, board_);

  ASSERT_EQ(commands.size(), 3);
  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);

  // RequestCommandのラムダ関数を実行してみる
  auto legal_moves = request_cmd->legalMoves(board_);

  // 結果の検証 - 実際の実装が呼ばれた証拠
  // 1. 例外が発生せずに完了すること
  EXPECT_NO_THROW({ auto moves = request_cmd->legalMoves(board_); });

  // 2. 期待される引数で直接呼び出した結果と比較
  // ラムダ関数のキャプチャ値が正しいことを間接的に検証
  std::vector<CountryEnum> expected_history = {CountryEnum::FRANCE};
  auto expected_moves = LegalMovesGenerator::realignmentRequestLegalMoves(
      board_, Side::USA, CardEnum::DUMMY, expected_history, 3,
      AdditionalOpsType::NONE);

  // 結果が同じであることを確認（引数が正しく渡されている証拠）
  ASSERT_EQ(legal_moves.size(), expected_moves.size());

  // 3. 各Moveオブジェクトが正しい型であること
  for (const auto& move : legal_moves) {
    EXPECT_NE(move.get(), nullptr);
    // 実際のRealignmentRequestMoveが含まれていることを確認
    auto* realignment_move = dynamic_cast<RealignmentRequestMove*>(move.get());
    if (realignment_move != nullptr) {
      // 期待される値を確認
      EXPECT_EQ(realignment_move->getCard(), CardEnum::DUMMY);
      EXPECT_EQ(realignment_move->getSide(), Side::USA);
    }
  }
}

// 統合テスト的なアプローチ（引数検証付き）
TEST_F(MoveTest, ActionRealigmentMove_RequestCommand_IntegrationTest) {
  // テストの目的：ラムダ関数が正しいパラメータで作成されていることを確認

  ActionRealigmentMove move(CardEnum::DUMMY, Side::USA, CountryEnum::FRANCE);
  auto commands = move.toCommand(dummy_card_4_ops_, board_);

  ASSERT_EQ(commands.size(), 3);
  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);

  // ラムダ関数のキャプチャ値が正しく設定されているかを間接的に検証
  // 実際の実装を呼び出して、期待される動作を確認
  auto legal_moves = request_cmd->legalMoves(board_);

  // 期待される引数で直接呼び出した結果と比較
  std::vector<CountryEnum> expected_history = {CountryEnum::FRANCE};
  auto expected_moves = LegalMovesGenerator::realignmentRequestLegalMoves(
      board_, Side::USA, CardEnum::DUMMY, expected_history, 3,
      AdditionalOpsType::NONE);

  // 結果が同じであることを確認（引数が正しく渡されている証拠）
  ASSERT_EQ(legal_moves.size(), expected_moves.size());

  // 生成されたMoveオブジェクトが期待される構造を持つことを確認
  for (const auto& move : legal_moves) {
    EXPECT_NE(move.get(), nullptr);
    // 実際のMove派生クラスが返されることを確認
    bool is_valid_move_type =
        dynamic_cast<RealignmentRequestMove*>(move.get()) != nullptr ||
        dynamic_cast<ActionPlaceInfluenceMove*>(move.get()) != nullptr;
    EXPECT_TRUE(is_valid_move_type);
  }
}

// 異なるOps値での検証（引数検証付き）
TEST_F(MoveTest, ActionRealigmentMove_RequestCommand_DifferentOps) {
  // 1 Opsカードの場合
  {
    ActionRealigmentMove move(CardEnum::DUMMY, Side::USSR, CountryEnum::ITALY);
    auto commands = move.toCommand(dummy_card_1_ops_, board_);

    ASSERT_EQ(commands.size(), 3);
    auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
    ASSERT_NE(request_cmd, nullptr);

    auto* finalize_cmd =
        dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
    ASSERT_NE(finalize_cmd, nullptr);

    // remainingOps=0の場合でも適切に動作することを確認
    auto legal_moves = request_cmd->legalMoves(board_);
    EXPECT_NO_THROW({ auto moves = request_cmd->legalMoves(board_); });

    // 1 Opsカードの場合はadditionalOpsRealignmentLegalMovesが呼ばれるため、
    // 期待される引数で直接呼び出した結果と比較
    std::vector<CountryEnum> expected_history = {CountryEnum::ITALY};
    auto expected_moves =
        LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
            board_, Side::USSR, CardEnum::DUMMY, expected_history,
            AdditionalOpsType::NONE);

    // 結果が同じであることを確認（引数が正しく渡されている証拠）
    ASSERT_EQ(legal_moves.size(), expected_moves.size());
  }

  // 3 Opsカードの場合
  {
    ActionRealigmentMove move(CardEnum::DUMMY, Side::USA, CountryEnum::FRANCE);
    auto commands = move.toCommand(dummy_card_neutral_, board_);

    ASSERT_EQ(commands.size(), 3);
    auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
    ASSERT_NE(request_cmd, nullptr);

    auto* finalize_cmd =
        dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
    ASSERT_NE(finalize_cmd, nullptr);

    // remainingOps=2の場合の動作を確認
    auto legal_moves = request_cmd->legalMoves(board_);

    // 期待される引数で直接呼び出した結果と比較
    std::vector<CountryEnum> expected_history = {CountryEnum::FRANCE};
    auto expected_moves = LegalMovesGenerator::realignmentRequestLegalMoves(
        board_, Side::USA, CardEnum::DUMMY, expected_history, 2,
        AdditionalOpsType::NONE);

    // 結果が同じであることを確認（引数が正しく渡されている証拠）
    ASSERT_EQ(legal_moves.size(), expected_moves.size());
  }
}

// RealignmentRequestMoveのラムダ関数内部実行テスト
TEST_F(MoveTest, RealignmentRequestMove_LambdaExecution_WithRemainingOps) {
  // remainingOps > 0の場合のテスト（109-111行のカバレッジ）
  std::vector<CountryEnum> history = {CountryEnum::FRANCE};
  RealignmentRequestMove move(CardEnum::DUMMY, Side::USSR, CountryEnum::ITALY,
                              history, 2, AdditionalOpsType::NONE);

  auto commands = move.toCommand(dummy_card_neutral_, board_);
  ASSERT_EQ(commands.size(), 2);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);

  // ラムダ関数内部を実行してカバレッジを通す
  auto legal_moves = request_cmd->legalMoves(board_);
  EXPECT_NO_THROW({ auto moves = request_cmd->legalMoves(board_); });

  // 期待される引数で直接呼び出した結果と比較
  std::vector<CountryEnum> expected_history = {CountryEnum::FRANCE,
                                               CountryEnum::ITALY};
  auto expected_moves = LegalMovesGenerator::realignmentRequestLegalMoves(
      board_, Side::USSR, CardEnum::DUMMY, expected_history, 1,
      AdditionalOpsType::NONE);

  // 結果が同じであることを確認（引数が正しく渡されている証拠）
  ASSERT_EQ(legal_moves.size(), expected_moves.size());

  // 生成されたMoveオブジェクトが期待される構造を持つことを確認
  for (const auto& move : legal_moves) {
    EXPECT_NE(move.get(), nullptr);
    // 実際のRealignmentRequestMoveが含まれていることを確認
    auto* realignment_move = dynamic_cast<RealignmentRequestMove*>(move.get());
    if (realignment_move != nullptr) {
      // 期待される値を確認
      EXPECT_EQ(realignment_move->getCard(), CardEnum::DUMMY);
      EXPECT_EQ(realignment_move->getSide(), Side::USSR);
    }
  }
}

TEST_F(MoveTest, RealignmentRequestMove_LambdaExecution_WithoutRemainingOps) {
  // remainingOps = 0の場合のテスト（121-123行のカバレッジ）
  std::vector<CountryEnum> history = {CountryEnum::FRANCE, CountryEnum::ITALY};
  RealignmentRequestMove move(CardEnum::DUMMY, Side::USA,
                              CountryEnum::WEST_GERMANY, history, 1,
                              AdditionalOpsType::NONE);

  auto commands = move.toCommand(dummy_card_neutral_, board_);
  ASSERT_EQ(commands.size(), 2);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);

  // ラムダ関数内部を実行してカバレッジを通す
  auto legal_moves = request_cmd->legalMoves(board_);
  EXPECT_NO_THROW({ auto moves = request_cmd->legalMoves(board_); });

  // 期待される引数で直接呼び出した結果と比較
  std::vector<CountryEnum> expected_history = {
      CountryEnum::FRANCE, CountryEnum::ITALY, CountryEnum::WEST_GERMANY};
  auto expected_moves = LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
      board_, Side::USA, CardEnum::DUMMY, expected_history,
      AdditionalOpsType::NONE);

  // 結果が同じであることを確認（引数が正しく渡されている証拠）
  ASSERT_EQ(legal_moves.size(), expected_moves.size());

  // 生成されたMoveオブジェクトが期待される構造を持つことを確認
  for (const auto& move : legal_moves) {
    EXPECT_NE(move.get(), nullptr);
    // 実際のRealignmentRequestMoveが含まれていることを確認
    auto* realignment_move = dynamic_cast<RealignmentRequestMove*>(move.get());
    if (realignment_move != nullptr) {
      // 期待される値を確認
      EXPECT_EQ(realignment_move->getCard(), CardEnum::DUMMY);
      EXPECT_EQ(realignment_move->getSide(), Side::USA);
    }
  }
}

// ActionEventMoveのラムダ関数内部実行テスト
TEST_F(MoveTest, ActionEventMove_LambdaExecution_OpponentSideEvent) {
  // 相手側イベントカードを使用した場合のテスト（154行のカバレッジ）
  ActionEventMove move(CardEnum::DUMMY, Side::USA, true);
  auto commands = move.toCommand(dummy_card_ussr_, board_);

  ASSERT_EQ(commands.size(), 3);

  auto* request_cmd = dynamic_cast<RequestCommand*>(commands[1].get());
  ASSERT_NE(request_cmd, nullptr);

  auto* finalize_cmd =
      dynamic_cast<FinalizeCardPlayCommand*>(commands.back().get());
  ASSERT_NE(finalize_cmd, nullptr);

  // ラムダ関数内部を実行してカバレッジを通す
  auto legal_moves = request_cmd->legalMoves(board_);
  EXPECT_NO_THROW({ auto moves = request_cmd->legalMoves(board_); });

  // LegalMovesGeneratorの新ヘルパーと結果が一致することを確認
  auto expected_moves = LegalMovesGenerator::actionLegalMovesForCard(
      board_, Side::USA, CardEnum::DUMMY);
  EXPECT_EQ(legal_moves.size(), expected_moves.size());

  // それぞれのMoveが正しいカードとサイドに紐づくことを確認
  for (const auto& move_ptr : legal_moves) {
    ASSERT_NE(move_ptr, nullptr);
    EXPECT_EQ(move_ptr->getCard(), CardEnum::DUMMY);
    EXPECT_EQ(move_ptr->getSide(), Side::USA);

    auto* place_move = dynamic_cast<ActionPlaceInfluenceMove*>(move_ptr.get());
    auto* realign_move = dynamic_cast<ActionRealigmentMove*>(move_ptr.get());
    auto* coup_move = dynamic_cast<ActionCoupMove*>(move_ptr.get());
    EXPECT_TRUE(place_move != nullptr || realign_move != nullptr ||
                coup_move != nullptr);
  }
}

TEST_F(MoveTest, PassMoveReturnsNoCommands) {
  PassMove move(Side::USSR);
  auto commands = move.toCommand(dummy_card_neutral_, board_);

  EXPECT_TRUE(commands.empty());
  EXPECT_EQ(move.getCard(), CardEnum::DUMMY);
}
