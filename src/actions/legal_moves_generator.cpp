#include "tsge/actions/legal_moves_generator.hpp"

#include <algorithm>
#include <cstddef>
#include <map>
#include <utility>
#include <vector>

#include "tsge/actions/move.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
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
  return std::ranges::all_of(countries, [&](const auto& elem) {
    const auto country =
        CountryExtractor<std::decay_t<decltype(elem)>>::extract(elem);
    const auto& country_obj = board.getWorldMap().getCountry(country);
    return country_obj.hasRegion(region);
  });
}

// DEFCON制限により特定の国がRealignmentできないかを判定
static inline bool isRegionRestrictedByDefcon(const Country& country,
                                              int defcon) {
  if (defcon <= 4 && country.hasRegion(Region::EUROPE)) {
    return true;  // ヨーロッパ制限
  }
  if (defcon <= 3 && country.hasRegion(Region::ASIA)) {
    return true;  // アジア制限
  }
  if (defcon <= 2 && country.hasRegion(Region::MIDDLE_EAST)) {
    return true;  // 中東制限
  }

  return false;  // 制限なし
}

struct BonusCondition {
  /// 全配置がこの条件(例えば全部アジアにおいてる)を満たしていれば true
  std::function<bool(const std::map<CountryEnum, int>&)> isSatisfied;
};

static std::vector<std::pair<int, const BonusCondition*>> computeOpsVariants(
    CardEnum cardId, const Board& board, Side side) {
  const int base_ops =
      board.getCardpool()[static_cast<size_t>(cardId)]->getOps();

  // NOLINTNEXTLINE(readability-identifier-naming)
  static const BonusCondition asia_only{
      /* すべての国がアジア地域か？ */
      [&board](const std::map<CountryEnum, int>& placed) {
        return isAllInRegion<Region::ASIA>(placed, board);
      }};
  // NOLINTNEXTLINE(readability-identifier-naming)
  static const BonusCondition se_asia_only{
      [&board](const std::map<CountryEnum, int>& placed) {
        return isAllInRegion<Region::SOUTH_EAST_ASIA>(placed, board);
      }};

  std::vector<std::pair<int, const BonusCondition*>> res;
  /* ---- 基本 Ops は必ず存在 ---- */
  res.push_back({base_ops, nullptr});

  // /* ---- 中国カードの +1 ---- */
  // if (cardId == CardEnum::CHINA_CARD) res.push_back({base_ops + 1,
  // &asia_only});

  // /* ---- ベトナム蜂起が効いていれば +1 ---- */
  // if (board.isVietnamRevoltsActive(side))
  //   res.push_back({base_ops + 1, &se_asia_only});

  /* ---- ベトナム蜂起 + 中国カード → +2 ---- */

  return res;
}

