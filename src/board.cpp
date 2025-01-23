#include "board.hpp"

#include <algorithm>

#include "country.hpp"

Country& Board::getCountry(const CountryEnum countryEnum) {
  auto it = std::find_if(countries_.begin(), countries_.end(),
                         [countryEnum](const Country& country) {
                           return country.getId() == countryEnum;
                         });
  if (it != countries_.end()) {
    return *it;
  }
}
