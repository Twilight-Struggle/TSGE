// どこで: src/actions/card_effect_legal_move_generator.cpp
// 何を: カードイベント専用の合法手生成ロジックとレジストリを実装
// なぜ: ゲーム進行とカード効果の責務を分離し、拡張を容易にするため
#include "tsge/actions/card_effect_legal_move_generator.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "tsge/actions/move.hpp"
#include "tsge/game_state/world_map.hpp"

void registerDeStalinizationCardEffectGenerator();

namespace {

struct CardEffectRegistry {
  // ゲームは逐次実行モデルのため排他制御は不要。単純なマップで十分。
  std::map<CardEnum, CardEffectLegalMoveGenerator::GeneratorFunction>
      generators;
};

CardEffectRegistry& registry() {
  static CardEffectRegistry instance;
  return instance;
}

bool& builtinGeneratorsInitialized() {
  static bool initialized = false;
  return initialized;
}

std::vector<CountryEnum> collectCardSpecialPlaceInfluenceCandidates(
    const WorldMap& world_map, Side side,
    const CardSpecialPlaceInfluenceConfig& config) {
  std::vector<CountryEnum> candidates;
  candidates.reserve(world_map.getCountriesCount());

  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < world_map.getCountriesCount(); ++i) {
    auto country_enum = static_cast<CountryEnum>(i);
    const auto& country = world_map.getCountry(country_enum);

    if (config.allowedRegions.has_value() &&
        !std::ranges::any_of(*config.allowedRegions, [&country](Region region) {
          return country.hasRegion(region);
        })) {
      continue;
    }

    if (config.excludeOpponentControlled &&
        country.getControlSide() == getOpponentSide(side)) {
      continue;
    }

    if (config.onlyEmptyCountries && (country.getInfluence(Side::USA) > 0 ||
                                      country.getInfluence(Side::USSR) > 0)) {
      continue;
    }

    candidates.push_back(country_enum);
  }

  return candidates;
}

void generateCardSpecificPlacementPatternsDfs(
    const std::vector<CountryEnum>& candidates,
    const CardSpecialPlaceInfluenceConfig& config, size_t idx, int remaining,
    std::map<CountryEnum, int>& current,
    std::vector<std::map<CountryEnum, int>>& patterns) {
  if (remaining == 0) {
    if (!current.empty()) {
      patterns.push_back(current);
    }
    return;
  }

  if (idx >= candidates.size()) {
    return;
  }

  generateCardSpecificPlacementPatternsDfs(candidates, config, idx + 1,
                                           remaining, current, patterns);

  auto country = candidates[idx];
  int max_place = config.maxPerCountry > 0
                      ? std::min(config.maxPerCountry, remaining)
                      : remaining;

  for (int amount = 1; amount <= max_place; ++amount) {
    current[country] = amount;
    generateCardSpecificPlacementPatternsDfs(
        candidates, config, idx + 1, remaining - amount, current, patterns);
    current.erase(country);
  }
}

std::vector<std::map<CountryEnum, int>>
generateCardSpecificInfluencePlacementPatterns(
    const std::vector<CountryEnum>& candidates,
    const CardSpecialPlaceInfluenceConfig& config, int actualTotalInfluence) {
  std::vector<std::map<CountryEnum, int>> patterns;
  std::map<CountryEnum, int> current_pattern;
  generateCardSpecificPlacementPatternsDfs(
      candidates, config, 0, actualTotalInfluence, current_pattern, patterns);
  return patterns;
}

std::vector<CountryEnum> collectRemoveInfluenceCandidates(
    const WorldMap& world_map, Side targetSide,
    const std::optional<std::vector<Region>>& allowedRegions,
    const std::optional<std::vector<CountryEnum>>& specificCountries) {
  std::vector<CountryEnum> candidates;

  if (specificCountries.has_value()) {
    for (const auto& country_enum : *specificCountries) {
      const auto& country = world_map.getCountry(country_enum);
      if (country.getInfluence(targetSide) > 0) {
        candidates.push_back(country_enum);
      }
    }
    return candidates;
  }

  candidates.reserve(world_map.getCountriesCount());
  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < world_map.getCountriesCount(); ++i) {
    auto country_enum = static_cast<CountryEnum>(i);
    const auto& country = world_map.getCountry(country_enum);

    if (country.getInfluence(targetSide) == 0) {
      continue;
    }

    if (allowedRegions.has_value() &&
        !std::ranges::any_of(*allowedRegions, [&country](Region region) {
          return country.hasRegion(region);
        })) {
      continue;
    }

    candidates.push_back(country_enum);
  }

  return candidates;
}

