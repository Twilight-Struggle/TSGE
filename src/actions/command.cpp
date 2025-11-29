// どこで: src/actions/command.cpp
// 何を: Command派生クラスのapplyロジックを実装し、副作用を安全に適用する
// なぜ: Board状態遷移と得点処理を集中管理し、ドキュメントと挙動の乖離を防ぐため
#include "tsge/actions/command.hpp"

#include <algorithm>
#include <array>

#include "tsge/core/board.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/card.hpp"
#include "tsge/game_state/country.hpp"

namespace {

// CARD.mdの「東南アジアの得点」仕様を正規化した重み付きテーブル。
constexpr std::array<std::pair<CountryEnum, int>, 7>
    SOUTHEAST_ASIA_SCORE_TABLE = {{{CountryEnum::BURMA, 1},
                                   {CountryEnum::LAOS, 1},
                                   {CountryEnum::VIETNAM, 1},
                                   {CountryEnum::MALAYSIA, 1},
                                   {CountryEnum::INDONESIA, 1},
                                   {CountryEnum::PHILIPPINES, 1},
                                   {CountryEnum::THAILAND, 2}}};

}  // namespace

void PlaceInfluenceCommand::apply(Board& board) const {
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
  auto& country = worldmap.getCountry(targetCountry_);

  auto ussr_dice = board.getRandomizer().rollDice();
  auto usa_dice = board.getRandomizer().rollDice();
  if (country.getInfluence(Side::USSR) > country.getInfluence(Side::USA)) {
    ussr_dice += 1;
  } else if (country.getInfluence(Side::USA) >
             country.getInfluence(Side::USSR)) {
    usa_dice += 1;
  }

  for (const auto adjacent_country_enum : country.getAdjacentCountries()) {
    const auto& adjacent_country = worldmap.getCountry(adjacent_country_enum);
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
  auto& worldmap = board.getWorldMap();
  auto& target_country = worldmap.getCountry(targetCountry_);

  int ops_value = card_->getOps();
  if (card_->getId() == CardEnum::CHINA_CARD &&
      target_country.hasRegion(Region::ASIA)) {
    ops_value += 1;
  }

  auto coup_dice = board.getRandomizer().rollDice();
  coup_dice += ops_value;
  board.getMilopsTrack().advanceMilopsTrack(side_, ops_value);

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
  if (target_country.isBattleground()) {
    board.pushState(std::make_shared<ChangeDefconCommand>(-1));
  }
}

void ActionSpaceRaceCommand::apply(Board& board) const {
  auto& space_track = board.getSpaceTrack();
  auto roll = board.getRandomizer().rollDice();
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
          board.pushState(std::make_shared<ChangeVpCommand>(side_, vp_data[0]));
        } else {
          // 得点計算不利
          board.pushState(std::make_shared<ChangeVpCommand>(side_, vp_data[1]));
        }
        break;
      }
    }
    board.getActionRoundTrack().updateExtraActionRound(space_track);
  }
  space_track.spaceTried(side_);
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

void SetHeadlineCardCommand::apply(Board& board) const {
  auto& hand = board.getPlayerHand(side_);
  if (auto iter = std::find(hand.begin(), hand.end(), card_);
      iter != hand.end()) {
    hand.erase(iter);
  }
  board.setHeadlineCard(side_, card_);
}

void FinalizeCardPlayCommand::apply(Board& board) const {
  if (card_ == CardEnum::CHINA_CARD) {
    board.giveChinaCardTo(getOpponentSide(side_), false);
    return;
  }

  auto& hand = board.getPlayerHand(side_);
  if (auto iter = std::find(hand.begin(), hand.end(), card_);
      iter != hand.end()) {
    hand.erase(iter);
  }

  auto& deck = board.getDeck();
  if (removeAfterEvent_) {
    deck.getRemovedCards().push_back(card_);
  } else {
    deck.getDiscardPile().push_back(card_);
  }
}

void DiscardCommand::apply(Board& board) const {
  auto& hand = board.getPlayerHand(side_);
  const auto iter = std::find(hand.begin(), hand.end(), card_);
  if (iter == hand.end()) {
    return;
  }
  hand.erase(iter);
  board.getDeck().getDiscardPile().push_back(card_);
}

void ScoreRegionCommand::apply(Board& board) const {
  const int delta = board.scoreRegion(region_, false);
  board.pushState(std::make_shared<ChangeVpCommand>(Side::USSR, delta));
}

void SoutheastAsiaScoringCommand::apply(Board& board) const {
  const auto& world_map = board.getWorldMap();
  int delta = 0;
  for (const auto& [country, weight] : SOUTHEAST_ASIA_SCORE_TABLE) {
    const Side controller = world_map.getCountry(country).getControlSide();
    if (controller == Side::NEUTRAL) {
      continue;
    }
    delta += weight * getVpMultiplier(controller);
  }
  board.pushState(std::make_shared<ChangeVpCommand>(Side::USSR, delta));
}

void RemoveInfluenceCommand::apply(Board& board) const {
  auto& world_map = board.getWorldMap();
  for (const auto& [country, amount] : targetCountries_) {
    world_map.getCountry(country).removeInfluence(targetSide_, amount);
  }
}

void RemoveAllInfluenceCommand::apply(Board& board) const {
  auto& world_map = board.getWorldMap();
  world_map.getCountry(country_).clearInfluence(targetSide_);
}
