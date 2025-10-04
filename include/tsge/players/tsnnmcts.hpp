// ファイル: include/tsge/players/tsnnmcts.hpp
// 役割:
// tsgeのプレイヤーレイヤにZero系MCTSの骨組みを提供し、推論器と探索制御の接続点を明確にする。
// 背景:
// 完全なゲームエンジン整備前からニューラル推論器とMCTSを繋ぐ基盤を用意し、後続の段階的拡張を容易にするため。

#pragma once

#include <memory>
#include <vector>

#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"

// TsNnMctsInferenceResult:
// ニューラルネット出力の保持構造体。policyは合法手ごとの事前確率、valueはサイド視点の評価値[-1,1]想定。
struct TsNnMctsInferenceResult {
  std::vector<double> policy;
  double value = 0.0;
};

// TsNnMctsInferenceEngine: 推論器の抽象インターフェース。
// TODO: 実際の実装ではバッチ推論・GPU管理などをここに接続する。
class TsNnMctsInferenceEngine {
 public:
  virtual ~TsNnMctsInferenceEngine() = default;

  // evaluate: 盤面と合法手を受け取り、1ステップ分のpolicyとvalueを返す。
  // - board: 推論対象の盤面。
  // - legal_moves: 現在展開可能な手。policyベクトルはこの順序に整列させる。
  // - side: 評価対象の手番サイド。
  // 返値: policy/value出力。policyサイズはlegal_movesと同数を期待。
  [[nodiscard]]
  virtual TsNnMctsInferenceResult evaluate(
      const Board& board, const std::vector<std::shared_ptr<Move>>& legal_moves,
      Side side) = 0;
};

// TsNnMctsConfig: 探索パラメータの集約。実数値はTODOで設計書と同期させる想定。
struct TsNnMctsConfig {
  int num_simulations = 32;  // TODO: 設計で定義された標準値に更新する。
  double c_puct = 1.5;  // TODO: チューニング値を設計アーカイブと連動させる。
  double dirichlet_alpha = 0.3;  // TODO: 盤面サイズ別に最適値を反映する。
  double dirichlet_epsilon = 0.25;  // TODO: Self-play/対局モードで分岐。
  bool add_dirichlet_noise = true;  // TODO: 実運用モードでフラグを切り替える。
};

// TsNnMctsNode: 1ノード分の統計を保持。Moveと統計値、子ノードを管理する。
class TsNnMctsNode {
 public:
  TsNnMctsNode(std::shared_ptr<Move> move, double prior, TsNnMctsNode* parent);

  // expand: 現在ノードの子をpolicyベクトルに基づいて生成。
  void expand(const std::vector<std::shared_ptr<Move>>& legal_moves,
              const std::vector<double>& priors);

  // selectChild: PUCTを用いて子ノードを選択。
  [[nodiscard]] TsNnMctsNode* selectChild(double c_puct);

  // backup: 葉ノード評価値をバックアップ。
  void backup(double value, Side root_side);

  // getter群: テストと統計解析用に公開。
  [[nodiscard]] int getVisitCount() const { return visit_count_; }
  [[nodiscard]] double getPrior() const { return prior_; }
  [[nodiscard]] double getValueSum() const { return value_sum_; }
  [[nodiscard]] const std::vector<std::unique_ptr<TsNnMctsNode>>& getChildren()
      const {
    return children_;
  }
  [[nodiscard]] std::vector<std::unique_ptr<TsNnMctsNode>>&
  getMutableChildren() {
    return children_;
  }
  [[nodiscard]] TsNnMctsNode* getParent() const { return parent_; }
  [[nodiscard]] Move* getMove() const { return move_.get(); }

 private:
  std::shared_ptr<Move> move_;
  double prior_;
  double value_sum_ = 0.0;
  int visit_count_ = 0;
  TsNnMctsNode* parent_;
  std::vector<std::unique_ptr<TsNnMctsNode>> children_;
};

// TsNnMctsController: Zero系MCTSの探索コントローラ。
class TsNnMctsController {
 public:
  TsNnMctsController(std::shared_ptr<TsNnMctsInferenceEngine> inference,
                     TsNnMctsConfig config);

  // runSearch: 探索を実行し、最終的な手を返す。
  std::shared_ptr<Move> runSearch(
      const Board& board, const std::vector<std::shared_ptr<Move>>& legal_moves,
      Side side);

  // exposeRoot: テスト用にルートノード統計へアクセス。
  [[nodiscard]] const TsNnMctsNode& getRoot() const { return *root_; }

 private:
  // dirichletノイズをroot priorに付加。
  void injectDirichletNoise(std::vector<double>& priors);

  // TODO: Board遷移を扱うためにPhaseMachine相当のサブシステムと接続する。
  std::unique_ptr<TsNnMctsNode> root_;
  std::shared_ptr<TsNnMctsInferenceEngine> inference_;
  TsNnMctsConfig config_;
};

// TsNnMctsPolicy: Playerラッパー用の決定ポリシー。
class TsNnMctsPolicy {
 public:
  TsNnMctsPolicy(std::shared_ptr<TsNnMctsInferenceEngine> inference,
                 TsNnMctsConfig config = {});

  // decideMove: Playerテンプレートと同等のインターフェース。
  std::shared_ptr<Move> decideMove(
      const Board& board, const std::vector<std::shared_ptr<Move>>& legal_moves,
      Side side);

 private:
  TsNnMctsController mcts_;
};