void generateRemovePatternsDfs(
    const std::vector<CountryEnum>& candidates, const WorldMap& world_map,
    Side targetSide, int maxPerCountry, size_t idx, int remaining,
    std::map<CountryEnum, int>& current,
    std::vector<std::map<CountryEnum, int>>& patterns) {
  if (remaining == 0) {
    if (!current.empty()) {
      patterns.push_back(current);
    }
    return;
  }

  if (idx >= candidates.size()) {
    return;
  }

  generateRemovePatternsDfs(candidates, world_map, targetSide, maxPerCountry,
                            idx + 1, remaining, current, patterns);

  auto country = candidates[idx];
  int available = world_map.getCountry(country).getInfluence(targetSide);
  int max_remove =
      maxPerCountry > 0 ? std::min(maxPerCountry, remaining) : remaining;
  max_remove = std::min(max_remove, available);

  for (int amount = 1; amount <= max_remove; ++amount) {
    current[country] = amount;
    generateRemovePatternsDfs(candidates, world_map, targetSide, maxPerCountry,
                              idx + 1, remaining - amount, current, patterns);
    current.erase(country);
  }
}

}  // namespace

void CardEffectLegalMoveGenerator::registerGenerator(
    CardEnum cardEnum, GeneratorFunction generator) {
  registry().generators[cardEnum] = std::move(generator);
}

void CardEffectLegalMoveGenerator::initializeBuiltinGenerators() {
  auto& initialized = builtinGeneratorsInitialized();
  if (initialized) {
    return;
  }
  registerDeStalinizationCardEffectGenerator();
  initialized = true;
}

#ifdef TEST
void CardEffectLegalMoveGenerator::resetBuiltinGeneratorsForTest() {
  registry().generators.clear();
  builtinGeneratorsInitialized() = false;
}
#endif

std::vector<std::shared_ptr<Move>> CardEffectLegalMoveGenerator::generate(
    CardEnum cardEnum, const Board& board, Side side) {
  auto& reg = registry();
  auto iter = reg.generators.find(cardEnum);
  if (iter == reg.generators.end()) {
    return {};
  }
  auto generator = iter->second;
  return generator(board, side);
}

std::vector<std::shared_ptr<Move>>
CardEffectLegalMoveGenerator::generateCardSpecificPlaceInfluenceMoves(
    const Board& board, Side side, CardEnum cardEnum,
    const CardSpecialPlaceInfluenceConfig& config) {
  auto candidates = collectCardSpecialPlaceInfluenceCandidates(
      board.getWorldMap(), side, config);
  if (candidates.empty()) {
    return {};
  }

  int max_placeable_influence = 0;
  if (config.maxPerCountry > 0) {
    max_placeable_influence =
        static_cast<int>(candidates.size()) * config.maxPerCountry;
  } else {
    max_placeable_influence = config.totalInfluence;
  }

  int actual_total_influence =
      std::min(config.totalInfluence, max_placeable_influence);

  if (actual_total_influence == 0) {
    return {};
  }

  auto patterns = generateCardSpecificInfluencePlacementPatterns(
      candidates, config, actual_total_influence);

  std::vector<std::shared_ptr<Move>> results;
  results.reserve(patterns.size());
  for (const auto& pattern : patterns) {
    results.emplace_back(
        std::make_shared<EventPlaceInfluenceMove>(cardEnum, side, pattern));
  }

  return results;
}

std::vector<std::map<CountryEnum, int>>
CardEffectLegalMoveGenerator::enumerateRemoveInfluencePatterns(
    const Board& board, Side targetSide, int totalRemove, int maxPerCountry,
    const std::optional<std::vector<Region>>& allowedRegions,
    const std::optional<std::vector<CountryEnum>>& specificCountries,
    RemovalSaturationStrategy saturation) {
  if (totalRemove <= 0) {
    return {};
  }

  auto candidates = collectRemoveInfluenceCandidates(
      board.getWorldMap(), targetSide, allowedRegions, specificCountries);
  if (candidates.empty()) {
    return {};
  }

  const auto& world_map = board.getWorldMap();
  int max_removable = 0;
  for (const auto& country_enum : candidates) {
    int available = world_map.getCountry(country_enum).getInfluence(targetSide);
    int removable_from_country =
        (maxPerCountry > 0) ? std::min(available, maxPerCountry) : available;
    max_removable += removable_from_country;
    if (max_removable >= totalRemove) {
      break;
    }
  }

  if (max_removable == 0) {
    return {};
  }

  if (saturation == RemovalSaturationStrategy::kRequireExact &&
      totalRemove > max_removable) {
    return {};
  }

  int actual_remove = saturation == RemovalSaturationStrategy::kAllowPartial
                          ? std::min(totalRemove, max_removable)
                          : totalRemove;

  std::vector<std::map<CountryEnum, int>> patterns;
  std::map<CountryEnum, int> current;
  generateRemovePatternsDfs(candidates, world_map, targetSide, maxPerCountry, 0,
                            actual_remove, current, patterns);
  return patterns;
}

