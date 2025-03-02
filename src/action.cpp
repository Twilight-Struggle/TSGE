#include "action.hpp"

#include "world_map.hpp"

bool PlaceInfluence::execute(WorldMap& worldMap) {
  int cumsumOpeValue = 0;
  for (const auto& targetCountry : targetCountries_) {
    // if targetCountry.first not in placeableCountries_
    if (placeableCountries_.find(targetCountry.first) ==
        placeableCountries_.end()) {
      return false;
    }
    const auto country = worldMap.getCountry(targetCountry.first);
    if (country.getControlSide() == side_ ||
        country.getControlSide() == Side::Neutral) {
      cumsumOpeValue += targetCountry.second;
    } else {
      // 相手が支配している場合ペナルティがある
      const auto overControlNum = country.getOverControlNum();
      if (overControlNum == 0) {
        cumsumOpeValue += targetCountry.second + 1;
      } else if (overControlNum == 1) {
        if (targetCountry.second == 1)
          cumsumOpeValue += targetCountry.second + 1;
        else  // targetCountry.second >= 2
          cumsumOpeValue += targetCountry.second + 2;

      } else {  // overControlNum >= 2
        if (targetCountry.second == 1)
          cumsumOpeValue += targetCountry.second + 1;
        else if (targetCountry.second == 2)
          cumsumOpeValue += targetCountry.second + 2;
        else  // targetCountry.second >= 3
          cumsumOpeValue += targetCountry.second + 3;
      }
      // 上限6のためここまででいい
    }
  }
  if (cumsumOpeValue == opeValue_) {
    for (const auto& targetCountry : targetCountries_) {
      worldMap.getCountry(targetCountry.first)
          .addInfluence(side_, targetCountry.second);
    }
    return true;
  } else {
    return false;
  }
}