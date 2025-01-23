#include "country.hpp"

bool Country::addInfluence(const Side side, int num) {
  if (num < 0) {
    return false;
  }

  influence_[static_cast<int>(side)] += num;

  return true;
}

bool Country::removeInfluence(const Side side, int num) {
  if (num < 0) {
    return false;
  }

  influence_[static_cast<int>(side)] -= num;
  if (influence_[static_cast<int>(side)] < 0) {
    influence_[static_cast<int>(side)] = 0;
  }

  return true;
}

bool Country::clearInfluence(const Side side) {
  influence_[static_cast<int>(side)] = 0;

  return true;
}