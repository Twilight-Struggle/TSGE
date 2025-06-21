#pragma once

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

enum class MoveType : uint8_t {
  COUP,
  REALIGNMENT,
  PLACE_INFLUENCE,
  EVENT,
  SPACE_RACE
};

enum class StateType : uint8_t {
  AR_USSR,
  AR_USA,
  AR_COMPLETE,
  USSR_WIN_END,
  USA_WIN_END,
  DRAW_END,
};

enum class Side : uint8_t {
  USSR,
  USA,
  NEUTRAL,
};

constexpr Side getOpponentSide(Side side) noexcept {
  constexpr Side opponents[] = {Side::USA, Side::USSR, Side::NEUTRAL};
  return opponents[static_cast<std::size_t>(side)];
}

constexpr int getVpMultiplier(Side side) noexcept {
  constexpr int multipliers[] = {1, -1, 0};
  return multipliers[static_cast<std::size_t>(side)];
}