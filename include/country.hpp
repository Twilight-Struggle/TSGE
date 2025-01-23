#pragma once
#include <array>
#include <vector>

#include "game_enums.hpp"

class Country {
 public:
  Country(const CountryEnum id, const int stability, const Continent continent,
          const bool battleground,
          const std::vector<CountryEnum>& adjacentCountries)
      : id_{id},
        stability_{stability},
        continent_{continent},
        battleground_{battleground},
        adjacentCountries_{adjacentCountries},
        influence_({0, 0}) {}

  bool addInfluence(const Side side, int num);
  bool removeInfluence(const Side side, int num);
  bool clearInfluence(const Side side);
  int getInfluence(const Side side) const {
    return influence_[static_cast<int>(side)];
  }
  CountryEnum getId() const { return id_; }

 private:
  CountryEnum id_;
  int stability_;
  Continent continent_;
  bool battleground_;
  std::vector<CountryEnum> adjacentCountries_;
  std::array<int, 2> influence_;

  //   bool isAdjacentTo(const CountryEnum &otherCountryId) const {
  //     return std::find(adjacentCountries.begin(), adjacentCountries.end(),
  //                      otherCountryId) != adjacentCountries.end();
  //   }
};