#pragma once

#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/cards_enum.hpp"

class LegalMovesGenerator {
 public:
  static std::vector<std::shared_ptr<Move>> arLegalMoves(const Board& board,
                                                         Side side);
  static std::vector<std::shared_ptr<Move>> actionLegalMovesForCard(
      const Board& board, Side side, CardEnum cardEnum);
  static std::vector<std::shared_ptr<Move>> headlineCardSelectLegalMoves(
      const Board& board, Side side);
  static std::vector<std::shared_ptr<Move>> extraActionRoundLegalMoves(
      const Board& board, Side side);
  static std::vector<std::shared_ptr<Move>> spaceTrackDiscardLegalMoves(
      const Board& board, Side side);

  static std::vector<std::shared_ptr<Move>> realignmentRequestLegalMoves(
      const Board& board, Side side, CardEnum cardEnum,
      const std::vector<CountryEnum>& history, int remainingOps,
      AdditionalOpsType appliedAdditionalOps = AdditionalOpsType::NONE);
  static std::vector<std::shared_ptr<Move>> additionalOpsRealignmentLegalMoves(
      const Board& board, Side side, CardEnum cardEnum,
      const std::vector<CountryEnum>& history,
      AdditionalOpsType appliedAdditionalOps);
  static std::vector<std::shared_ptr<Move>> actionPlaceInfluenceLegalMoves(
      const Board& board, Side side);
  static std::vector<std::shared_ptr<Move>>
  actionPlaceInfluenceLegalMovesForCard(const Board& board, Side side,
                                        CardEnum cardEnum);
  static std::vector<std::shared_ptr<Move>> actionRealignmentLegalMoves(
      const Board& board, Side side);
  static std::vector<std::shared_ptr<Move>> actionRealignmentLegalMovesForCard(
      const Board& board, Side side, CardEnum cardEnum);
  static std::vector<std::shared_ptr<Move>> actionCoupLegalMoves(
      const Board& board, Side side);
  static std::vector<std::shared_ptr<Move>> actionCoupLegalMovesForCard(
      const Board& board, Side side, CardEnum cardEnum);
  static std::vector<std::shared_ptr<Move>> actionEventLegalMoves(
      const Board& board, Side side);
  static std::vector<std::shared_ptr<Move>> actionSpaceRaceLegalMoves(
      const Board& board, Side side);
};
