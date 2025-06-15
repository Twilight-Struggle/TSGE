#pragma once

#include "board.hpp"
#include "cards_enum.hpp"
#include "move.hpp"

class LegalMovesGenerator {
 public:
  static std::vector<std::unique_ptr<Move>> ArLegalMoves(const Board& board,
                                                         Side side);
  static std::vector<std::unique_ptr<Move>> ActionRealignmentLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> RealignmentRequestLegalMoves(
      const Board& board, Side side, CardEnum cardEnum,
      const std::vector<CountryEnum>& history, int remainingOps, 
      AdditionalOpsType appliedAdditionalOps = AdditionalOpsType::NONE);
  static std::vector<std::unique_ptr<Move>> AdditionalOpsRealignmentLegalMoves(
      const Board& board, Side side, CardEnum cardEnum,
      const std::vector<CountryEnum>& history, 
      AdditionalOpsType appliedAdditionalOps);

 private:
  static std::vector<std::unique_ptr<Move>> ActionPlaceInfluenceLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> ActionCoupLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> ActionEventLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> ActionSpaceRaceLegalMoves(
      const Board& board, Side side);
};