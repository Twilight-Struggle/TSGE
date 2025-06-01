#include "phase_machine.hpp"

#include <cstddef>
#include <utility>
#include <variant>

#include "board.hpp"
#include "command.hpp"
#include "legal_moves_generator.hpp"

/// 入力 (Move) がある場合はそれを使って１フェーズ進め，
/// まだ入力が必要なら合法 Move を返す
std::pair<std::vector<std::unique_ptr<Move>>, Side> PhaseMachine::step(
    Board& board, const std::array<std::unique_ptr<Card>, 111>& cardpool_,
    std::optional<std::unique_ptr<Move>>&& answer) {
  auto& states = board.getStates();

  /*===============================
   * A) もし前回の答え (Move) が渡されたら
   *    → Command 列に変換し stack へ push
   *==============================*/
  if (answer) {
    // Requestが残っている場合pop
    if (!states.empty() && std::holds_alternative<CommandPtr>(states.back())) {
      auto& cmdPtr = std::get<CommandPtr>(states.back());
      if (dynamic_cast<Request*>(cmdPtr.get())) {
        states.pop_back();
      }
    }
    Side side =
        answer.value()
            ->getSide();  // Sideをどこから取得するかは未定　仮にMoveからとする
    auto cmds = answer.value()->toCommand(
        cardpool_[static_cast<size_t>(answer.value()->getCard())], side);
    for (auto it = cmds.rbegin(); it != cmds.rend(); ++it)
      states.emplace_back(*it);

    answer.reset();
  }

  while (!states.empty()) {
    auto& topState = states.back();
    // ----- スタック最上段が CommandPtr の場合 --------------------
    if (auto* cmdPtr = std::get_if<CommandPtr>(&topState)) {
      // A) Request なら入力待ち
      // cmdPtrがRequestなら
      if (auto* req = dynamic_cast<Request*>(cmdPtr->get())) {
        return {req->legalMoves(board), req->waitingForSide};
      }
      (*cmdPtr)->apply(board);
      // board.history.push_back(*cmdPtr);  // undo ログ
      states.pop_back();
      continue;

    } else {
      // ----- スタック最上段が StateType (フェーズ) -------------------
      auto stateType = std::get<StateType>(topState);
      states.pop_back();

      switch (stateType) {
        case StateType::AR_USSR:
        case StateType::AR_USA: {
          Side side = stateType == StateType::AR_USSR ? Side::USSR : Side::USA;
          states.emplace_back(StateType::AR_COMPLETE);
          return {LegalMovesGenerator::ArLegalMoves(board, side),
                  side};  // 合法手を返して停止
        }

        case StateType::AR_COMPLETE:
          push_next_ar(states);  // AR 終了処理して続行(仮コード)
          break;

          /* HEADLINE, TURN_END など他のフェーズも同様に */
      }
    }
  }
  return {std::vector<std::unique_ptr<Move>>{},
          Side::NEUTRAL};  // スタックが空→ゲーム終了
}
