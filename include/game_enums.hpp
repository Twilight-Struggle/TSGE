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
};

enum class Side : uint8_t {
  USSR,
  USA,
  NEUTRAL,
};

inline Side getOpponentSide(Side side) {
  switch (side) {
    case Side::USSR:
      return Side::USA;
    case Side::USA:
      return Side::USSR;
    case Side::NEUTRAL:
      return Side::NEUTRAL;
  }
}

inline int getVpMultiplier(Side side) {
  switch (side) {
    case Side::USSR:
      return 1;
    case Side::USA:
      return -1;
    case Side::NEUTRAL:
      return 0;
  }
}