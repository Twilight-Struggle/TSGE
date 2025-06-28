#include "tsge/actions/move.hpp"

#include <memory>

#include "tsge/actions/command.hpp"
#include "tsge/actions/legal_moves_generator.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/game_enums.hpp"

std::vector<CommandPtr> ActionPlaceInfluenceMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_unique<ActionPlaceInfluenceCommand>(
      getSide(), card, targetCountries_));
  if (getOpponentSide(getSide()) == card->getSide()) {
    // Opponent's event is triggered after the action
    auto event_commands = card->event(getSide());
    for (auto& cmd : event_commands) {
      commands.emplace_back(std::move(cmd));
    }
  }
  return commands;
}

std::vector<CommandPtr> ActionCoupMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(
      std::make_unique<ActionCoupCommand>(getSide(), card, targetCountry_));
  if (getOpponentSide(getSide()) == card->getSide()) {
    // Opponent's event is triggered after the action
    auto event_commands = card->event(getSide());
    for (auto& cmd : event_commands) {
      commands.emplace_back(std::move(cmd));
    }
  }
  return commands;
}

std::vector<CommandPtr> ActionSpaceRaceMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(
      std::make_unique<ActionSpaceRaceCommand>(getSide(), card));
  return commands;
}

std::vector<CommandPtr> ActionRealigmentMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_unique<ActionRealigmentCommand>(
      getSide(), card, targetCountry_));

  // カードのOps数に応じてRequestコマンドを追加（最初の1回分は既に実行されるため-1）
  const int remaining_ops = card->getOps() - 1;
  if (remaining_ops > 0) {
    // 最初の実行履歴を作成
    std::vector<CountryEnum> initial_history = {targetCountry_};

    commands.emplace_back(std::make_unique<RequestCommand>(
        getSide(),
        [side = getSide(), card_enum = getCard(), history = initial_history,
         ops = remaining_ops](
            const Board& board) -> std::vector<std::unique_ptr<Move>> {
          return LegalMovesGenerator::realignmentRequestLegalMoves(
              board, side, card_enum, history, ops, AdditionalOpsType::NONE);
        }));
  }

  // Check if opponent's event should be triggered
  if (getOpponentSide(getSide()) == card->getSide()) {
    // Opponent's event is triggered after all realignment actions
    auto event_commands = card->event(getSide());
    for (auto& cmd : event_commands) {
      commands.emplace_back(std::move(cmd));
    }
  }

  return commands;
}

std::vector<CommandPtr> RealignmentRequestMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_unique<ActionRealigmentCommand>(
      getSide(), card, targetCountry_));

  // 実行履歴を更新
  std::vector<CountryEnum> updated_history = realignmentHistory_;
  updated_history.push_back(targetCountry_);

  // 残りのOps数を計算
  const int new_remaining_ops = remainingOps_ - 1;

  if (new_remaining_ops > 0) {
    // まだOpsが残っている場合は、次のRequestを生成
    commands.emplace_back(std::make_unique<RequestCommand>(
        getSide(),
        [side = getSide(), card_enum = getCard(), history = updated_history,
         ops = new_remaining_ops, applied_ops = appliedAdditionalOps_](
            const Board& board) -> std::vector<std::unique_ptr<Move>> {
          return LegalMovesGenerator::realignmentRequestLegalMoves(
              board, side, card_enum, history, ops, applied_ops);
        }));
  } else {
    // すべてのOpsを使い切った場合、追加Opsの処理をチェック
    // 追加Opsの可能性がある場合は常にRequestを生成
    // 実際の判定はLegalMovesGeneratorで行う
    commands.emplace_back(std::make_unique<RequestCommand>(
        getSide(),
        [side = getSide(), card_enum = getCard(), history = updated_history,
         applied_ops = appliedAdditionalOps_](
            const Board& board) -> std::vector<std::unique_ptr<Move>> {
          return LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
              board, side, card_enum, history, applied_ops);
        }));
  }

  return commands;
}

std::vector<CommandPtr> ActionEventMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;

  // Get the card's side and player's side
  const Side card_side = card->getSide();
  const Side player_side = getSide();

  // Execute the event
  auto event_commands = card->event(player_side);
  for (auto& cmd : event_commands) {
    commands.emplace_back(std::move(cmd));
  }

  // If the card is of the opponent's side, allow the player to perform a
  // non-event action
  if (card_side != player_side && card_side != Side::NEUTRAL) {
    // Add a RequestCommand for the player to choose Place/Realign/Coup action
    commands.emplace_back(std::make_unique<RequestCommand>(
        player_side,
        [card_enum = getCard(), side = player_side](
            const Board& board) -> std::vector<std::unique_ptr<Move>> {
          // TODO: This should be implemented in LegalMovesGenerator
          // For now, return empty vector as placeholder
          return {};
        }));
  }

  return commands;
}