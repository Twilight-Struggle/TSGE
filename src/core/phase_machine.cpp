#include "tsge/core/phase_machine.hpp"

#include <cstddef>
#include <ranges>
#include <variant>

#include "tsge/actions/command.hpp"
#include "tsge/actions/legal_moves_generator.hpp"
#include "tsge/core/board.hpp"

/// 入力 (Move) がある場合はそれを使って１フェーズ進め，
/// まだ入力が必要なら合法 Move を返す
std::tuple<std::vector<std::unique_ptr<Move>>, Side, std::optional<Side>>
PhaseMachine::step(Board& board,
                   std::optional<std::unique_ptr<Move>>&& answer) {
  auto& states = board.getStates();

  /*===============================
   * A) もし前回の答え (Move) が渡されたら
   *    → Command 列に変換し stack へ push
   *==============================*/
  if (answer) {
    // Requestが残っている場合pop
    if (!states.empty() && std::holds_alternative<CommandPtr>(states.back())) {
      auto& cmd_ptr = std::get<CommandPtr>(states.back());
      if (dynamic_cast<RequestCommand*>(cmd_ptr.get()) != nullptr) {
        states.pop_back();
      }
    }
    const auto& cardpool = board.getCardpool();
    auto cmds = answer.value()->toCommand(
        cardpool[static_cast<size_t>(answer.value()->getCard())]);
    // 逆順にスタックに積む
    for (auto& cmd : std::ranges::reverse_view(cmds)) {
      states.emplace_back(std::move(cmd));
    }

    answer.reset();
  }

  while (!states.empty()) {
    auto& top_state = states.back();
    // ----- スタック最上段が CommandPtr の場合 --------------------
    if (auto* cmd_ptr = std::get_if<CommandPtr>(&top_state)) {
      // A) Request なら入力待ち
      // cmdPtrがRequestなら
      if (auto* req = dynamic_cast<RequestCommand*>(cmd_ptr->get())) {
        // TODO:
        // req->legalMoves(board)={}の可能性があるのでこの場合はPhaseを進める処理が必要
        return {req->legalMoves(board), req->getSide(), std::nullopt};
      }
      (*cmd_ptr)->apply(board);
      // board.history.push_back(*cmdPtr);  // undo ログ
      states.pop_back();
      continue;

      // NOLINTNEXTLINE(readability-else-after-return
    } else {
      // ----- スタック最上段が StateType (フェーズ) -------------------
      auto state_type = std::get<StateType>(top_state);
      states.pop_back();

      switch (state_type) {
        case StateType::AR_USSR:
        case StateType::AR_USA: {
          Side side = state_type == StateType::AR_USSR ? Side::USSR : Side::USA;
          board.setCurrentArPlayer(side);  // 現在のARプレイヤーを記録
          states.emplace_back(StateType::AR_COMPLETE);
          // TODO:
          // 手札が空等で合法手={}の場合があるためこの場合はPhaseを進める処理が必要
          return {LegalMovesGenerator::ArLegalMoves(board, side), side,
                  std::nullopt};  // 合法手を返して停止
        }

        case StateType::AR_COMPLETE:
          // AR 終了処理して続行(仮コード)
          // push_next_ar(states);  // TODO: 実装が必要
          break;

        case StateType::USSR_WIN_END: {
          return {std::vector<std::unique_ptr<Move>>{}, Side::NEUTRAL,
                  Side::USSR};
        }
        case StateType::USA_WIN_END: {
          return {std::vector<std::unique_ptr<Move>>{}, Side::NEUTRAL,
                  Side::USA};
        }
        case StateType::DRAW_END: {
          return {std::vector<std::unique_ptr<Move>>{}, Side::NEUTRAL,
                  Side::NEUTRAL};
        }

          /* HEADLINE, TURN_END など他のフェーズも同様に */
      }
    }
  }
  return {std::vector<std::unique_ptr<Move>>{}, Side::NEUTRAL,
          Side::NEUTRAL};  // スタックが空→ゲーム終了
}
