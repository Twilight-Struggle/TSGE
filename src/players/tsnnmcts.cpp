// ファイル: src/players/tsnnmcts.cpp
// 役割:
// TsNnMcts骨組みのロジックを実装し、推論器とプレイヤーの橋渡しを担当する。
// 背景:
// 盤面遷移の完全実装前にMCTSの制御フローを確立しておき、後続で段階的に拡張するため。

#include "tsge/players/tsnnmcts.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>

// TsNnMctsNode::TsNnMctsNode: Moveとpriorを束ね、親ノード参照を保持。
TsNnMctsNode::TsNnMctsNode(std::shared_ptr<Move> move, double prior,
                           TsNnMctsNode* parent)
    : move_(std::move(move)), prior_(prior), parent_(parent) {
  // TODO: 将来的にはゲーム固有のMove IDで去重管理を行う。
}

// expand: policyベクトルと合法手に基づき子ノードを生成。
void TsNnMctsNode::expand(const std::vector<std::shared_ptr<Move>>& legal_moves,
                          const std::vector<double>& priors) {
  children_.clear();

  if (legal_moves.empty()) {
    return;
  }

  std::vector<double> normalized = priors;
  if (normalized.size() != legal_moves.size()) {
    normalized.assign(legal_moves.size(),
                      1.0 / static_cast<double>(legal_moves.size()));
    // TODO: ロギング基盤整備後、サイズ不一致を警告ログに送る。
  }

  double sum = std::accumulate(normalized.begin(), normalized.end(), 0.0);
  if (sum <= 0.0) {
    normalized.assign(legal_moves.size(),
                      1.0 / static_cast<double>(legal_moves.size()));
    sum = 1.0;
  }

  for (size_t i = 0; i < legal_moves.size(); ++i) {
    double prior = normalized[i] / sum;
    children_.push_back(
        std::make_unique<TsNnMctsNode>(legal_moves[i], prior, this));
  }
}

// selectChild: シンプルなPUCT式で子を選択。
TsNnMctsNode* TsNnMctsNode::selectChild(double c_puct) {
  if (children_.empty()) {
    return nullptr;
  }

  double parent_visit = std::max(1, visit_count_);
  TsNnMctsNode* best_child = nullptr;
  double best_score = -std::numeric_limits<double>::infinity();

  for (auto& child : children_) {
    double q_value = 0.0;
    if (child->visit_count_ > 0) {
      q_value = child->value_sum_ / static_cast<double>(child->visit_count_);
    }
    double u_value = c_puct * child->prior_ *
                     std::sqrt(static_cast<double>(parent_visit)) /
                     (1.0 + static_cast<double>(child->visit_count_));
    double puct = q_value + u_value;
    if (puct > best_score) {
      best_score = puct;
      best_child = child.get();
    }
  }

  return best_child;
}

// backup: 葉評価値を親まで伝播。
void TsNnMctsNode::backup(double value, Side /*root_side*/) {
  TsNnMctsNode* node = this;
  double propagated = value;

  while (node != nullptr) {
    node->visit_count_ += 1;
    node->value_sum_ += propagated;

    // TODO: サイド切り替えを明示的に格納し、反転規則を柔軟化する。
    propagated = -propagated;
    node = node->parent_;
  }
}

// TsNnMctsController::TsNnMctsController: 推論器と設定を受け取って初期化。
TsNnMctsController::TsNnMctsController(
    std::shared_ptr<TsNnMctsInferenceEngine> inference, TsNnMctsConfig config)
    : root_(std::make_unique<TsNnMctsNode>(nullptr, 1.0, nullptr)),
      inference_(std::move(inference)),
      config_(config) {
  if (inference_ == nullptr) {
    throw std::invalid_argument(
        "TsNnMctsController requires a valid inference engine");
  }
}

// injectDirichletNoise: ルート事前分布に探索ノイズを付与。
void TsNnMctsController::injectDirichletNoise(std::vector<double>& priors) {
  if (!config_.add_dirichlet_noise || priors.empty()) {
    return;
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::gamma_distribution<double> gamma(config_.dirichlet_alpha, 1.0);

  std::vector<double> noise(priors.size(), 0.0);
  double noise_sum = 0.0;
  for (double& n : noise) {
    n = gamma(gen);
    noise_sum += n;
  }

  if (noise_sum <= 0.0) {
    return;
  }

  for (size_t i = 0; i < priors.size(); ++i) {
    double normalized_noise = noise[i] / noise_sum;
    priors[i] = priors[i] * (1.0 - config_.dirichlet_epsilon) +
                config_.dirichlet_epsilon * normalized_noise;
  }
}

// runSearch: Skeleton実装では推論結果を用いて単純に手を選択。
std::shared_ptr<Move> TsNnMctsController::runSearch(
    const Board& board, const std::vector<std::shared_ptr<Move>>& legal_moves,
    Side side) {
  if (legal_moves.empty()) {
    return nullptr;
  }

  // ルートノードをリセット。
  root_ = std::make_unique<TsNnMctsNode>(nullptr, 1.0, nullptr);

  TsNnMctsInferenceResult inference_output =
      inference_->evaluate(board, legal_moves, side);

  std::vector<double> priors = inference_output.policy;
  if (priors.size() != legal_moves.size()) {
    priors.assign(legal_moves.size(),
                  1.0 / static_cast<double>(legal_moves.size()));
  }

  injectDirichletNoise(priors);
  root_->expand(legal_moves, priors);

  // TODO:
  // PhaseMachineによる盤面遷移を組み込み、num_simulations回の探索を実行する。
  // 現状は推論器のprior最大値を返却するのみ。

  size_t best_index = 0;
  double best_prior = -1.0;
  auto& children = root_->getMutableChildren();
  for (size_t i = 0; i < children.size(); ++i) {
    auto* child = children[i].get();
    double child_prior = child->getPrior();
    if (child_prior > best_prior) {
      best_prior = child_prior;
      best_index = i;
    }
    // 初期構造では直接バックアップして統計を保存する。
    child->backup(inference_output.value, side);
  }

  // TODO: root visit数に基づく温度付きサンプリングを導入する。
  return legal_moves[best_index];
}

// TsNnMctsPolicy: ポリシーをPlayerテンプレートへ提供。
TsNnMctsPolicy::TsNnMctsPolicy(
    std::shared_ptr<TsNnMctsInferenceEngine> inference, TsNnMctsConfig config)
    : mcts_(std::move(inference), config) {}

std::shared_ptr<Move> TsNnMctsPolicy::decideMove(
    const Board& board, const std::vector<std::shared_ptr<Move>>& legal_moves,
    Side side) {
  // TODO: 時間制約やモードに応じて探索回数/温度を調整する。
  return mcts_.runSearch(board, legal_moves, side);
}
