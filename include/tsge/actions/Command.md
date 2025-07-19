# Command 仕様詳細

CommandはBoardの状態を変更する唯一の手段であり、ゲームのすべての状態変更を表現する。
MoveからtoCommand()によって生成され、PhaseMachineによってBoardに適用される。

## 基本構造

### 基底クラス Command
- 純粋仮想関数 `apply(Board& board) const` を持つ抽象クラス
- すべてのCommandは `Side side_` メンバを保持（USSR/USA/NEUTRAL）
- Boardの状態変更は`apply()`メソッドを通してのみ可能
- undo機能は将来実装予定（MCTS用）

## 派生Commandクラス

### ActionPlaceInfluenceCommand
影響力を配置するCommand。Moveから生成される際は、複数国への配置情報を保持。

#### メンバ変数
- `Side side_`: 実行プレイヤー
- `const Card& card_`: 使用カード
- `std::unordered_map<CountryEnum, int> placements_`: 配置情報（国→配置数）

#### apply()の処理
1. 各国に指定された数の影響力を配置

### ActionRealigmentCommand
リアライメントを実行するCommand。ダイスロールで相手の影響力削減を試みる。

#### メンバ変数
- `Side side_`: 実行プレイヤー
- `const Card& card_`: 使用カード
- `CountryEnum targetCountry_`: 対象国

#### apply()の処理
1. 両プレイヤーのダイスロール実行
2. 修正値の計算：
   - 対象国での影響力差
   - 隣接する超大国の有無
   - 対象国の支配状態
3. 勝者が相手の影響力を差分だけ削減

### ActionCoupCommand
クーデターを実行するCommand。Military Opsトラックを進め、成功時に影響力を変更。

#### メンバ変数
- `Side side_`: 実行プレイヤー
- `const Card& card_`: 使用カード
- `CountryEnum targetCountry_`: 対象国

#### apply()の処理
1. Military Opsトラックを進める（カードのOps値分）
2. 戦場国の場合DEFCONを1低下
3. ダイスロール + Ops値 vs 安定度×2で成否判定
4. 成功時：
   - 相手の影響力を成功度分削減
   - 余剰分を自分の影響力として配置

### ActionSpaceRaceCommand
宇宙開発を実行するCommand。ダイスロールで宇宙開発トラックの前進を試みる。

#### メンバ変数
- `Side side_`: 実行プレイヤー
- `const Card& card_`: 使用カード

#### apply()の処理
1. ダイスロール実行
2. 現在位置に応じた必要ロール値と比較
3. 成功時：
   - 宇宙開発トラックを1前進
   - 特定位置到達時の効果（VP獲得等）を適用
4. 成否に関わらず試行回数を記録

### ChangeDefconCommand
DEFCONレベルを変更するCommand。

#### メンバ変数
- `Side side_`: 実行プレイヤー（NEUTRAL可）
- `int delta_`: 変更量（正：改善、負：悪化）

#### apply()の処理
1. DEFCONトラックを指定量変更
2. DEFCON 1到達時：
   - 現在のフェイジングプレイヤーの相手が勝利
   - ゲーム終了状態を設定

### ChangeVpCommand
勝利点を変更するCommand。

#### メンバ変数
- `Side side_`: 実行プレイヤー（NEUTRAL可）
- `int delta_`: VP変更量（正：USSR有利、負：USA有利）

#### apply()の処理
1. `board.changeVp(delta_)`でVP変更
2. VP ±20到達時：
   - 該当側の勝利
   - ゲーム終了状態を設定

### RequestCommand
プレイヤーに合法手の選択を要求するCommand。実際の状態変更は行わない。

#### メンバ変数
- `Side side_`: 要求対象プレイヤー
- `std::function<std::vector<std::unique_ptr<Move>>(const Board&)> legalMoves;`: Requestの合法手を返すメソッド

#### apply()の処理
何もしない

### DefconBasedVpChangeCommand（cards.hppに定義）
現在のDEFCONレベルに基づいてVPを変更するCommand。特定カードで使用。

#### メンバ変数
- `Side side_`: 実行プレイヤー

#### apply()の処理
1. DEFCONレベルに応じて点数変更
2. DEFCONレベル変更
3. 1, 2の順番はカード依存

## Boardへのアクセス権限

Commandクラスは以下のBoardメソッドにアクセス可能：
- `getWorldMap()`: 国家情報の取得・変更
- `getSpaceTrack()`: 宇宙開発トラックの取得・変更
- `getDefconTrack()`: DEFCONトラックの取得・変更
- `getMilitaryOpsTrack()`: Military Opsトラックの取得・変更
- `changeVp()`: 勝利点の変更
- `pushState()`: ゲーム状態の追加
- `getCurrentArPlayer()`: 現在のARプレイヤー取得

## ランダム要素の扱い

- `Randomizer::getInstance().rollDice()` を使用
- リアライメント、クーデター、宇宙開発でダイスロールを実行
- 結果は即座にBoardの状態に反映

## 実装上の注意点

1. **状態変更の原子性**
   - 各Commandのapply()は原子的に実行される
   - 途中で例外が発生した場合の挙動は未定義

2. **イベント発動の連鎖**
   - カードイベントはCommandを生成して実装
   - 複雑なイベントは複数のCommandの組み合わせで表現

3. **パフォーマンス考慮**
   - MCTSでの頻繁なコピーを想定
   - Commandオブジェクトは軽量に保つ

## 将来の拡張予定

- undo/redo機能の実装（MCTS最適化用）
- より複雑なカードイベント用の特殊Command
- 履歴記録機能の強化