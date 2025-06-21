#pragma once
#include <array>
#include <set>
#include <vector>

#include "tsge/enums/game_enums.hpp"

class Country {
 public:
  Country(CountryEnum id, int stability, bool battleground,
          std::vector<CountryEnum>&& adjacentCountries,
          std::set<Region>&& regions)
      : id_{id},
        stability_{stability},
        battleground_{battleground},
        adjacentCountries_{std::move(adjacentCountries)},
        regions_{std::move(regions)},
        influence_({0, 0}) {}

  Country(CountryEnum id, int stability, bool battleground,
          std::vector<CountryEnum>&& adjacentCountries, Region region)
      : Country{id, stability, battleground, std::move(adjacentCountries),
                std::set<Region>{region}} {}

  bool addInfluence(Side side, int num);
  bool removeInfluence(Side side, int num);
  bool clearInfluence(Side side);
  int getInfluence(Side side) const {
    return influence_[static_cast<int>(side)];
  }
  int getStability() const { return stability_; }
  bool isBattleground() const { return battleground_; }
  CountryEnum getId() const { return id_; }
  const std::set<Region>& getRegions() const { return regions_; }
  const std::vector<CountryEnum>& getAdjacentCountries() const {
    return adjacentCountries_;
  }
  Side getControlSide() const;
  int getOverControlNum() const;
  bool operator<(const Country& other) const { return id_ < other.id_; }

 private:
  const CountryEnum id_;
  const int stability_;
  const bool battleground_;
  const std::vector<CountryEnum> adjacentCountries_;
  std::array<int, 2> influence_;
  const std::set<Region> regions_;
};