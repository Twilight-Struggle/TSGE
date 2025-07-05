#pragma once

#include <array>
#include <cstddef>

#include "tsge/enums/game_enums.hpp"

namespace tsge {

constexpr size_t MAX_ADJACENT_COUNTRIES = 5;
constexpr size_t MAX_REGIONS = 3;

struct CountryStaticData {
  CountryEnum id;
  int stability;
  bool isBattleground;
  std::array<CountryEnum, MAX_ADJACENT_COUNTRIES> adjacentCountries;
  size_t adjacentCountriesCount;
  std::array<Region, MAX_REGIONS> regions;
  size_t regionsCount;
};

extern const std::array<CountryStaticData, 86> COUNTRY_STATIC_DATA;

struct InitialInfluenceData {
  CountryEnum country;
  Side side;
  int influence;
};

extern const std::array<InitialInfluenceData, 20> INITIAL_INFLUENCE_DATA;

}  // namespace tsge