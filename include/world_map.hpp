#pragma once
#include <array>

#include "country.hpp"
#include "game_enums.hpp"

class WorldMap {
 public:
  WorldMap();
  ~WorldMap() = default;

  Country& getCountry(const CountryEnum countryEnum);

 private:
  std::array<Country, 86> countries_;
};