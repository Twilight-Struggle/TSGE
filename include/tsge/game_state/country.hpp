#pragma once
#include <array>
#include <cmath>
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

  bool addInfluence(Side side, int num) {
    if (num < 0) [[unlikely]]
      return false;
    influence_[static_cast<int>(side)] += num;
    return true;
  }
  bool removeInfluence(Side side, int num) {
    if (num < 0) [[unlikely]]
      return false;
    influence_[static_cast<int>(side)] -= num;
    if (influence_[static_cast<int>(side)] < 0) {
      influence_[static_cast<int>(side)] = 0;
    }
    return true;
  }
  bool clearInfluence(Side side) {
    influence_[static_cast<int>(side)] = 0;
    return true;
  }
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
  Side getControlSide() const {
    if (influence_[static_cast<int>(Side::USSR)] -
            influence_[static_cast<int>(Side::USA)] >=
        stability_) {
      return Side::USSR;
    }
    if (influence_[static_cast<int>(Side::USA)] -
            influence_[static_cast<int>(Side::USSR)] >=
        stability_) {
      return Side::USA;
    }
    return Side::NEUTRAL;
  }
  bool operator<(const Country& other) const { return id_ < other.id_; }

 private:
  const CountryEnum id_;
  const int stability_;
  const bool battleground_;
  const std::vector<CountryEnum> adjacentCountries_;
  std::array<int, 2> influence_;
  const std::set<Region> regions_;
};