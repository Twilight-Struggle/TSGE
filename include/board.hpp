#pragma once
#include <array>
#include <vector>

#include "country.hpp"

class Board {
 public:
  Board()
      : countries_{Country{CountryEnum::NORTH_KOREA, 3, Continent::ASIA, true,
                           std::vector<CountryEnum>{CountryEnum::SOUTH_KOREA}},
                   Country{CountryEnum::SOUTH_KOREA, 3, Continent::ASIA, true,
                           std::vector<CountryEnum>{
                               CountryEnum::JAPAN,
                               CountryEnum::TAIWAN,
                           }},
                   Country{CountryEnum::JAPAN, 4, Continent::ASIA, true,
                           std::vector<CountryEnum>{
                               CountryEnum::SOUTH_KOREA,
                               CountryEnum::TAIWAN,
                               CountryEnum::PHILIPPINES,
                           }},
                   Country{CountryEnum::TAIWAN, 3, Continent::ASIA, true,
                           std::vector<CountryEnum>{
                               CountryEnum::SOUTH_KOREA,
                               CountryEnum::JAPAN,
                           }},
                   Country{CountryEnum::PHILIPPINES, 2, Continent::ASIA, true,
                           std::vector<CountryEnum>{CountryEnum::JAPAN}}} {};
  ~Board() = default;

  Country& getCountry(const CountryEnum countryEnum);

 private:
  std::array<Country, 5> countries_;
};