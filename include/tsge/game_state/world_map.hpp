#pragma once
#include <array>
#include <set>

#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/country.hpp"

class WorldMap {
 public:
  WorldMap();
  ~WorldMap() = default;
  WorldMap(const WorldMap&) = default;
  WorldMap& operator=(const WorldMap&) = delete;
  WorldMap(WorldMap&&) = default;
  WorldMap& operator=(WorldMap&&) = delete;

  Country& getCountry(CountryEnum countryEnum) {
    return countries_[static_cast<size_t>(countryEnum)];
  }
  const Country& getCountry(CountryEnum countryEnum) const {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<WorldMap*>(this)->getCountry(countryEnum);
  }
  const std::set<Country>& countriesInRegion(Region region) const {
    return regionCountries_[static_cast<size_t>(region)];
  }
  std::set<CountryEnum> placeableCountries(Side side) const;
  const size_t getCountriesCount() const { return countries_.size(); }
  const size_t getRegionsCount() const { return regionCountries_.size(); }

 private:
  std::array<Country, 86> countries_;
  std::array<std::set<Country>, 10> regionCountries_;
};