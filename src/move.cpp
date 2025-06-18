#include "move.hpp"

#include <memory>

#include "board.hpp"
#include "command.hpp"
#include "game_enums.hpp"
#include "legal_moves_generator.hpp"

std::vector<CommandPtr> ActionPlaceInfluenceMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_unique<ActionPlaceInfluenceCommand>(
      getSide(), card, targetCountries_));
  if (getOpponentSide(getSide()) == card->getSide()) {
    // Opponent's event is triggered after the action
    auto eventCommands = card->event(getSide());
    commands.insert(commands.end(), eventCommands.begin(), eventCommands.end());
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
    auto eventCommands = card->event(getSide());
    commands.insert(commands.end(), eventCommands.begin(), eventCommands.end());
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
  const int remainingOps = card->getOps() - 1;
  if (remainingOps > 0) {
    // 最初の実行履歴を作成
    std::vector<CountryEnum> initialHistory = {targetCountry_};

    commands.emplace_back(std::make_unique<RequestCommand>(
        getSide(),
        [side = getSide(), card_enum = getCard(), history = initialHistory,
         ops = remainingOps](
            const Board& board) -> std::vector<std::unique_ptr<Move>> {
          return LegalMovesGenerator::RealignmentRequestLegalMoves(
              board, side, card_enum, history, ops, AdditionalOpsType::NONE);
        }));
  }

  // Check if opponent's event should be triggered
  if (getOpponentSide(getSide()) == card->getSide()) {
    // Opponent's event is triggered after all realignment actions
    auto eventCommands = card->event(getSide());
    commands.insert(commands.end(), eventCommands.begin(), eventCommands.end());
  }

  return commands;
}

std::vector<CommandPtr> RealignmentRequestMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_unique<ActionRealigmentCommand>(
      getSide(), card, targetCountry_));

  // 実行履歴を更新
  std::vector<CountryEnum> updatedHistory = realignmentHistory_;
  updatedHistory.push_back(targetCountry_);

  // 残りのOps数を計算
  const int newRemainingOps = remainingOps_ - 1;

  if (newRemainingOps > 0) {
    // まだOpsが残っている場合は、次のRequestを生成
    commands.emplace_back(std::make_unique<RequestCommand>(
        getSide(),
        [side = getSide(), card_enum = getCard(), history = updatedHistory,
         ops = newRemainingOps, appliedOps = appliedAdditionalOps_](
            const Board& board) -> std::vector<std::unique_ptr<Move>> {
          return LegalMovesGenerator::RealignmentRequestLegalMoves(
              board, side, card_enum, history, ops, appliedOps);
        }));
  } else {
    // すべてのOpsを使い切った場合、追加Opsの処理をチェック
    // 追加Opsの可能性がある場合は常にRequestを生成
    // 実際の判定はLegalMovesGeneratorで行う
    commands.emplace_back(std::make_unique<RequestCommand>(
        getSide(),
        [side = getSide(), card_enum = getCard(), history = updatedHistory,
         appliedOps = appliedAdditionalOps_](
            const Board& board) -> std::vector<std::unique_ptr<Move>> {
          return LegalMovesGenerator::AdditionalOpsRealignmentLegalMoves(
              board, side, card_enum, history, appliedOps);
        }));
  }

  return commands;
}

std::vector<CommandPtr> ActionEventMove::toCommand(
    const std::unique_ptr<Card>& card) const {
  std::vector<CommandPtr> commands;

  // Get the card's side and player's side
  const Side cardSide = card->getSide();
  const Side playerSide = getSide();

  // Execute the event
  auto eventCommands = card->event(playerSide);
  commands.insert(commands.end(), eventCommands.begin(), eventCommands.end());

  // If the card is of the opponent's side, allow the player to perform a
  // non-event action
  if (cardSide != playerSide && cardSide != Side::NEUTRAL) {
    // Add a RequestCommand for the player to choose Place/Realign/Coup action
    commands.emplace_back(std::make_unique<RequestCommand>(
        playerSide,
        [card_enum = getCard(), side = playerSide](
            const Board& board) -> std::vector<std::unique_ptr<Move>> {
          // TODO: This should be implemented in LegalMovesGenerator
          // For now, return empty vector as placeholder
          return {};
        }));
  }

  return commands;
}