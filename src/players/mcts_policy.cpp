#include "tsge/players/mcts_policy.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

#include "tsge/actions/legal_moves_generator.hpp"
#include "tsge/core/phase_machine.hpp"

namespace mcts {

// Node implementation
Node::Node(Board&& board, std::shared_ptr<Move> last_move, Side current_side,
           DeterminizedState det_state, Node* parent)
    : board_(std::move(board)),
      last_move_(std::move(last_move)),
      parent_(parent),
      current_side_(current_side),
      det_state_(std::move(det_state)),
      is_terminal_(false),
      is_expanded_(false) {
  // 終端状態のチェック
  auto& states = board_.getStates();
  if (!states.empty()) {
    if (auto* state_type = std::get_if<StateType>(&states.back())) {
      is_terminal_ = (*state_type == StateType::USSR_WIN_END ||
                      *state_type == StateType::USA_WIN_END ||
                      *state_type == StateType::DRAW_END);
    }
  }
}

void Node::expand(const std::vector<std::shared_ptr<Move>>& legal_moves) {
  std::lock_guard<std::mutex> lock(stats_.expansion_mutex);

  if (is_expanded_) {
    return;
  }

  children_.reserve(legal_moves.size());

  for (const auto& move : legal_moves) {
    // 各子ノード用にボードをコピー
    Board child_board = board_.copyForMCTS(current_side_);

    // 相手の手札を決定化状態に設定
    Side opponent = getOpponentSide(current_side_);
    auto& opponent_hand = child_board.getPlayerHand(opponent);
    opponent_hand.clear();
    for (CardEnum card : det_state_.opponent_hand) {
      opponent_hand.push_back(card);
    }

    // Moveを適用
    auto [next_legal_moves, next_side, winner] =
        PhaseMachine::step(child_board, move);

    children_.emplace_back(std::make_unique<Node>(std::move(child_board), move,
                                                  next_side, det_state_, this));
  }

  is_expanded_ = true;
}

double Node::getUCBValue(double exploration_constant) const {
  int parent_visits = (parent_ != nullptr) ? parent_->stats_.visits.load() : 0;
  int node_visits = stats_.visits.load();

  if (node_visits == 0) {
    return std::numeric_limits<double>::max();
  }

  double average_value = stats_.total_value.load() / node_visits;
  double exploration_term =
      exploration_constant *
      std::sqrt(std::log(parent_visits + 1) / node_visits);

  return average_value + exploration_term;
}

Node* Node::selectBestChild(double exploration_constant) {
  Node* best_child = nullptr;
  double best_value = -std::numeric_limits<double>::max();

  for (const auto& child : children_) {
    double ucb_value = child->getUCBValue(exploration_constant);
    if (ucb_value > best_value) {
      best_value = ucb_value;
      best_child = child.get();
    }
  }

  return best_child;
}

void Node::backpropagate(double value) {
  stats_.visits.fetch_add(1);
  stats_.total_value.fetch_add(value);

  if (parent_ != nullptr) {
    parent_->backpropagate(value);
  }
}

// RolloutPolicy implementation
std::shared_ptr<Move> RolloutPolicy::selectMove(
    const std::vector<std::shared_ptr<Move>>& legal_moves) {
  if (legal_moves.empty()) {
    return nullptr;
  }

  std::uniform_int_distribution<size_t> dist(0, legal_moves.size() - 1);
  size_t selected_index = dist(rng_);

  return legal_moves[selected_index];
}

// MCTSExecutor implementation
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
MCTSExecutor::MCTSExecutor(double exploration_constant, int num_threads)
    : exploration_constant_(exploration_constant), num_threads_(num_threads) {
  // 各スレッド用のRNGを初期化
  thread_rngs_.reserve(num_threads);
  // NOLINTNEXTLINE(readability-identifier-length)
  std::random_device rd;
  for (int i = 0; i < num_threads; ++i) {
    thread_rngs_.emplace_back(rd());
  }
}

std::shared_ptr<Move> MCTSExecutor::search(
    const Board& root_board, Side side, int num_iterations,
    std::chrono::milliseconds time_limit) {
  // 決定化パターンを生成
  auto determinizations = generateDeterminizations(root_board, side);

  if (determinizations.empty()) {
    return nullptr;
  }

  // 各決定化パターンに対してルートノードを作成
  std::vector<std::unique_ptr<Node>> root_nodes;
  root_nodes.reserve(determinizations.size());

  for (auto&& det : determinizations) {
    Board board_copy = root_board.copyForMCTS(side);

    // 相手の手札を決定化
    Side opponent = getOpponentSide(side);
    auto& opponent_hand = board_copy.getPlayerHand(opponent);
    opponent_hand.clear();
    opponent_hand.assign(det.opponent_hand.begin(), det.opponent_hand.end());

    root_nodes.emplace_back(std::make_unique<Node>(
        std::move(board_copy), nullptr, side, std::move(det)));
  }

  // Root Parallelization: 各スレッドが独立してMCTSを実行
  std::atomic<bool> should_stop(false);
  std::vector<std::thread> threads;

  int iterations_per_thread = num_iterations / num_threads_;

  for (int i = 0; i < num_threads_; ++i) {
    threads.emplace_back([this, &root_nodes, iterations_per_thread, time_limit,
                          &should_stop, i]() {
      // 各スレッドが全ての決定化パターンに対して均等に実行
      int iterations_per_root =
          iterations_per_thread / static_cast<int>(root_nodes.size());

      for (auto& root : root_nodes) {
        runMCTSThread(root.get(), iterations_per_root, time_limit, should_stop);
      }
    });
  }

  // 全スレッドの完了を待つ
  for (auto& thread : threads) {
    thread.join();
  }

  // 最良の手を選択
  return selectBestMove(root_nodes);
}

std::vector<DeterminizedState> MCTSExecutor::generateDeterminizations(
    const Board& board, Side side, int max_determinizations) {
  // 返却すべき決定化パターンのリスト
  std::vector<DeterminizedState> determinizations;

  Side opponent = getOpponentSide(side);
  const auto& my_hand = board.getPlayerHand(side);
  // opponent_handには基本Dummyカードのみ
  const auto& opponent_hand = board.getPlayerHand(opponent);
  // TODO:
  // Deckは不可視！有効なカードから逆算する必要あり(というかDeckという名前は不適切で、available_cardsとかにすべき)
  // なぜなら不可視カードはデッキ+相手の手札のためデッキそのものではない。相手の手札を決定したあとデッキを決定化する必要がある
  // ただしデッキは決定化せず乱数に任せる方法もある
  const auto& deck = board.getDeck().getDeck();
  const auto& discard = board.getDeck().getDiscardPile();

  size_t opponent_hand_size = opponent_hand.size();

  // TODO: 決定化のアルゴリズム
  // 相手の手札をランダムに決定化する、その後ランダムで選ばれたをused_cardsに登録する。また相手の手札が決定化されたしたのでデッキも決定化される。
  // 次の決定化では、used_cardsに登録されたカードは選ばれないようにする。
  // 例えばused_cardsに登録された無いカードがあと3枚で、決定化しなければならないカードが4枚あった場合、used_cardsに登録されたカードを1枚ランダムに外して決定化する。
  // used_cardsがすべてのカードを含む場合、used_cardsをクリアして再度決定化を行う。これをmax_determinizationsまで繰り返す

  // 使用可能なカードプールを作成
  std::vector<CardEnum> available_cards;
  available_cards.reserve(deck.size() + discard.size());

  // デッキから自分の手札を除外
  for (CardEnum card : deck) {
    if (std::find(my_hand.begin(), my_hand.end(), card) == my_hand.end()) {
      available_cards.push_back(card);
    }
  }

  // 捨て札から自分の手札を除外
  for (CardEnum card : discard) {
    if (std::find(my_hand.begin(), my_hand.end(), card) == my_hand.end()) {
      available_cards.push_back(card);
    }
  }

  if (available_cards.size() < opponent_hand_size) {
    // 使用可能なカードが足りない場合は現在の状態をそのまま使用
    DeterminizedState det;
    det.opponent_hand =
        std::vector<CardEnum>(opponent_hand.begin(), opponent_hand.end());
    det.determinization_id = 0;
    determinizations.push_back(det);
    return determinizations;
  }

  // 脅威となるカードを優先的に配分
  std::vector<CardEnum> threat_cards;
  std::vector<CardEnum> normal_cards;

  for (CardEnum card : available_cards) {
    // スコアリングカードや高Opsカードを脅威として扱う
    const auto& card_obj = board.getCardpool()[static_cast<size_t>(card)];
    if (card_obj && (card_obj->getOps() == 0 || card_obj->getOps() >= 3)) {
      threat_cards.push_back(card);
    } else {
      normal_cards.push_back(card);
    }
  }

  // 決定化パターンを生成
  // NOLINTNEXTLINE(readability-identifier-length)
  std::random_device rd;
  std::mt19937_64 rng(rd());

  for (int i = 0;
       i < max_determinizations &&
       i < static_cast<int>(available_cards.size() / opponent_hand_size);
       ++i) {
    DeterminizedState det;
    det.determinization_id = i;

    // 使用済みカードを記録
    std::vector<bool> used(available_cards.size(), false);

    // 脅威カードを均等に配分
    size_t threat_per_det = std::min(
        threat_cards.size() / max_determinizations + 1, opponent_hand_size);
    size_t added_count = 0;

    for (size_t j = 0; j < threat_per_det && added_count < opponent_hand_size;
         ++j) {
      size_t threat_index = (i * threat_per_det + j) % threat_cards.size();
      if (threat_index < threat_cards.size()) {
        det.opponent_hand.push_back(threat_cards[threat_index]);
        added_count++;
      }
    }

    // 残りを通常カードで埋める
    std::shuffle(normal_cards.begin(), normal_cards.end(), rng);
    for (size_t j = 0;
         j < normal_cards.size() && added_count < opponent_hand_size; ++j) {
      det.opponent_hand.push_back(normal_cards[j]);
      added_count++;
    }

    determinizations.push_back(det);
  }

  return determinizations;
}

void MCTSExecutor::runMCTSThread(Node* root, int iterations_per_thread,
                                 std::chrono::milliseconds time_limit,
                                 std::atomic<bool>& should_stop) {
  // スレッドIDからRNGを取得
  size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id()) %
                     thread_rngs_.size();
  auto& rng = thread_rngs_[thread_id];

