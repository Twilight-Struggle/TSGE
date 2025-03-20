#include "action.hpp"

#include "country.hpp"
#include "game_enums.hpp"
#include "randomizer.hpp"
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

bool Realigment::execute(WorldMap& worldmap) {
  auto country = worldmap.getCountry(targetCountry_);
  if ((side_ == Side::USSR && country.getInfluence(Side::USA) == 0) ||
      (side_ == Side::USA && country.getInfluence(Side::USSR) == 0)) {
    return false;
  }
  auto ussr_dice = Randomizer::getInstance().rollDice();
  auto usa_dice = Randomizer::getInstance().rollDice();
  if (country.getInfluence(Side::USSR) > country.getInfluence(Side::USA)) {
    ussr_dice += 1;
  } else if (country.getInfluence(Side::USA) >
             country.getInfluence(Side::USSR)) {
    usa_dice += 1;
  }
  std::vector<Country> adjacentCountries;
  for (const auto& adjacentCountry : country.getAdjacentCountries()) {
    adjacentCountries.push_back(worldmap.getCountry(adjacentCountry));
  }
  for (const auto& adjacentCountry : adjacentCountries) {
    if (adjacentCountry.getControlSide() == Side::USSR) {
      ussr_dice += 1;
    } else if (adjacentCountry.getControlSide() == Side::USA) {
      usa_dice += 1;
    }
  }
  auto difference = ussr_dice - usa_dice;
  if (difference > 0) {
    country.removeInfluence(Side::USA, difference);
  } else if (difference < 0) {
    country.removeInfluence(Side::USSR, -difference);
  }
  return true;
}