#pragma once
#include <array>
#include <cmath>
#include <span>

#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/world_map_constants.hpp"

class Country {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  Country(CountryEnum id, const tsge::CountryStaticData& staticData)
      : id_{id}, staticData_{staticData}, influence_({0, 0}) {}

  bool addInfluence(Side side, int num) {
    if (num < 0) [[unlikely]] {
      return false;
    }
    influence_[static_cast<int>(side)] += num;
    return true;
  }
  bool removeInfluence(Side side, int num) {
    if (num < 0) [[unlikely]] {
      return false;
    }
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
  int getStability() const { return staticData_.stability; }
  bool isBattleground() const { return staticData_.isBattleground; }
  CountryEnum getId() const { return id_; }
  std::span<const Region> getRegions() const {
    return std::span<const Region>(staticData_.regions.data(),
                                   staticData_.regionsCount);
  }
  std::span<const CountryEnum> getAdjacentCountries() const {
    return std::span<const CountryEnum>(staticData_.adjacentCountries.data(),
                                        staticData_.adjacentCountriesCount);
  }
  bool hasRegion(Region region) const {
    for (size_t i = 0; i < staticData_.regionsCount; ++i) {
      if (staticData_.regions[i] == region) {
        return true;
      }
    }
    return false;
  }
  Side getControlSide() const {
    if (influence_[static_cast<int>(Side::USSR)] -
            influence_[static_cast<int>(Side::USA)] >=
        staticData_.stability) {
      return Side::USSR;
    }
    if (influence_[static_cast<int>(Side::USA)] -
            influence_[static_cast<int>(Side::USSR)] >=
        staticData_.stability) {
      return Side::USA;
    }
    return Side::NEUTRAL;
  }
  bool operator<(const Country& other) const { return id_ < other.id_; }

 private:
  const CountryEnum id_;
  const tsge::CountryStaticData& staticData_;
  std::array<int, 2> influence_;
};