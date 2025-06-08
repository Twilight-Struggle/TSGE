#include "legal_moves_generator.hpp"

#include <map>
#include <utility>
#include <vector>

#include "cards_enum.hpp"
#include "game_enums.hpp"
#include "world_map.hpp"

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
        for (auto& [c, _] : placed)
          if (!board.getWorldMap().getCountry(c).getRegions().contains(
                  Region::ASIA))
            return false;
        return true;
      }};
  static const BonusCondition seAsiaOnly{
      [&board](const std::map<CountryEnum, int>& placed) {
        for (auto& [c, _] : placed)
          if (!board.getWorldMap().getCountry(c).getRegions().contains(
                  Region::SOUTH_EAST_ASIA))
            return false;
        return true;
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
      for (const auto& pattern : cache[key])
        results.emplace_back(std::make_unique<ActionPlaceInfluenceMove>(
            cardEnum, side, pattern));
    }
  }
  return results;
}