# Command 仕様概要

CommandはBoardの状態を更新するほぼ唯一の手段(例外はPhaseMahine)であり、ゲーム内のあらゆる状態遷移を表現する。Moveは`toCommand()`で`std::vector<CommandPtr>`へ展開され、PhaseMachineが先頭から順に`apply()`を実行する。`Board::pushState(std::variant<StateType, CommandPtr>)`によりCommandやStateを同一キューで扱う。2025-12時点では入力要求の判定もCommandの仮想インタフェース経由で行い、`dynamic_cast`削減とホットパスのオーバーヘッド低減を図っている。

## Command基礎
- `apply(Board&) const`のみが状態変更の入り口。Commandはコピー構築可・代入禁止で、生成後の差し替えを想定しない。
- `Side side_`を必須メンバとし、`Side::NEUTRAL`を含む。外部からサイドを読む必要があるのは`RequestCommand::getSide()`程度。
- undoは未実装（MCTS向けに将来追加予定）。

## Moveとの関係
- アクション系Moveは「主Command → 相手イベントCommand → `FinalizeCardPlayCommand`」の順で積む。
- リアライメント系Moveは`RequestCommand`を差し込み、残Opsに応じて追加の合法手を取得する。
- イベント系Moveはカード効果で発生するCommandを順次生成し、副次効果（VP/DEFCON変更など）はCommandチェーンとして追加される。

## 主なCommandカテゴリ

### アクション実行
- `ActionPlaceInfluenceCommand`：配列で受け取った国ごとの配置数を`Country::addInfluence`へ適用。
- `ActionRealigmentCommand`：ダイス2個＋修正（対象国の影響差、隣接支配）で差分を求め、相手影響力を削る。USSR/USAが対象の場合は安全装置として何もしない。
- `ActionCoupCommand`：ダイス＋Ops値から安定度×2を引いた結果で相手影響力を除去し、余剰を自勢力に加算。実行側のMilOpsを更新し、戦場国なら`ChangeDefconCommand(-1)`をキューに積む。
- `ActionSpaceRaceCommand`：成功時にSpaceTrackを前進し、特定マスで`ChangeVpCommand`を追加。結果に関わらず`spaceTried`で試行済み扱いとし、追加行動トラックを更新。

### 状態トラック調整
- `ChangeDefconCommand`：`DefconTrack::changeDefcon`の結果が1以下なら、現在のARプレイヤーと逆側を勝者ステートとして`pushState`。DEFCON 2時のNORAD処理はTODO。
- `ChangeVpCommand`：`getVpMultiplier(side_)`を使ってVP符号を決定。±20到達で勝敗ステートを追加。

### カード/要求処理
- `RequestCommand`：合法手生成コールバックを保持し、`apply()`では何もしない。Move生成側が`getSide()`で対象プレイヤーを把握するほか、`requiresPlayerInput()/legalMoves(Board)`をオーバーライドしてPhaseMachineへ入力要求情報を提供する。
- `SetHeadlineCardCommand`：手札からカードを除去し、ヘッドライン枠に登録。
- `FinalizeCardPlayCommand`：手札からカードを抜き、イベント除去なら`Deck::getRemovedCards()`、通常は捨て札へ。
- `LambdaCommand`：即席処理をラムダで包むユーティリティ（カード固有処理・テスト用）。

## Boardアクセスの前提
- `getWorldMap()`, `getSpaceTrack()`, `getDefconTrack()`, `getMilopsTrack()`, `getActionRoundTrack()`などのトラック参照。
- `getDeck()`, `getPlayerHand()`, `setHeadlineCard()`でカード在庫を管理。
- `pushState()`, `changeVp()`, `getCurrentArPlayer()`, `getRandomizer()`を通じ、副次Commandの連鎖やダイス判定を扱う。

## 入力要求インタフェース

- `Command::requiresPlayerInput()`：入力要求であればtrueを返す。デフォルト実装はfalse。
- `Command::legalMoves(const Board&)`：入力要求時に提示する合法手を返す。デフォルトは空vector。
- `Command::getSide()`：Command生成時に束縛された`Side`を返す。`requiresPlayerInput()`がtrueのケースでは、このサイドが入力対象を示す。
- PhaseMachineは上記メソッドを組み合わせ、RequestCommandかどうかを意識せず入力待ちの判定と合法手返却を行う。

## 実装メモ
- Command単位で処理は原子的に完結させ、中途例外時の仕様は未定義。
- 連鎖イベントはCommandを追加で`pushState`して表現し、再帰的呼び出しによる順序ズレに注意する。
- 専有リソースを避け、MCTS向けに軽量な状態保持を心掛ける。
- TODO：DEFCON 2時のNORAD効果／undoサポート。
