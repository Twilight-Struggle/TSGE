#include "tsge/game_state/country.hpp"

#include <cmath>

bool Country::addInfluence(Side side, int num) {
  if (num < 0) {
    return false;
  }

  influence_[static_cast<int>(side)] += num;

  return true;
}

bool Country::removeInfluence(Side side, int num) {
  if (num < 0) {
    return false;
  }

  influence_[static_cast<int>(side)] -= num;
  if (influence_[static_cast<int>(side)] < 0) {
    influence_[static_cast<int>(side)] = 0;
  }

  return true;
}

bool Country::clearInfluence(Side side) {
  influence_[static_cast<int>(side)] = 0;

  return true;
}

Side Country::getControlSide() const {
  if (influence_[static_cast<int>(Side::USSR)] -
          influence_[static_cast<int>(Side::USA)] >=
      stability_) {
    return Side::USSR;
  } else if (influence_[static_cast<int>(Side::USA)] -
                 influence_[static_cast<int>(Side::USSR)] >=
             stability_) {
    return Side::USA;
  } else {
    return Side::NEUTRAL;
  }
}

int Country::getOverControlNum() const {
  const auto diff = std::abs(influence_[static_cast<int>(Side::USSR)] -
                             influence_[static_cast<int>(Side::USA)]);
  if (diff >= stability_) {
    return diff - stability_;
  } else {
    return 0;
  }
}