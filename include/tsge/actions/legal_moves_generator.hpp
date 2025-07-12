#pragma once

#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/cards_enum.hpp"

class LegalMovesGenerator {
 public:
  static std::vector<std::unique_ptr<Move>> arLegalMoves(const Board& board,
                                                         Side side);
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
  static std::vector<std::unique_ptr<Move>> actionRealignmentLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> actionCoupLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> actionEventLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> actionSpaceRaceLegalMoves(
      const Board& board, Side side);
};