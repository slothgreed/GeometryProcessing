# HarnessAgent — C# / Claude API

Claudeに依頼文と点群を渡し、Toolの実行、結果返送、評価、ログ保存を行う学習用ハーネスです。
実行コードとテストはすべてC#です。Node.js、JavaScript、Pythonは使いません。

## 必要なもの

- .NET 9 SDK（この環境の9.0.308で検証）
- Claudeモードのみ: Anthropic APIキーと、アカウントで利用できるモデルID

NuGet外部パッケージは不要です。`HttpClient`でClaude Messages APIを直接呼びます。
Claude Agent SDK / Claude Codeのラッパーではありません。
独立した `HarnessAgent.csproj` なので、既存C++ソリューションのビルドは不要です。
Visual Studioでは `HarnessAgent.sln` を開いてください。HarnessAgentプロジェクトを登録済みです。
ソリューション単位では `dotnet build HarnessAgent/HarnessAgent.sln -c Release` でビルドできます。

## UIを起動する

Windowsで `HarnessAgent.sln` を開いて実行するか、次を実行します。

```powershell
dotnet run --project HarnessAgent/HarnessAgent.csproj -c Release
```

引数なしではWindows Formsの画面が開きます。文字を入力して「送信」を押すと、
入力内容にかかわらず固定の「こんにちは」が渡されたものとして、次の回答を表示します。

> 「こんにちは」を受け取りました。
> HarnessAgentのデモ応答です。

UIは固定応答のみで、Claude API・点群Tool・評価処理には接続していません。
APIキー不要で課金も発生しません。処理は `MainForm.Send()` → `GetDemoReply()` と追えます。
空入力でも同じ回答です。繰り返し送信すると回答欄と時刻を更新します。

`Program.cs` は `[STAThread] private static void Main(string[] args)` から始まり、
CLI引数がある場合だけ従来の評価・テストを実行します。
明示的な `--all` は従来どおりClaude API評価なので、UIデモとは異なりAPIキーが必要です。

## まずオフラインで試す

リポジトリルートで実行します。

```powershell
dotnet build HarnessAgent/HarnessAgent.csproj -c Release
dotnet run --project HarnessAgent/HarnessAgent.csproj -c Release --no-build -- --self-test
dotnet run --project HarnessAgent/HarnessAgent.csproj -c Release --no-build -- --agent demo --all
```

`--self-test` はハーネスとClaude通信のテスト群です。HTTP通信を疑似応答に差し替えるため、
APIキー・ネット接続・API利用料は不要です。外部テストライブラリは使わず、
`SelfTests.cs` 内のテスト一覧を実行します。`dotnet test`で実行する構成ではありません。

`--agent demo --all` は点群評価5ケースを実行します。デモは依頼文を解釈せず集計Toolを呼びます。
正常系3件と、空入力・Tool未許可を正しく拒否する異常系2件です。拒否が期待どおりならPASSです。

## Claude APIで実行する

既定のAgentはClaudeです。APIキーはコードやケースJSON、チャットに書かず環境変数に設定します。
PowerShell 7では次のように入力できます（キーは画面とコマンド履歴に残しません）。

```powershell
$env:ANTHROPIC_API_KEY = Read-Host 'Anthropic API key' -MaskInput
$env:ANTHROPIC_MODEL = Read-Host '利用できるClaudeモデルID'
dotnet run --project HarnessAgent/HarnessAgent.csproj -c Release -- --all
```

`--agent claude --all` と明示しても同じです。
ケースのプロンプトと点群をAnthropicに送信し、API利用料金が発生します。
正解の `expected` やリポジトリのファイルは送信しません。
キーとモデルIDが未設定なら、API呼び出し前にエラーになります。

## 処理の流れと読む順番

```text
Program.cs                            CLIの入口、Agent選択、ケースの反復
  → AgentContext.Build()              依頼文・点群を取り出す。正解は渡さない
  → Harness.RunAsync()            実行ID、ログ、Tool許可、再試行
    → ClaudeAgent.RunAsync()          Messages APIでClaudeへ依頼
      → tool_use                     ClaudeがTool名と引数を返す
      → ToolRegistry.Call()          C#で点数・境界ボックスを実計算
      → tool_result                  結果を会話履歴に追加してClaudeへ返す
      → end_turn                     Claudeの最終JSONを受け取る
  → Harness.Evaluate()              expectedと比較してPASS/FAIL
```