std::vector<std::shared_ptr<Move>>
CardEffectLegalMoveGenerator::generateRemoveInfluenceMoves(
    const Board& board, CardEnum cardEnum, Side moveSide, Side targetSide,
    int totalRemove, int maxPerCountry,
    const std::optional<std::vector<Region>>& allowedRegions,
    const std::optional<std::vector<CountryEnum>>& specificCountries) {
  auto patterns = enumerateRemoveInfluencePatterns(
      board, targetSide, totalRemove, maxPerCountry, allowedRegions,
      specificCountries, RemovalSaturationStrategy::kAllowPartial);
  if (patterns.empty()) {
    return {};
  }

  std::vector<std::shared_ptr<Move>> results;
  results.reserve(patterns.size());
  for (const auto& pattern : patterns) {
    results.emplace_back(std::make_shared<EventRemoveInfluenceMove>(
        cardEnum, moveSide, pattern));
  }
  return results;
}

std::vector<std::shared_ptr<Move>>
CardEffectLegalMoveGenerator::generateSelectCountriesRemoveInfluenceMoves(
    const Board& board, CardEnum cardEnum, Side moveSide, Side targetSide,
    Region region, int countriesToSelect, int removePerCountry) {
  std::vector<CountryEnum> candidates;
  const auto& world_map = board.getWorldMap();

  for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
       i < world_map.getCountriesCount(); ++i) {
    auto country_enum = static_cast<CountryEnum>(i);
    const auto& country = world_map.getCountry(country_enum);
    if (country.getInfluence(targetSide) == 0) {
      continue;
    }

    if (!country.hasRegion(region)) {
      continue;
    }

    candidates.push_back(country_enum);
  }

  if (candidates.empty()) {
    return {};
  }
  int actual_countries_to_select =
      std::min(countriesToSelect, static_cast<int>(candidates.size()));

  std::vector<std::shared_ptr<Move>> results;
  std::vector<CountryEnum> selected;
  selected.reserve(actual_countries_to_select);

  std::function<void(size_t)> generate_combinations = [&](size_t start) {
    if (selected.size() == static_cast<size_t>(actual_countries_to_select)) {
      std::map<CountryEnum, int> pattern;
      for (const auto& country : selected) {
        pattern[country] = removePerCountry;
      }
      results.emplace_back(std::make_shared<EventRemoveInfluenceMove>(
          cardEnum, moveSide, pattern));
      return;
    }

    for (size_t i = start; i < candidates.size(); ++i) {
      selected.push_back(candidates[i]);
      generate_combinations(i + 1);
      selected.pop_back();
    }
  };

  generate_combinations(0);
  return results;
}

std::vector<std::shared_ptr<Move>>
CardEffectLegalMoveGenerator::generateSelectCountriesRemoveAllInfluenceMoves(
    const Board& board, CardEnum cardEnum, Side moveSide, Side targetSide,
    const std::vector<CountryEnum>& candidates, int countriesToSelect) {
  std::vector<CountryEnum> valid_candidates;
  const auto& world_map = board.getWorldMap();

  for (const auto& country_enum : candidates) {
    const auto& country = world_map.getCountry(country_enum);
    if (country.getInfluence(targetSide) > 0) {
      valid_candidates.push_back(country_enum);
    }
  }

  if (valid_candidates.empty()) {
    return {};
  }
  int actual_countries_to_select =
      std::min(countriesToSelect, static_cast<int>(valid_candidates.size()));

  std::vector<std::shared_ptr<Move>> results;
  std::vector<CountryEnum> selected;
  selected.reserve(actual_countries_to_select);

  std::function<void(size_t)> generate_combinations = [&](size_t start) {
    if (selected.size() == static_cast<size_t>(actual_countries_to_select)) {
      results.emplace_back(std::make_shared<EventRemoveAllInfluenceMove>(
          cardEnum, moveSide, selected));
      return;
    }

    for (size_t i = start; i < valid_candidates.size(); ++i) {
      selected.push_back(valid_candidates[i]);
      generate_combinations(i + 1);
      selected.pop_back();
    }
  };

  generate_combinations(0);
  return results;
}
