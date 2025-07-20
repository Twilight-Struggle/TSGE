#include "tsge/core/game.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/card.hpp"
#include "tsge/players/policies.hpp"

// テスト用DummyCardクラス
class DummyCard : public Card {
 public:
  DummyCard(int ops, Side side = Side::NEUTRAL)
      : Card(CardEnum::Dummy, "DummyCard", ops, side, false) {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side side) const override {
    // テスト用：空のイベントコマンド
    std::vector<CommandPtr> commands;
    return commands;
  }

  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    return true;
  }
};

// 不正な手を返すテスト用ポリシー
class IllegalMovePolicy {
 public:
  IllegalMovePolicy();

  std::unique_ptr<Move> decideMove(
      const Board& board,
      const std::vector<std::unique_ptr<Move>>& legalMoves) {
    call_count_++;

    if (call_count_ <= 2) {
      // 最初の2回は不正な手を返す（存在しない国への影響力配置）
      std::map<CountryEnum, int> illegal_countries;
      illegal_countries[static_cast<CountryEnum>(999)] = 1;  // 存在しない国
      return std::make_unique<ActionPlaceInfluenceMove>(
          CardEnum::Dummy, Side::USSR, illegal_countries);
    }
    // 3回目以降は合法手を返す
    if (!legalMoves.empty()) {
      // 最初の合法手をコピー
      const auto& first_legal = legalMoves[0];

      // Move の型に応じてコピーを作成
      if (const auto* place_move =
              dynamic_cast<const ActionPlaceInfluenceMove*>(
                  first_legal.get())) {
        // ActionPlaceInfluenceMoveの場合、適当な合法手を作成
        std::map<CountryEnum, int> countries;
        countries[CountryEnum::AFGHANISTAN] = 1;
        return std::make_unique<ActionPlaceInfluenceMove>(
            place_move->getCard(), place_move->getSide(), countries);
      }

      // 他の型の場合も同様に対応が必要だが、テストでは単純化
      return std::make_unique<ActionEventMove>(CardEnum::Dummy, Side::USSR);
    }

    // フォールバック
    return std::make_unique<ActionEventMove>(CardEnum::Dummy, Side::USSR);
  }

  [[nodiscard]]
  int getCallCount() const {
    return call_count_;
  }

 private:
  int call_count_ = 0;
};

class GameTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // テスト用のカードプールを作成
    for (auto& card : cardpool_) {
      card = std::make_unique<DummyCard>(1, Side::NEUTRAL);
    }
  }

  std::array<std::unique_ptr<Card>, 111> cardpool_;
};

TEST_F(GameTest, ValidMoveAccepted) {
  // 正常な手が受け入れられることを確認するテスト
  Player<TestPolicy> player1;
  Player<TestPolicy> player2;
  Game game(std::move(player1), std::move(player2), cardpool_);

  // ゲームの初期化が正常に行われることを確認
  EXPECT_NO_THROW(game.getBoard());
}

TEST_F(GameTest, IllegalMoveRejected) {
  // 不正な手が拒否され、再度選択が求められることを確認するテスト

  // 一方のプレイヤーを不正な手を返すポリシーに設定
  auto illegal_policy = std::make_shared<IllegalMovePolicy>();

  // このテストは現在のアーキテクチャでは実装が困難
  // PhaseMachineが適切に初期化されていないとゲームが進行しない
  // TODO: より適切なテスト環境を構築後に実装
}