  auto start_time = std::chrono::steady_clock::now();

  for (int i = 0; i < iterations_per_thread; ++i) {
    if (should_stop.load()) {
      break;
    }

    // 時間制限チェック
    auto current_time = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - start_time) > time_limit) {
      should_stop.store(true);
      break;
    }

    // Selection
    Node* selected = select(root);

    // Expansion
    if (!selected->isTerminal() && selected->getVisits() > 0) {
      selected = expand(selected);
    }

    // Simulation
    if (selected != nullptr) {
      Board sim_board =
          selected->getBoard().copyForMCTS(selected->getCurrentSide());
      sim_board.getRandomizer().setRng(&rng);
      double value = simulate(std::move(sim_board), root->getCurrentSide());

      // Backpropagation
      selected->backpropagate(value);
    }
  }
}

Node* MCTSExecutor::select(Node* start_node) const {
  Node* current = start_node;
  while (current != nullptr && !current->isTerminal() &&
         current->isExpanded()) {
    current = current->selectBestChild(exploration_constant_);
  }
  return current;
}

Node* MCTSExecutor::expand(Node* node) {
  if (node->isTerminal() || node->isExpanded()) {
    return node;
  }

  // 現在のノードから合法手を生成
  auto [legal_moves, next_side, winner] =
      PhaseMachine::step(const_cast<Board&>(node->getBoard()));

  if (legal_moves.empty() || winner.has_value()) {
    return node;
  }

  node->expand(legal_moves);

  // ランダムに子ノードを選択して返す
  const auto& children = node->getChildren();
  if (!children.empty()) {
    size_t thread_id =
        std::hash<std::thread::id>{}(std::this_thread::get_id()) %
        thread_rngs_.size();
    std::uniform_int_distribution<size_t> dist(0, children.size() - 1);
    return children[dist(thread_rngs_[thread_id])].get();
  }

  return node;
}

