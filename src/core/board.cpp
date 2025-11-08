// core/board.cpp
// Boardクラスの振る舞いを実装し、影響力・得点処理をドキュメント通りに保持する。
// 特に最終得点は勝敗を左右するため、ここで一貫したアルゴリズムを提供する。
#include "tsge/core/board.hpp"

#include <ranges>

namespace {

struct RegionScoreProfile {
  Region region;
  int controlPoints;
  int dominationPoints;
  int presencePoints;
};

constexpr std::size_t REGION_COUNT =
    static_cast<std::size_t>(Region::CENTRAL_AMERICA) + 1;

// CARD.mdとタスク仕様の数値を正規化した得点テーブル。
constexpr std::array<RegionScoreProfile, REGION_COUNT> REGION_SCORE_PROFILES = {
    RegionScoreProfile{Region::EUROPE, 1000, 7, 3},
    RegionScoreProfile{Region::ASIA, 9, 7, 3},
    RegionScoreProfile{Region::MIDDLE_EAST, 7, 5, 3},
    RegionScoreProfile{Region::AFRICA, 6, 4, 1},
    RegionScoreProfile{Region::SOUTH_AMERICA, 6, 5, 2},
    RegionScoreProfile{Region::CENTRAL_AMERICA, 5, 3, 1},
};

constexpr std::array<Region, 6> GLOBAL_SCORING_REGIONS = {
    Region::EUROPE, Region::ASIA,          Region::MIDDLE_EAST,
    Region::AFRICA, Region::SOUTH_AMERICA, Region::CENTRAL_AMERICA};

[[nodiscard]] const RegionScoreProfile& findRegionProfile(Region region) {
  return REGION_SCORE_PROFILES[static_cast<std::size_t>(region)];
}

}  // namespace

void Board::giveChinaCardTo(Side newOwner, bool faceUp) {
  if (newOwner != Side::USSR && newOwner != Side::USA) [[unlikely]] {
    return;
  }
  chinaCard_.owner = newOwner;
  chinaCard_.faceUp = faceUp;
}

void Board::revealChinaCard() {
  if (chinaCard_.owner == Side::NEUTRAL) [[unlikely]] {
    return;
  }
  chinaCard_.faceUp = true;
}

void Board::finalScoring() {
  int final_score = 0;
  // 6地域それぞれの最終得点を積み上げ、最後にチャイナカードの1点を加算。
  for (const auto region : GLOBAL_SCORING_REGIONS) {
    final_score += scoreRegion(region, true);
  }
  final_score += getVpMultiplier(chinaCard_.owner);
  pushState(std::make_shared<ChangeVpCommand>(Side::USSR, final_score));
}

int Board::scoreRegion(Region region,
                       [[maybe_unused]] bool isFinalScoring) const {
  // Presence→Domination→Controlの順で条件を満たすか精査し、
  // その差分と戦闘国・超大国隣接ボーナスをすべてUSSR視点の符号で返す。
  const auto& profile = findRegionProfile(region);

  std::array<int, 2> total_countries = {0, 0};
  std::array<int, 2> battleground_countries = {0, 0};
  std::array<int, 2> adjacency_bonuses = {0, 0};
  std::array<bool, 2> has_non_battleground = {false, false};
  int total_battlegrounds = 0;

  const auto adjacent_to_opponent_superpower = [](const Country& country,
                                                  Side side) {
    const CountryEnum opponent =
        side == Side::USSR ? CountryEnum::USA : CountryEnum::USSR;
    return std::ranges::any_of(country.getAdjacentCountries(),
                               [opponent](const CountryEnum adjacent) {
                                 return adjacent == opponent;
                               });
  };

  const auto& region_countries = worldMap_.countriesInRegion(region);
  for (const auto& snapshot : region_countries) {
    const auto& country = worldMap_.getCountry(snapshot.getId());
    if (country.isBattleground()) {
      ++total_battlegrounds;
    }

    const Side controller = country.getControlSide();
    if (controller != Side::USSR && controller != Side::USA) {
      continue;
    }

    const auto index = static_cast<size_t>(controller);
    ++total_countries[index];
    if (country.isBattleground()) {
      ++battleground_countries[index];
    } else {
      has_non_battleground[index] = true;
    }
    if (adjacent_to_opponent_superpower(country, controller)) {
      ++adjacency_bonuses[index];
    }
  }

  const auto classification_points_for = [&](Side side) {
    const auto side_index = static_cast<size_t>(side);
    const auto opponent_index = static_cast<size_t>(getOpponentSide(side));
    if (total_countries[side_index] == 0) {
      return 0;
    }

    const bool controls_all_battlegrounds =
        total_battlegrounds > 0 &&
        battleground_countries[side_index] == total_battlegrounds &&
        total_countries[side_index] > total_countries[opponent_index];
    if (controls_all_battlegrounds) {
      return profile.controlPoints;
    }

    const bool has_domination =
        battleground_countries[side_index] >
            battleground_countries[opponent_index] &&
        total_countries[side_index] > total_countries[opponent_index] &&
        has_non_battleground[side_index];
    if (has_domination) {
      return profile.dominationPoints;
    }

    return profile.presencePoints;
  };

  const int ussr_index = static_cast<size_t>(Side::USSR);
  const int usa_index = static_cast<size_t>(Side::USA);

  const int ussr_score = classification_points_for(Side::USSR) +
                         battleground_countries[ussr_index] +
                         adjacency_bonuses[ussr_index];
  const int usa_score = classification_points_for(Side::USA) +
                        battleground_countries[usa_index] +
                        adjacency_bonuses[usa_index];

  return ussr_score * getVpMultiplier(Side::USSR) +
         usa_score * getVpMultiplier(Side::USA);
}

