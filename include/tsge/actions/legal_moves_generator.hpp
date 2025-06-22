#pragma once

#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/game_state/cards_enum.hpp"

class LegalMovesGenerator {
 public:
  static std::vector<std::unique_ptr<Move>> ArLegalMoves(const Board& board,
                                                         Side side);
  static std::vector<std::unique_ptr<Move>> actionRealignmentLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> realignmentRequestLegalMoves(
      const Board& board, Side side, CardEnum cardEnum,
      const std::vector<CountryEnum>& history, int remainingOps,
      AdditionalOpsType appliedAdditionalOps = AdditionalOpsType::NONE);
  static std::vector<std::unique_ptr<Move>> additionalOpsRealignmentLegalMoves(
      const Board& board, Side side, CardEnum cardEnum,
      const std::vector<CountryEnum>& history,
      AdditionalOpsType appliedAdditionalOps);

 private:
  static std::vector<std::unique_ptr<Move>> actionPlaceInfluenceLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> ActionCoupLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> ActionEventLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> ActionSpaceRaceLegalMoves(
      const Board& board, Side side);
};