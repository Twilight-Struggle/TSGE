#pragma once

#include "move.hpp"

class LegalMovesGenerator {
 public:
  static std::vector<std::unique_ptr<Move>> ArLegalMoves(const Board& board,
                                                         Side side);

 private:
  static std::vector<std::unique_ptr<Move>> ActionPlaceInfluenceLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> ActionCoupLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> ActionRealignLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> ActionEventLegalMoves(
      const Board& board, Side side);
  static std::vector<std::unique_ptr<Move>> ActionSpaceRaceLegalMoves(
      const Board& board, Side side);
};