std::array<int, 2> Board::calculateDrawCount(int turn) const {
  const int required_cards = actionRoundTrack_.getDefinedActionRounds(turn) + 2;

  const int ussr_current_cards =
      static_cast<int>(playerHands_[static_cast<size_t>(Side::USSR)].size());
  const int usa_current_cards =
      static_cast<int>(playerHands_[static_cast<size_t>(Side::USA)].size());

  const int ussr_draw_count = std::max(0, required_cards - ussr_current_cards);
  const int usa_draw_count = std::max(0, required_cards - usa_current_cards);

  return {ussr_draw_count, usa_draw_count};
}

void Board::drawCardsForPlayers(int ussrDrawCount, int usaDrawCount) {
  const int total_draw_count = ussrDrawCount + usaDrawCount;
  if (total_draw_count == 0) {
    return;
  }

  auto& deck_cards = deck_.getDeck();
  const int deck_size = static_cast<int>(deck_cards.size());

  const auto draw_for_side = [&](Side side, int count) {
    auto& hand = playerHands_[static_cast<size_t>(side)];
    for (int i = 0; i < count; ++i) {
      hand.push_back(deck_cards.back());
      deck_cards.pop_back();
    }
  };

  const auto draw_limited_for_side = [&](Side side, int& remaining, int cap) {
    const int draw_cap = std::min(remaining, cap);
    const int draw_count =
        std::min(draw_cap, static_cast<int>(deck_cards.size()));
    draw_for_side(side, draw_count);
    remaining -= draw_count;
  };

  if (total_draw_count <= deck_size) {
    draw_for_side(Side::USSR, ussrDrawCount);
    draw_for_side(Side::USA, usaDrawCount);
    if (total_draw_count == deck_size) {
      deck_.reshuffleFromDiscard();
    }
    return;
  }

  int ussr_remaining = ussrDrawCount;
  int usa_remaining = usaDrawCount;

  const int ussr_share = (deck_size + 1) / 2;
  const int usa_share = deck_size - ussr_share;

  draw_limited_for_side(Side::USSR, ussr_remaining, ussr_share);
  draw_limited_for_side(Side::USA, usa_remaining, usa_share);

  deck_.reshuffleFromDiscard();

  draw_limited_for_side(Side::USSR, ussr_remaining, ussr_remaining);
  draw_limited_for_side(Side::USA, usa_remaining, usa_remaining);
}

bool Board::isHeadlineCardVisible(Side viewer, Side target) const {
  // 自分のカードは常に見える
  if (viewer == target) {
    return true;
  }

  // 宇宙開発トラック4の優位性チェック
  // viewerがトラック4以上でtargetがトラック4未満の場合、targetのカードが見える
  return spaceTrack_.effectEnabled(viewer, 4);
}

Board Board::copyForMCTS(Side viewerSide) const {
  // shared_ptrに変更したことで、完全なコピーコンストラクタが使用可能
  Board copy = *this;

  // 相手側のSideを取得
  const Side opponent_side = getOpponentSide(viewerSide);

  // 相手の手札を隠蔽（カード枚数は維持し、内容をDummyに置換）
  // TODO: 将来activeEvents_メンバを追加して、CIA Createdなどのイベントによる
  // 手札可視性の変更に対応する（例：activeEvents_にCIA_Createdが含まれている場合は隠蔽しない）
  auto& opponent_hand = copy.playerHands_[static_cast<size_t>(opponent_side)];
  for (auto& card : opponent_hand) {
    card = CardEnum::DUMMY;
  }

  // デッキは見えない
  auto& deck = copy.getDeck().getDeck();
  for (auto& card : deck) {
    card = CardEnum::DUMMY;
  }

  // ヘッドラインカードの隠蔽
  if (!copy.isHeadlineCardVisible(viewerSide, opponent_side)) {
    copy.headlineCards_[static_cast<size_t>(opponent_side)] = CardEnum::DUMMY;
  }

  // Randomizerの外部RNGポインタをリセット（MCTSで独自に設定される）
  // copy.randomizer_.setRng(nullptr);

  return copy;
}
