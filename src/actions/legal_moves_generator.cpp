#include "tsge/actions/legal_moves_generator.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "tsge/actions/move.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/world_map.hpp"

namespace {

std::vector<CardEnum> gatherOpsPlayableCards(const Board& board, Side side) {
  const auto& hand = board.getPlayerHand(side);
  std::vector<CardEnum> cards;
  cards.reserve(hand.size() + 1);
  cards.insert(cards.end(), hand.begin(), hand.end());
  if (board.isChinaCardAvailableFor(side)) {
    cards.push_back(CardEnum::CHINA_CARD);
  }
  return cards;
}

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
bool isAllInRegion(const Container& countries, const Board& board) {
  return std::ranges::all_of(countries, [&](const auto& elem) {
    const auto country =
        CountryExtractor<std::decay_t<decltype(elem)>>::extract(elem);
    const auto& country_obj = board.getWorldMap().getCountry(country);
    return country_obj.hasRegion(region);
  });
}

// DEFCON制限により特定の国がRealignmentできないかを判定
bool isRegionRestrictedByDefcon(const Country& country, int defcon) {
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

struct OpponentCountryFilter {
  /// 地域条件（例: アジア限定）を課す場合に設定する
  std::optional<Region> requiredRegion;
  /// 局所的な絞り込み（追加Ops要件など）が必要な場合に設定する(将来用)
  // std::function<bool(const Country&)> additionalCondition;
};

std::vector<CountryEnum> collectOpponentInfluencedCountries(
    const Board& board, Side side, const OpponentCountryFilter& filter = {}) {
  const auto& world_map = board.getWorldMap();
  Side opponent_side = getOpponentSide(side);
  int defcon = board.getDefconTrack().getDefcon();

  std::vector<CountryEnum> candidates;
  candidates.reserve(world_map.getCountriesCount());

  // 全ての国を調べて、相手の影響力がある国を対象とする
  // USA/USSRを除く84か国（インデックス2から85まで）
  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < world_map.getCountriesCount(); ++i) {
    auto country_enum = static_cast<CountryEnum>(i);
    const auto& country = world_map.getCountry(country_enum);

    if (country.getInfluence(opponent_side) == 0) {
      continue;  // 相手影響力なし
    }
    if (isRegionRestrictedByDefcon(country, defcon)) {
      continue;  // DEFCONによる地域制限
    }
    if (filter.requiredRegion.has_value() &&
        !country.hasRegion(*filter.requiredRegion)) {
      continue;  // 地域条件を満たさない
    }
    // if (filter.additionalCondition != nullptr &&
    //     !filter.additionalCondition(country)) {
    //   continue;  // カスタム条件不一致
    // }

    candidates.push_back(country_enum);
  }

  return candidates;
}

struct BonusCondition {
  /// 全配置がこの条件(例えば全部アジアにおいてる)を満たしていれば true
  std::function<bool(const std::map<CountryEnum, int>&)> isSatisfied;
};

std::vector<std::pair<int, const BonusCondition*>> computeOpsVariants(
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
  // NOLINTNEXTLINE(readability-identifier-naming)
  static const BonusCondition not_asia_only{
      [&board](const std::map<CountryEnum, int>& placed) {
        return !isAllInRegion<Region::ASIA>(placed, board);
      }};
  // NOLINTNEXTLINE(readability-identifier-naming)
  static const BonusCondition asia_only_without_se_asia{
      [&board](const std::map<CountryEnum, int>& placed) {
        if (!isAllInRegion<Region::ASIA>(placed, board)) {
          return false;
        }
        return !isAllInRegion<Region::SOUTH_EAST_ASIA>(placed, board);
      }};

  const bool is_china_card = cardId == CardEnum::CHINA_CARD;
  const auto& effect_of_side = board.getCardsEffectsInThisTurn(side);
  const bool vietnam_revolts_active =
      std::find(effect_of_side.begin(), effect_of_side.end(),
                CardEnum::VIETNAM_REVOLTS) != effect_of_side.end();

  std::vector<std::pair<int, const BonusCondition*>> res;
  /* ---- 基本 Ops は必ず存在 ---- */
  res.emplace_back(base_ops, is_china_card ? &not_asia_only : nullptr);

  /* ---- 中国カードの +1 ---- */
  if (is_china_card) {
    if (vietnam_revolts_active) {
      res.emplace_back(base_ops + 1, &asia_only_without_se_asia);
      res.emplace_back(base_ops + 2, &se_asia_only);
    } else {
      res.emplace_back(base_ops + 1, &asia_only);
    }
  }

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

struct PlaceInfluenceCacheKey {
  int ops;
  const BonusCondition* bonus;
};

struct PlaceInfluenceCacheComparator {
  bool operator()(const PlaceInfluenceCacheKey& lhs,
                  const PlaceInfluenceCacheKey& rhs) const {
    return std::tie(lhs.ops, lhs.bonus) < std::tie(rhs.ops, rhs.bonus);
  }
};

using PlaceInfluenceCache =
    std::map<PlaceInfluenceCacheKey, std::vector<std::map<CountryEnum, int>>,
             PlaceInfluenceCacheComparator>;

void placeInfluenceMovesForCard(const Board& board, Side side,
                                CardEnum cardEnum,
                                const std::vector<CountryEnum>& placeableVec,
                                PlaceInfluenceCache& cache,
                                std::vector<std::shared_ptr<Move>>& out) {
  const auto& card = board.getCardpool()[static_cast<size_t>(cardEnum)];
  if (card->getOps() == 0) {
    return;
  }

  for (auto [totalOps, bonus] : computeOpsVariants(cardEnum, board, side)) {
    PlaceInfluenceCacheKey key{totalOps, bonus};
    auto [iter, inserted] = cache.try_emplace(key);
    if (inserted) {
      WorldMap tmp_world_map(board.getWorldMap());
      std::map<CountryEnum, int> placed;
      placeInfluenceDfs(0, 0, tmp_world_map, placed, iter->second, totalOps,
                        placeableVec, side, bonus);
    }

    const auto& place_patterns = iter->second;
    out.reserve(out.size() + place_patterns.size());
    for (const auto& pattern : place_patterns) {
      out.emplace_back(
          std::make_shared<ActionPlaceInfluenceMove>(cardEnum, side, pattern));
    }
  }
}

std::vector<std::shared_ptr<Move>> generatePlaceInfluenceMoves(
    const Board& board, Side side, const std::vector<CardEnum>& cards) {
  if (cards.empty()) {
    return {};
  }

  const auto& world_map = board.getWorldMap();
  auto placeable = world_map.placeableCountries(side);
  if (placeable.empty()) [[unlikely]] {
    return {};
  }

  std::vector<CountryEnum> placeable_vec;
  placeable_vec.assign(placeable.begin(), placeable.end());

  PlaceInfluenceCache cache;
  std::vector<std::shared_ptr<Move>> results;
  for (CardEnum card_enum : cards) {
    placeInfluenceMovesForCard(board, side, card_enum, placeable_vec, cache,
                               results);
  }
  return results;
}

}  // namespace

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::actionPlaceInfluenceLegalMoves(const Board& board,
                                                    Side side) {
  auto cards = gatherOpsPlayableCards(board, side);
  return generatePlaceInfluenceMoves(board, side, cards);
}

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::actionPlaceInfluenceLegalMovesForCard(const Board& board,
                                                           Side side,
                                                           CardEnum cardEnum) {
  std::vector<CardEnum> single_card = {cardEnum};
  return generatePlaceInfluenceMoves(board, side, single_card);
}

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::actionRealignmentLegalMoves(const Board& board,
                                                 Side side) {
  auto cards = gatherOpsPlayableCards(board, side);
  if (cards.empty()) {
    return {};
  }

  auto target_countries = collectOpponentInfluencedCountries(board, side);
  if (target_countries.empty()) {
    return {};
  }

  size_t playable_cards =
      std::count_if(cards.begin(), cards.end(), [&](auto card_enum) {
        const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];
        return card->getOps() > 0;
      });
  if (playable_cards == 0) {
    return {};
  }

