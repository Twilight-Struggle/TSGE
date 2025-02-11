#pragma once
#include <array>
#include <set>
#include <vector>

#include "game_enums.hpp"

class Country {
 public:
  Country(const CountryEnum id, const int stability, const bool battleground,
          const std::vector<CountryEnum>& adjacentCountries,
          const std::set<Region>& regions)
      : id_{id},
        stability_{stability},
        battleground_{battleground},
        adjacentCountries_{adjacentCountries},
        regions_{regions},
        influence_({0, 0}) {}

  Country(const CountryEnum id, const int stability, const bool battleground,
          const std::vector<CountryEnum>& adjacentCountries,
          const Region region)
      : Country(id, stability, battleground, adjacentCountries,
                std::set<Region>{region}) {}

  bool addInfluence(const Side side, int num);
  bool removeInfluence(const Side side, int num);
  bool clearInfluence(const Side side);
  int getInfluence(const Side side) const {
    return influence_[static_cast<int>(side)];
  }
  CountryEnum getId() const { return id_; }
  const std::set<Region>& getRegions() const { return regions_; }
  bool operator<(const Country& other) const { return id_ < other.id_; }

 private:
  const CountryEnum id_;
  const int stability_;
  const bool battleground_;
  const std::vector<CountryEnum> adjacentCountries_;
  std::array<int, 2> influence_;
  const std::set<Region> regions_;

  //   bool isAdjacentTo(const CountryEnum &otherCountryId) const {
  //     return std::find(adjacentCountries.begin(), adjacentCountries.end(),
  //                      otherCountryId) != adjacentCountries.end();
  //   }
};