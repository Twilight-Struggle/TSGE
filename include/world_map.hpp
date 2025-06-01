#pragma once
#include <array>

#include "country.hpp"
#include "game_enums.hpp"

class WorldMap {
 public:
  WorldMap();
  ~WorldMap() = default;

  Country& getCountry(CountryEnum countryEnum);
  const Country& getCountry(CountryEnum countryEnum) const;
  const std::set<Country>& countriesInRegion(Region region) const;
  const std::set<CountryEnum> placeableCountries(Side side) const;

 private:
  std::array<Country, 86> countries_;
  std::array<std::set<Country>, 10> regionCountries_;
};