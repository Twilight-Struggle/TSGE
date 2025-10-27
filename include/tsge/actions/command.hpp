#pragma once

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "tsge/enums/cards_enum.hpp"
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

  virtual void apply(Board& board) const = 0;
  // MCTSで必要
  // virtual bool undo(Board& board) const = 0;

 protected:
  const Side side_;
};

using CommandPtr = std::shared_ptr<Command>;

class ActionPlaceInfluenceCommand final : public Command {
 public:
  ActionPlaceInfluenceCommand(Side side, const std::unique_ptr<Card>& card,
                              const std::map<CountryEnum, int>& targetCountries)
      : Command{side}, card_{card}, targetCountries_{targetCountries} {};

  void apply(Board& board) const override;

 private:
  const std::unique_ptr<Card>& card_;
  const std::map<CountryEnum, int> targetCountries_;
};

class ActionRealigmentCommand final : public Command {
 public:
  ActionRealigmentCommand(Side side, const std::unique_ptr<Card>& card,
                          CountryEnum targetCountry)
      : Command{side}, card_{card}, targetCountry_{targetCountry} {};

  void apply(Board& board) const override;

 private:
  const std::unique_ptr<Card>& card_;
  const CountryEnum targetCountry_;
};

class ActionCoupCommand final : public Command {
 public:
  ActionCoupCommand(Side side, const std::unique_ptr<Card>& card,
                    CountryEnum targetCountry)
      : Command{side}, card_{card}, targetCountry_{targetCountry} {};

  void apply(Board& board) const override;

 private:
  const std::unique_ptr<Card>& card_;
  const CountryEnum targetCountry_;
};

class ActionSpaceRaceCommand final : public Command {
 public:
  ActionSpaceRaceCommand(Side side, const std::unique_ptr<Card>& card)
      : Command{side}, card_{card} {};

  void apply(Board& board) const override;

 private:
  const std::unique_ptr<Card>& card_;
};

class LambdaCommand final : public Command {
 public:
  LambdaCommand(std::function<void(Board&)> lambda)
      : Command(Side::NEUTRAL), lambda_(std::move(lambda)) {}

  void apply(Board& board) const override { lambda_(board); }

 private:
  std::function<void(Board&)> lambda_;
};

class ChangeDefconCommand final : public Command {
 public:
  explicit ChangeDefconCommand(int delta)
      : Command(Side::NEUTRAL), delta_{delta} {}

  void apply(Board& board) const override;

 private:
  const int delta_;
};

class ChangeVpCommand final : public Command {
 public:
  explicit ChangeVpCommand(Side side, int delta)
      : Command{side}, delta_{delta} {}

  void apply(Board& board) const override;

 private:
  const int delta_;
};

class RequestCommand final : public Command {
 public:
  RequestCommand(Side side,
                 std::function<std::vector<std::shared_ptr<Move>>(const Board&)>
                     legalMoves)
      : Command(side), legalMoves(std::move(legalMoves)) {}

  std::function<std::vector<std::shared_ptr<Move>>(const Board&)> legalMoves;
  // std::function<std::vector<CommandPtr>(const Move&)> resume; いらないかも

  void apply(Board& board) const override;

  [[nodiscard]]
  Side getSide() const {
    return side_;
  }
};

class SetHeadlineCardCommand final : public Command {
 public:
  SetHeadlineCardCommand(Side side, CardEnum card)
      : Command{side}, card_{card} {}

  void apply(Board& board) const override;

 private:
  const CardEnum card_;
};

class FinalizeCardPlayCommand final : public Command {
 public:
  FinalizeCardPlayCommand(Side side, CardEnum card, bool removeAfterEvent)
      : Command{side}, card_{card}, removeAfterEvent_{removeAfterEvent} {}

  void apply(Board& board) const override;

 private:
  const CardEnum card_;
  const bool removeAfterEvent_;
};

class DiscardCommand final : public Command {
 public:
  DiscardCommand(Side side, CardEnum card) : Command{side}, card_{card} {}

  void apply(Board& board) const override;

 private:
  const CardEnum card_;
};
