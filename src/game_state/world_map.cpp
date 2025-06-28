#include "tsge/game_state/world_map.hpp"

#include <sys/stat.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/country.hpp"

WorldMap::WorldMap()
    : regionCountries_{},
      countries_{
          Country{CountryEnum::USSR, 100, false,
                  std::vector<CountryEnum>{
                      CountryEnum::NORTH_KOREA, CountryEnum::AFGHANISTAN,
                      CountryEnum::ROMANIA, CountryEnum::POLAND,
                      CountryEnum::FINLAND},
                  Region::SPECIAL},
          Country{
              CountryEnum::USA, 100, false,
              std::vector<CountryEnum>{CountryEnum::MEXICO, CountryEnum::CANADA,
                                       CountryEnum::CUBA},
              Region::SPECIAL},
          Country{CountryEnum::MEXICO, 2, true,
                  std::vector<CountryEnum>{CountryEnum::USA,
                                           CountryEnum::GUATEMALA},
                  Region::CENTRAL_AMERICA},
          Country{
              CountryEnum::CUBA, 3, true,
              std::vector<CountryEnum>{CountryEnum::NICARAGUA, CountryEnum::USA,
                                       CountryEnum::HAITI},
              Region::CENTRAL_AMERICA},
          Country{CountryEnum::GUATEMALA, 1, false,
                  std::vector<CountryEnum>{CountryEnum::MEXICO,
                                           CountryEnum::EL_SALVADOR,
                                           CountryEnum::HONDURAS},
                  Region::CENTRAL_AMERICA},
          Country{CountryEnum::HONDURAS, 2, false,
                  std::vector<CountryEnum>{
                      CountryEnum::GUATEMALA, CountryEnum::EL_SALVADOR,
                      CountryEnum::NICARAGUA, CountryEnum::COSTA_RICA},
                  Region::CENTRAL_AMERICA},
          Country{CountryEnum::EL_SALVADOR, 1, false,
                  std::vector<CountryEnum>{CountryEnum::GUATEMALA,
                                           CountryEnum::HONDURAS},
                  Region::CENTRAL_AMERICA},
          Country{
              CountryEnum::NICARAGUA, 1, false,
              std::vector<CountryEnum>{CountryEnum::HONDURAS, CountryEnum::CUBA,
                                       CountryEnum::COSTA_RICA},
              Region::CENTRAL_AMERICA},
          Country{CountryEnum::COSTA_RICA, 3, false,
                  std::vector<CountryEnum>{CountryEnum::HONDURAS,
                                           CountryEnum::NICARAGUA,
                                           CountryEnum::PANAMA},
                  Region::CENTRAL_AMERICA},
          Country{CountryEnum::PANAMA, 2, true,
                  std::vector<CountryEnum>{CountryEnum::COSTA_RICA,
                                           CountryEnum::COLOMBIA},
                  Region::CENTRAL_AMERICA},
          Country{CountryEnum::HAITI, 1, false,
                  std::vector<CountryEnum>{CountryEnum::CUBA,
                                           CountryEnum::DOMINICAN_REPUBLIC},
                  Region::CENTRAL_AMERICA},
          Country{CountryEnum::DOMINICAN_REPUBLIC, 1, false,
                  std::vector<CountryEnum>{CountryEnum::HAITI},
                  Region::CENTRAL_AMERICA},
          Country{CountryEnum::VENEZUELA, 2, true,
                  std::vector<CountryEnum>{CountryEnum::COLOMBIA,
                                           CountryEnum::BRAZIL},
                  Region::SOUTH_AMERICA},
          Country{CountryEnum::COLOMBIA, 1, false,
                  std::vector<CountryEnum>{CountryEnum::VENEZUELA,
                                           CountryEnum::PANAMA,
                                           CountryEnum::ECUADOR},
                  Region::SOUTH_AMERICA},
          Country{CountryEnum::ECUADOR, 2, false,
                  std::vector<CountryEnum>{CountryEnum::COLOMBIA,
                                           CountryEnum::PERU},
                  Region::SOUTH_AMERICA},
          Country{CountryEnum::PERU, 2, false,
                  std::vector<CountryEnum>{
                      CountryEnum::ECUADOR,
                      CountryEnum::BOLIVIA,
                      CountryEnum::CHILE,
                  },
                  Region::SOUTH_AMERICA},
          Country{CountryEnum::BOLIVIA, 2, false,
                  std::vector<CountryEnum>{CountryEnum::PERU,
                                           CountryEnum::PARAGUAY},
                  Region::SOUTH_AMERICA},
          Country{CountryEnum::PARAGUAY, 2, false,
                  std::vector<CountryEnum>{CountryEnum::BOLIVIA,
                                           CountryEnum::URUGUAY,
                                           CountryEnum::ARGENTINA},
                  Region::SOUTH_AMERICA},
          Country{CountryEnum::CHILE, 3, true,
                  std::vector<CountryEnum>{CountryEnum::PERU,
                                           CountryEnum::ARGENTINA},
                  Region::SOUTH_AMERICA},
          Country{CountryEnum::URUGUAY, 2, false,
                  std::vector<CountryEnum>{CountryEnum::PARAGUAY,
                                           CountryEnum::ARGENTINA,
                                           CountryEnum::BRAZIL},
                  Region::SOUTH_AMERICA},
          Country{CountryEnum::ARGENTINA, 2, true,
                  std::vector<CountryEnum>{CountryEnum::PARAGUAY,
                                           CountryEnum::URUGUAY,
                                           CountryEnum::CHILE},
                  Region::SOUTH_AMERICA},
          Country{CountryEnum::BRAZIL, 2, true,
                  std::vector<CountryEnum>{
                      CountryEnum::VENEZUELA,
                      CountryEnum::URUGUAY,
                  },
                  Region::SOUTH_AMERICA},
          Country{CountryEnum::ALGERIA, 2, true,
                  std::vector<CountryEnum>{
                      CountryEnum::TUNISIA, CountryEnum::MOROCCO,
                      CountryEnum::FRANCE, CountryEnum::SAHARA_STATES},
                  Region::AFRICA},
          Country{CountryEnum::TUNISIA, 2, false,
                  std::vector<CountryEnum>{CountryEnum::ALGERIA,
                                           CountryEnum::LIBYA},
                  Region::AFRICA},
          Country{CountryEnum::MOROCCO, 3, false,
                  std::vector<CountryEnum>{CountryEnum::ALGERIA,
                                           CountryEnum::WEST_AFRICAN_STATES,
                                           CountryEnum::SPAIN},
                  Region::AFRICA},
          Country{CountryEnum::WEST_AFRICAN_STATES, 2, false,
                  std::vector<CountryEnum>{CountryEnum::MOROCCO,
                                           CountryEnum::IVORY_COAST},
                  Region::AFRICA},
          Country{CountryEnum::SAHARA_STATES, 1, false,
                  std::vector<CountryEnum>{CountryEnum::ALGERIA,
                                           CountryEnum::NIGERIA},
                  Region::AFRICA},
          Country{CountryEnum::IVORY_COAST, 2, false,
                  std::vector<CountryEnum>{CountryEnum::WEST_AFRICAN_STATES,
                                           CountryEnum::NIGERIA},
                  Region::AFRICA},
          Country{CountryEnum::NIGERIA, 1, true,
                  std::vector<CountryEnum>{CountryEnum::SAHARA_STATES,
                                           CountryEnum::IVORY_COAST,
                                           CountryEnum::CAMEROON},
                  Region::AFRICA},
          Country{CountryEnum::CAMEROON, 1, false,
                  std::vector<CountryEnum>{CountryEnum::NIGERIA,
                                           CountryEnum::ZAIRE},
                  Region::AFRICA},
          Country{CountryEnum::ZAIRE, 1, true,
                  std::vector<CountryEnum>{CountryEnum::CAMEROON,
                                           CountryEnum::ANGOLA,
                                           CountryEnum::ZIMBABWE},
                  Region::AFRICA},
          Country{CountryEnum::ANGOLA, 1, true,
                  std::vector<CountryEnum>{CountryEnum::ZAIRE,
                                           CountryEnum::SOUTH_AFRICA,
                                           CountryEnum::BOTSWANA},
                  Region::AFRICA},
          Country{CountryEnum::SOUTH_AFRICA, 3, true,
                  std::vector<CountryEnum>{CountryEnum::ANGOLA,
                                           CountryEnum::BOTSWANA},
                  Region::AFRICA},
          Country{CountryEnum::BOTSWANA, 2, false,
                  std::vector<CountryEnum>{CountryEnum::SOUTH_AFRICA,
                                           CountryEnum::ANGOLA,
                                           CountryEnum::ZIMBABWE},
                  Region::AFRICA},
          Country{CountryEnum::ZIMBABWE, 1, false,
                  std::vector<CountryEnum>{CountryEnum::ZAIRE,
                                           CountryEnum::BOTSWANA,
                                           CountryEnum::MOZAMBIQUE},
                  Region::AFRICA},
          Country{CountryEnum::MOZAMBIQUE, 1, false,
                  std::vector<CountryEnum>{CountryEnum::ZIMBABWE,
                                           CountryEnum::KENYA},
                  Region::AFRICA},
          Country{CountryEnum::KENYA, 2, false,
                  std::vector<CountryEnum>{CountryEnum::MOZAMBIQUE,
                                           CountryEnum::SOMALIA},
                  Region::AFRICA},
          Country{CountryEnum::ETHIOPIA, 1, false,
                  std::vector<CountryEnum>{CountryEnum::SOMALIA,
                                           CountryEnum::SUDAN},
                  Region::AFRICA},
          Country{CountryEnum::SUDAN, 1, false,
                  std::vector<CountryEnum>{CountryEnum::EGYPT,
                                           CountryEnum::ETHIOPIA},
                  Region::AFRICA},
          Country{CountryEnum::SOMALIA, 2, false,
                  std::vector<CountryEnum>{CountryEnum::KENYA,
                                           CountryEnum::ETHIOPIA},
                  Region::AFRICA},
          Country{
              CountryEnum::EGYPT, 2, true,
              std::vector<CountryEnum>{CountryEnum::SUDAN, CountryEnum::LIBYA,
                                       CountryEnum::ISRAEL},
              Region::MIDDLE_EAST},
          Country{CountryEnum::LIBYA, 2, true,
                  std::vector<CountryEnum>{CountryEnum::TUNISIA,
                                           CountryEnum::EGYPT},
                  Region::MIDDLE_EAST},
          Country{
              CountryEnum::ISRAEL, 4, true,
              std::vector<CountryEnum>{CountryEnum::EGYPT, CountryEnum::LEBANON,
                                       CountryEnum::SYRIA, CountryEnum::JORDAN},
              Region::MIDDLE_EAST},
          Country{
              CountryEnum::LEBANON, 1, false,
              std::vector<CountryEnum>{CountryEnum::ISRAEL, CountryEnum::SYRIA,
                                       CountryEnum::JORDAN},
              Region::MIDDLE_EAST},
          Country{CountryEnum::SYRIA, 2, false,
                  std::vector<CountryEnum>{CountryEnum::LEBANON,
                                           CountryEnum::ISRAEL,
                                           CountryEnum::TURKEY},
                  Region::MIDDLE_EAST},
          Country{CountryEnum::JORDAN, 2, false,
                  std::vector<CountryEnum>{
                      CountryEnum::ISRAEL, CountryEnum::LEBANON,
                      CountryEnum::IRAQ, CountryEnum::SAUDI_ARABIA},
                  Region::MIDDLE_EAST},
          Country{CountryEnum::IRAQ, 3, true,
                  std::vector<CountryEnum>{
                      CountryEnum::JORDAN, CountryEnum::SAUDI_ARABIA,
                      CountryEnum::IRAN, CountryEnum::GULF_STATES},
                  Region::MIDDLE_EAST},
          Country{CountryEnum::IRAN, 2, true,
                  std::vector<CountryEnum>{CountryEnum::IRAQ,
                                           CountryEnum::AFGHANISTAN,
                                           CountryEnum::PAKISTAN},
                  Region::MIDDLE_EAST},
          Country{
              CountryEnum::SAUDI_ARABIA, 3, true,
              std::vector<CountryEnum>{CountryEnum::GULF_STATES,
                                       CountryEnum::JORDAN, CountryEnum::IRAQ},
              Region::MIDDLE_EAST},
          Country{CountryEnum::GULF_STATES, 3, false,
                  std::vector<CountryEnum>{CountryEnum::SAUDI_ARABIA,
                                           CountryEnum::IRAQ},
                  Region::MIDDLE_EAST},
          Country{
              CountryEnum::AFGHANISTAN, 2, false,
              std::vector<CountryEnum>{CountryEnum::IRAN, CountryEnum::PAKISTAN,
                                       CountryEnum::USSR},
              Region::ASIA},
          Country{
              CountryEnum::PAKISTAN, 2, true,
              std::vector<CountryEnum>{CountryEnum::AFGHANISTAN,
                                       CountryEnum::IRAN, CountryEnum::INDIA},
              Region::ASIA},
          Country{CountryEnum::INDIA, 3, true,
                  std::vector<CountryEnum>{
                      CountryEnum::PAKISTAN,
                      CountryEnum::BURMA,
                  },
                  Region::ASIA},
          Country{
              CountryEnum::BURMA, 2, false,
              std::vector<CountryEnum>{CountryEnum::INDIA, CountryEnum::LAOS},
              std::set<Region>{Region::ASIA, Region::SOUTH_EAST_ASIA}},
          Country{
              CountryEnum::THAILAND, 2, true,
              std::vector<CountryEnum>{CountryEnum::LAOS, CountryEnum::VIETNAM,
                                       CountryEnum::MALAYSIA},
              std::set<Region>{Region::ASIA, Region::SOUTH_EAST_ASIA}},
          Country{CountryEnum::VIETNAM, 1, false,
                  std::vector<CountryEnum>{CountryEnum::THAILAND,
                                           CountryEnum::LAOS},
                  std::set<Region>{Region::ASIA, Region::SOUTH_EAST_ASIA}},
          Country{CountryEnum::LAOS, 1, false,
                  std::vector<CountryEnum>{CountryEnum::BURMA,
                                           CountryEnum::THAILAND,
                                           CountryEnum::VIETNAM},
                  std::set<Region>{Region::ASIA, Region::SOUTH_EAST_ASIA}},
          Country{CountryEnum::MALAYSIA, 2, false,
                  std::vector<CountryEnum>{CountryEnum::THAILAND,
                                           CountryEnum::INDONESIA,
                                           CountryEnum::AUSTRALIA},
                  std::set<Region>{Region::ASIA, Region::SOUTH_EAST_ASIA}},
          Country{CountryEnum::INDONESIA, 1, false,
                  std::vector<CountryEnum>{CountryEnum::MALAYSIA,
                                           CountryEnum::PHILIPPINES},
                  std::set<Region>{Region::ASIA, Region::SOUTH_EAST_ASIA}},
          Country{CountryEnum::PHILIPPINES, 2, false,
                  std::vector<CountryEnum>{CountryEnum::JAPAN,
                                           CountryEnum::INDONESIA},
                  std::set<Region>{Region::ASIA, Region::SOUTH_EAST_ASIA}},
          Country{CountryEnum::JAPAN, 4, true,
                  std::vector<CountryEnum>{
                      CountryEnum::SOUTH_KOREA, CountryEnum::TAIWAN,
                      CountryEnum::PHILIPPINES, CountryEnum::USA},
                  Region::ASIA},
          Country{
              CountryEnum::SOUTH_KOREA, 3, true,
              std::vector<CountryEnum>{CountryEnum::JAPAN, CountryEnum::TAIWAN,
                                       CountryEnum::NORTH_KOREA},
              Region::ASIA},
          Country{CountryEnum::NORTH_KOREA, 3, true,
                  std::vector<CountryEnum>{CountryEnum::SOUTH_KOREA,
                                           CountryEnum::USSR},
                  Region::ASIA},
          Country{CountryEnum::TAIWAN, 3, false,
                  std::vector<CountryEnum>{
                      CountryEnum::SOUTH_KOREA,
                      CountryEnum::JAPAN,
                  },
                  Region::ASIA},
          Country{CountryEnum::AUSTRALIA, 4, false,
                  std::vector<CountryEnum>{CountryEnum::MALAYSIA},
                  Region::ASIA},
          Country{CountryEnum::POLAND, 3, true,
                  std::vector<CountryEnum>{CountryEnum::USSR,
                                           CountryEnum::CZECHOSLOVAKIA,
                                           CountryEnum::EAST_GERMANY},
                  std::set<Region>{Region::EUROPE, Region::EAST_EUROPE}},
          Country{CountryEnum::EAST_GERMANY, 3, true,
                  std::vector<CountryEnum>{
                      CountryEnum::POLAND, CountryEnum::CZECHOSLOVAKIA,
                      CountryEnum::WEST_GERMANY, CountryEnum::AUSTRIA},
                  std::set<Region>{Region::EUROPE, Region::EAST_EUROPE}},
          Country{CountryEnum::CZECHOSLOVAKIA, 3, false,
                  std::vector<CountryEnum>{CountryEnum::POLAND,
                                           CountryEnum::EAST_GERMANY,
                                           CountryEnum::HUNGARY},
                  std::set<Region>{Region::EUROPE, Region::EAST_EUROPE}},
          Country{CountryEnum::HUNGARY, 3, false,
                  std::vector<CountryEnum>{
                      CountryEnum::CZECHOSLOVAKIA, CountryEnum::ROMANIA,
                      CountryEnum::YUGOSLAVIA, CountryEnum::AUSTRIA},
                  std::set<Region>{Region::EUROPE, Region::EAST_EUROPE}},
          Country{CountryEnum::ROMANIA, 3, false,
                  std::vector<CountryEnum>{
                      CountryEnum::USSR, CountryEnum::HUNGARY,
                      CountryEnum::YUGOSLAVIA, CountryEnum::TURKEY},
                  std::set<Region>{Region::EUROPE, Region::EAST_EUROPE}},
          Country{CountryEnum::BULGARIA, 3, false,
                  std::vector<CountryEnum>{CountryEnum::GREECE,
                                           CountryEnum::TURKEY},
                  std::set<Region>{Region::EUROPE, Region::EAST_EUROPE}},
          Country{CountryEnum::YUGOSLAVIA, 3, false,
                  std::vector<CountryEnum>{
                      CountryEnum::ITALY, CountryEnum::GREECE,
                      CountryEnum::HUNGARY, CountryEnum::ROMANIA},
                  std::set<Region>{Region::EUROPE, Region::EAST_EUROPE}},
          Country{CountryEnum::AUSTRIA, 4, false,
                  std::vector<CountryEnum>{
                      CountryEnum::HUNGARY, CountryEnum::ITALY,
                      CountryEnum::WEST_GERMANY, CountryEnum::EAST_GERMANY},
                  std::set<Region>{Region::EUROPE, Region::EAST_EUROPE,
                                   Region::WEST_EUROPE}},
          Country{CountryEnum::WEST_GERMANY, 4, true,
                  std::vector<CountryEnum>{
                      CountryEnum::DENMARK, CountryEnum::EAST_GERMANY,
                      CountryEnum::AUSTRIA, CountryEnum::BENELUX,
                      CountryEnum::FRANCE},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{CountryEnum::GREECE, 2, false,
                  std::vector<CountryEnum>{
                      CountryEnum::ITALY, CountryEnum::YUGOSLAVIA,
                      CountryEnum::BULGARIA, CountryEnum::TURKEY},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{CountryEnum::TURKEY, 2, false,
                  std::vector<CountryEnum>{
                      CountryEnum::ROMANIA, CountryEnum::BULGARIA,
                      CountryEnum::GREECE, CountryEnum::SYRIA},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{CountryEnum::ITALY, 2, true,
                  std::vector<CountryEnum>{
                      CountryEnum::SPAIN, CountryEnum::FRANCE,
                      CountryEnum::YUGOSLAVIA, CountryEnum::GREECE,
                      CountryEnum::AUSTRIA},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{CountryEnum::FRANCE, 3, true,
                  std::vector<CountryEnum>{
                      CountryEnum::SPAIN, CountryEnum::ITALY,
                      CountryEnum::WEST_GERMANY, CountryEnum::UNITED_KINGDOM,
                      CountryEnum::ALGERIA},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{
              CountryEnum::SPAIN, 2, false,
              std::vector<CountryEnum>{CountryEnum::MOROCCO,
                                       CountryEnum::FRANCE, CountryEnum::ITALY},
              std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{CountryEnum::BENELUX, 3, false,
                  std::vector<CountryEnum>{CountryEnum::UNITED_KINGDOM,
                                           CountryEnum::WEST_GERMANY},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{CountryEnum::UNITED_KINGDOM, 5, false,
                  std::vector<CountryEnum>{
                      CountryEnum::FRANCE, CountryEnum::BENELUX,
                      CountryEnum::CANADA, CountryEnum::NORWAY},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{CountryEnum::CANADA, 4, false,
                  std::vector<CountryEnum>{CountryEnum::USA,
                                           CountryEnum::UNITED_KINGDOM},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{CountryEnum::NORWAY, 4, false,
                  std::vector<CountryEnum>{CountryEnum::UNITED_KINGDOM,
                                           CountryEnum::SWEDEN},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{CountryEnum::DENMARK, 3, false,
                  std::vector<CountryEnum>{CountryEnum::SWEDEN,
                                           CountryEnum::WEST_GERMANY},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{CountryEnum::SWEDEN, 4, false,
                  std::vector<CountryEnum>{CountryEnum::DENMARK,
                                           CountryEnum::NORWAY,
                                           CountryEnum::FINLAND},
                  std::set<Region>{Region::EUROPE, Region::WEST_EUROPE}},
          Country{
              CountryEnum::FINLAND, 4, false,
              std::vector<CountryEnum>{CountryEnum::USSR, CountryEnum::SWEDEN},
              std::set<Region>{Region::EUROPE, Region::EAST_EUROPE,
                               Region::WEST_EUROPE}},
      } {
  for (size_t i = static_cast<size_t>(CountryEnum::USSR);
       i <= static_cast<size_t>(CountryEnum::FINLAND); ++i) {
    for (const auto& region : countries_[i].getRegions()) {
      regionCountries_[static_cast<size_t>(region)].insert(countries_[i]);
    }
  }
  // map初期化
  countries_[static_cast<size_t>(CountryEnum::USSR)].addInfluence(Side::USSR,
                                                                  999);
  countries_[static_cast<size_t>(CountryEnum::NORTH_KOREA)].addInfluence(
      Side::USSR, 3);
  countries_[static_cast<size_t>(CountryEnum::EAST_GERMANY)].addInfluence(
      Side::USSR, 3);
  countries_[static_cast<size_t>(CountryEnum::FINLAND)].addInfluence(Side::USSR,
                                                                     1);
  countries_[static_cast<size_t>(CountryEnum::SYRIA)].addInfluence(Side::USSR,
                                                                   1);
  countries_[static_cast<size_t>(CountryEnum::IRAQ)].addInfluence(Side::USSR,
                                                                  1);

  countries_[static_cast<size_t>(CountryEnum::USA)].addInfluence(Side::USA,
                                                                 999);
  countries_[static_cast<size_t>(CountryEnum::AUSTRALIA)].addInfluence(
      Side::USA, 4);
  countries_[static_cast<size_t>(CountryEnum::PHILIPPINES)].addInfluence(
      Side::USA, 1);
  countries_[static_cast<size_t>(CountryEnum::CANADA)].addInfluence(Side::USA,
                                                                    2);
  countries_[static_cast<size_t>(CountryEnum::UNITED_KINGDOM)].addInfluence(
      Side::USA, 5);
  countries_[static_cast<size_t>(CountryEnum::PANAMA)].addInfluence(Side::USA,
                                                                    1);
  countries_[static_cast<size_t>(CountryEnum::ISRAEL)].addInfluence(Side::USA,
                                                                    1);
  countries_[static_cast<size_t>(CountryEnum::IRAN)].addInfluence(Side::USA, 1);
  countries_[static_cast<size_t>(CountryEnum::SOUTH_KOREA)].addInfluence(
      Side::USA, 1);
  countries_[static_cast<size_t>(CountryEnum::JAPAN)].addInfluence(Side::USA,
                                                                   1);
  countries_[static_cast<size_t>(CountryEnum::SOUTH_AFRICA)].addInfluence(
      Side::USA, 1);
};

Country& WorldMap::getCountry(CountryEnum countryEnum) {
  auto* iterator = std::find_if(countries_.begin(), countries_.end(),
                                [countryEnum](const Country& country) {
                                  return country.getId() == countryEnum;
                                });
  if (iterator != countries_.end()) {
    return *iterator;
  }
  // This should never happen
  throw std::runtime_error("Country not found");
}

const Country& WorldMap::getCountry(CountryEnum countryEnum) const {
  return const_cast<WorldMap*>(this)->getCountry(countryEnum);
}

std::set<CountryEnum> WorldMap::placeableCountries(Side side) const {
  std::set<CountryEnum> placeable_countries;
  for (const auto& country : countries_) {
    // USSRとUSAはここで除外
    if (country.getRegions().count(Region::SPECIAL) > 0) {
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

const std::set<Country>& WorldMap::countriesInRegion(Region region) const {
  return regionCountries_[static_cast<size_t>(region)];
}