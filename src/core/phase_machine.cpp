#include "tsge/core/phase_machine.hpp"

#include <cstddef>
#include <ranges>
#include <variant>

#include "tsge/actions/command.hpp"
#include "tsge/actions/legal_moves_generator.hpp"
#include "tsge/core/board.hpp"

//===----------------------------------------------------------------------===//
// File: src/core/phase_machine.cpp
// 内容: PhaseMachine::stepに関する状態遷移ロジックを実装する。
// 目的: 責務を分割しテスト容易性と可読性を高める。
//===----------------------------------------------------------------------===//

namespace {

using StateVariant = std::variant<StateType, CommandPtr>;
using StateStack = std::vector<StateVariant>;
using LegalMoves = std::vector<std::shared_ptr<Move>>;
using StepOutput = std::tuple<LegalMoves, Side, std::optional<Side>>;
using MaybeStepOutput = std::optional<StepOutput>;

constexpr StepOutput makeTerminalResult(Side winner) {
  return StepOutput{LegalMoves{}, Side::NEUTRAL, winner};
}

MaybeStepOutput makeInputResult(LegalMoves moves, Side side) {
  return StepOutput{std::move(moves), side, std::nullopt};
}

void pushCommandsForAnswer(Board& board, StateStack& states,
                           const std::shared_ptr<Move>& move) {
  const auto& card_pool = board.getCardpool();
  auto commands =
      move->toCommand(card_pool[static_cast<std::size_t>(move->getCard())]);
  for (auto& command : std::ranges::reverse_view(commands)) {
    states.emplace_back(std::move(command));
  }
}

void processAnswer(Board& board, StateStack& states,
                   std::optional<std::shared_ptr<Move>>& answer) {
  if (!answer) {
    return;
  }

  auto selected_move = std::move(*answer);
  answer.reset();

  // 回答が来たらRequestCommandをスタックから取り除く
  if (!states.empty() && std::holds_alternative<CommandPtr>(states.back())) {
    auto& ptr = std::get<CommandPtr>(states.back());
    if (dynamic_cast<RequestCommand*>(ptr.get()) != nullptr) {
      states.pop_back();
    }
  }

  pushCommandsForAnswer(board, states, selected_move);
}

MaybeStepOutput handleCommandOnTop(Board& board, StateStack& states) {
  auto& top_variant = states.back();
  auto* command_ptr = std::get_if<CommandPtr>(&top_variant);
  if (command_ptr == nullptr) {
    return std::nullopt;
  }

  if (auto* request = dynamic_cast<RequestCommand*>(command_ptr->get())) {
    auto legal_moves = request->legalMoves(board);
    if (legal_moves.empty()) {
      states.pop_back();
      return std::nullopt;
    }
    return makeInputResult(std::move(legal_moves), request->getSide());
  }

  (*command_ptr)->apply(board);
  states.pop_back();
  return std::nullopt;
}

struct ActionRoundStatus {
  bool opponent_has_normal = false;
  bool current_has_normal = false;
  bool current_has_extra = false;
  bool opponent_has_extra = false;
  Side opponent_side = Side::NEUTRAL;
};

ActionRoundStatus updateActionRoundStatus(Board& board, Side currentSide) {
  ActionRoundStatus status{};
  status.opponent_side = getOpponentSide(currentSide);

  auto& action_round_track = board.getActionRoundTrack();
  const int current_turn = board.getTurnTrack().getTurn();
  action_round_track.advanceActionRound(currentSide, current_turn);

  const int defined_rounds =
      action_round_track.getDefinedActionRounds(current_turn);
  const int current_ar = action_round_track.getActionRound(currentSide);
  const int opponent_ar =
      action_round_track.getActionRound(status.opponent_side);

  status.opponent_has_normal = opponent_ar < defined_rounds;
  status.current_has_normal = current_ar < defined_rounds;
  status.current_has_extra =
      action_round_track.hasExtraActionRound(currentSide);
  status.opponent_has_extra =
      action_round_track.hasExtraActionRound(status.opponent_side);

  return status;
}

MaybeStepOutput handleActionRound(Board& board, StateStack& states, Side side,
                                  StateType completion_state) {
  board.setCurrentArPlayer(side);
  states.emplace_back(completion_state);

  auto legal_moves = LegalMovesGenerator::arLegalMoves(board, side);
  if (legal_moves.empty()) {
    return std::nullopt;
  }
  return makeInputResult(std::move(legal_moves), side);
}

MaybeStepOutput handleExtraActionRound(Board& board, StateStack& states,
                                       Side side, StateType completion_state) {
  board.setCurrentArPlayer(side);
  board.getActionRoundTrack().clearExtraActionRound(side);
  states.emplace_back(completion_state);

  // TODO(tsge-phase-machine): 追加AR専用のパスムーブを提供する。
  auto legal_moves = LegalMovesGenerator::arLegalMoves(board, side);
  if (legal_moves.empty()) {
    return std::nullopt;
  }
  return makeInputResult(std::move(legal_moves), side);
}

void handleArCompleteForUssr(Board& board, StateStack& states) {
  const auto status = updateActionRoundStatus(board, Side::USSR);

  if (status.opponent_has_normal) {
    states.emplace_back(StateType::AR_USA);
    return;
  }
  if (status.current_has_normal) {
    states.emplace_back(StateType::AR_USSR);
    return;
  }
  if (status.opponent_has_extra) {
    states.emplace_back(StateType::EXTRA_AR_USA);
    return;
  }
  if (status.current_has_extra) {
    states.emplace_back(StateType::EXTRA_AR_USSR);
    return;
  }

  states.emplace_back(StateType::TURN_END);
}

void handleArCompleteForUsa(Board& board, StateStack& states) {
  const auto status = updateActionRoundStatus(board, Side::USA);

  if (status.opponent_has_normal) {
    states.emplace_back(StateType::AR_USSR);
    return;
  }
  if (status.current_has_normal) {
    states.emplace_back(StateType::AR_USA);
    return;
  }
  if (status.current_has_extra || status.opponent_has_extra) {
    if (status.current_has_extra) {
      states.emplace_back(StateType::EXTRA_AR_USSR);
    }
    if (status.opponent_has_extra) {
      states.emplace_back(StateType::EXTRA_AR_USA);
    }
    return;
  }

  states.emplace_back(StateType::TURN_END);
}

void enqueueHeadlineSelections(Board& board, StateStack& states) {
  const bool ussr_has_headline_advantage =
      board.getSpaceTrack().effectEnabled(Side::USSR, 4);

  states.emplace_back(StateType::HEADLINE_PROCESS_EVENTS);
  if (ussr_has_headline_advantage) {
    states.emplace_back(StateType::HEADLINE_CARD_SELECT_USSR);
    states.emplace_back(StateType::HEADLINE_CARD_SELECT_USA);
  } else {
    states.emplace_back(StateType::HEADLINE_CARD_SELECT_USA);
    states.emplace_back(StateType::HEADLINE_CARD_SELECT_USSR);
  }
}

void enqueueHeadlineEvents(Board& board, StateStack& states,
                           const std::unique_ptr<Card>& card, Side side) {
  if (!card->canEvent(board)) {
    return;
  }

  auto events = card->event(side);
  for (auto& event : std::ranges::reverse_view(events)) {
    states.emplace_back(std::move(event));
  }
}

MaybeStepOutput handleHeadlineProcess(Board& board, StateStack& states) {
  const CardEnum ussr_card_id = board.getHeadlineCard(Side::USSR);
  const CardEnum usa_card_id = board.getHeadlineCard(Side::USA);

  states.emplace_back(StateType::AR_USSR);

  if (ussr_card_id != CardEnum::Dummy && usa_card_id != CardEnum::Dummy) {
    const auto& card_pool = board.getCardpool();
    const auto& ussr_card = card_pool[static_cast<std::size_t>(ussr_card_id)];
    const auto& usa_card = card_pool[static_cast<std::size_t>(usa_card_id)];

    const int ussr_ops = ussr_card->getOps();
    const int usa_ops = usa_card->getOps();

    if (ussr_ops > usa_ops) {
      enqueueHeadlineEvents(board, states, usa_card, Side::USA);
      enqueueHeadlineEvents(board, states, ussr_card, Side::USSR);
    } else {
      enqueueHeadlineEvents(board, states, ussr_card, Side::USSR);
      enqueueHeadlineEvents(board, states, usa_card, Side::USA);
    }
  }

  board.clearHeadlineCards();
  return std::nullopt;
}

MaybeStepOutput handleState(Board& board, StateStack& states, StateType state) {
  switch (state) {
    case StateType::AR_USSR:
      return handleActionRound(board, states, Side::USSR,
                               StateType::AR_USSR_COMPLETE);
    case StateType::AR_USA:
      return handleActionRound(board, states, Side::USA,
                               StateType::AR_USA_COMPLETE);
    case StateType::AR_USSR_COMPLETE:
      handleArCompleteForUssr(board, states);
      // TODO(tsge-phase-machine): DEFCON=2およびNORADの即時発火を実装する。
      return std::nullopt;
    case StateType::AR_USA_COMPLETE:
      handleArCompleteForUsa(board, states);
      // TODO(tsge-phase-machine): DEFCON=2およびNORADの即時発火を実装する。
      return std::nullopt;
    case StateType::EXTRA_AR_USSR:
      return handleExtraActionRound(board, states, Side::USSR,
                                    StateType::AR_USSR_COMPLETE);
    case StateType::EXTRA_AR_USA:
      return handleExtraActionRound(board, states, Side::USA,
                                    StateType::AR_USA_COMPLETE);
    case StateType::TURN_END:
      // TODO(tsge-phase-machine): 補給・得点計算などターン終了処理を網羅する。
      board.getTurnTrack().nextTurn();
      board.getActionRoundTrack().resetActionRounds();
      states.emplace_back(StateType::TURN_START);
      return std::nullopt;
    case StateType::USSR_WIN_END:
      return makeTerminalResult(Side::USSR);
    case StateType::USA_WIN_END:
      return makeTerminalResult(Side::USA);
    case StateType::DRAW_END:
      return makeTerminalResult(Side::NEUTRAL);
    case StateType::TURN_START:
      board.getActionRoundTrack().updateExtraActionRound(board.getSpaceTrack());
      // TODO(tsge-phase-machine): North Sea Oil等ターン開始時効果を評価する。
      states.emplace_back(StateType::HEADLINE_PHASE);
      return std::nullopt;
    case StateType::HEADLINE_PHASE:
      enqueueHeadlineSelections(board, states);
      return std::nullopt;
    case StateType::HEADLINE_CARD_SELECT_USSR:
      return makeInputResult(
          LegalMovesGenerator::headlineCardSelectLegalMoves(board, Side::USSR),
          Side::USSR);
    case StateType::HEADLINE_CARD_SELECT_USA:
      return makeInputResult(
          LegalMovesGenerator::headlineCardSelectLegalMoves(board, Side::USA),
          Side::USA);
    case StateType::HEADLINE_PROCESS_EVENTS:
      return handleHeadlineProcess(board, states);
  }
  return std::nullopt;
}

}  // namespace

// TODO(tsge-phase-machine):
// - 追加ARのパス選択肢と処理結果のテストシナリオを整備する。
// - DEFCON=2とNORAD発動時の分岐をPhaseMachineへ統合する。
// - ターン開始/終了時のリソース補充やイベントを段階的に実装する。

/// 入力 (Move) がある場合はそれを使って１フェーズ進め，
/// まだ入力が必要なら合法 Move を返す
std::tuple<std::vector<std::shared_ptr<Move>>, Side, std::optional<Side>>
PhaseMachine::step(Board& board,
                   std::optional<std::shared_ptr<Move>>&& answer) {
  auto& states = board.getStates();
  processAnswer(board, states, answer);

  while (!states.empty()) {
    if (std::holds_alternative<CommandPtr>(states.back())) {
      if (auto result = handleCommandOnTop(board, states)) {
        return std::move(*result);
      }
      continue;
    }

    const auto state = std::get<StateType>(states.back());
    states.pop_back();
    if (auto result = handleState(board, states, state)) {
      return std::move(*result);
    }
  }

  return makeTerminalResult(Side::NEUTRAL);
}
