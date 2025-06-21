#pragma once

#include <random>

// singleton
class Randomizer {
 private:
  Randomizer();
  Randomizer(const Randomizer&);
  Randomizer& operator=(const Randomizer&);
  ~Randomizer() {}

  std::mt19937_64 rng;
  std::uniform_int_distribution<int> dice;

 public:
  static Randomizer& getInstance() {
    static Randomizer inst;  // private なコンストラクタを呼び出す。
    return inst;
  }

  int rollDice() { return dice(rng); }
};