  std::vector<std::shared_ptr<Move>> results;
  results.reserve(playable_cards * target_countries.size());

  for (CardEnum card_enum : cards) {
    const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];
    if (card->getOps() == 0) {
      continue;
    }
    for (auto country_enum : target_countries) {
      results.emplace_back(std::make_shared<ActionRealigmentMove>(
          card_enum, side, country_enum));
    }
  }

  return results;
}

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::actionRealignmentLegalMovesForCard(const Board& board,
                                                        Side side,
                                                        CardEnum cardEnum) {
  auto target_countries = collectOpponentInfluencedCountries(board, side);
  if (target_countries.empty()) {
    return {};
  }

  const auto& card = board.getCardpool()[static_cast<size_t>(cardEnum)];
  if (card->getOps() == 0) {
    return {};
  }

  std::vector<std::shared_ptr<Move>> results;
  results.reserve(target_countries.size());
  for (auto country_enum : target_countries) {
    results.emplace_back(
        std::make_shared<ActionRealigmentMove>(cardEnum, side, country_enum));
  }

  return results;
}

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::realignmentRequestLegalMoves(
    const Board& board, Side side, CardEnum cardEnum,
    const std::vector<CountryEnum>& history, int remainingOps,
    AdditionalOpsType appliedAdditionalOps) {
  auto target_countries = collectOpponentInfluencedCountries(board, side);
  if (target_countries.empty()) {
    return {};
  }

  std::vector<std::shared_ptr<Move>> results;
  results.reserve(target_countries.size() + 1);

  for (auto country_enum : target_countries) {
    results.emplace_back(std::make_shared<RealignmentRequestMove>(
        cardEnum, side, country_enum, history, remainingOps,
        appliedAdditionalOps));
  }

  // RealignRequestMoveではUSSR=パスも選択可能
  results.emplace_back(std::make_shared<RealignmentRequestMove>(
      cardEnum, side, CountryEnum::USSR, history, remainingOps,
      appliedAdditionalOps));

  return results;
}

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::additionalOpsRealignmentLegalMoves(
    const Board& board, Side side, CardEnum cardEnum,
    const std::vector<CountryEnum>& history,
    AdditionalOpsType appliedAdditionalOps) {
  std::vector<std::shared_ptr<Move>> results;

  // 中国カードの追加Opsチェック
  bool china_card_bonus = false;
  if ((static_cast<uint8_t>(appliedAdditionalOps) &
       static_cast<uint8_t>(AdditionalOpsType::CHINA_CARD)) != 0) {
    // 既に中国カードボーナスが適用されている
  } else if (cardEnum == CardEnum::CHINA_CARD && !history.empty() &&
             isAllInRegion<Region::ASIA>(history, board)) {
    china_card_bonus = true;
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
    OpponentCountryFilter filter;
    filter.requiredRegion = Region::ASIA;
    auto asia_targets = collectOpponentInfluencedCountries(board, side, filter);
    results.reserve(results.size() + asia_targets.size());
    for (auto country_enum : asia_targets) {
      results.emplace_back(std::make_shared<RealignmentRequestMove>(
          cardEnum, side, country_enum, history, 1, new_applied_ops));
    }
  }

  // ベトナム蜂起ボーナスの処理
  if (vietnam_revolts_bonus) {
    auto new_applied_ops = static_cast<AdditionalOpsType>(
        static_cast<uint8_t>(appliedAdditionalOps) |
        static_cast<uint8_t>(AdditionalOpsType::VIETNAM_REVOLTS));

    // 東南アジア地域限定の合法手を生成
    OpponentCountryFilter filter;
    filter.requiredRegion = Region::SOUTH_EAST_ASIA;
    auto se_asia_targets =
        collectOpponentInfluencedCountries(board, side, filter);
    results.reserve(results.size() + se_asia_targets.size());
    for (auto country_enum : se_asia_targets) {
      results.emplace_back(std::make_shared<RealignmentRequestMove>(
          cardEnum, side, country_enum, history, 1, new_applied_ops));
    }
  }

  if (!results.empty()) {
    results.emplace_back(std::make_shared<RealignmentRequestMove>(
        cardEnum, side, CountryEnum::USSR, history, 1, appliedAdditionalOps));
  }

  return results;
}