double MCTSExecutor::simulate(Board board, Side maximizing_side) {
  size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id()) %
                     thread_rngs_.size();
  auto& rng = thread_rngs_[thread_id];

  RolloutPolicy rollout_policy(rng);

  // シミュレーション深さ制限
  const int max_depth = 100;
  int depth = 0;

  std::optional<std::shared_ptr<Move>> pending;
  while (depth <= max_depth) {
    auto [legal_moves, next_side, winner] =
        PhaseMachine::step(board, std::move(pending));

    // 終端状態チェック
    if (winner.has_value()) {
      if (*winner == maximizing_side) {
        return 1.0;
        // NOLINTNEXTLINE(readability-else-after-return)
      } else if (*winner == getOpponentSide(maximizing_side)) {
        return -1.0;
      } else {
        return 0.0;  // 引き分け
      }
    }

    if (legal_moves.empty()) {
      break;
    }

    // ランダムに手を選択
    pending = rollout_policy.selectMove(legal_moves);

    depth++;
  }

  // 深さ制限に達した場合はVPで評価
  // NOLINTNEXTLINE(readability-identifier-length)
  int vp = board.getVp();
  if (maximizing_side == Side::USSR) {
    return std::tanh(vp / 10.0);  // [-1, 1]に正規化
    // NOLINTNEXTLINE(readability-else-after-return)
  } else {
    return std::tanh(-vp / 10.0);
  }
}

