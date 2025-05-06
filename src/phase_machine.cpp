#include "phase_machine.hpp"

#include <variant>

#include "board.hpp"
#include "command.hpp"

/// 入力 (Move) がある場合はそれを使って１フェーズ進め，
/// まだ入力が必要なら合法 Move を返す
std::vector<Move> PhaseMachine::step(
    Board& board, std::optional<std::vector<std::unique_ptr<Move>>> answer) {
  while (!board.getStates().empty()) {
    // ----- スタック最上段が CommandPtr の場合 --------------------
    if (auto* cmdPtr = std::get_if<CommandPtr>(&board.getStates().top())) {
      // A) Request なら入力待ち
      // cmdPtrがRequestなら
      if (auto* req = dynamic_cast<Request*>(cmdPtr->get())) {
        // まだ答えが来ていなければ合法手を列挙して返す
        if (!answer) return enumerate_moves_for_request(*req, board);

        // 答えが来ていれば request を pop して続きを push
        board.getStates().pop();  // Request 除去
        for (auto it = answer->rbegin(); it != answer->rend(); ++it)
          board.pushState((*it)->toCommand());
        answer.reset();  // 答えは消費済み
        continue;        // 先頭からループ
      }

      // B) ふつうの Command なら即時実行
      (*cmdPtr)->apply(board);
      // board.history.push_back(*cmdPtr);  // undo ログ
      board.getStates().pop();
      continue;
    }

    // ----- スタック最上段が StateType (フェーズ) -------------------
    auto stateType = std::get<StateType>(board.getStates().top());
    board.getStates().pop();

    switch (stateType) {
      case StateType::AR_USSR:
      case StateType::AR_USA: {
        Side side = stateType == StateType::AR_USSR ? Side::USSR : Side::USA;
        return enumerate_player_moves(side, board);  // 合法手を返して停止
      }

      case StateType::AR_COMPLETE:
        push_next_ar(s);  // AR 終了処理して続行
        break;

        /* HEADLINE, TURN_END など他のフェーズも同様に */
    }
  }
  return {};  // スタックが空→ゲーム終了
}
