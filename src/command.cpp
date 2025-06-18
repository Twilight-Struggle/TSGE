#include "command.hpp"

#include "board.hpp"
#include "card.hpp"
#include "country.hpp"
#include "game_enums.hpp"
#include "randomizer.hpp"

bool ActionPlaceInfluenceCommand::apply(Board& board) const {
  for (const auto& targetCountry : targetCountries_) {
    board.getWorldMap()
        .getCountry(targetCountry.first)
        .addInfluence(side_, targetCountry.second);
  }
  return true;
}

bool ActionRealigmentCommand::apply(Board& board) const {
  // USSRかUSAならパス
  if (targetCountry_ == CountryEnum::USSR ||
      targetCountry_ == CountryEnum::USA) {
    return true;
  }
  auto& worldmap = board.getWorldMap();
  auto country = worldmap.getCountry(targetCountry_);

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

bool ActionCoupCommand::apply(Board& board) const {
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
    coup_dice += card_->getOps();
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

bool ActionSpaceRaceCommand::apply(Board& board) const {
  auto& spaceTrack = board.getSpaceTrack();
  if (spaceTrack.canSpace(side_, card_->getOps())) {
    auto roll = Randomizer::getInstance().rollDice();
    if (roll <= spaceTrack.getRollMax(side_)) {
      // スペーストラックを進める
      spaceTrack.advanceSpaceTrack(side_, 1);

      // 新しい位置を取得
      int newPosition = spaceTrack.getSpaceTrackPosition(side_);

      // VP計算
      for (const auto& i : {1, 3, 5, 7, 8}) {
        if (newPosition == i) {
          auto vpData = SpaceTrack::getSpaceVp(i);
          int opponentPosition =
              spaceTrack.getSpaceTrackPosition(getOpponentSide(side_));
          if (opponentPosition < i) {
            // 得点計算有利
            board.changeVp(vpData[0] * getVpMultiplier(side_));
          } else {
            // 得点計算不利
            board.changeVp(vpData[1] * getVpMultiplier(side_));
          }
          break;
        }
      }
      // TODO:8に到達した場合そのターンのARを増やす
    }
    spaceTrack.spaceTried(side_);
    return true;
  } else {
    return false;
  }
}

bool ChangeDefconCommand::apply(Board& board) const {
  auto& defcon = board.getDefconTrack();
  int oldDefcon = defcon.getDefcon();
  defcon.changeDefcon(delta_);
  int newDefcon = defcon.getDefcon();

  // Defcon 1でゲーム終了
  if (newDefcon <= 1) {
    // DEFCON 1到達でUSA勝利（仮実装）
    board.pushState(StateType::USA_WIN_END);
  }

  // NORAD効果チェック（Defconが2に変更された場合）
  if (oldDefcon != newDefcon && newDefcon == 2) {
    // TODO: NORADの効果を適用
  }

  return true;
}

bool ChangeVPCommand::apply(Board& board) const {
  board.changeVp(delta_);

  // VP ±20でゲーム終了
  int vp = board.getVp();
  if (vp <= -20) {
    board.pushState(StateType::USA_WIN_END);
  } else if (vp >= 20) {
    board.pushState(StateType::USSR_WIN_END);
  }

  return true;
}