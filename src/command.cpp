#include "command.hpp"

#include "country.hpp"
#include "game.hpp"
#include "game_enums.hpp"
#include "randomizer.hpp"

bool PlaceInfluence::apply(Board& board) const {
  auto& worldMap = board.getWorldMap();
  int cumsumOpeValue = 0;
  for (const auto& targetCountry : targetCountries_) {
    // if targetCountry.first not in placeableCountries_
    auto placeableCountries = board.getWorldMap().placeableCountries(side_);
    if (placeableCountries.find(targetCountry.first) ==
        placeableCountries.end()) {
      return false;
    }
    const auto country = worldMap.getCountry(targetCountry.first);
    if (country.getControlSide() == side_ ||
        country.getControlSide() == Side::NEUTRAL) {
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

bool Realigment::apply(Board& board) const {
  // USSRかUSAならパス
  if (targetCountry_ == CountryEnum::USSR ||
      targetCountry_ == CountryEnum::USA) {
    return true;
  }
  auto& worldmap = board.getWorldMap();
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

bool Coup::apply(Board& board) const {
  if (targetCountry_ == CountryEnum::USSR ||
      targetCountry_ == CountryEnum::USA) {
    return false;
  }
  auto& worldmap = board.getWorldMap();
  auto targetCountry = worldmap.getCountry(targetCountry_);
  if ((side_ == Side::USSR && targetCountry.getInfluence(Side::USA) == 0) ||
      (side_ == Side::USA && targetCountry.getInfluence(Side::USSR) == 0)) {
    return false;
  } else {
    auto coup_dice = Randomizer::getInstance().rollDice();
    coup_dice += opeValue_;
    const auto defence_value = targetCountry.getStability() * 2;
    coup_dice -= defence_value;
    if (coup_dice < 0) {
      coup_dice = 0;
    }
    bool success = (coup_dice == 0) ? false : true;
    bool diff = targetCountry.getInfluence(getOpponentSide(side_)) - coup_dice;
    if (diff > 0) {
      targetCountry.removeInfluence(getOpponentSide(side_), coup_dice);
    } else {
      targetCountry.clearInfluence(getOpponentSide(side_));
      targetCountry.addInfluence(side_, -diff);
    }
    return true;
  }
}

bool SpaceRace::apply(Board& board) const {
  auto& spaceTrack = board.getSpaceTrack();
  if (spaceTrack.canSpace(side_, opeValue_)) {
    auto roll = Randomizer::getInstance().rollDice();
    if (roll <= spaceTrack.getRollMax(side_)) {
      spaceTrack.advanceSpaceTrack(board, side_, 1);
    }
    return true;
  } else {
    return false;
  }
}