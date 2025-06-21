#include "tsge/actions/legal_moves_generator.hpp"

#include <cstddef>
#include <map>
#include <utility>
#include <vector>

#include "tsge/actions/move.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/cards_enum.hpp"
#include "tsge/game_state/world_map.hpp"

// CountryEnum抽出用のヘルパー
template <typename T>
struct CountryExtractor {
  static CountryEnum extract(const T& elem) { return elem; }
};

template <typename V>
struct CountryExtractor<std::pair<const CountryEnum, V>> {
  static CountryEnum extract(const std::pair<const CountryEnum, V>& elem) {
    return elem.first;
  }
};

// ヘルパー関数：コンテナ内のすべての国が特定の地域に属するかチェック
template <Region region, typename Container>
static inline bool isAllInRegion(const Container& countries,
                                 const Board& board) {
  for (const auto& elem : countries) {
    const auto country =
        CountryExtractor<std::decay_t<decltype(elem)>>::extract(elem);
    const auto& countryObj = board.getWorldMap().getCountry(country);
    if (!countryObj.getRegions().contains(region)) {
      return false;
    }
  }
  return true;
}

struct BonusCondition {
  /// 全配置がこの条件(例えば全部アジアにおいてる)を満たしていれば true
  std::function<bool(const std::map<CountryEnum, int>&)> isSatisfied;
};

static std::vector<std::pair<int, const BonusCondition*>> computeOpsVariants(
    CardEnum cardId, const Board& board, Side side) {
  const int baseOps =
      board.getCardpool()[static_cast<size_t>(cardId)]->getOps();

  static const BonusCondition asiaOnly{
      /* すべての国がアジア地域か？ */
      [&board](const std::map<CountryEnum, int>& placed) {
        return isAllInRegion<Region::ASIA>(placed, board);
      }};
  static const BonusCondition seAsiaOnly{
      [&board](const std::map<CountryEnum, int>& placed) {
        return isAllInRegion<Region::SOUTH_EAST_ASIA>(placed, board);
      }};

  std::vector<std::pair<int, const BonusCondition*>> res;
  /* ---- 基本 Ops は必ず存在 ---- */
  res.push_back({baseOps, nullptr});

  // /* ---- 中国カードの +1 ---- */
  // if (cardId == CardEnum::CHINA_CARD) res.push_back({baseOps + 1,
  // &asiaOnly});

  // /* ---- ベトナム蜂起が効いていれば +1 ---- */
  // if (board.isVietnamRevoltsActive(side))
  //   res.push_back({baseOps + 1, &seAsiaOnly});

  /* ---- ベトナム蜂起 + 中国カード → +2 ---- */

  return res;
}

/// その国に「影響力を +1」するのに必要な OP コストを返す
inline int costToAddOneInfluence(const WorldMap& worldMap,
                                 CountryEnum countryEnum, Side side) {
  Side opponentSide = getOpponentSide(side);

  // 相手が現在その国を支配しているか？
  const bool opponentControls =
      worldMap.getCountry(countryEnum).getControlSide() == opponentSide;

  return opponentControls ? 2 : 1;
}