/// その国に「影響力を +1」するのに必要な OP コストを返す
inline int costToAddOneInfluence(const WorldMap& worldMap,
                                 CountryEnum countryEnum, Side side) {
  Side opponent_side = getOpponentSide(side);

  // 相手が現在その国を支配しているか？
  const bool opponent_controls =
      worldMap.getCountry(countryEnum).getControlSide() == opponent_side;

  return opponent_controls ? 2 : 1;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void placeInfluenceDfs(int usedOps, size_t startIdx, WorldMap& tmpWorldMap,
                       std::map<CountryEnum, int>& placed,
                       std::vector<std::map<CountryEnum, int>>& out,
                       int totalOps,
                       const std::vector<CountryEnum>& placeableVec, Side side,
                       const BonusCondition* bonus) {
  if (usedOps == totalOps) {
    if (bonus == nullptr || bonus->isSatisfied(placed)) {
      out.emplace_back(placed);
    }
    return;
  }
  for (size_t i = startIdx; i < placeableVec.size(); ++i) {
    auto country_enum = placeableVec[i];
    int cost = costToAddOneInfluence(tmpWorldMap, country_enum, side);
    if (usedOps + cost > totalOps) {
      continue;
    }

    // 影響力を１追加して再帰
    placed[country_enum] += 1;
    tmpWorldMap.getCountry(country_enum)
        .addInfluence(side, 1);  // 軽量盤面を更新
    placeInfluenceDfs(usedOps + cost, i, tmpWorldMap, placed, out, totalOps,
                      placeableVec, side, bonus);
    tmpWorldMap.getCountry(country_enum)
        .removeInfluence(side, 1);  // バックトラック
    placed[country_enum] -= 1;
    if (placed[country_enum] == 0) {
      placed.erase(country_enum);
    }
  }
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::actionPlaceInfluenceLegalMoves(const Board& board,
                                                    Side side) {
  const auto& hands = board.getPlayerHand(side);
  if (hands.empty()) {
    return {};
  }

  const auto& world_map = board.getWorldMap();
  auto placeable = world_map.placeableCountries(side);
  if (placeable.empty()) [[unlikely]] {
    return {};
  }

  std::vector<CountryEnum> placeable_vec;
  placeable_vec.assign(placeable.begin(), placeable.end());

  /*----  Ops×Bonus ごとに DFS を 1 度だけ ----*/
  struct Key {
    int ops;
    const BonusCondition* bonus;
  };
  // bonusの比較は単にポインタを比較している
  // ポインタが同じならば同じボーナス条件とみなされるため、mapのinsertに失敗する
  // これにより同じボーナス条件がかぶるのを防ぐ
  auto cmp = [](Key key_a, Key key_b) {
    return std::tie(key_a.ops, key_a.bonus) < std::tie(key_b.ops, key_b.bonus);
  };
  std::map<Key, std::vector<std::map<CountryEnum, int>>, decltype(cmp)> cache(
      cmp);

  std::vector<std::unique_ptr<Move>> results;

  for (CardEnum card_enum : hands) {
    // Ops 0のカードは除外（スコアカード相当）
    const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];
    if (card->getOps() == 0) {
      continue;
    }

    for (auto [totalOps, bonus] : computeOpsVariants(card_enum, board, side)) {
      Key key{totalOps, bonus};

      if (!cache.contains(key)) {
        WorldMap tmp_world_map(world_map);
        std::map<CountryEnum, int> placed;
        cache[key] = {};

        placeInfluenceDfs(0, 0, tmp_world_map, placed, cache[key], totalOps,
                          placeable_vec, side, bonus);
      }
      results.reserve(results.size() + cache[key].size());
      for (const auto& pattern : cache[key]) {
        results.emplace_back(std::make_unique<ActionPlaceInfluenceMove>(
            card_enum, side, pattern));
      }
    }
  }
  return results;
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::actionRealignmentLegalMoves(const Board& board,
                                                 Side side) {
  const auto& hands = board.getPlayerHand(side);
  if (hands.empty()) {
    return {};
  }

  const auto& world_map = board.getWorldMap();
  std::vector<std::unique_ptr<Move>> results;

  // DEFCON値を取得
  int defcon = board.getDefconTrack().getDefcon();

  // 相手側を取得
  Side opponent_side = getOpponentSide(side);

  // 全ての国を調べて、相手の影響力がある国を対象とする
  // USA/USSRを除く84か国（インデックス2から85まで）
  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < world_map.getCountriesCount(); ++i) {
    auto country_enum = static_cast<CountryEnum>(i);

    const auto& country = world_map.getCountry(country_enum);

    // 相手の影響力が0の国は対象外
    if (country.getInfluence(opponent_side) == 0) {
      continue;
    }

    // DEFCON制限チェック
    if (isRegionRestrictedByDefcon(country, defcon)) {
      continue;
    }

    // 各手札から対象国へのRealignmentMoveを生成
    results.reserve(results.size() + hands.size());
    for (CardEnum card_enum : hands) {
      const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];
      if (card->getOps() == 0) {
        continue;  // Opsが0のカードは除外
      }
      results.emplace_back(std::make_unique<ActionRealigmentMove>(
          card_enum, side, country_enum));
    }
  }

  return results;
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::realignmentRequestLegalMoves(
    const Board& board, Side side, CardEnum cardEnum,
    const std::vector<CountryEnum>& history, int remainingOps,
    AdditionalOpsType appliedAdditionalOps) {
  const auto& world_map = board.getWorldMap();

  std::vector<std::unique_ptr<Move>> results;
  // 事前に容量を確保（最大で国数+1個のMOVEが生成される）
  results.reserve(world_map.getCountriesCount());

  // DEFCON値を取得
  int defcon = board.getDefconTrack().getDefcon();

  // 相手側を取得
  Side opponent_side = getOpponentSide(side);

  // 全ての国を調べて、相手の影響力がある国を対象とする
  // USA/USSRを除く84か国（インデックス2から85まで）
  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < world_map.getCountriesCount(); ++i) {
    auto country_enum = static_cast<CountryEnum>(i);

    const auto& country = world_map.getCountry(country_enum);

    // 相手の影響力が0の国は対象外
    if (country.getInfluence(opponent_side) == 0) {
      continue;
    }

    // DEFCON制限チェック
    if (isRegionRestrictedByDefcon(country, defcon)) {
      continue;
    }

    results.emplace_back(std::make_unique<RealignmentRequestMove>(
        cardEnum, side, country_enum, history, remainingOps,
        appliedAdditionalOps));
  }
  // RealignRequestMoveではUSSR=パスも選択可能
  if (!results.empty()) {
    results.emplace_back(std::make_unique<RealignmentRequestMove>(
        cardEnum, side, CountryEnum::USSR, history, remainingOps,
        appliedAdditionalOps));
  }

  return results;
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
    const Board& board, Side side, CardEnum cardEnum,
    const std::vector<CountryEnum>& history,
    AdditionalOpsType appliedAdditionalOps) {
  std::vector<std::unique_ptr<Move>> results;
  const auto& world_map = board.getWorldMap();
  Side opponent_side = getOpponentSide(side);

  // DEFCON値を取得
  int defcon = board.getDefconTrack().getDefcon();

  // 中国カードの追加Opsチェック
  bool china_card_bonus = false;
  if ((static_cast<uint8_t>(appliedAdditionalOps) &
       static_cast<uint8_t>(AdditionalOpsType::CHINA_CARD)) != 0) {
    // 既に中国カードボーナスが適用されている
  } else {
    // TODO: カードが中国カードかどうかの判定が必要
    // if (cardEnum == CHINA_CARD && isAllInRegion<Region::ASIA>(history,
    // board)) {
    //   chinaCardBonus = true;
    // }
  }

  // ベトナム蜂起の追加Opsチェック
  bool vietnam_revolts_bonus = false;
  if ((static_cast<uint8_t>(appliedAdditionalOps) &
       static_cast<uint8_t>(AdditionalOpsType::VIETNAM_REVOLTS)) != 0) {
    // 既にベトナム蜂起ボーナスが適用されている
  } else {
    // TODO: ベトナム蜂起が有効かどうかの判定が必要
    // if (board.isVietnamRevoltsActive() &&
    // isAllInRegion<Region::SOUTH_EAST_ASIA>(history, board)) {
    //   vietnamRevoltsBonus = true;
    // }
  }

  // 中国カードボーナスの処理
  if (china_card_bonus) {
    auto new_applied_ops = static_cast<AdditionalOpsType>(
        static_cast<uint8_t>(appliedAdditionalOps) |
        static_cast<uint8_t>(AdditionalOpsType::CHINA_CARD));

    // アジア地域限定の合法手を生成
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < world_map.getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      const auto& country = world_map.getCountry(country_enum);

      if (country.getInfluence(opponent_side) == 0) {
        continue;
      }
      if (!country.hasRegion(Region::ASIA)) {
        continue;
      }

      // DEFCON制限チェック
      if (isRegionRestrictedByDefcon(country, defcon)) {
        continue;
      }

      results.emplace_back(std::make_unique<RealignmentRequestMove>(
          cardEnum, side, country_enum, history, 1, new_applied_ops));
    }
  }

  // ベトナム蜂起ボーナスの処理
  if (vietnam_revolts_bonus) {
    auto new_applied_ops = static_cast<AdditionalOpsType>(
        static_cast<uint8_t>(appliedAdditionalOps) |
        static_cast<uint8_t>(AdditionalOpsType::VIETNAM_REVOLTS));

    // 東南アジア地域限定の合法手を生成
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < world_map.getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      const auto& country = world_map.getCountry(country_enum);

      if (country.getInfluence(opponent_side) == 0) {
        continue;
      }
      if (!country.hasRegion(Region::SOUTH_EAST_ASIA)) {
        continue;
      }

      // DEFCON制限チェック
      if (isRegionRestrictedByDefcon(country, defcon)) {
        continue;
      }

      results.emplace_back(std::make_unique<RealignmentRequestMove>(
          cardEnum, side, country_enum, history, 1, new_applied_ops));
    }
  }

  if (!results.empty()) {
    results.emplace_back(std::make_unique<RealignmentRequestMove>(
        cardEnum, side, CountryEnum::USSR, history, 1, appliedAdditionalOps));
  }

  return results;
}

