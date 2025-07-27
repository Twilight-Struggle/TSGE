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
  CENTRAL_AMERICA,
  SOUTH_AMERICA,
  AFRICA,
  MIDDLE_EAST,
  ASIA,
  EUROPE,
  EAST_EUROPE,
  WEST_EUROPE,
  SOUTH_EAST_ASIA,
  SPECIAL,
};

enum class StateType : uint8_t {
  TURN_START,
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
  USSR,
  USA,
  NEUTRAL,
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