void placeInfluenceDfs(int usedOps, size_t startIdx, WorldMap& tmpWorldMap,
                       std::map<CountryEnum, int>& placed,
                       std::vector<std::map<CountryEnum, int>>& out,
                       int totalOps,
                       const std::vector<CountryEnum>& placeableVec, Side side,
                       const BonusCondition* bonus) {
  if (usedOps == totalOps) {
    if (!bonus || bonus->isSatisfied(placed)) {
      out.emplace_back(placed);
    }
    return;
  }
  for (size_t i = startIdx; i < placeableVec.size(); ++i) {
    auto countryEnum = placeableVec[i];
    int cost = costToAddOneInfluence(tmpWorldMap, countryEnum, side);
    if (usedOps + cost > totalOps) continue;

    // 影響力を１追加して再帰
    placed[countryEnum] += 1;
    tmpWorldMap.getCountry(countryEnum)
        .addInfluence(side, 1);  // 軽量盤面を更新
    placeInfluenceDfs(usedOps + cost, i, tmpWorldMap, placed, out, totalOps,
                      placeableVec, side, bonus);
    tmpWorldMap.getCountry(countryEnum)
        .removeInfluence(side, 1);  // バックトラック
    placed[countryEnum] -= 1;
    if (placed[countryEnum] == 0) placed.erase(countryEnum);
  }
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::ActionPlaceInfluenceLegalMoves(const Board& board,
                                                    Side side) {
  auto& hands = board.getPlayerHand(side);
  if (hands.empty()) return {};

  const auto worldMap = board.getWorldMap();
  auto placeable = worldMap.placeableCountries(side);
  if (placeable.empty()) return {};

  std::vector<CountryEnum> placeableVec;
  placeableVec.assign(placeable.begin(), placeable.end());

  /*----  Ops×Bonus ごとに DFS を 1 度だけ ----*/
  struct Key {
    int ops;
    const BonusCondition* bonus;
  };
  // bonusの比較は単にポインタを比較している
  // ポインタが同じならば同じボーナス条件とみなされるため、mapのinsertに失敗する
  // これにより同じボーナス条件がかぶるのを防ぐ
  auto cmp = [](Key a, Key b) {
    return std::tie(a.ops, a.bonus) < std::tie(b.ops, b.bonus);
  };
  std::map<Key, std::vector<std::map<CountryEnum, int>>, decltype(cmp)> cache(
      cmp);

  std::vector<std::unique_ptr<Move>> results;

  for (CardEnum cardEnum : hands) {
    for (auto [totalOps, bonus] : computeOpsVariants(cardEnum, board, side)) {
      Key key{totalOps, bonus};

      if (!cache.count(key)) {
        WorldMap tmpWorldMap(worldMap);
        std::map<CountryEnum, int> placed;
        cache[key] = {};

        placeInfluenceDfs(0, 0, tmpWorldMap, placed, cache[key], totalOps,
                          placeableVec, side, bonus);
      }
      results.reserve(results.size() + cache[key].size());
      for (const auto& pattern : cache[key])
        results.emplace_back(std::make_unique<ActionPlaceInfluenceMove>(
            cardEnum, side, pattern));
    }
  }
  return results;
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::ActionRealignmentLegalMoves(const Board& board,
                                                 Side side) {
  auto& hands = board.getPlayerHand(side);
  if (hands.empty()) return {};

  const auto& worldMap = board.getWorldMap();
  std::vector<std::unique_ptr<Move>> results;

  // 相手側を取得
  Side opponentSide = getOpponentSide(side);

  // 全ての国を調べて、相手の影響力がある国を対象とする
  // USA/USSRを除く84か国（インデックス2から85まで）
  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < worldMap.getCountriesCount(); ++i) {
    CountryEnum countryEnum = static_cast<CountryEnum>(i);

    const auto& country = worldMap.getCountry(countryEnum);

    // 相手の影響力が0の国は対象外
    if (country.getInfluence(opponentSide) == 0) {
      continue;
    }

    // 各手札から対象国へのRealignmentMoveを生成
    results.reserve(results.size() + hands.size());
    for (CardEnum cardEnum : hands) {
      results.emplace_back(
          std::make_unique<ActionRealigmentMove>(cardEnum, side, countryEnum));
    }
  }

  return results;
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::RealignmentRequestLegalMoves(
    const Board& board, Side side, CardEnum cardEnum,
    const std::vector<CountryEnum>& history, int remainingOps,
    AdditionalOpsType appliedAdditionalOps) {
  const auto& worldMap = board.getWorldMap();

  std::vector<std::unique_ptr<Move>> results;
  // 事前に容量を確保（最大で国数+1個のMOVEが生成される）
  results.reserve(worldMap.getCountriesCount());

  // 相手側を取得
  Side opponentSide = getOpponentSide(side);

  // 全ての国を調べて、相手の影響力がある国を対象とする
  // USA/USSRを除く84か国（インデックス2から85まで）
  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < worldMap.getCountriesCount(); ++i) {
    CountryEnum countryEnum = static_cast<CountryEnum>(i);

    const auto& country = worldMap.getCountry(countryEnum);

    // 相手の影響力が0の国は対象外
    if (country.getInfluence(opponentSide) == 0) {
      continue;
    }

    results.emplace_back(std::make_unique<RealignmentRequestMove>(
        cardEnum, side, countryEnum, history, remainingOps,
        appliedAdditionalOps));
  }
  // RealignRequestMoveではUSSR=パスも選択可能
  results.emplace_back(std::make_unique<RealignmentRequestMove>(
      cardEnum, side, CountryEnum::USSR, history, remainingOps,
      appliedAdditionalOps));

  return results;
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::AdditionalOpsRealignmentLegalMoves(
    const Board& board, Side side, CardEnum cardEnum,
    const std::vector<CountryEnum>& history,
    AdditionalOpsType appliedAdditionalOps) {
  std::vector<std::unique_ptr<Move>> results;
  const auto& worldMap = board.getWorldMap();
  Side opponentSide = getOpponentSide(side);

  // 中国カードの追加Opsチェック
  bool chinaCardBonus = false;
  if (static_cast<uint8_t>(appliedAdditionalOps) &
      static_cast<uint8_t>(AdditionalOpsType::CHINA_CARD)) {
    // 既に中国カードボーナスが適用されている
  } else {
    // TODO: カードが中国カードかどうかの判定が必要
    // if (cardEnum == CHINA_CARD && isAllInRegion<Region::ASIA>(history,
    // board)) {
    //   chinaCardBonus = true;
    // }
  }

  // ベトナム蜂起の追加Opsチェック
  bool vietnamRevoltsBonus = false;
  if (static_cast<uint8_t>(appliedAdditionalOps) &
      static_cast<uint8_t>(AdditionalOpsType::VIETNAM_REVOLTS)) {
    // 既にベトナム蜂起ボーナスが適用されている
  } else {
    // TODO: ベトナム蜂起が有効かどうかの判定が必要
    // if (board.isVietnamRevoltsActive() &&
    // isAllInRegion<Region::SOUTH_EAST_ASIA>(history, board)) {
    //   vietnamRevoltsBonus = true;
    // }
  }

  // 中国カードボーナスの処理
  if (chinaCardBonus) {
    AdditionalOpsType newAppliedOps = static_cast<AdditionalOpsType>(
        static_cast<uint8_t>(appliedAdditionalOps) |
        static_cast<uint8_t>(AdditionalOpsType::CHINA_CARD));

    // アジア地域限定の合法手を生成
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < worldMap.getCountriesCount(); ++i) {
      CountryEnum countryEnum = static_cast<CountryEnum>(i);
      const auto& country = worldMap.getCountry(countryEnum);

      if (country.getInfluence(opponentSide) == 0) continue;
      if (!country.getRegions().contains(Region::ASIA)) continue;

      results.emplace_back(std::make_unique<RealignmentRequestMove>(
          cardEnum, side, countryEnum, history, 1, newAppliedOps));
    }
  }

  // ベトナム蜂起ボーナスの処理
  if (vietnamRevoltsBonus) {
    AdditionalOpsType newAppliedOps = static_cast<AdditionalOpsType>(
        static_cast<uint8_t>(appliedAdditionalOps) |
        static_cast<uint8_t>(AdditionalOpsType::VIETNAM_REVOLTS));

    // 東南アジア地域限定の合法手を生成
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < worldMap.getCountriesCount(); ++i) {
      CountryEnum countryEnum = static_cast<CountryEnum>(i);
      const auto& country = worldMap.getCountry(countryEnum);

      if (country.getInfluence(opponentSide) == 0) continue;
      if (!country.getRegions().contains(Region::SOUTH_EAST_ASIA)) continue;

      results.emplace_back(std::make_unique<RealignmentRequestMove>(
          cardEnum, side, countryEnum, history, 1, newAppliedOps));
    }
  }

  return results;
}