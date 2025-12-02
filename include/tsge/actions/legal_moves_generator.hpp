#pragma once

#include <optional>
#include <vector>

#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"

// カード固有の影響力配置設定
struct CardSpecialPlaceInfluenceConfig {
  int totalInfluence = 0;  // 配置する総影響力数
  int maxPerCountry = 0;  // 一カ国あたりの最大配置数（0=無制限）
  std::optional<std::vector<Region>> allowedRegions;  // 許可される地域
  bool excludeOpponentControlled = false;  // 相手支配国を除外するか
  bool onlyEmptyCountries = false;         // 影響力のない国のみか
  CardSpecialPlaceInfluenceConfig() = default;
};

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

  // カード固有の影響力配置Moveを生成
  static std::vector<std::shared_ptr<Move>>
  generateCardSpecificPlaceInfluenceMoves(
      const Board& board, Side side, CardEnum cardEnum,
      const CardSpecialPlaceInfluenceConfig& config);

  // カード固有の影響力除去Moveを生成
  static std::vector<std::shared_ptr<Move>> generateRemoveInfluenceMoves(
      const Board& board, CardEnum cardEnum, Side moveSide, Side targetSide,
      int totalRemove, int maxPerCountry,
      const std::optional<std::vector<Region>>& allowedRegions,
      const std::optional<std::vector<CountryEnum>>& specificCountries =
          std::nullopt);

  // N国選択して固定数除去
  static std::vector<std::shared_ptr<Move>>
  generateSelectCountriesRemoveInfluenceMoves(const Board& board,
                                              CardEnum cardEnum, Side moveSide,
                                              Side targetSide, Region region,
                                              int countriesToSelect,
                                              int removePerCountry);

  // N国選択して全除去
  static std::vector<std::shared_ptr<Move>>
  generateSelectCountriesRemoveAllInfluenceMoves(
      const Board& board, CardEnum cardEnum, Side moveSide, Side targetSide,
      const std::vector<CountryEnum>& candidates, int countriesToSelect);

  // De-Stalinization: USSR影響力を1-4除去するパターンを生成
  static std::vector<std::shared_ptr<Move>> generateDeStalinizationRemoveMoves(
      const Board& board, CardEnum cardEnum, Side side);
};
