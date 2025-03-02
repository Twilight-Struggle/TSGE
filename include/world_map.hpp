#pragma once
#include <array>

#include "country.hpp"
#include "game_enums.hpp"

class WorldMap {
 public:
  WorldMap();
  ~WorldMap() = default;

  Country& getCountry(const CountryEnum countryEnum);
  const std::set<Country>& getCountriesInRegion(const Region region) const;
  const std::set<CountryEnum> placeableCountries(const Side side) const;

 private:
  std::array<Country, 86> countries_;
  std::array<std::set<Country>, 10> regionCountries_;
};