std::shared_ptr<Move> MCTSExecutor::selectBestMove(
    const std::vector<std::unique_ptr<Node>>& roots) {
  if (roots.empty()) {
    return nullptr;
  }

  // 各Moveの統計を集計
  std::unordered_map<std::string, std::pair<int, double>> move_stats;

  for (const auto& root : roots) {
    for (const auto& child : root->getChildren()) {
      if (child->getLastMove() != nullptr) {
        // Moveを文字列化して識別（簡易的な実装）
        std::string move_key =
            std::to_string(static_cast<int>(child->getLastMove()->getCard()));

        if (auto* place_move = dynamic_cast<const ActionPlaceInfluenceMove*>(
                child->getLastMove())) {
          move_key += "_place";
        } else if (auto* coup_move = dynamic_cast<const ActionCoupMove*>(
                       child->getLastMove())) {
          move_key += "_coup";
        }
        // 他の型も同様に処理

        move_stats[move_key].first += child->getVisits();
        move_stats[move_key].second +=
            child->getAverageValue() * child->getVisits();
      }
    }
  }

  // 最も訪問回数の多い手を選択（Robust Child）
  std::string best_move_key;
  int max_visits = 0;

  for (const auto& [key, stats] : move_stats) {
    if (stats.first > max_visits) {
      max_visits = stats.first;
      best_move_key = key;
    }
  }

  // 該当するMoveを見つけて返す
  for (const auto& root : roots) {
    for (const auto& child : root->getChildren()) {
      if (child->getLastMove() != nullptr) {
        std::string move_key =
            std::to_string(static_cast<int>(child->getLastMove()->getCard()));

        if (auto* place_move = dynamic_cast<const ActionPlaceInfluenceMove*>(
                child->getLastMove())) {
          move_key += "_place";
          if (move_key == best_move_key) {
            return std::make_unique<ActionPlaceInfluenceMove>(*place_move);
          }
        } else if (auto* coup_move = dynamic_cast<const ActionCoupMove*>(
                       child->getLastMove())) {
          move_key += "_coup";
          if (move_key == best_move_key) {
            return std::make_unique<ActionCoupMove>(*coup_move);
          }
        }
        // 他の型も同様に処理
      }
    }
  }

  return nullptr;
}

}  // namespace mcts

// MCTSPolicy implementation
MCTSPolicy::MCTSPolicy(int iterations_per_move,
                       std::chrono::milliseconds time_limit, int num_threads)
    : executor_(std::sqrt(2.0), num_threads),
      iterations_per_move_(iterations_per_move),
      time_limit_(time_limit) {}

std::shared_ptr<Move> MCTSPolicy::decideMove(
    const Board& board, const std::vector<std::shared_ptr<Move>>& legal_moves,
    Side side) {
  if (legal_moves.empty()) {
    return nullptr;
  }

  // 合法手が1つしかない場合は即座に返す
  if (legal_moves.size() == 1) {
    return legal_moves[0];
  }

  // MCTSを実行
  auto best_move =
      executor_.search(board, side, iterations_per_move_, time_limit_);

  // MCTSが失敗した場合は最初の合法手を返す
  if (!best_move && !legal_moves.empty()) {
    return legal_moves[0];
  }

  return best_move;
}
