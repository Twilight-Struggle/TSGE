#pragma once

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "tsge/enums/game_enums.hpp"

class Board;
class Move;
class Card;

class Command {
 public:
  Command(Side side) : side_{side} {};
  virtual ~Command() = default;
  Command(const Command&) = default;
  Command& operator=(const Command&) = delete;
  Command(Command&&) = default;
  Command& operator=(Command&&) = delete;

  virtual bool apply(Board& board) const = 0;
  // MCTSで必要
  // virtual bool undo(Board& board) const = 0;

 protected:
  const Side side_;
};

using CommandPtr = std::unique_ptr<Command>;

class ActionPlaceInfluenceCommand : public Command {
 public:
  ActionPlaceInfluenceCommand(Side side, const std::unique_ptr<Card>& card,
                              const std::map<CountryEnum, int>& targetCountries)
      : Command{side}, card_{card}, targetCountries_{targetCountries} {};

  bool apply(Board& board) const override;

 private:
  const std::unique_ptr<Card>& card_;
  const std::map<CountryEnum, int> targetCountries_;
};

class ActionRealigmentCommand : public Command {
 public:
  ActionRealigmentCommand(Side side, const std::unique_ptr<Card>& card,
                          CountryEnum targetCountry)
      : Command{side}, card_{card}, targetCountry_{targetCountry} {};

  bool apply(Board& board) const override;

 private:
  const std::unique_ptr<Card>& card_;
  const CountryEnum targetCountry_;
};

class ActionCoupCommand : public Command {
 public:
  ActionCoupCommand(Side side, const std::unique_ptr<Card>& card,
                    CountryEnum targetCountry)
      : Command{side}, card_{card}, targetCountry_{targetCountry} {};

  bool apply(Board& board) const override;

 private:
  const std::unique_ptr<Card>& card_;
  const CountryEnum targetCountry_;
};

class ActionSpaceRaceCommand : public Command {
 public:
  ActionSpaceRaceCommand(Side side, const std::unique_ptr<Card>& card)
      : Command{side}, card_{card} {};

  bool apply(Board& board) const override;

 private:
  const std::unique_ptr<Card>& card_;
};

class ChangeDefconCommand : public Command {
 public:
  explicit ChangeDefconCommand(int delta)
      : Command(Side::NEUTRAL), delta_{delta} {}

  bool apply(Board& board) const override;

 private:
  const int delta_;
};

class ChangeVpCommand : public Command {
 public:
  explicit ChangeVpCommand(Side side, int delta)
      : Command{side}, delta_{delta} {}

  bool apply(Board& board) const override;

 private:
  const int delta_;
};

class RequestCommand : public Command {
 public:
  RequestCommand(Side side,
                 std::function<std::vector<std::unique_ptr<Move>>(const Board&)>
                     legalMoves)
      : Command(side), legalMoves(std::move(legalMoves)) {}

  std::function<std::vector<std::unique_ptr<Move>>(const Board&)> legalMoves;
  // std::function<std::vector<CommandPtr>(const Move&)> resume; いらないかも

  bool apply(Board&) const override { return true; }
  Side getSide() const { return side_; }
};