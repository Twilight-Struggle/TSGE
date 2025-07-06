#include "tsge/game_state/world_map.hpp"

#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/country.hpp"
#include "tsge/game_state/world_map_constants.hpp"

WorldMap::WorldMap()
    : regionCountries_{},
      countries_{Country{tsge::COUNTRY_STATIC_DATA[0].id,
                         tsge::COUNTRY_STATIC_DATA[0]},
                 Country{tsge::COUNTRY_STATIC_DATA[1].id,
                         tsge::COUNTRY_STATIC_DATA[1]},
                 Country{tsge::COUNTRY_STATIC_DATA[2].id,
                         tsge::COUNTRY_STATIC_DATA[2]},
                 Country{tsge::COUNTRY_STATIC_DATA[3].id,
                         tsge::COUNTRY_STATIC_DATA[3]},
                 Country{tsge::COUNTRY_STATIC_DATA[4].id,
                         tsge::COUNTRY_STATIC_DATA[4]},
                 Country{tsge::COUNTRY_STATIC_DATA[5].id,
                         tsge::COUNTRY_STATIC_DATA[5]},
                 Country{tsge::COUNTRY_STATIC_DATA[6].id,
                         tsge::COUNTRY_STATIC_DATA[6]},
                 Country{tsge::COUNTRY_STATIC_DATA[7].id,
                         tsge::COUNTRY_STATIC_DATA[7]},
                 Country{tsge::COUNTRY_STATIC_DATA[8].id,
                         tsge::COUNTRY_STATIC_DATA[8]},
                 Country{tsge::COUNTRY_STATIC_DATA[9].id,
                         tsge::COUNTRY_STATIC_DATA[9]},
                 Country{tsge::COUNTRY_STATIC_DATA[10].id,
                         tsge::COUNTRY_STATIC_DATA[10]},
                 Country{tsge::COUNTRY_STATIC_DATA[11].id,
                         tsge::COUNTRY_STATIC_DATA[11]},
                 Country{tsge::COUNTRY_STATIC_DATA[12].id,
                         tsge::COUNTRY_STATIC_DATA[12]},
                 Country{tsge::COUNTRY_STATIC_DATA[13].id,
                         tsge::COUNTRY_STATIC_DATA[13]},
                 Country{tsge::COUNTRY_STATIC_DATA[14].id,
                         tsge::COUNTRY_STATIC_DATA[14]},
                 Country{tsge::COUNTRY_STATIC_DATA[15].id,
                         tsge::COUNTRY_STATIC_DATA[15]},
                 Country{tsge::COUNTRY_STATIC_DATA[16].id,
                         tsge::COUNTRY_STATIC_DATA[16]},
                 Country{tsge::COUNTRY_STATIC_DATA[17].id,
                         tsge::COUNTRY_STATIC_DATA[17]},
                 Country{tsge::COUNTRY_STATIC_DATA[18].id,
                         tsge::COUNTRY_STATIC_DATA[18]},
                 Country{tsge::COUNTRY_STATIC_DATA[19].id,
                         tsge::COUNTRY_STATIC_DATA[19]},
                 Country{tsge::COUNTRY_STATIC_DATA[20].id,
                         tsge::COUNTRY_STATIC_DATA[20]},
                 Country{tsge::COUNTRY_STATIC_DATA[21].id,
                         tsge::COUNTRY_STATIC_DATA[21]},
                 Country{tsge::COUNTRY_STATIC_DATA[22].id,
                         tsge::COUNTRY_STATIC_DATA[22]},
                 Country{tsge::COUNTRY_STATIC_DATA[23].id,
                         tsge::COUNTRY_STATIC_DATA[23]},
                 Country{tsge::COUNTRY_STATIC_DATA[24].id,
                         tsge::COUNTRY_STATIC_DATA[24]},
                 Country{tsge::COUNTRY_STATIC_DATA[25].id,
                         tsge::COUNTRY_STATIC_DATA[25]},
                 Country{tsge::COUNTRY_STATIC_DATA[26].id,
                         tsge::COUNTRY_STATIC_DATA[26]},
                 Country{tsge::COUNTRY_STATIC_DATA[27].id,
                         tsge::COUNTRY_STATIC_DATA[27]},
                 Country{tsge::COUNTRY_STATIC_DATA[28].id,
                         tsge::COUNTRY_STATIC_DATA[28]},
                 Country{tsge::COUNTRY_STATIC_DATA[29].id,
                         tsge::COUNTRY_STATIC_DATA[29]},
                 Country{tsge::COUNTRY_STATIC_DATA[30].id,
                         tsge::COUNTRY_STATIC_DATA[30]},
                 Country{tsge::COUNTRY_STATIC_DATA[31].id,
                         tsge::COUNTRY_STATIC_DATA[31]},
                 Country{tsge::COUNTRY_STATIC_DATA[32].id,
                         tsge::COUNTRY_STATIC_DATA[32]},
                 Country{tsge::COUNTRY_STATIC_DATA[33].id,
                         tsge::COUNTRY_STATIC_DATA[33]},
                 Country{tsge::COUNTRY_STATIC_DATA[34].id,
                         tsge::COUNTRY_STATIC_DATA[34]},
                 Country{tsge::COUNTRY_STATIC_DATA[35].id,
                         tsge::COUNTRY_STATIC_DATA[35]},
                 Country{tsge::COUNTRY_STATIC_DATA[36].id,
                         tsge::COUNTRY_STATIC_DATA[36]},
                 Country{tsge::COUNTRY_STATIC_DATA[37].id,
                         tsge::COUNTRY_STATIC_DATA[37]},
                 Country{tsge::COUNTRY_STATIC_DATA[38].id,
                         tsge::COUNTRY_STATIC_DATA[38]},
                 Country{tsge::COUNTRY_STATIC_DATA[39].id,
                         tsge::COUNTRY_STATIC_DATA[39]},
                 Country{tsge::COUNTRY_STATIC_DATA[40].id,
                         tsge::COUNTRY_STATIC_DATA[40]},
                 Country{tsge::COUNTRY_STATIC_DATA[41].id,
                         tsge::COUNTRY_STATIC_DATA[41]},
                 Country{tsge::COUNTRY_STATIC_DATA[42].id,
                         tsge::COUNTRY_STATIC_DATA[42]},
                 Country{tsge::COUNTRY_STATIC_DATA[43].id,
                         tsge::COUNTRY_STATIC_DATA[43]},
                 Country{tsge::COUNTRY_STATIC_DATA[44].id,
                         tsge::COUNTRY_STATIC_DATA[44]},
                 Country{tsge::COUNTRY_STATIC_DATA[45].id,
                         tsge::COUNTRY_STATIC_DATA[45]},
                 Country{tsge::COUNTRY_STATIC_DATA[46].id,
                         tsge::COUNTRY_STATIC_DATA[46]},
                 Country{tsge::COUNTRY_STATIC_DATA[47].id,
                         tsge::COUNTRY_STATIC_DATA[47]},
                 Country{tsge::COUNTRY_STATIC_DATA[48].id,
                         tsge::COUNTRY_STATIC_DATA[48]},
                 Country{tsge::COUNTRY_STATIC_DATA[49].id,
                         tsge::COUNTRY_STATIC_DATA[49]},
                 Country{tsge::COUNTRY_STATIC_DATA[50].id,
                         tsge::COUNTRY_STATIC_DATA[50]},
                 Country{tsge::COUNTRY_STATIC_DATA[51].id,
                         tsge::COUNTRY_STATIC_DATA[51]},
                 Country{tsge::COUNTRY_STATIC_DATA[52].id,
                         tsge::COUNTRY_STATIC_DATA[52]},
                 Country{tsge::COUNTRY_STATIC_DATA[53].id,
                         tsge::COUNTRY_STATIC_DATA[53]},
                 Country{tsge::COUNTRY_STATIC_DATA[54].id,
                         tsge::COUNTRY_STATIC_DATA[54]},
                 Country{tsge::COUNTRY_STATIC_DATA[55].id,
                         tsge::COUNTRY_STATIC_DATA[55]},
                 Country{tsge::COUNTRY_STATIC_DATA[56].id,
                         tsge::COUNTRY_STATIC_DATA[56]},
                 Country{tsge::COUNTRY_STATIC_DATA[57].id,
                         tsge::COUNTRY_STATIC_DATA[57]},
                 Country{tsge::COUNTRY_STATIC_DATA[58].id,
                         tsge::COUNTRY_STATIC_DATA[58]},
                 Country{tsge::COUNTRY_STATIC_DATA[59].id,
                         tsge::COUNTRY_STATIC_DATA[59]},
                 Country{tsge::COUNTRY_STATIC_DATA[60].id,
                         tsge::COUNTRY_STATIC_DATA[60]},
                 Country{tsge::COUNTRY_STATIC_DATA[61].id,
                         tsge::COUNTRY_STATIC_DATA[61]},
                 Country{tsge::COUNTRY_STATIC_DATA[62].id,
                         tsge::COUNTRY_STATIC_DATA[62]},
                 Country{tsge::COUNTRY_STATIC_DATA[63].id,
                         tsge::COUNTRY_STATIC_DATA[63]},
                 Country{tsge::COUNTRY_STATIC_DATA[64].id,
                         tsge::COUNTRY_STATIC_DATA[64]},
                 Country{tsge::COUNTRY_STATIC_DATA[65].id,
                         tsge::COUNTRY_STATIC_DATA[65]},
                 Country{tsge::COUNTRY_STATIC_DATA[66].id,
                         tsge::COUNTRY_STATIC_DATA[66]},
                 Country{tsge::COUNTRY_STATIC_DATA[67].id,
                         tsge::COUNTRY_STATIC_DATA[67]},
                 Country{tsge::COUNTRY_STATIC_DATA[68].id,
                         tsge::COUNTRY_STATIC_DATA[68]},
                 Country{tsge::COUNTRY_STATIC_DATA[69].id,
                         tsge::COUNTRY_STATIC_DATA[69]},
                 Country{tsge::COUNTRY_STATIC_DATA[70].id,
                         tsge::COUNTRY_STATIC_DATA[70]},
                 Country{tsge::COUNTRY_STATIC_DATA[71].id,
                         tsge::COUNTRY_STATIC_DATA[71]},
                 Country{tsge::COUNTRY_STATIC_DATA[72].id,
                         tsge::COUNTRY_STATIC_DATA[72]},
                 Country{tsge::COUNTRY_STATIC_DATA[73].id,
                         tsge::COUNTRY_STATIC_DATA[73]},
                 Country{tsge::COUNTRY_STATIC_DATA[74].id,
                         tsge::COUNTRY_STATIC_DATA[74]},
                 Country{tsge::COUNTRY_STATIC_DATA[75].id,
                         tsge::COUNTRY_STATIC_DATA[75]},
                 Country{tsge::COUNTRY_STATIC_DATA[76].id,
                         tsge::COUNTRY_STATIC_DATA[76]},
                 Country{tsge::COUNTRY_STATIC_DATA[77].id,
                         tsge::COUNTRY_STATIC_DATA[77]},
                 Country{tsge::COUNTRY_STATIC_DATA[78].id,
                         tsge::COUNTRY_STATIC_DATA[78]},
                 Country{tsge::COUNTRY_STATIC_DATA[79].id,
                         tsge::COUNTRY_STATIC_DATA[79]},
                 Country{tsge::COUNTRY_STATIC_DATA[80].id,
                         tsge::COUNTRY_STATIC_DATA[80]},
                 Country{tsge::COUNTRY_STATIC_DATA[81].id,
                         tsge::COUNTRY_STATIC_DATA[81]},
                 Country{tsge::COUNTRY_STATIC_DATA[82].id,
                         tsge::COUNTRY_STATIC_DATA[82]},
                 Country{tsge::COUNTRY_STATIC_DATA[83].id,
                         tsge::COUNTRY_STATIC_DATA[83]},
                 Country{tsge::COUNTRY_STATIC_DATA[84].id,
                         tsge::COUNTRY_STATIC_DATA[84]},
                 Country{tsge::COUNTRY_STATIC_DATA[85].id,
                         tsge::COUNTRY_STATIC_DATA[85]}} {
  // regionCountries_の初期化
  for (size_t i = 0; i < 86; ++i) {
    for (const auto& region : countries_[i].getRegions()) {
      regionCountries_[static_cast<size_t>(region)].insert(countries_[i]);
    }
  }

  // 初期影響力の設定
  for (const auto& influence : tsge::INITIAL_INFLUENCE_DATA) {
    countries_[static_cast<size_t>(influence.country)].addInfluence(
        influence.side, influence.influence);
  }
}

std::set<CountryEnum> WorldMap::placeableCountries(Side side) const {
  std::set<CountryEnum> placeable_countries;
  for (const auto& country : countries_) {
    // USSRとUSAはここで除外
    if (country.hasRegion(Region::SPECIAL)) {
      continue;
    }
    // 1つでも影響力があれば
    if (country.getInfluence(side) > 0) {
      placeable_countries.insert(country.getId());
    }
    // 隣接国に影響力があれば
    for (const auto& adjacent_country : country.getAdjacentCountries()) {
      if (countries_[static_cast<size_t>(adjacent_country)].getInfluence(side) >
          0) {
        placeable_countries.insert(country.getId());
        break;
      }
    }
  }
  return placeable_countries;
}
