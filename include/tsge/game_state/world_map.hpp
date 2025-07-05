#pragma once
#include <array>
#include <set>

#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/country.hpp"

class WorldMap {
 public:
  WorldMap();
  ~WorldMap() = default;

  Country& getCountry(CountryEnum countryEnum);
  const Country& getCountry(CountryEnum countryEnum) const;
  const std::set<Country>& countriesInRegion(Region region) const;
  std::set<CountryEnum> placeableCountries(Side side) const;
  const size_t getCountriesCount() const { return countries_.size(); }
  const size_t getRegionsCount() const { return regionCountries_.size(); }

 private:
  std::array<Country, 86> countries_;
  std::array<std::set<Country>, 10> regionCountries_;
};