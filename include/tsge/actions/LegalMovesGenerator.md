# LegalMovesGenerator 概要

`LegalMovesGenerator`は現在の`Board`と`Side`から選択可能な`Move`を列挙する静的ユーティリティ。返却された`Move`は`Move::toCommand()`で`Command`列へ変換され、`PhaseMachine`による適用順序を決定する。

## 前提条件
- 影響力配置候補は`WorldMap::placeableCountries(side)`で判定（自勢力影響力または隣接国保持）。
- リアルインメント／クーデター対象国は`collectOpponentInfluencedCountries()`で抽出し、DEFCON制限（4=ヨーロッパ禁止、3=アジア追加禁止、2=中東も禁止）を自動適用。
- 手札は`Board::getPlayerHand(side)`から取得し、Ops値0のカードは各アクション生成時に除外される。

## 主要メソッドと生成Move
- `actionPlaceInfluenceLegalMoves` → `ActionPlaceInfluenceMove`
  - Ops値とボーナス条件ごとにDFSを一度実行し、国→配置数パターンをキャッシュ共有。Move実行時は`ActionPlaceInfluenceCommand`→`FinalizeCardPlayCommand`を積む。
- `actionRealignmentLegalMoves` → `ActionRealigmentMove`
  - Ops>0のカードと全合法国の直積。初回リアルインメント後のOps消費はRequest系に委譲。
- `realignmentRequestLegalMoves` → `RealignmentRequestMove`
  - 履歴・残Opsを保持しつつ再度対象国を提示。`CountryEnum::USSR`をパス扱いで必ず追加。
- `additionalOpsRealignmentLegalMoves` → `RealignmentRequestMove`
  - 中国カード／ベトナム蜂起ボーナス用の拡張。現状は判定未実装のため結果は空。
- `actionCoupLegalMoves` → `ActionCoupMove`
  - 全合法国×Ops>0カード。`ActionCoupCommand`が連鎖でDEFCON変更等を生成。
- `actionLegalMovesForCard` → `ActionPlaceInfluenceMove` / `ActionRealigmentMove` / `ActionCoupMove`
  - 指定カードのOpsを用いる通常アクション一式を連結して返す。イベント後の追加Ops選択など、`Move`合成責務を`LegalMovesGenerator`へ集約するためのユーティリティ。
- `actionSpaceRaceLegalMoves` → `ActionSpaceRaceMove`
  - `SpaceTrack::canSpace`を満たすカードのみ。Moveは単一の`ActionSpaceRaceCommand`を返す。
- `actionEventLegalMoves` → `ActionEventMove`
  - `card->canEvent(board)`がtrueのカード。スコアリングカードは常に含まれる想定。
- `headlineCardSelectLegalMoves` → `HeadlineCardSelectMove`
  - 手札をそのまま候補化。UN Intervention除外はTODO。
- `arLegalMoves`
  - 上記アクション系Moveを連結し、アクションラウンドで提示する全集を返す。

## 実装メモ
- 影響力配置DFSは一時`WorldMap`を更新しながらバックトラックし、支配状態の変化によるコスト調整に対応。
- `std::vector`は事前`reserve`で確保し、`std::make_shared`でMoveを生成。
- 追加Opsやボーナス地域判定、中国カード固有処理は未接続。コメント付きTODOが`computeOpsVariants`および追加Ops系に残る。
- ヘッドライン選択時のカード除外条件、および`canEvent`側の発動条件精緻化は今後の課題。

## TODO一覧
- 中国カード／ベトナム蜂起によるOpsボーナス実装と`additionalOpsRealignmentLegalMoves`での結果生成。
- `computeOpsVariants()`へのボーナスOps追加とキャッシュ共有。
- ヘッドライン候補からUN Interventionを除外するロジック。
- カード側`canEvent`の条件判定をドキュメントと同期させる。
