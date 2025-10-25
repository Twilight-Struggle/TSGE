
# Move 仕様概要
MoveはLegalMovesGeneratorによって生成され、PhaseMachineからPlayerに提示される。Playerが選んだMoveは`toCommand`を通じて`std::vector<CommandPtr>`へ展開され、先頭から順に実行される。アクション系Moveは最後に`FinalizeCardPlayCommand`を積み、イベント発動済みかどうかに応じてカード除去まで処理する。

## 補助関数
- `addEventAfterAction(commands, card, side)`
  - 使用カードが相手陣営ならイベントのコマンド列を追加し、発動有無を`bool`で返す。
- `addFinalizeCardPlayCommand(commands, side, cardEnum, card, eventTriggered)`
  - カードプレイ確定と除去判定を行う`FinalizeCardPlayCommand`を末尾に積む。

## アクション系Move
- `ActionPlaceInfluenceMove`
  - `ActionPlaceInfluenceCommand` → 相手イベントがあれば追加 → `FinalizeCardPlayCommand`。
- `ActionCoupMove`
  - `ActionCoupCommand` → 相手イベントがあれば追加 → `FinalizeCardPlayCommand`。
- `ActionSpaceRaceMove`
  - `ActionSpaceRaceCommand`のみ。Space Raceでは相手イベントを誘発しないため、`FinalizeCardPlayCommand`はイベントなしとして追加される。
- `ActionRealigmentMove`
  - `ActionRealigmentCommand`を積み、履歴`{targetCountry_}`と残Ops`card->getOps() - 1`を基に`RequestCommand`を追加。
    - 残Opsが正なら`LegalMovesGenerator::realignmentRequestLegalMoves`へ、0以下なら`additionalOpsRealignmentLegalMoves`へ委譲し、`AdditionalOpsType::NONE`から処理を開始する。
  - 相手イベントがあれば追加し、最後に`FinalizeCardPlayCommand`で締める。

## RealignmentRequestMove
- `targetCountry_ == CountryEnum::USSR`ならパスとして空配列を返す。
- それ以外は`ActionRealigmentCommand`を積み、履歴に`targetCountry_`を追加する。
- 残Ops(`remainingOps_ - 1`)に応じて`realignmentRequestLegalMoves`または`additionalOpsRealignmentLegalMoves`を呼ぶ`RequestCommand`を積み、`appliedAdditionalOps_`でChina Card/Vietnam Revoltsの重複適用を防ぐ。

## ActionEventMove
- カードイベントをプレイヤーサイドで実行してコマンド列を追加する。
- カードが相手陣営（かつ`Side::NEUTRAL`でない）なら、後続行動を選ばせる`RequestCommand`を積む。現状LegalMovesGenerator未実装のため空リストを返すプレースホルダー。
- 常にイベント発動済みとして`FinalizeCardPlayCommand`を追加し、除去判定を行う。

## HeadlineCardSelectMove
- `SetHeadlineCardCommand`のみを積む。ヘッドライン処理は別フェーズでFinalizeされるため、ここでは追加コマンドを持たない。