std::vector<std::shared_ptr<Move>> LegalMovesGenerator::actionCoupLegalMoves(
    const Board& board, Side side) {
  auto cards = gatherOpsPlayableCards(board, side);
  if (cards.empty()) {
    return {};
  }

  auto target_countries = collectOpponentInfluencedCountries(board, side);
  if (target_countries.empty()) {
    return {};
  }

  size_t playable_cards =
      std::count_if(cards.begin(), cards.end(), [&](auto card_enum) {
        const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];
        return card->getOps() > 0;
      });
  if (playable_cards == 0) {
    return {};
  }

  std::vector<std::shared_ptr<Move>> results;
  results.reserve(playable_cards * target_countries.size());

  for (CardEnum card_enum : cards) {
    const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];
    if (card->getOps() == 0) {
      continue;
    }
    for (auto country_enum : target_countries) {
      results.emplace_back(
          std::make_shared<ActionCoupMove>(card_enum, side, country_enum));
    }
  }

  return results;
}

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::actionCoupLegalMovesForCard(const Board& board, Side side,
                                                 CardEnum cardEnum) {
  auto target_countries = collectOpponentInfluencedCountries(board, side);
  if (target_countries.empty()) {
    return {};
  }

  const auto& card = board.getCardpool()[static_cast<size_t>(cardEnum)];
  if (card->getOps() == 0) {
    return {};
  }

  std::vector<std::shared_ptr<Move>> results;
  results.reserve(target_countries.size());
  for (auto country_enum : target_countries) {
    results.emplace_back(
        std::make_shared<ActionCoupMove>(cardEnum, side, country_enum));
  }

  return results;
}

