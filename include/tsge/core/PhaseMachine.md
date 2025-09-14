# PhaseMachine 仕様書

## 概要

`PhaseMachine`は、Twilight Struggleのゲームフローを管理する中核コンポーネントです。プレイヤーからの入力（`Move`）を受け取り、それを`Command`に変換してゲーム状態に適用し、次に必要な合法手を生成する役割を担います。

## 主要責務

1. **Move → Command 変換**: プレイヤーの選択（`Move`）を実行可能な`Command`列に変換
2. **ゲーム状態管理**: `Board`の状態スタックを操作してゲームフェーズを進行
3. **合法手生成**: 現在のゲーム状態に基づいて次のプレイヤーが選択可能な合法手を生成
4. **ゲーム終了判定**: 勝利条件の確認とゲーム終了処理

## アーキテクチャ

### クラス構造

```cpp
class PhaseMachine {
public:
  static std::tuple<std::vector<std::shared_ptr<Move>>, Side, std::optional<Side>>
  step(Board& board, std::optional<std::shared_ptr<Move>>&& answer = std::nullopt);
};
```

### step() メソッドの戻り値

- **第1要素**: `std::vector<std::shared_ptr<Move>>` - 次のプレイヤーが選択可能な合法手のリスト
- **第2要素**: `Side` - 次に行動すべきプレイヤー（USSR/USA/NEUTRAL）
- **第3要素**: `std::optional<Side>` - 勝者（ゲーム終了時のみ有効）

## 処理フロー

### 1. Move の処理（answerが渡された場合）

1. 前回の`RequestCommand`が残っていればスタックから削除
2. `Move::toCommand()`を呼び出して`Command`列に変換
3. 生成された`Command`を逆順でスタックに積む（LIFO実行のため）

### 2. 状態スタックの処理

スタックが空になるまで以下を繰り返す：

#### A. CommandPtr の場合

- **RequestCommand**: 合法手を生成してプレイヤーからの入力待ち状態で返却
- **その他のCommand**: `apply(board)`を実行してゲーム状態を変更し、スタックから削除

#### B. StateType の場合

フェーズに応じた処理を実行：

- **AR_USSR / AR_USA**: 
  - 現在のARプレイヤーを設定
  - AR_COMPLETEをスタックに積む
  - 該当プレイヤーのAR合法手を生成して返却

- **AR_COMPLETE**: AR終了処理（TODO: 実装予定）

- **USSR_WIN_END / USA_WIN_END / DRAW_END**:
  - ゲーム終了を示す空の合法手リストと勝者を返却

## 状態管理

### Board の states_ スタック

`Board`クラスは`std::vector<std::variant<StateType, CommandPtr>>`型のスタックを保持し、以下の2種類の要素を格納：

1. **StateType**: ゲームフェーズを表す列挙型
   - AR_USSR: ソ連のアクションラウンド
   - AR_USA: アメリカのアクションラウンド
   - AR_COMPLETE: アクションラウンド完了
   - USSR_WIN_END: ソ連勝利によるゲーム終了
   - USA_WIN_END: アメリカ勝利によるゲーム終了
   - DRAW_END: 引き分けによるゲーム終了

2. **CommandPtr**: 実行可能なコマンドへのポインタ
   - RequestCommand: プレイヤー入力を要求する特殊なコマンド
   - その他の具体的なCommand（ActionPlaceInfluence等）

### スタックの操作

- **push**: 新しい状態やコマンドを追加
- **pop**: 処理済みの要素を削除
- **LIFO**: 後入れ先出しで処理

## 合法手生成

`LegalMovesGenerator`を使用して現在のゲーム状態に応じた合法手を生成：

- ARフェーズ: `LegalMovesGenerator::arLegalMoves(board, side)`
- その他のフェーズ: 各フェーズ専用の合法手生成メソッド（実装予定）

## 今後の実装予定

1. **合法手が空の場合の処理**: 手札が空などで選択可能な手がない場合の自動フェーズ進行
2. **AR_COMPLETE処理**: アクションラウンド終了時の次フェーズへの遷移
3. **その他のフェーズ**: HEADLINE、TURN_ENDなどの実装
4. **履歴管理**: undo/redo機能のためのコマンド履歴保存

## 使用例

```cpp
Board board(cardpool);
// 初期状態をセットアップ
board.pushState(StateType::AR_USSR);

// ゲームループ
while (true) {
  auto [legalMoves, nextPlayer, winner] = PhaseMachine::step(board);
  
  if (winner.has_value()) {
    // ゲーム終了
    break;
  }
  
  // プレイヤーに合法手を提示して選択を待つ
  auto selectedMove = player.selectMove(legalMoves, nextPlayer);
  
  // 選択された手を適用
  PhaseMachine::step(board, std::move(selectedMove));
}
```

## 注意事項

- `PhaseMachine`は状態を持たない静的クラスとして設計
- すべてのゲーム状態は`Board`オブジェクトに保持
- MCTSなどでの並列実行を考慮し、スレッドセーフな設計を維持