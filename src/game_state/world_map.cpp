#include "tsge/game_state/world_map.hpp"

#include <tuple>
#include <utility>

#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/country.hpp"
#include "tsge/game_state/world_map_constants.hpp"

namespace {

template <typename StaticDataArray, std::size_t... Indices>
std::array<Country, sizeof...(Indices)> makeCountriesImpl(
    const StaticDataArray& data, std::index_sequence<Indices...> indices) {
  return {Country{data[Indices].id, data[Indices]}...};
}

template <typename StaticDataArray>
auto makeCountries(const StaticDataArray& data) {
  return makeCountriesImpl(
      data, std::make_index_sequence<std::tuple_size_v<StaticDataArray>>{});
}

}  // namespace

WorldMap::WorldMap()
    : regionCountries_{}, countries_{makeCountries(tsge::COUNTRY_STATIC_DATA)} {
  // regionCountries_の初期化
  for (size_t i = 0; i < 86; ++i) {
    for (const auto& region : countries_[i].getRegions()) {
      regionCountries_[static_cast<size_t>(region)].insert(countries_[i]);
    }
  }

  // 初期影響力の設定
  for (const auto& influence : tsge::INITIAL_INFLUENCE_DATA) {
    countries_[static_cast<size_t>(influence.country)].addInfluence(
        influence.side, influence.influence);
  }
}

std::set<CountryEnum> WorldMap::placeableCountries(Side side) const {
  std::set<CountryEnum> placeable_countries;
  for (const auto& country : countries_) {
    // USSRとUSAはここで除外
    if (country.hasRegion(Region::SPECIAL)) {
      continue;
    }
    // 1つでも影響力があれば
    if (country.getInfluence(side) > 0) {
      placeable_countries.insert(country.getId());
    }
    // 隣接国に影響力があれば
    for (const auto& adjacent_country : country.getAdjacentCountries()) {
      if (countries_[static_cast<size_t>(adjacent_country)].getInfluence(side) >
          0) {
        placeable_countries.insert(country.getId());
        break;
      }
    }
  }
  return placeable_countries;
}
