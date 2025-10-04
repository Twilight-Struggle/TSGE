// ファイル: tests/players/tsnnmcts_policy_test.cpp
// 役割:
// TsNnMctsPolicy雛形の基本動作を検証し、将来の詳細実装で壊れないことを保証する。
// 背景:
// 最小限のUNITテストを維持することで、骨組み段階でも回帰を検出しやすくするため。

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "tsge/core/board.hpp"
#include "tsge/game_state/card.hpp"
#include "tsge/players/tsnnmcts.hpp"

namespace {

// DummyCard: policyテスト用の最小限カード。
class DummyCard final : public Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  DummyCard(CardEnum id, WarPeriod war_period)
      : Card(id, "Dummy", 2, Side::NEUTRAL, war_period, false) {}

  [[nodiscard]] std::vector<CommandPtr> event(Side side) const override {
    return {};
  }

  [[nodiscard]] bool canEvent(const Board& board) const override {
    return true;
  }
};

// DummyInference: TsNnMctsInferenceEngineを模擬し、意図的にpolicy偏重を返す。
class DummyInference final : public TsNnMctsInferenceEngine {
 public:
  explicit DummyInference(double preferred_prior)
      : preferred_prior_(preferred_prior) {}

  TsNnMctsInferenceResult evaluate(
      const Board& board, const std::vector<std::shared_ptr<Move>>& legal_moves,
      Side side) override {
    TsNnMctsInferenceResult result;
    result.policy.assign(legal_moves.size(), 0.1);
    if (!legal_moves.empty()) {
      const size_t preferred_index =
          std::min<size_t>(1, legal_moves.size() - 1);
      result.policy[preferred_index] = preferred_prior_;
    }
    result.value = 0.2;
    return result;
  }

 private:
  double preferred_prior_;
};

}  // namespace

// TsNnMctsPolicy雛形テスト: 推論器が返したpriorに従って手が選ばれるか確認。
TEST(TsNnMctsPolicySkeletonTest, PicksMoveWithHighestPrior) {
  std::array<std::unique_ptr<Card>, 111> cardpool{};
  for (int i = 0; i < 111; ++i) {
    cardpool[static_cast<size_t>(i)] = std::make_unique<DummyCard>(
        static_cast<CardEnum>(i), WarPeriod::EARLY_WAR);
  }

  Board board(cardpool);
  board.addCardToHand(Side::USSR, CardEnum::DuckAndCover);

  std::vector<std::shared_ptr<Move>> legal_moves;
  legal_moves.push_back(
      std::make_shared<ActionEventMove>(CardEnum::DuckAndCover, Side::USSR));
  legal_moves.push_back(
      std::make_shared<ActionEventMove>(CardEnum::Fidel, Side::USSR));

  auto inference = std::make_shared<DummyInference>(0.9);
  TsNnMctsConfig config;
  config.add_dirichlet_noise = false;  // テストでは決定論的結果を優先。

  TsNnMctsPolicy policy(std::move(inference), config);
  auto selected_move = policy.decideMove(board, legal_moves, Side::USSR);

  ASSERT_NE(selected_move, nullptr);
  EXPECT_EQ(selected_move->getCard(), CardEnum::Fidel);
}
