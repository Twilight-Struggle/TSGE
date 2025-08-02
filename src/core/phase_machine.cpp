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

      // NOLINTNEXTLINE(readability-else-after-return)
    } else {
      // ----- スタック最上段が StateType (フェーズ) -------------------
      auto state_type = std::get<StateType>(top_state);
      states.pop_back();

      switch (state_type) {
        case StateType::AR_USSR: {
          Side side = Side::USSR;
          board.setCurrentArPlayer(side);  // 現在のARプレイヤーを記録
          // Push the corresponding AR_COMPLETE state
          states.emplace_back(StateType::AR_USSR_COMPLETE);
          // TODO:
          // 手札が空等で合法手={}の場合があるためこの場合はPhaseを進める処理が必要
          return {LegalMovesGenerator::arLegalMoves(board, side), side,
                  std::nullopt};  // 合法手を返して停止
        }
        case StateType::AR_USA: {
          Side side = Side::USA;
          board.setCurrentArPlayer(side);  // 現在のARプレイヤーを記録
          // Push the corresponding AR_COMPLETE state
          states.emplace_back(StateType::AR_USA_COMPLETE);
          // TODO:
          // 手札が空等で合法手={}の場合があるためこの場合はPhaseを進める処理が必要
          return {LegalMovesGenerator::arLegalMoves(board, side), side,
                  std::nullopt};  // 合法手を返して停止
        }

        case StateType::AR_USSR_COMPLETE: {
          Side current_side = Side::USSR;
          Side opponent_side = getOpponentSide(current_side);

          // 1. Increment current AR player's action round count
          int current_turn = board.getTurnTrack().getTurn();
          board.getActionRoundTrack().advanceActionRound(current_side,
                                                         current_turn);

          // 2. Check both players' action round counts
          int current_ar =
              board.getActionRoundTrack().getActionRound(current_side);
          int opponent_ar =
              board.getActionRoundTrack().getActionRound(opponent_side);
          int defined_ar =
              board.getActionRoundTrack().getDefinedActionRounds(current_turn);

          // 3. Determine next phase transition
          if (opponent_ar < defined_ar) {
            // Opponent still has normal ARs remaining
            states.emplace_back(StateType::AR_USA);
          } else if (current_ar < defined_ar) [[unlikely]] {
            // Current player still has normal ARs remaining
            states.emplace_back(StateType::AR_USSR);
          } else {
            // All normal ARs are complete, check for extra ARs
            bool current_has_extra =
                board.getActionRoundTrack().hasExtraActionRound(current_side);
            bool opponent_has_extra =
                board.getActionRoundTrack().hasExtraActionRound(opponent_side);

            if (current_has_extra || opponent_has_extra) {
              if (opponent_has_extra) {
                states.emplace_back(StateType::EXTRA_AR_USA);
              } else {
                states.emplace_back(StateType::EXTRA_AR_USSR);
              }
            } else {
              // All action rounds complete, proceed to turn end
              states.emplace_back(StateType::TURN_END);
            }
          }

          // 4. TODO: Check for DEFCON 2 and NORAD effectiveness
          // if (board.getDefconTrack().getDefcon() == 2 && /* NORAD is active
          // */) {
          //   return NORAD legal moves;
          // }

          break;
        }
        case StateType::AR_USA_COMPLETE: {
          Side current_side = Side::USA;
          Side opponent_side = getOpponentSide(current_side);

          // 1. Increment current AR player's action round count
          int current_turn = board.getTurnTrack().getTurn();
          board.getActionRoundTrack().advanceActionRound(current_side,
                                                         current_turn);

          // 2. Check both players' action round counts
          int current_ar =
              board.getActionRoundTrack().getActionRound(current_side);
          int opponent_ar =
              board.getActionRoundTrack().getActionRound(opponent_side);
          int defined_ar =
              board.getActionRoundTrack().getDefinedActionRounds(current_turn);

          // 3. Determine next phase transition
          if (opponent_ar < defined_ar) {
            // Opponent still has normal ARs remaining
            states.emplace_back(StateType::AR_USSR);
          } else if (current_ar < defined_ar) [[unlikely]] {
            // Current player still has normal ARs remaining
            states.emplace_back(StateType::AR_USA);
          } else {
            // All normal ARs are complete, check for extra ARs
            bool current_has_extra =
                board.getActionRoundTrack().hasExtraActionRound(current_side);
            bool opponent_has_extra =
                board.getActionRoundTrack().hasExtraActionRound(opponent_side);

            // 追加ARはたかだか1回しかない。
            if (current_has_extra || opponent_has_extra) {
              if (current_has_extra) {
                states.emplace_back(StateType::EXTRA_AR_USSR);
              }
              if (opponent_has_extra) {
                states.emplace_back(StateType::EXTRA_AR_USA);
              }
            } else {
              // All action rounds complete, proceed to turn end
              states.emplace_back(StateType::TURN_END);
            }
          }

          // 4. TODO: Check for DEFCON 2 and NORAD effectiveness
          // if (board.getDefconTrack().getDefcon() == 2 && /* NORAD is active
          // */) {
          //   return NORAD legal moves;
          // }

          break;
        }

        case StateType::EXTRA_AR_USSR: {
          Side side = Side::USSR;
          board.setCurrentArPlayer(side);  // Set current AR player

          // Clear extra action round flag for USSR
          board.getActionRoundTrack().clearExtraActionRound(Side::USSR);

          // Transition to AR_USSR_COMPLETE
          states.emplace_back(StateType::AR_USSR_COMPLETE);

          // Generate legal moves with pass option for extra AR
          // TODO: Add pass option to the legal moves list
          // For now, return normal AR legal moves (pass functionality to be
          // added)
          return {LegalMovesGenerator::arLegalMoves(board, side), side,
                  std::nullopt};
        }
        case StateType::EXTRA_AR_USA: {
          Side side = Side::USA;
          board.setCurrentArPlayer(Side::USA);  // Set current AR player

          // Clear extra action round flag for USA
          board.getActionRoundTrack().clearExtraActionRound(Side::USA);

          // Transition to AR_USA_COMPLETE
          states.emplace_back(StateType::AR_USA_COMPLETE);

          // Generate legal moves with pass option for extra AR
          // TODO: Add pass option to the legal moves list
          // For now, return normal AR legal moves (pass functionality to be
          // added)
          return {LegalMovesGenerator::arLegalMoves(board, side), side,
                  std::nullopt};
        }

        case StateType::TURN_END: {
          // TODO: Implement complete turn end processing
          // For now, just advance to next turn
          board.getTurnTrack().nextTurn();
          board.getActionRoundTrack().resetActionRounds();

          // Push TURN_START for next turn
          states.emplace_back(StateType::TURN_START);
          break;
        }

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

        case StateType::TURN_START: {
          // Check if either side has space track position 8
          if (board.getSpaceTrack().getSpaceTrackPosition(Side::USSR) == 8) {
            board.getActionRoundTrack().setExtraActionRound(Side::USSR);
          }
          if (board.getSpaceTrack().getSpaceTrackPosition(Side::USA) == 8) {
            board.getActionRoundTrack().setExtraActionRound(Side::USA);
          }

          // TODO: NorthSeaOil check for USA

          // ヘッドラインフェイズを追加
          states.emplace_back(StateType::HEADLINE_PHASE);
          break;
        }

        case StateType::HEADLINE_PHASE: {
          // 宇宙開発トラック4の優位性チェック
          bool ussr_space4 = board.getSpaceTrack().effectEnabled(Side::USSR, 4);

          // 最後にHEADLINE_PROCESS_EVENTSをプッシュ
          states.emplace_back(StateType::HEADLINE_PROCESS_EVENTS);

          if (ussr_space4) {
            // USSRが優位：USA→USSRの順で選択
            states.emplace_back(StateType::HEADLINE_CARD_SELECT_USSR);
            states.emplace_back(StateType::HEADLINE_CARD_SELECT_USA);
          } else {
            // ソ連の優位なし：USSR→USAの順で選択
            states.emplace_back(StateType::HEADLINE_CARD_SELECT_USA);
            states.emplace_back(StateType::HEADLINE_CARD_SELECT_USSR);
          }
          break;
        }

        case StateType::HEADLINE_CARD_SELECT_USSR: {
          return {LegalMovesGenerator::headlineCardSelectLegalMoves(board,
                                                                    Side::USSR),
                  Side::USSR, std::nullopt};
        }

        case StateType::HEADLINE_CARD_SELECT_USA: {
          return {LegalMovesGenerator::headlineCardSelectLegalMoves(board,
                                                                    Side::USA),
                  Side::USA, std::nullopt};
        }

        case StateType::HEADLINE_PROCESS_EVENTS: {
          // 両側のヘッドラインカードを取得
          CardEnum ussr_card_enum = board.getHeadlineCard(Side::USSR);
          CardEnum usa_card_enum = board.getHeadlineCard(Side::USA);

          if (ussr_card_enum != CardEnum::Dummy &&
              usa_card_enum != CardEnum::Dummy) {
            const auto& cardpool = board.getCardpool();
            const auto& ussr_card =
                cardpool[static_cast<size_t>(ussr_card_enum)];
            const auto& usa_card = cardpool[static_cast<size_t>(usa_card_enum)];

            int ussr_ops = ussr_card->getOps();
            int usa_ops = usa_card->getOps();

            // 作戦値を比較してイベント処理順序を決定
            if (ussr_ops > usa_ops) {
              // USSRから処理つまりUSAから積む
              if (usa_card->canEvent(board)) {
                auto usa_events = usa_card->event(Side::USA);
                for (auto& event : std::ranges::reverse_view(usa_events)) {
                  states.emplace_back(std::move(event));
                }
              }
              if (ussr_card->canEvent(board)) {
                auto ussr_events = ussr_card->event(Side::USSR);
                for (auto& event : std::ranges::reverse_view(ussr_events)) {
                  states.emplace_back(std::move(event));
                }
              }
            } else {
              // USAから処理（同値の場合もUSAから）つまりUSSRから積む
              if (ussr_card->canEvent(board)) {
                auto ussr_events = ussr_card->event(Side::USSR);
                for (auto& event : std::ranges::reverse_view(ussr_events)) {
                  states.emplace_back(std::move(event));
                }
              }
              if (usa_card->canEvent(board)) {
                auto usa_events = usa_card->event(Side::USA);
                for (auto& event : std::ranges::reverse_view(usa_events)) {
                  states.emplace_back(std::move(event));
                }
              }
            }
          }

          // ヘッドラインカードをクリア
          board.clearHeadlineCards();

          states.emplace_back(StateType::AR_USSR);
          break;
        }

          /* その他のフェーズ */
      }
    }
  }
  return {std::vector<std::unique_ptr<Move>>{}, Side::NEUTRAL,
          Side::NEUTRAL};  // スタックが空→ゲーム終了
}