| ファイル | 役割 |
| --- | --- |
| `Program.cs` | 明示的なvoid Main。引数なしはUI、引数ありはCLI |
| `MainForm.cs` | 入力欄・送信ボタン・回答欄、固定応答 |
| `Harness.cs` | 実行・再試行・評価。入力データ、ログ、デモAgentも同じファイル内 |
| `ClaudeAgent.cs` | Claude APIとの通信とTool呼び出しの会話ループ |
| `Tools.cs` | 点群集計、Toolの定義・許可・入力検証 |
| `SelfTests.cs` | オフラインのテスト群 |
| `Evals/cases.json` | 入力と期待値の一覧 |

実行コードはUIを含め6ファイルです。評価と再試行は専用クラスを廃止し、
`Harness`のメソッドにまとめています。`IAgent`はClaude・デモ・テストを同じ入口で動かすため、
入力レコードとログ用クラスは型と状態を保持するために残しています。

まず `cases.json` → `Program.cs` → `Harness.cs` → `ClaudeAgent.cs` と読むと流れが追えます。
以前の `harness.test.mjs` に対応するものが `SelfTests.cs` です。

## ケースを追加する

`Evals/cases.json`にオブジェクトを追加します。変更後は再ビルドし、出力先のコピーを更新します。
次のようなオブジェクト1個を別JSONに保存して、単独評価もできます。

```json
{
  "id": "my-points",
  "prompt": "点群の点数と範囲を調べてください。",
  "points": [[0, 0, 0], [10, 20, 30]],
  "allowedTools": ["point_cloud_summary"],
  "maxAttempts": 2,
  "expected": {
    "ok": true,
    "output": { "count": 2, "min": [0, 0, 0], "max": [10, 20, 30] }
  }
}
```

```powershell
dotnet run --project HarnessAgent/HarnessAgent.csproj -c Release -- --case HarnessAgent/my-case.json
```

XYZは有限な数値で、単位は入力に従います。評価はJSONの値の完全一致です。
将来、誤差のある幾何処理を追加するときは許容誤差やトポロジーの評価を追加してください。

## ログと終了コード

実行ファイルの隣の `Runs/<batch UUID>/` に保存します。
Releaseなら `HarnessAgent/bin/Release/net9.0-windows/Runs/` です。

- `summary.json`: Agent名、モデル指定、完了済みケースの出力・合否・理由・時間
- `<run UUID>.jsonl`: コンテキスト、試行、Tool引数・結果、APIのusage・停止理由、最終回答

APIキー、HTTPヘッダー、サーバーの生エラー本文はログに書きません。
ログにはプロンプトや点群が含まれます。過去のJavaScript版の `HarnessAgent/Runs/` は移行前の記録として保持しています。

終了コード: 0=全件PASS、1=評価/テストFAIL、2=CLIや設定エラー、130=Ctrl+C。

## 2種類のやり直し

- **通信の再試行**: 429、5xx、ネットワーク障害、30秒のタイムアウトを`RetryableException`にします。
  Agent全体を初めから再実行し、既定2試行・最大3試行で止めます。
- **Claudeによる引数修正**: Tool入力エラーを`is_error: true`で返します。
  Claudeは元の入力に合うよう引数を直して再度Toolを呼べます。

Tool権限違反や上限超過は即時終了です。Tool上限4回は通信の再試行をまたいで共有します。
1試行のAPI往復は最大6回、応答は各1024トークンまでです。途中で切れた応答やToolを
使わない回答を正常結果として扱いません。Ctrl+Cは通信と待機をキャンセルします。
再試行待ちは100/200msの簡易実装で、Retry-After対応は未実装です。
将来、書き込みToolを追加するときは二重実行対策が必要です。

最終評価がFAILでも、正解を渡して修正させる処理はありません。
空入力をClaudeがToolに渡さず拒否すると、現行ケースはFAILになることがあります。
Tool未許可ケースはAPI送信前のローカル拒否です。5ケース全体のPASS率をモデル単独の精度と
解釈せず、ケースごとのログを確認してください。

## 現時点の範囲

実装済み: C#の実行・評価・テスト、Claude API接続、Toolの往復と引数修正、ログ、usage記録、
限定的リトライ、タイムアウト、キャンセル。

未接続: PointCloudAppのC++関数、GeometryAIの名前付きパイプ、CAD生成、料金計算。
実APIの動作確認にはAPIキーと利用可能なモデルIDが必要です。

参考: [Messages API](https://platform.claude.com/docs/en/api/messages/create)、
[Tool呼び出し](https://platform.claude.com/docs/en/agents-and-tools/tool-use/handle-tool-calls)。


