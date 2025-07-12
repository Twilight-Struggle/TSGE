#include "tsge/actions/command.hpp"

#include "tsge/core/board.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/card.hpp"
#include "tsge/game_state/country.hpp"
#include "tsge/utils/randomizer.hpp"

void ActionPlaceInfluenceCommand::apply(Board& board) const {
  for (const auto& target_country : targetCountries_) {
    board.getWorldMap()
        .getCountry(target_country.first)
        .addInfluence(side_, target_country.second);
  }
}

void ActionRealigmentCommand::apply(Board& board) const {
  // USSRかUSAならパス（まれなケース）
  if (targetCountry_ == CountryEnum::USSR || targetCountry_ == CountryEnum::USA)
      [[unlikely]] {
    return;
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
  std::vector<Country> adjacent_countries;
  for (const auto& adjacent_country : country.getAdjacentCountries()) {
    adjacent_countries.push_back(worldmap.getCountry(adjacent_country));
  }
  for (const auto& adjacent_country : adjacent_countries) {
    if (adjacent_country.getControlSide() == Side::USSR) {
      ussr_dice += 1;
    } else if (adjacent_country.getControlSide() == Side::USA) {
      usa_dice += 1;
    }
  }
  auto difference = ussr_dice - usa_dice;
  if (difference > 0) {
    country.removeInfluence(Side::USA, difference);
  } else if (difference < 0) {
    country.removeInfluence(Side::USSR, -difference);
  }
}

void ActionCoupCommand::apply(Board& board) const {
  if (targetCountry_ == CountryEnum::USSR || targetCountry_ == CountryEnum::USA)
      [[unlikely]] {
    return;
  }
  auto& worldmap = board.getWorldMap();
  auto& target_country = worldmap.getCountry(targetCountry_);
  if ((side_ == Side::USSR && target_country.getInfluence(Side::USA) == 0) ||
      (side_ == Side::USA && target_country.getInfluence(Side::USSR) == 0)) {
    return;
  }
  auto coup_dice = Randomizer::getInstance().rollDice();
  coup_dice += card_->getOps();
  const auto defence_value = target_country.getStability() * 2;
  coup_dice = std::max(coup_dice - defence_value, 0);
  bool success = coup_dice != 0;
  int influence_diff =
      target_country.getInfluence(getOpponentSide(side_)) - coup_dice;
  if (influence_diff > 0) {
    target_country.removeInfluence(getOpponentSide(side_), coup_dice);
  } else {
    target_country.clearInfluence(getOpponentSide(side_));
    target_country.addInfluence(side_, -influence_diff);
  }
}

void ActionSpaceRaceCommand::apply(Board& board) const {
  auto& space_track = board.getSpaceTrack();
  if (space_track.canSpace(side_, card_->getOps())) {
    auto roll = Randomizer::getInstance().rollDice();
    if (roll <= space_track.getRollMax(side_)) {
      // スペーストラックを進める
      space_track.advanceSpaceTrack(side_, 1);

      // 新しい位置を取得
      int new_position = space_track.getSpaceTrackPosition(side_);

      // VP計算
      // NOLINTNEXTLINE(readability-identifier-length)
      for (const auto& i : {1, 3, 5, 7, 8}) {
        if (new_position == i) {
          auto vp_data = SpaceTrack::getSpaceVp(i);
          int opponent_position =
              space_track.getSpaceTrackPosition(getOpponentSide(side_));
          if (opponent_position < i) {
            // 得点計算有利
            board.changeVp(vp_data[0] * getVpMultiplier(side_));
          } else {
            // 得点計算不利
            board.changeVp(vp_data[1] * getVpMultiplier(side_));
          }
          break;
        }
      }
      // TODO:8に到達した場合そのターンのARを増やす
    }
    space_track.spaceTried(side_);
  }
}

void ChangeDefconCommand::apply(Board& board) const {
  auto& defcon = board.getDefconTrack();
  int old_defcon = defcon.getDefcon();
  defcon.changeDefcon(delta_);
  int new_defcon = defcon.getDefcon();

  // Defcon 1でゲーム終了
  if (new_defcon <= 1) {
    // DEFCON 1到達で現在のARプレイヤーが敗北
    Side ar_player = board.getCurrentArPlayer();
    if (ar_player == Side::USSR) {
      board.pushState(StateType::USA_WIN_END);
    } else {
      board.pushState(StateType::USSR_WIN_END);
    }
  }

  // NORAD効果チェック（Defconが2に変更された場合）
  if (old_defcon != new_defcon && new_defcon == 2) {
    // TODO: NORADの効果を適用
  }
}

void ChangeVpCommand::apply(Board& board) const {
  board.changeVp(delta_ * getVpMultiplier(side_));

  // VP ±20でゲーム終了
  int victory_points = board.getVp();
  if (victory_points <= -20) {
    board.pushState(StateType::USA_WIN_END);
  } else if (victory_points >= 20) {
    board.pushState(StateType::USSR_WIN_END);
  }
}

void RequestCommand::apply(Board& board) const {}