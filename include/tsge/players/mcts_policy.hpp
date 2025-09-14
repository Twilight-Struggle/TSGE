#pragma once

#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <unordered_map>
#include <vector>

#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"

namespace mcts {

// MCTSノードの統計情報
struct NodeStats {
  std::atomic<int> visits{0};
  std::atomic<double> total_value{0.0};
  std::mutex expansion_mutex;  // 展開時の排他制御用
};

// 決定化されたゲーム状態（相手の手札が確定している状態）
struct DeterminizedState {
  std::vector<CardEnum> opponent_hand;
  size_t determinization_id;  // どの決定化パターンかを識別
};

// MCTSノード
class Node {
 public:
  Node(Board&& board, std::shared_ptr<Move> last_move, Side current_side,
       DeterminizedState det_state, Node* parent = nullptr);

  // 子ノードを展開
  void expand(const std::vector<std::shared_ptr<Move>>& legal_moves);

  // UCB値を計算
  [[nodiscard]]
  double getUCBValue(double exploration_constant) const;

  // 最良の子ノードを選択
  Node* selectBestChild(double exploration_constant);

  // バックプロパゲーション
  void backpropagate(double value);

  // ゲッター
  [[nodiscard]] Board& getBoard() { return board_; }
  [[nodiscard]] const Board& getBoard() const { return board_; }
  [[nodiscard]] bool isTerminal() const { return is_terminal_; }
  [[nodiscard]] bool isExpanded() const { return is_expanded_; }
  [[nodiscard]] const std::vector<std::unique_ptr<Node>>& getChildren() const {
    return children_;
  }
  [[nodiscard]] Move* getLastMove() const {
    return last_move_ ? last_move_.get() : nullptr;
  }
  [[nodiscard]] int getVisits() const { return stats_.visits.load(); }
  [[nodiscard]] double getAverageValue() const {
    int value = stats_.visits.load();
    return value > 0 ? stats_.total_value.load() / value : 0.0;
  }
  [[nodiscard]] Side getCurrentSide() const { return current_side_; }
  [[nodiscard]] const DeterminizedState& getDeterminizedState() const {
    return det_state_;
  }

 private:
  Board board_;
  std::shared_ptr<Move> last_move_;
  Node* parent_;
  std::vector<std::unique_ptr<Node>> children_;
  NodeStats stats_;
  Side current_side_;
  DeterminizedState det_state_;
  bool is_terminal_;
  bool is_expanded_;
};

// ロールアウトポリシー（ランダムプレイアウト）
class RolloutPolicy {
 public:
  explicit RolloutPolicy(std::mt19937_64& rng) : rng_(rng) {}

  // ランダムに手を選択
  std::shared_ptr<Move> selectMove(
      const std::vector<std::shared_ptr<Move>>& legal_moves);

 private:
  std::mt19937_64& rng_;
};

// MCTS実行クラス
class MCTSExecutor {
 public:
  MCTSExecutor(double exploration_constant = std::sqrt(2.0),
               int num_threads = 1);

  // MCTSを実行して最良の手を返す
  std::shared_ptr<Move> search(const Board& root_board, Side side,
                               int num_iterations,
                               std::chrono::milliseconds time_limit);

 private:
  // 決定化パターンを生成
  static std::vector<DeterminizedState> generateDeterminizations(
      const Board& board, Side side, int max_determinizations = 50);

  // 単一のMCTSスレッドを実行
  void runMCTSThread(Node* root, int iterations_per_thread,
                     std::chrono::milliseconds time_limit,
                     std::atomic<bool>& should_stop);

  // Selection phase
  Node* select(Node* node) const;

  // Expansion phase
  Node* expand(Node* node);

  // Simulation phase
  double simulate(Board board, Side maximizing_side);

  // 最良の手を選択
  std::shared_ptr<Move> selectBestMove(
      const std::vector<std::unique_ptr<Node>>& roots);

  // 決定化ごとのルートノードからMove統計を集計
  std::unordered_map<Move*, double> aggregateMoveStats(
      const std::vector<std::unique_ptr<Node>>& roots);

  double exploration_constant_;
  int num_threads_;
  std::vector<std::mt19937_64> thread_rngs_;  // 各スレッド用のRNG
};

}  // namespace mcts

// MCTSPolicy: Player用のポリシークラス
class MCTSPolicy {
 public:
  MCTSPolicy(
      int iterations_per_move = 10000,
      std::chrono::milliseconds time_limit = std::chrono::milliseconds(5000),
      int num_threads = 4);

  std::shared_ptr<Move> decideMove(
      const Board& board, const std::vector<std::shared_ptr<Move>>& legal_moves,
      Side side);

 private:
  mcts::MCTSExecutor executor_;
  int iterations_per_move_;
  std::chrono::milliseconds time_limit_;
};