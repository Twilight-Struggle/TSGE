// enums/game_enums.hpp
// ゲーム全体で共有する列挙体とVP係数ユーティリティを定義し、配列インデックスの基準を統一する。
#pragma once

#include <array>
#include <cstdint>

enum class CountryEnum : uint8_t {
  USSR,
  USA,
  MEXICO,
  CUBA,
  GUATEMALA,
  HONDURAS,
  EL_SALVADOR,
  NICARAGUA,
  COSTA_RICA,
  PANAMA,
  HAITI,
  DOMINICAN_REPUBLIC,
  VENEZUELA,
  COLOMBIA,
  ECUADOR,
  PERU,
  BOLIVIA,
  PARAGUAY,
  CHILE,
  URUGUAY,
  ARGENTINA,
  BRAZIL,
  ALGERIA,
  TUNISIA,
  MOROCCO,
  WEST_AFRICAN_STATES,
  SAHARA_STATES,
  IVORY_COAST,
  NIGERIA,
  CAMEROON,
  ZAIRE,
  ANGOLA,
  SOUTH_AFRICA,
  BOTSWANA,
  ZIMBABWE,
  MOZAMBIQUE,
  KENYA,
  ETHIOPIA,
  SUDAN,
  SOMALIA,
  EGYPT,
  LIBYA,
  ISRAEL,
  LEBANON,
  SYRIA,
  JORDAN,
  IRAQ,
  IRAN,
  SAUDI_ARABIA,
  GULF_STATES,
  AFGHANISTAN,
  PAKISTAN,
  INDIA,
  BURMA,
  THAILAND,
  VIETNAM,
  LAOS,
  MALAYSIA,
  INDONESIA,
  PHILIPPINES,
  JAPAN,
  SOUTH_KOREA,
  NORTH_KOREA,
  TAIWAN,
  AUSTRALIA,
  POLAND,
  EAST_GERMANY,
  CZECHOSLOVAKIA,
  HUNGARY,
  ROMANIA,
  BULGARIA,
  YUGOSLAVIA,
  AUSTRIA,
  WEST_GERMANY,
  GREECE,
  TURKEY,
  ITALY,
  FRANCE,
  SPAIN,
  BENELUX,
  UNITED_KINGDOM,
  CANADA,
  NORWAY,
  DENMARK,
  SWEDEN,
  FINLAND,
};

enum class Region : uint8_t {
  EUROPE = 0,
  ASIA = 1,
  MIDDLE_EAST = 2,
  AFRICA = 3,
  SOUTH_AMERICA = 4,
  CENTRAL_AMERICA = 5,
  EAST_EUROPE = 6,
  WEST_EUROPE = 7,
  SOUTH_EAST_ASIA = 8,
  SPECIAL = 9,
};

enum class StateType : uint8_t {
  TURN_START,
  HEADLINE_PHASE,
  HEADLINE_CARD_SELECT_USSR,
  HEADLINE_CARD_SELECT_USA,
  HEADLINE_PROCESS_EVENTS,
  AR_USSR,
  AR_USA,
  AR_USSR_COMPLETE,
  AR_USA_COMPLETE,
  EXTRA_AR_USSR,
  EXTRA_AR_USA,
  TURN_END,
  USSR_WIN_END,
  USA_WIN_END,
  DRAW_END,
};

enum class Side : uint8_t {
  USSR = 0,
  USA = 1,
  NEUTRAL = 2,
};

enum class WarPeriod : uint8_t {
  DUMMY,
  EARLY_WAR,
  MID_WAR,
  LATE_WAR,
};

constexpr Side getOpponentSide(Side side) noexcept {
  constexpr std::array<Side, 3> OPPONENTS = {Side::USA, Side::USSR,
                                             Side::NEUTRAL};
  return OPPONENTS[static_cast<std::size_t>(side)];
}

constexpr int getVpMultiplier(Side side) noexcept {
  constexpr std::array<int, 3> MULTIPLIERS = {1, -1, 0};
  return MULTIPLIERS[static_cast<std::size_t>(side)];
}
