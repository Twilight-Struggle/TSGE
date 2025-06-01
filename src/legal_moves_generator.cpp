#include "legal_moves_generator.hpp"

#include <map>
#include <vector>

#include "game_enums.hpp"
#include "world_map.hpp"

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
                       std::vector<std::unique_ptr<Move>>& out, int totalOps,
                       const std::vector<CountryEnum>& placeableVec,
                       Side side) {
  if (usedOps == totalOps) {  // ちょうど使い切った
    out.push_back(std::make_unique<ActionPlaceInfluenceMove>(placed));  // TODO
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
                      placeableVec, side);
    tmpWorldMap.getCountry(countryEnum)
        .removeInfluence(side, 1);  // バックトラック
    placed[countryEnum] -= 1;
    if (placed[countryEnum] == 0) placed.erase(countryEnum);
  }
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::ActionPlaceInfluenceLegalMoves(const Board& board,
                                                    Side side) {
  auto totalOps = card.getOps();  // TODO

  auto worldMap = board.getWorldMap();
  auto placeable = worldMap.placeableCountries(side);
  if (placeable.empty()) return {};

  std::vector<CountryEnum> placeableVec;
  placeableVec.assign(placeable.begin(), placeable.end());

  WorldMap tmpWorldMap(worldMap);
  std::vector<std::unique_ptr<Move>> res;
  std::map<CountryEnum, int> placed;

  placeInfluenceDfs(0, 0, tmpWorldMap, placed, res, totalOps, placeableVec,
                    side);
  return res;
}