#pragma once

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "card.hpp"
#include "game_enums.hpp"

class Board;
class Move;

class Command {
 public:
  Command(Side side, const std::unique_ptr<Card>& card)
      : side_{side}, card_{card} {};
  virtual ~Command() = default;
  virtual bool apply(Board& board) const = 0;
  // MCTSで必要
  // virtual bool undo(Board& board) const = 0;

 protected:
  const Side side_;
  const std::unique_ptr<Card>& card_;
};

using CommandPtr = std::shared_ptr<Command>;

class ActionPlaceInfluence : public Command {
 public:
  ActionPlaceInfluence(Side side, const std::unique_ptr<Card>& card,
                       const std::map<CountryEnum, int>& targetCountries)
      : Command{side, card}, targetCountries_{targetCountries} {};

  bool apply(Board& board) const override;

 private:
  const std::map<CountryEnum, int> targetCountries_;
};

class ActionRealigment : public Command {
 public:
  ActionRealigment(Side side, const std::unique_ptr<Card>& card,
                   CountryEnum targetCountry)
      : Command{side, card}, targetCountry_{targetCountry} {};

  bool apply(Board& board) const override;

 private:
  const CountryEnum targetCountry_;
};

class ActionCoup : public Command {
 public:
  ActionCoup(Side side, const std::unique_ptr<Card>& card,
             CountryEnum targetCountry)
      : Command{side, card}, targetCountry_{targetCountry} {};

  bool apply(Board& board) const override;

 private:
  const CountryEnum targetCountry_;
};

class ActionSpaceRace : public Command {
 public:
  ActionSpaceRace(Side side, const std::unique_ptr<Card>& card)
      : Command{side, card} {};

  bool apply(Board& board) const override;
};

class ChangeDefconCommand : public Command {
 public:
  explicit ChangeDefconCommand(int delta)
      : Command(Side::USSR, getNullCard()), delta_{delta} {}

  bool apply(Board& board) const override;

 private:
  const int delta_;

  static const std::unique_ptr<Card>& getNullCard() {
    static std::unique_ptr<Card> nullCard;
    return nullCard;
  }
};

class ChangeVPCommand : public Command {
 public:
  explicit ChangeVPCommand(int delta)
      : Command(Side::USSR, getNullCard()), delta_{delta} {}

  bool apply(Board& board) const override;

 private:
  const int delta_;

  static const std::unique_ptr<Card>& getNullCard() {
    static std::unique_ptr<Card> nullCard;
    return nullCard;
  }
};

class Request : public Command {
 public:
  Request(Side side,
          std::function<std::vector<std::unique_ptr<Move>>(const Board&)>
              legalMoves)
      : Command(side, getNullCard()), legalMoves(std::move(legalMoves)) {}

  std::function<std::vector<std::unique_ptr<Move>>(const Board&)> legalMoves;
  // std::function<std::vector<CommandPtr>(const Move&)> resume; いらないかも

  bool apply(Board&) const override { return true; }
  Side getSide() const { return side_; }

 private:
  static const std::unique_ptr<Card>& getNullCard() {
    static std::unique_ptr<Card> nullCard;
    return nullCard;
  }
};