std::vector<std::shared_ptr<Move>> LegalMovesGenerator::actionLegalMovesForCard(
    const Board& board, Side side, CardEnum cardEnum) {
  auto place_moves =
      actionPlaceInfluenceLegalMovesForCard(board, side, cardEnum);
  auto realign_moves =
      actionRealignmentLegalMovesForCard(board, side, cardEnum);
  auto coup_moves = actionCoupLegalMovesForCard(board, side, cardEnum);

  std::vector<std::shared_ptr<Move>> ops_moves;
  ops_moves.reserve(place_moves.size() + realign_moves.size() +
                    coup_moves.size());

  auto append_moves = [&ops_moves](std::vector<std::shared_ptr<Move>>& moves) {
    for (auto& move : moves) {
      ops_moves.emplace_back(std::move(move));
    }
  };

  append_moves(place_moves);
  append_moves(realign_moves);
  append_moves(coup_moves);

  return ops_moves;
}

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::actionSpaceRaceLegalMoves(const Board& board, Side side) {
  auto cards = gatherOpsPlayableCards(board, side);
  if (cards.empty()) {
    return {};
  }

  std::vector<std::shared_ptr<Move>> results;
  results.reserve(cards.size());

  for (CardEnum card_enum : cards) {
    const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];

    // スコアリングカード除外（Ops=0）
    if (card->getOps() == 0) {
      continue;
    }

    // canSpaceチェック（試行回数・位置8チェック込み）
    if (board.getSpaceTrack().canSpace(side, card->getOps())) {
      results.emplace_back(
          std::make_shared<ActionSpaceRaceMove>(card_enum, side));
    }
  }

  return results;
}

std::vector<std::shared_ptr<Move>> LegalMovesGenerator::actionEventLegalMoves(
    const Board& board, Side side) {
  const auto& hands = board.getPlayerHand(side);
  if (hands.empty()) {
    return {};
  }

  std::vector<std::shared_ptr<Move>> results;
  results.reserve(hands.size());

  for (CardEnum card_enum : hands) {
    const auto& card = board.getCardpool()[static_cast<size_t>(card_enum)];

    // canEventチェック（中国カードは自動的にfalseで除外される）
    if (card->canEvent(board)) {
      results.emplace_back(std::make_shared<ActionEventMove>(card_enum, side));
    }
  }

  return results;
}

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::headlineCardSelectLegalMoves(const Board& board,
                                                  Side side) {
  std::vector<std::shared_ptr<Move>> legal_moves;
  const auto& hand = board.getPlayerHand(side);
  const auto& cardpool = board.getCardpool();

  legal_moves.reserve(hand.size());
  // 手札から選択可能なカードを追加
  for (const auto& card_enum : hand) {
    // TODO: UN Interventionを除外する処理を追加
    // なお発動条件を満たさないカードを選択することは可能
    legal_moves.emplace_back(
        std::make_shared<HeadlineCardSelectMove>(card_enum, side));
  }

  return legal_moves;
}

std::vector<std::shared_ptr<Move>> LegalMovesGenerator::arLegalMoves(
    const Board& board, Side side) {
  std::vector<std::shared_ptr<Move>> legal_moves;

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

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::extraActionRoundLegalMoves(const Board& board, Side side) {
  auto legal_moves = arLegalMoves(board, side);
  legal_moves.emplace_back(std::make_shared<PassMove>(side));
  return legal_moves;
}

std::vector<std::shared_ptr<Move>>
LegalMovesGenerator::spaceTrackDiscardLegalMoves(const Board& board,
                                                 Side side) {
  const auto& hand = board.getPlayerHand(side);
  if (hand.empty()) {
    return {};
  }

  std::vector<std::shared_ptr<Move>> moves;
  moves.reserve(hand.size() + 1);
  for (const auto card_enum : hand) {
    moves.emplace_back(std::make_shared<DiscardMove>(card_enum, side));
  }
  moves.emplace_back(std::make_shared<PassMove>(side));
  return moves;
}