std::vector<std::unique_ptr<Move>> LegalMovesGenerator::actionCoupLegalMoves(
    const Board& board, Side side) {
  const auto& hands = board.getPlayerHand(side);
  if (hands.empty()) {
    return {};
  }

  const auto& world_map = board.getWorldMap();
  std::vector<std::unique_ptr<Move>> results;

  // DEFCON値を取得
  int defcon = board.getDefconTrack().getDefcon();

  // 相手側を取得
  Side opponent_side = getOpponentSide(side);

  // 全ての国を調べて、相手の影響力がある国を対象とする
  // USA/USSRを除く84か国（インデックス2から85まで）
  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < world_map.getCountriesCount(); ++i) {
    auto country_enum = static_cast<CountryEnum>(i);

    const auto& country = world_map.getCountry(country_enum);

    // 相手の影響力が0の国は対象外
    if (country.getInfluence(opponent_side) == 0) {
      continue;
    }

    // DEFCON制限チェック
    if (isRegionRestrictedByDefcon(country, defcon)) {
      continue;
    }

    // 各手札から対象国へのCoupMoveを生成
    results.reserve(results.size() + hands.size());
    for (CardEnum card_enum : hands) {
      const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];
      if (card->getOps() == 0) {
        continue;  // Opsが0のカードは除外
      }
      results.emplace_back(
          std::make_unique<ActionCoupMove>(card_enum, side, country_enum));
    }
  }

  return results;
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::actionSpaceRaceLegalMoves(const Board& board, Side side) {
  const auto& hands = board.getPlayerHand(side);
  if (hands.empty()) {
    return {};
  }

  std::vector<std::unique_ptr<Move>> results;
  results.reserve(hands.size());

  for (CardEnum card_enum : hands) {
    const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];

    // スコアリングカード除外（Ops=0）
    if (card->getOps() == 0) {
      continue;
    }

    // canSpaceチェック（試行回数・位置8チェック込み）
    if (board.getSpaceTrack().canSpace(side, card->getOps())) {
      results.emplace_back(
          std::make_unique<ActionSpaceRaceMove>(card_enum, side));
    }
  }

  return results;
}

