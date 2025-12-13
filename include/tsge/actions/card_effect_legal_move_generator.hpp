#pragma once
// どこで: include/tsge/actions/card_effect_legal_move_generator.hpp
// 何を: カード固有イベント用の合法手生成とレジストリの宣言
// なぜ: カードごとの分岐ロジックを集約し、拡張と保守を容易にするため

#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"

// カード固有の影響力配置設定
struct CardSpecialPlaceInfluenceConfig {
  int totalInfluence = 0;  // 配置する総影響力数
  int maxPerCountry = 0;   // 1カ国あたりの上限（0=無制限）
  std::optional<std::vector<Region>> allowedRegions;  // 許可地域
  bool excludeOpponentControlled = false;  // 相手支配国を除外
  bool onlyEmptyCountries = false;         // 無影響国のみ対象
  CardSpecialPlaceInfluenceConfig() = default;
};

enum class RemovalSaturationStrategy { kAllowPartial, kRequireExact };

class CardEffectLegalMoveGenerator {
 public:
  using GeneratorFunction =
      std::function<std::vector<std::shared_ptr<Move>>(const Board&, Side)>;

  // ビルトインカード効果（De-Stalinizationなど）をレジストリに登録する。
  // ゲームのトップレベル(例:
  // Gameコンストラクタ)やテストのセットアップで一度だけ呼ぶ想定。
  static void initializeBuiltinGenerators();
#ifdef TEST
  // テスト専用: レジストリ状態をリセットし、初期化フラグを解除する。
  static void resetBuiltinGeneratorsForTest();
#endif

  static void registerGenerator(CardEnum cardEnum, GeneratorFunction generator);
  static std::vector<std::shared_ptr<Move>> generate(CardEnum cardEnum,
                                                     const Board& board,
                                                     Side side);

  static std::vector<std::shared_ptr<Move>>
  generateCardSpecificPlaceInfluenceMoves(
      const Board& board, Side side, CardEnum cardEnum,
      const CardSpecialPlaceInfluenceConfig& config);

  static std::vector<std::shared_ptr<Move>> generateRemoveInfluenceMoves(
      const Board& board, CardEnum cardEnum, Side moveSide, Side targetSide,
      int totalRemove, int maxPerCountry,
      const std::optional<std::vector<Region>>& allowedRegions,
      const std::optional<std::vector<CountryEnum>>& specificCountries =
          std::nullopt);

  static std::vector<std::shared_ptr<Move>>
  generateSelectCountriesRemoveInfluenceMoves(const Board& board,
                                              CardEnum cardEnum, Side moveSide,
                                              Side targetSide, Region region,
                                              int countriesToSelect,
                                              int removePerCountry);

  static std::vector<std::shared_ptr<Move>>
  generateSelectCountriesRemoveAllInfluenceMoves(
      const Board& board, CardEnum cardEnum, Side moveSide, Side targetSide,
      const std::vector<CountryEnum>& candidates, int countriesToSelect);

  static std::vector<std::map<CountryEnum, int>>
  enumerateRemoveInfluencePatterns(
      const Board& board, Side targetSide, int totalRemove, int maxPerCountry,
      const std::optional<std::vector<Region>>& allowedRegions,
      const std::optional<std::vector<CountryEnum>>& specificCountries,
      RemovalSaturationStrategy saturation);
};
