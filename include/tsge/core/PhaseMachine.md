# PhaseMachine 仕様書

## 概要

- Twilight Struggle のターン／フェーズ進行を単一の状態スタックで管理する stateless コンポーネント。
- プレイヤー入力（`Move`）を `Command` 列へ展開し、`Board` に適用した結果として次に必要な入力を導出する。
- ヘッドライン、アクションラウンド（以下 AR）、追加AR、ターン終了処理、勝敗判定を一つのループで扱う。

## I/O (PhaseMachine::step)

```cpp
std::tuple<std::vector<std::shared_ptr<Move>>, Side, std::optional<Side>>
PhaseMachine::step(Board& board,
                   std::optional<std::shared_ptr<Move>>&& answer = std::nullopt);
```

- 返り値は `(合法手, 入力を待つ陣営, 勝者)`。
- 合法手が空で `Side::NEUTRAL` が返った場合は自動処理中。`winner` に値が入ればゲーム終了。

## コアループ

1. `answer` があれば `RequestCommand` を除去し、`Move::toCommand()`（参照: `include/tsge/actions/Move.md`）で得た `Command` 列を逆順で状態スタックに積む。
2. スタックが尽きるか入力待ちが発生するまで末尾を処理する。
   - **CommandPtr**: `RequestCommand` なら `legalMoves(board)` を返却し、空なら破棄。その他は `apply(board)` 実行後にポップ。
   - **StateType**: 以下の表に従って次状態を積むか合法手を返す。
3. スタックが空になった場合は `winner = Side::NEUTRAL` を含む終端タプルを返す（ゲーム継続不可時のセーフガード）。

## StateType サマリ

- **TURN_START**: 宇宙開発による追加AR権更新、ターン開始効果TODO評価後に `HEADLINE_PHASE` を積む。
- **HEADLINE_PHASE**: スペースレース優位側に合わせて `HEADLINE_CARD_SELECT_*` を並べる。
- **HEADLINE_CARD_SELECT_USSR / USA**: `GameLogicGameLogicLegalMovesGenerator::headlineCardSelectLegalMoves()` で合法手を提示。
- **HEADLINE_PROCESS_EVENTS**: 両陣営のヘッドラインカードを OPS 値（同値時は USSR 優先）順に解決。各カードについて `FinalizeCardPlayCommand` を先に積み、`event()` が返すコマンド列を後続投入。処理後はヘッドラインカードをクリアし `AR_USSR` を積む。
- **AR_USSR / AR_USA**: 現在手番を設定し `AR_*_COMPLETE` を積んだ上で `GameLogicGameLogicLegalMovesGenerator::arLegalMoves()` を返す。合法手が空の場合は直ちに完了処理へ移行。
- **AR_USSR_COMPLETE / AR_USA_COMPLETE**: `updateActionRoundStatus()` でARカウンタを進め、優先順位「相手通常AR → 自分通常AR → 相手追加AR → 自分追加AR → TURN_END」に従って次状態を積む。
- **EXTRA_AR_USSR / EXTRA_AR_USA**: 追加AR権を `clearExtraActionRound()` で消費し、`GameLogicGameLogicLegalMovesGenerator::extraActionRoundLegalMoves()` から取得した合法手（通常Ops手 + 追加AR専用パスムーブ）を提示。
- **TURN_END**: `TurnTrack::nextTurn()` と `ActionRoundTrack::resetActionRounds()` を実行後に `TURN_START` を積む。
- **USSR_WIN_END / USA_WIN_END / DRAW_END**: 空の合法手と勝者情報を返す終端結果を生成。

## Move / Command の関係

- `Move` は `GameLogicLegalMovesGenerator` が生成し、`Move::toCommand()` はカードプールから該当カードを引き当てて `Command` 列を返す。
- PhaseMachine は `Board::getCardpool()` を通じてカード定義にアクセスし、AR／ヘッドラインをまたぐ複数コマンドを順次発火させる。
- `FinalizeCardPlayCommand` によってカード移動を遅延させ、イベント群が完了した後で手札や捨て札への移動を確定する。

## 入力要求の扱い

- `RequestCommand` は合法手が無ければ自動破棄し、ハングを防止する。
- プレイヤー回答を受け取ると `processAnswer()` が直前の `RequestCommand` を除去し、回答 `Move` から展開した `Command` を積んだ上でループ処理を再開する。

## TODO

1. ~~追加AR専用パスムーブとテストケース整備。~~ ✅ `GameLogicGameLogicLegalMovesGenerator::extraActionRoundLegalMoves()` で実装済み。
2. DEFCON=2・NORAD 発動タイミングの統合。
3. ターン開始／終了時の補給・継続効果処理（North Sea Oil 等）。

## 使い方

```cpp
Board board(cardpool);
board.pushState(StateType::TURN_START);

std::optional<std::shared_ptr<Move>> pending;
while (true) {
  auto [legalMoves, side, winner] =
      PhaseMachine::step(board, std::move(pending));
  if (winner) break;
  if (side == Side::NEUTRAL) continue;
  pending = player.selectMove(legalMoves, side);
}
```

## 注意事項

- `PhaseMachine` は状態を持たず、共有状態は `Board` に集約される。
- `Board::getStates()` を直接操作するのは `PhaseMachine` と `Command` のみに限定する。
- 並列 MCTS での使用を想定し、`step` 呼び出しは入力以外に副作用を持たない。