std::vector<std::unique_ptr<Move>> LegalMovesGenerator::actionEventLegalMoves(
    const Board& board, Side side) {
  const auto& hands = board.getPlayerHand(side);
  if (hands.empty()) {
    return {};
  }

  std::vector<std::unique_ptr<Move>> results;
  results.reserve(hands.size());

  for (CardEnum card_enum : hands) {
    const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];

    // canEventチェック（中国カードは自動的にfalseで除外される）
    if (card->canEvent(board)) {
      results.emplace_back(std::make_unique<ActionEventMove>(card_enum, side));
    }
  }

  return results;
}

std::vector<std::unique_ptr<Move>>
LegalMovesGenerator::headlineCardSelectLegalMoves(const Board& board,
                                                  Side side) {
  std::vector<std::unique_ptr<Move>> legal_moves;
  const auto& hand = board.getPlayerHand(side);
  const auto& cardpool = board.getCardpool();

  legal_moves.reserve(hand.size());
  // 手札から選択可能なカードを追加
  for (const auto& card_enum : hand) {
    // TODO: UN Interventionを除外する処理を追加
    // なお発動条件を満たさないカードを選択することは可能
    legal_moves.emplace_back(
        std::make_unique<HeadlineCardSelectMove>(card_enum, side));
  }

  return legal_moves;
}

std::vector<std::unique_ptr<Move>> LegalMovesGenerator::arLegalMoves(
    const Board& board, Side side) {
  std::vector<std::unique_ptr<Move>> legal_moves;

  auto place_influence_moves = actionPlaceInfluenceLegalMoves(board, side);
  auto realignment_moves = actionRealignmentLegalMoves(board, side);
  auto coup_moves = actionCoupLegalMoves(board, side);
  auto space_race = actionSpaceRaceLegalMoves(board, side);
  auto event_moves = actionEventLegalMoves(board, side);

  legal_moves.reserve(place_influence_moves.size() + realignment_moves.size() +
                      coup_moves.size() + space_race.size() +
                      event_moves.size());
  // 各アクションの合法手をlegal_movesに追加
  for (auto& move : place_influence_moves) {
    legal_moves.push_back(std::move(move));
  }
  for (auto& move : realignment_moves) {
    legal_moves.push_back(std::move(move));
  }
  for (auto& move : coup_moves) {
    legal_moves.push_back(std::move(move));
  }
  for (auto& move : space_race) {
    legal_moves.push_back(std::move(move));
  }
  for (auto& move : event_moves) {
    legal_moves.push_back(std::move(move));
  }

  return legal_moves;
}